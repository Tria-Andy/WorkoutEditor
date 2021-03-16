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
    QStringList *mapList = settings::get_xmlMapping("jsonfile");

    actObject = jsonobj.value(QString(mapList->at(0))).toObject();
    tagObject = actObject.value(QString(mapList->at(1))).toObject();

    workDateTime = QDateTime::fromString(actObject.value("STARTTIME").toString().trimmed(),"yyyy/MM/dd hh:mm:ss UTC").addSecs(localTime.offsetFromUtc());

    QVector<QString> actValues;
    actValues.insert(0,QLocale().dayName(workDateTime.date().dayOfWeek(),QLocale::ShortFormat));
    actValues.insert(1,workDateTime.toString("dd.MM.yyyy hh:mm"));
    actValues.insert(2,tagObject.value("Sport").toString().trimmed());
    actValues.insert(3,tagObject.value("Workout Code").toString().trimmed());
    actValues.insert(4,QString::number(counter));

    return actValues;
}

QString jsonHandler::read_jsonContent(QString jsonfile,bool fullPath)
{
    QFile file;

    if(fullPath)
    {
        file.setFileName(jsonfile);
    }
    else
    {
        file.setFileName(gcValues->value("actpath")+QDir::separator()+jsonfile);
    }

    if (!file.open(QFile::ReadOnly | QFile::Text))
    {
        qDebug() << "File not open:"+jsonfile;
    }
    else
    {
        QString jsonContent = file.readAll();
        QJsonObject actObject = QJsonDocument::fromJson(jsonContent.toUtf8()).object();
        file.close();

        QJsonObject  activityObject,itemObject,objValue;
        QJsonArray itemArray;
        QStringList *mapList = settings::get_xmlMapping("jsonfile");
        QStringList *keyList;

        //Fill RIDE Map
        activityObject = actObject.value(mapList->at(0)).toObject();
        this->fill_qmap(&rideData,&activityObject);
        //this->fill_activityData(mapList->at(0),&activityObject);

        //Fill TAGS Map
        itemObject = activityObject.value(mapList->at(1)).toObject();
        this->fill_qmap(&tagData,&itemObject);
        //this->fill_activityData(mapList->at(1),&itemObject);

        //Check if File contains OVERRIDES
        if(activityObject.keys().contains(mapList->at(2)))
        {
            itemArray = activityObject[mapList->at(2)].toArray();
            if(itemArray.count() > 0) hasOverride = true;

            for(int i = 0; i < itemArray.count(); ++i)
            {
                itemObject = itemArray.at(i).toObject();
                objValue = itemObject[itemObject.keys().first()].toObject();
                overrideData.insert(itemObject.keys().first(),objValue["value"].toString());
            }
        }

        //Fill IntervallMap
        itemArray = activityObject.value(mapList->at(3)).toArray();
        keyList = settings::get_xmlMapping("intervals");

        for(int i = 0; i < itemArray.count(); ++i)
        {
            itemObject = itemArray.at(i).toObject();
            intervallMap.insert(i,qMakePair(itemObject[keyList->at(1)].toInt(),itemObject[keyList->at(2)].toInt()));
        }

        //Fill SampleMap
        itemArray = activityObject.value(mapList->at(4)).toArray();
        keyList = settings::get_xmlMapping("samples");
        QStringList sampleKeys = itemArray.first().toObject().keys();

        if(sampleKeys.count() <= 1)
        {
            sampleKeys << keyList->at(1);
        }

        this->check_keyList(&sampleUseKeys,keyList,&sampleKeys);
        powerFlag = sampleUseKeys.contains(keyList->at(5));

        QVector<double> sampleValues(sampleUseKeys.count()-1,0);

        for(int i = 0; i < itemArray.count(); ++i)
        {
            itemObject = itemArray.at(i).toObject();
            for(int value = 1; value < sampleUseKeys.count(); ++value)
            {
                sampleValues[value-1] = itemObject[sampleUseKeys.at(value)].toDouble();
            }
            sampleMap.insert(itemObject[sampleUseKeys.at(0)].toInt(),sampleValues);
        }

        //Adjust Fist and Last Interval
        intervallMap.insert(intervallMap.firstKey(),qMakePair(0,intervallMap.first().second));
        intervallMap.insert(intervallMap.lastKey(),qMakePair(intervallMap.last().first,sampleMap.lastKey()));

        //Check and Fill XDATA Map
        if(activityObject.keys().contains(mapList->at(5)))
        {
            hasXdata = true;
            xdataHeader = settings::get_xmlMapping("xdata");
            xValuesKeys = settings::get_xmlMapping("xdatavalues");
            QVector<double> dataValues;

            itemArray = activityObject[mapList->at(5)].toArray();

            for(int item = 0; item < itemArray.count(); ++item)
            {
                itemObject = itemArray.at(item).toObject();
                xDataMap.insert(item,itemObject.toVariantMap());
            }

            dataValues.resize(xDataMap.first().value(xdataHeader->at(1)).toList().count()+1);
            QList<QVariant> xdataList = xDataMap.first().value(xdataHeader->at(3)).toList();
            QMap<QString,QVariant> xdataMap;

            for(int xdata = 0; xdata < xdataList.count();++xdata)
            {
                xdataMap = xdataList.at(xdata).toMap();

                dataValues[0] = xdataMap.value(xValuesKeys->at(1)).toDouble();

                for(int value = 0; value < xdataMap.value(xdataHeader->at(1)).toList().toVector().count(); ++value)
                {
                    dataValues[value+1] = xdataMap.value(xdataHeader->at(1)).toList().toVector().at(value).toDouble();
                }
                xDataValues.insert(xdataMap.value(xValuesKeys->at(0)).toInt(),dataValues);
            }
        }
        //activityData.value(mapList->at(1)).value("Sport").toString().trimmed();
        return tagData.value("Sport");
    }

    return QString();
}

