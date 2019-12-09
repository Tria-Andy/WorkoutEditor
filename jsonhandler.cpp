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
}

void jsonHandler::fill_qmap(QHash<QString, QString> *qmap,QJsonObject *objItem)
{
    QString keyValue;
    for(int i = 0; i < objItem->keys().count(); ++i)
    {
        keyValue = objItem->keys().at(i);
        qmap->insert(keyValue,objItem->value(keyValue).toString().trimmed());
        if(keyValue == "OVERRIDES") hasOverride = true;
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

QString jsonHandler::readJsonContent(QString jsonfile)
{
    hasOverride = hasXdata = false;
    QJsonObject itemObject;
    QJsonArray itemArray;

    QJsonDocument d = QJsonDocument::fromJson(jsonfile.toUtf8());
    QJsonObject jsonobj = d.object();

    activityItem = jsonobj.value(QString("RIDE")).toObject();
    this->fill_qmap(&rideData,&activityItem);

    itemObject = activityItem.value(QString("TAGS")).toObject();
    this->fill_qmap(&tagData,&itemObject);

    if(hasOverride)
    {
        QJsonObject objOverride,objValue;
        itemArray = activityItem["OVERRIDES"].toArray();

        for(int i = 0; i < itemArray.count(); ++i)
        {
            objOverride = itemArray.at(i).toObject();
            objValue = objOverride[objOverride.keys().first()].toObject();
            overrideData.insert(objOverride.keys().first(),objValue["value"].toString());
        }
    }
    if(activityItem.contains("XDATA")) hasXdata = true;

    fileName = tagData.value("Filename").trimmed();

    return tagData.value("Sport");
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
    int offset;
    int xdataCol;
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
