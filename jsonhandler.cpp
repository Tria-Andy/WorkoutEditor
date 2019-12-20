/*
 * Copyright (c) 2016 Andreas Hunner (andy-atech@gmx.net)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "jsonhandler.h"
#include "settings.h"

jsonHandler::jsonHandler()
{
    gcValues = settings::getStringMapPointer(settings::stingMap::GC);
    generalValues = settings::getStringMapPointer(settings::stingMap::General);
    hasOverride = hasXdata = false;
}


QVector<QString> jsonHandler::read_activityMeta(QString filePath, int counter)
{
    QFile file(filePath);
    file.open(QFile::ReadOnly | QFile::Text);
    QString jsonfile = file.readAll();
    file.close();

    QDateTime workDateTime;
    workDateTime.setTimeSpec(Qt::UTC);

    QDateTime localTime(QDateTime::currentDateTime());
    localTime.setTimeSpec(Qt::LocalTime);

    QJsonObject actObject,tagObject;
    QJsonObject jsonobj = QJsonDocument::fromJson(jsonfile.toUtf8()).object();

    actObject = jsonobj.value(QString("RIDE")).toObject();
    tagObject = actObject.value(QString("TAGS")).toObject();

    workDateTime = QDateTime::fromString(actObject.value("STARTTIME").toString().trimmed(),"yyyy/MM/dd hh:mm:ss UTC").addSecs(localTime.offsetFromUtc());

    QVector<QString> actValues;
    actValues.insert(0,QLocale().dayName(workDateTime.date().dayOfWeek(),QLocale::ShortFormat));
    actValues.insert(1,workDateTime.toString("dd.MM.yyyy hh:mm"));
    actValues.insert(2,tagObject.value("Sport").toString().trimmed());
    actValues.insert(3,tagObject.value("Workout Code").toString().trimmed());
    actValues.insert(4,QString::number(counter));

    return actValues;
}

QString jsonHandler::read_jsonContent(QString jsonfile)
{
    QFile file(gcValues->value("actpath")+QDir::separator()+jsonfile);

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "File not open:"+jsonfile;
    }
    else
    {
        QString jsonContent = file.readAll();
        QJsonObject activityObject = QJsonDocument::fromJson(jsonContent.toUtf8()).object();
        file.close();

        QJsonObject itemObject,objValue;
        QJsonArray itemArray;
        QStringList *mapList = settings::get_xmlMapping("jsonfile");
        QStringList *keyList;

        //Fill RIDE Map
        activityItem = activityObject.value(mapList->at(0)).toObject();
        this->fill_qmap(&rideData,&activityItem);

        //Fill TAGS Map
        itemObject = activityItem.value(mapList->at(1)).toObject();
        this->fill_qmap(&tagData,&itemObject);

        //Check if File contains OVERRIDES
        if(activityItem.contains(mapList->at(2)))
        {
            hasOverride = true;
            itemArray = activityItem[mapList->at(2)].toArray();

            for(int i = 0; i < itemArray.count(); ++i)
            {
                itemObject = itemArray.at(i).toObject();
                objValue = itemObject[itemObject.keys().first()].toObject();
                overrideData.insert(itemObject.keys().first(),objValue["value"].toString());
            }
        }

        //Fill IntervallMap
        itemArray = activityItem.value(mapList->at(3)).toArray();
        keyList = settings::get_xmlMapping("intervals");

        for(int i = 0; i < itemArray.count(); ++i)
        {
            itemObject = itemArray.at(i).toObject();
            intervallMap.insert(i,qMakePair(itemObject[keyList->at(1)].toInt(),itemObject[keyList->at(2)].toInt()));
        }

        //Fill SampleMap
        itemArray = activityItem.value(mapList->at(4)).toArray();
        keyList = settings::get_xmlMapping("samples");
        QStringList sampleKeys = itemArray.first().toObject().keys();
        QStringList useKeys;

        this->check_keyList(&useKeys,keyList,&sampleKeys);
        if(useKeys.contains(keyList->at(5))) hasPMData = true;

        QVector<double> sampleValues(useKeys.count()-1,0);

        for(int i = 0; i < itemArray.count(); ++i)
        {
            itemObject = itemArray.at(i).toObject();
            for(int value = 1; value < useKeys.count(); ++value)
            {
                sampleValues[value-1] = itemObject[useKeys.at(value)].toDouble();
            }
            sampleMap.insert(itemObject[useKeys.at(0)].toInt(),sampleValues);
        }

        //Check and Fill XDATA Map
        if(activityItem.contains(mapList->at(5)))
        {
            hasXdata = true;
            xdataHeader = settings::get_xmlMapping("xdata");
            xValuesHeader = settings::get_xmlMapping("xdatavalues");

            itemObject = activityItem[mapList->at(5)].toArray().at(0).toObject();
            this->fill_qmap(&xData,&itemObject);

            itemArray = itemObject[xdataHeader->at(2)].toArray();
            this->fill_list(&itemArray,&xdataUnits);

            itemArray = itemObject[xdataHeader->at(1)].toArray();
            this->fill_list(&itemArray,&xdataValues);

            QVector<double> xDataValues(xdataUnits.count()+1,0);
            itemArray = itemObject[xdataHeader->at(3)].toArray();
            QJsonArray arrValues;

            for(int value = 0; value < itemArray.count(); ++value)
            {
                itemObject = itemArray.at(value).toObject();
                arrValues = itemObject[xdataHeader->at(1)].toArray();
                xDataValues[0] = itemObject[xValuesHeader->at(1)].toDouble();

                for(int x = 0; x < arrValues.count(); ++x)
                {
                    xDataValues[x+1] = arrValues.at(x).toDouble();
                }
                xDataMap.insert(itemObject[xValuesHeader->at(0)].toInt(),xDataValues);
            }
        }

        return tagData.value("Sport");
    }

    return QString();
}

void jsonHandler::fill_qmap(QHash<QString, QString> *qmap,QJsonObject *objItem)
{
    QString keyValue;
    for(int i = 0; i < objItem->keys().count(); ++i)
    {
        keyValue = objItem->keys().at(i);
        qmap->insert(keyValue,objItem->value(keyValue).toString().trimmed());
    }
}

void jsonHandler::check_keyList(QStringList *targetList,QStringList *map, QStringList *list)
{
    for(int i = 0; i < map->count(); ++i)
    {
        if(list->contains(map->value(i)))
        {
            targetList->insert(i,map->value(i));
        }
        else
        {
            if(map->value(i) == map->at(1))
            {
               targetList->insert(i,map->at(1));
            }
            if(map->value(i) == map->at(2))
            {
               targetList->insert(i,map->at(2));
            }
        }
    }
    for(int x = 0; x < list->count();++x)
    {
        if(!targetList->contains(list->at(x)))
        {
            (*targetList) << list->at(x);
        }
    }
}


void jsonHandler::fill_keyList(QStringList *targetList,QMap<int, QString> *map, QStringList *list)
{
    for(int i = 0; i < map->count(); ++i)
    {
        if(list->contains(map->value(i)))
        {
            targetList->insert(i,map->value(i));

            if(map->value(i) == "WATTS")
            {
                hasPMData = true;
            }
        }
        else
        {
            if(map->value(i) == "KM")
            {
               targetList->insert(i,"KM");
            }
            if(map->value(i) == "KPH")
            {
               targetList->insert(i,"KPH");
            }
        }
    }
    for(int x = 0; x < list->count();++x)
    {
        if(!targetList->contains(list->at(x)))
        {
            (*targetList) << list->at(x);
        }
    }
}

void jsonHandler::fill_model(QStandardItemModel *model, QJsonArray *jArray, QStringList *list)
{
    QJsonObject objItem;

    for(int row = 0; row < jArray->count(); ++row)
    {
        objItem = jArray->at(row).toObject();

        for(int col = 0; col < list->count(); ++col)
        {
            if(objItem[list->at(col)].isNull())
            {
                model->setData(model->index(row,col,QModelIndex()),0);
            }
            else
            {
                model->setData(model->index(row,col,QModelIndex()),objItem[list->at(col)].toVariant());
            }
        }
    }
}

void jsonHandler::fill_list(QJsonArray *jArray,QStringList *list)
{
    for(int i = 0; i < jArray->count(); ++i)
    {
        (*list) << jArray->at(i).toString();
    }
}

QJsonObject jsonHandler::mapToJson(QHash<QString,QString> *map)
{
    QJsonObject item;

    for(QHash<QString,QString>::const_iterator it =  map->cbegin(), end = map->cend(); it != end; ++it)
    {
        if(it.key() == "RECINTSECS")
        {
            item.insert(it.key(),1);
        }
        else
        {
            item.insert(it.key(),it.value()+" ");
        }
    }
    return item;
}

QJsonArray jsonHandler::listToJson(QStringList *list)
{
    QJsonArray jArray;

    for(int i = 0; i < list->count(); ++i)
    {
        jArray.insert(i,list->at(i));
    }
    return jArray;
}

QJsonArray jsonHandler::modelToJson(QStandardItemModel *model, QStringList *list)
{
    QJsonArray jArray;
    QVariant modelValue;
    QVariantHash valueMap;

    for(int row = 0; row < model->rowCount(); ++row)
    {
        for(int col = 0; col < list->count(); ++col)
        {
            valueMap.insert(list->at(col),model->data(model->index(row,col)));
        }
        jArray.insert(row,QJsonObject::fromVariantHash(valueMap));
        valueMap.clear();
    }
    return jArray;
}

void jsonHandler::init_actModel(QString tagName, QMap<int,QString> *mapValues,QStandardItemModel *model, QStringList *list,int addCol)
{
    QStringList valueList;
    QJsonArray itemArray;

    itemArray = activityItem[tagName].toArray();
    valueList << itemArray.at(0).toObject().keys();

    this->fill_keyList(list,mapValues,&valueList);

    model->setRowCount(itemArray.count());
    model->setColumnCount(list->count()+addCol);

    this->fill_model(model,&itemArray,list);
}

void jsonHandler::init_xdataModel(QStandardItemModel *model)
{
    QStringList xdataList = settings::get_jsonTags("xdata");

    QJsonArray itemArray;
    QJsonObject item_xdata = activityItem["XDATA"].toArray().at(0).toObject();
    this->fill_qmap(&xData,&item_xdata);

    itemArray = item_xdata[xdataList.at(2)].toArray();
    this->fill_list(&itemArray,&xdataUnits);

    itemArray = QJsonArray();
    itemArray = item_xdata[xdataList.at(1)].toArray();
    this->fill_list(&itemArray,&xdataValues);

    itemArray = QJsonArray();
    itemArray = item_xdata[xdataList.at(3)].toArray();
    QJsonObject obj_xdata = itemArray.at(0).toObject();

    model->setRowCount(itemArray.count());
    model->setColumnCount((obj_xdata.keys().count()+xdataValues.count()));


    for(int i = 0; i < itemArray.count(); ++i)
    {
        obj_xdata = itemArray.at(i).toObject();
        QJsonArray arrValues = obj_xdata[xdataList.at(1)].toArray();
        model->setData(model->index(i,0,QModelIndex()),obj_xdata["SECS"].toInt());
        model->setData(model->index(i,1,QModelIndex()),obj_xdata["KM"].toDouble());
        for(int x = 0; x < arrValues.count(); ++x)
        {
            model->setData(model->index(i,x+2,QModelIndex()),arrValues.at(x).toVariant());
        }
    }
}

void jsonHandler::init_jsonFile()
{
    QJsonArray intArray;
    activityItem = QJsonObject();
    activityItem = mapToJson(&rideData);
    activityItem["TAGS"] = mapToJson(&tagData);

    if(hasOverride)
    {
        int i = 0;
        for(QHash<QString,QString>::const_iterator it =  overrideData.cbegin(), end = overrideData.cend(); it != end; ++it,++i)
        {
            QJsonObject objOverride,objValue;
            objValue.insert("value",it.value());
            objOverride.insert(it.key(),objValue);
            intArray.insert(i,objOverride);
        }
        activityItem["OVERRIDES"] = intArray;
    }
}

void jsonHandler::write_actModel(QString tagName, QStandardItemModel *model, QStringList *list)
{
    activityItem[tagName] = modelToJson(model,list);
}

void jsonHandler::write_xdataModel(QStandardItemModel *model)
{
    QJsonArray item_xdata,intArray,value_array;
    QJsonObject xdataObj,item_array;
    int offset = 3;
    int xdataCol = 1;
    /*
    if(isSwim)
    {
        offset = 3;
        xdataCol = 1;
    }
    else
    {
        offset = 2;
        xdataCol = 0;
    }
    */
    xdataObj.insert("NAME",xData.value("NAME"));
    xdataObj.insert("UNITS",listToJson(&xdataUnits));
    xdataObj.insert("VALUES",listToJson(&xdataValues));

    for(int i = 0; i < model->rowCount(); ++i)
    {
        item_array.insert("SECS",QJsonValue::fromVariant(model->data(model->index(i,xdataCol,QModelIndex()))));
        item_array.insert("KM",QJsonValue::fromVariant(model->data(model->index(i,xdataCol+1,QModelIndex()))));

        for(int x = 0; x < xdataValues.count(); ++x)
        {
            value_array.insert(x,QJsonValue::fromVariant(model->data(model->index(i,x+offset,QModelIndex()))));
        }

        item_array["VALUES"] = value_array;
        intArray.insert(i,item_array);
        item_array = QJsonObject();
        value_array = QJsonArray();
    }

    xdataObj.insert("SAMPLES",intArray);
    item_xdata.insert(0,xdataObj);
    activityItem["XDATA"] = item_xdata;
}

void jsonHandler::write_jsonFile()
{
    QJsonDocument jsonDoc;
    QJsonObject rideFile;

    rideFile["RIDE"] = activityItem;
    jsonDoc.setObject(rideFile);

    QFile file(gcValues->value("actpath") + QDir::separator() + fileName);
    //QFile file(QCoreApplication::applicationDirPath() + QDir::separator() + fileName); //Test
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "File not open!" + fileName;
        return;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Compact));
    //file.write(jsonDoc.toJson()); //Test
    file.flush();
    file.close();
}