void jsonHandler::fill_activityData(QString jsonTag, QJsonObject *jsonObject)
{
    QString tagKey;
    QHash<QString,QVariant> actDataValues;

    for(int i = 0; i < jsonObject->keys().count(); ++i)
    {
        tagKey = jsonObject->keys().at(i);
        actDataValues.insert(tagKey,jsonObject->value(tagKey));
    }
    activityData.insert(jsonTag,actDataValues);
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

void jsonHandler::prepareWrite_JsonFile(bool manual)
{
    QString jsonFile = tagData.value("Filename");
    tagData.insert("Updated","1");
    QJsonArray jsonArray;
    QStringList *mapList = settings::get_xmlMapping("jsonfile");
    QStringList *valueList = settings::get_xmlMapping("intervals");
    QJsonObject activityItem = QJsonObject();
    activityItem = mapToJson(&rideData);
    activityItem[mapList->at(1)] = mapToJson(&tagData);

    int counter = 0;

    if(hasOverride)
    {
        for(QHash<QString,QString>::const_iterator it =  overrideData.cbegin(), end = overrideData.cend(); it != end; ++it)
        {
            QJsonObject objOverride,objValue;
            objValue.insert("value",it.value());
            objOverride.insert(it.key(),objValue);
            jsonArray.insert(counter++,objOverride);
        }
        activityItem[mapList->at(2)] = jsonArray;
    }

    jsonArray = QJsonArray();
    QVariantHash valueMap;

    //Set Intervalls
    for(QMap<int,QPair<int,int>>::const_iterator interval = intervallMap.cbegin(), end = intervallMap.cend(); interval != end; ++interval)
    {
        valueMap.insert(valueList->at(0),intNameMap.value(interval.key()));
        valueMap.insert(valueList->at(1),interval.value().first);
        valueMap.insert(valueList->at(2),interval.value().second);
        jsonArray.insert(interval.key(),QJsonObject::fromVariantHash(valueMap));
        valueMap.clear();
    }

    activityItem[mapList->at(3)] = jsonArray;
    jsonArray = QJsonArray();

    //Set Sample
    for(QMap<int,QVector<double>>::const_iterator sampleSec = sampleMap.cbegin(), end = sampleMap.cend(); sampleSec != end; ++sampleSec)
    {
        valueMap.insert(sampleUseKeys.at(0),sampleSec.key());

        for(int value = 1; value < sampleUseKeys.count(); ++value)
        {
                valueMap.insert(sampleUseKeys.at(value),sampleSec.value().at(value-1));
        }

        jsonArray.insert(sampleSec.key(),QJsonObject::fromVariantHash(valueMap));
        valueMap.clear();
    }
    activityItem[mapList->at(4)] = jsonArray;
    jsonArray = QJsonArray();

    //Set XDATA
    counter = 0;

    if(hasXdata)
    {
        QStringList *xdataKeys = settings::get_xmlMapping("xdata");
        valueList = settings::get_xmlMapping("xdatavalues");

        QVector<QVariant> xdataValue(xDataValues.first().count()-1);
        QVariantMap xdataMap,xdataUpdateMap;
        QList<QVariant> xdataList;

        for(QMap<int,QVector<double>>::const_iterator xdata = xDataValues.cbegin(), end = xDataValues.cend(); xdata != end; ++xdata)
        {
            xdataMap.insert(valueList->at(0),xdata.key());
            xdataMap.insert(valueList->at(1),xdata.value().at(0));

            for(int i = 1,pos = 0; i < xdata.value().count(); ++i,++pos)
            {
                xdataValue[pos] = xdata.value().at(i);
            }

            xdataMap.insert(xdataKeys->at(1),xdataValue.toList());
            xdataList.append(xdataMap);
            xdataMap.clear();
        }
        xdataUpdateMap = xDataMap.first();
        xdataUpdateMap.insert(xdataKeys->at(3),xdataList);
        xDataMap.insert(0,xdataUpdateMap);

        for(QMap<int,QVariantMap>::const_iterator it = xDataMap.cbegin(), end = xDataMap.cend(); it != end; ++it)
        {
            jsonArray.insert(counter++,QJsonObject::fromVariantMap(it.value()));
        }

        activityItem[mapList->at(5)] = jsonArray;
    }

    QJsonDocument jsonDoc;
    QJsonObject activityFile;
    QString importPath;

    activityFile[mapList->at(0)] = activityItem;
    jsonDoc.setObject(activityFile);

    if(manual)
    {
        importPath = gcValues->value("uploadpath");
    }
    else
    {
        importPath = gcValues->value("actpath");
    }


    QFile file(importPath + QDir::separator() + jsonFile);
    //QFile file(QCoreApplication::applicationDirPath() + QDir::separator() + jsonFile); //Test

    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "File not open!" + jsonFile;
        return;
    }

    file.write(jsonDoc.toJson(QJsonDocument::Compact));
    //file.write(jsonDoc.toJson(QJsonDocument::Indented)); //Test
    file.flush();
    file.close();

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
