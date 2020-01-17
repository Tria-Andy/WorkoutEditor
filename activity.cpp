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

#include "activity.h"
#include <math.h>
#include <iostream>
#include <random>
#include <functional>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>

Activity::Activity()
{
    fileMap = settings::getStringMapPointer(settings::stingMap::File);
    infoHeader = settings::getHeaderMap("activityinfo");
    levels = settings::get_listValues("Level");
    this->xml_toListMap(fileMap->value("activityfile"));
    this->fill_actMap();
}

void Activity::update_activityMap(QPair<int, QString> intKey, QMap<QPair<int, QString>, QVector<double> > intValues)
{
    activityMap.insert(intKey,intValues);
}

void Activity::update_intervalMap(int intKey, QString intName,QPair<int, int> startStop)
{
    intervallMap.insert(intKey,startStop);
    intNameMap.insert(intKey,intName);
}

void Activity::update_xDataMap(int secKey, QVector<double> xdata)
{
    xDataValues.insert(secKey,xdata);
}

void Activity::update_paceInZone(QPair<QString,QString> level, int time)
{
    if(level.first != level.second)
    {
        int oldLevelTime = paceTimeInZone.value(level.first);
        int newLevelTime = paceTimeInZone.value(level.second);

        paceTimeInZone.insert(level.first,oldLevelTime-time);
        paceTimeInZone.insert(level.second,newLevelTime+time);
    }
}

bool Activity::clear_loadedActivity()
{
    xDataValues.clear();
    xDataMap.clear();

    intervallMap.clear();
    intNameMap.clear();
    averageMap.clear();

    powerFlag = false;
    moveTime = 0;

    paceTimeInZone.clear();
    swimHFZoneFactor.clear();
    rangeLevels.clear();

    sampleUseKeys.clear();
    sampleMap.clear();

    activityData.clear();
    activityMap.clear();
    activityInfo.clear();

    return true;
}

void Activity::save_actvitiyFile()
{
    QStringList listValues;
    int counter = 0;
    int maxFileCount = generalValues->value("filecount").toInt();
    int writeEntry = gcActivtiesMap.last().at(4).toInt() - maxFileCount;
    mapList.clear();

    for(QMap<QString,QVector<QString>>::const_iterator it = gcActivtiesMap.cbegin(), end = gcActivtiesMap.cend(); it != end; ++it)
    {
        if(it.value().at(4).toInt() >= writeEntry)
        {
            listValues << it.key();
            for(int value = 0; value < it.value().count(); ++value)
            {
                listValues << it.value().at(value);
            }
            mapList.insert(counter++,listValues);
            listValues.clear();
        }
    }
    this->listMap_toXml(fileMap->value("activityfile"));
}

void Activity::prepare_mapToJson()
{
    QVector<double> xValues(4,0);
    double distStep = 0.0;
    QVector<double> sampleValues(sampleMap.first().count(),0);

    for(QMap<QPair<int,QString>,QMap<QPair<int,QString>,QVector<double>>>::const_iterator interval = activityMap.cbegin(), end = activityMap.cend(); interval != end; ++interval)
    {
        if(isSwim)
        {
            for(QMap<QPair<int,QString>,QVector<double>>::const_iterator lapStart = interval.value().cbegin(), lapEnd = interval.value().cend(); lapStart != lapEnd; ++lapStart)
            {
                xValues[0] = lapStart.value().at(1);
                xValues[1] = lapStart.value().at(2);
                xValues[2] = lapStart.value().at(3);
                xValues[3] = lapStart.value().at(4);
                xDataValues.insert(lapStart.value().at(0),xValues);

                if(interval.key().second != breakName)
                {
                    for(int intSec = lapStart.value().at(0); intSec <  lapStart.value().at(0)+lapStart.value().at(3); ++intSec)
                    {
                        if(intSec == lapStart.value().at(0))
                        {
                            sampleValues[0] = distStep + (lapStart.value().at(6)/2);
                        }
                        else
                        {
                            sampleValues[0] = distStep;
                        }
                        sampleValues[1] = lapStart.value().at(5);
                        sampleValues[2] = lapStart.value().at(4);
                        distStep = distStep + lapStart.value().at(6);
                        sampleMap.insert(intSec,sampleValues);
                    }
                }
                else
                {
                    for(int intSec = lapStart.value().at(0); intSec <=  intervallMap.value(interval.key().first).second; ++intSec)
                    {
                        sampleValues[0] = distStep;
                        sampleValues[1] = lapStart.value().at(5);
                        sampleValues[2] = lapStart.value().at(4);
                        sampleMap.insert(intSec,sampleValues);
                    }
                }
            }

            if(interval.key().second != breakName)
            {
                sampleValues[0] = interval->last().at(1)-(interval->last().at(6)/2) + (poolLength/1000.0);
                sampleValues[1] = interval->last().at(5);
                sampleValues[2] = interval->last().at(4);
                sampleMap.insert(intervallMap.value(interval.key().first).second ,sampleValues);
            }
        }
        else
        {
            for(QMap<QPair<int,QString>,QVector<double>>::const_iterator lapStart = interval.value().cbegin(), lapEnd = interval.value().cend(); lapStart != lapEnd; ++lapStart)
            {
                for(int intSec = intervallMap.value(interval.key().first).first; intSec <= intervallMap.value(interval.key().first).second; ++intSec)
                {
                    sampleValues = sampleMap.value(intSec);
                    sampleValues[0] = distStep;
                    sampleValues[1] = this->polish_SpeedValues(sampleValues.at(1),lapStart.value().at(2),true);
                    distStep = distStep + lapStart.value().at(1);
                    sampleMap.insert(intSec,sampleValues);
                }
            }
        }
    }
    this->prepareWrite_JsonFile();
}

bool Activity::check_activityFiles()
{
    QString filePath;
    QString jsonFile;
    bool newActivity = false;
    int maxFileCount = generalValues->value("filecount").toInt();
    QDir directory(gcValues->value("actpath"));
    directory.setSorting(QDir::Name | QDir::Reversed);
    directory.setFilter(QDir::Files);
    QFileInfoList fileList = directory.entryInfoList();
    maxFileCount = fileList.count() > maxFileCount ? maxFileCount : fileList.count();
    QSet<QString> currentFiles;

    for(int fileCount = 0; fileCount < maxFileCount; ++fileCount)
    {
        filePath = fileList.at(fileCount).path()+QDir::separator()+fileList.at(fileCount).fileName();
        jsonFile = fileList.at(fileCount).fileName();
        currentFiles.insert(jsonFile);

        if(!gcActivtiesMap.contains(jsonFile))
        {
            gcActivtiesMap.insert(jsonFile,this->read_activityMeta(filePath,gcActivtiesMap.count()));
            newActivity = true;
        }
    }

    for(QMap<QString,QVector<QString>>::const_iterator  it = gcActivtiesMap.cbegin(); it != gcActivtiesMap.cend(); ++it)
    {
        //if(!currentFiles.contains(it.key())) gcActivtiesMap.remove(it.key());
    }

    return newActivity;
}


void Activity::fill_actMap()
{
    QVector<QString> actValues;
    QDir directory(gcValues->value("actpath"));

    bool refresh = false;

    for(QMap<int,QStringList>::const_iterator it = mapList.cbegin(), end = mapList.cend(); it != end; ++it)
    {
        actValues.insert(0,it.value().at(1));
        actValues.insert(1,it.value().at(2));
        actValues.insert(2,it.value().at(3));
        actValues.insert(3,it.value().at(4));
        actValues.insert(4,QString::number(it.key()));

        for(int i = 0; i < it.value().count(); ++i)
        {
            if(it.value().at(i).isEmpty()) refresh = true;
        }

        if(refresh) actValues = read_activityMeta(directory.path()+QDir::separator()+it.value().at(0),it.key());

        gcActivtiesMap.insert(it.value().at(0),actValues);
        actValues.clear();
    }
    if(this->check_activityFiles() || refresh) save_actvitiyFile();
}


bool Activity::read_jsonFile(QString jsonfile,bool fullPath)
{
    QFileInfo fileinfo(jsonfile);

    if(fileinfo.suffix() == "json")
    {
        this->set_currentSport(this->read_jsonContent(jsonfile,fullPath));
        usePMData = powerFlag;

        QStringList valueList;
        QString stgValue;
        valueList = settings::get_listValues("JsonFile");

        for(int i = 0; i < valueList.count();++i)
        {
            stgValue = valueList.at(i);
            activityInfo.insert(stgValue,tagData.value(stgValue));
        }
        activityInfo.insert("Date",rideData.value("STARTTIME"));

        return true;
    }
    return false;
}

QMap<int,QVector<double>> Activity::get_xData(QPair<int,int> intKey)
{
    QMap<int,QVector<double>> xDataValues;

    for(QMap<int,QVector<double>>::const_iterator xdata = xDataValues.lowerBound(intKey.first), end = xDataValues.lowerBound(intKey.second); xdata != end; ++xdata)
    {
        xDataValues.insert(xdata.key(),xdata.value());
    }

    return xDataValues;
}

QMap<QPair<int,QString>,QVector<double>> Activity::get_swimLapData(int intCount,QPair<int, int> intKey)
{
    QMap<QPair<int,QString>,QVector<double>> xDataValue;
    QPair<int,QString> xDataKey;
    int lapCount = 1;
    QString lapLevel;
    int timezone = 0;
    int lapTime = 0;

    for(QMap<int,QVector<double>>::const_iterator xdata = xDataValues.lowerBound(intKey.first), end = xDataValues.lowerBound(intKey.second); xdata != end; ++xdata)
    {
        lapTime = round(xdata.value().at(2));

        if(xdata.value().at(1) != 0)
        {
            lapLevel = this->checkRangeLevel(this->calc_lapPace(lapTime,poolLength));
            timezone = paceTimeInZone.value(lapLevel);
            xDataKey.first = lapCount;
            xDataKey.second = QString::number(intCount)+"_"+QString::number((lapCount++)*poolLength)+"_"+lapLevel;
            xDataValue.insert(xDataKey,xdata.value());
            paceTimeInZone.insert(lapLevel,timezone+lapTime);
            moveTime = moveTime + lapTime;
        }
        else
        {
            timezone = paceTimeInZone.value(levels.at(0));
            xDataKey.first = 0;
            xDataKey.second = breakName;
            xDataValue.insert(xDataKey,xdata.value());
            paceTimeInZone.insert(levels.at(0),timezone+lapTime);
        }
    }

    return xDataValue;
}

QMap<QPair<int, QString>, QVector<double> > Activity::get_intervalData(int intCount, QPair<int, int> intKey)
{
    QMap<QPair<int,QString>,QVector<double>> intervalValues;
    QPair<int,QString> intervalKey;
    QVector<double> intervalSum(sampleMap.first().count(),0);

    if(intKey.first < 0) intKey.first = 0;

    int lapOffSet;
    if((sampleMap.lastKey() < intKey.second+1) || isUpdated)
    {
        lapOffSet = intKey.second;
    }
    else
    {
        lapOffSet = intKey.second+1;
    }

    intervalSum[0] = intKey.second - intKey.first;
    intervalSum[1] = sampleMap.value(lapOffSet).at(0) - sampleMap.value(intKey.first).at(0);

    for(QMap<int,QVector<double>>::const_iterator sampleData = sampleMap.lowerBound(intKey.first), end = sampleMap.lowerBound(intKey.second); sampleData != end; ++sampleData)
    {
        intervalSum[2] = intervalSum.at(2) + sampleData.value().at(1);
        intervalSum[3] = intervalSum.at(3) + sampleData.value().at(3);
        intervalSum[4] = intervalSum.at(4) + sampleData.value().at(2);
        intervalSum[5] = intervalSum.at(5) + sampleData.value().at(4);
    }

    intervalSum[2] = intervalSum.at(2) / intervalSum.at(0);
    intervalSum[3] = intervalSum.at(3) / intervalSum.at(0);
    intervalSum[4] = intervalSum.at(4) / intervalSum.at(0);
    intervalSum[5] = intervalSum.at(5) / intervalSum.at(0);

    intervalKey.first = intCount;
    intervalKey.second = checkRangeLevel(intervalSum.at(3));

    intervalValues.insert(intervalKey,intervalSum);

    return intervalValues;
}

QMap<QPair<int, QString>, QVector<double> > Activity::get_simpleData(int intCount, QPair<int, int> intKey)
{
    QMap<QPair<int,QString>,QVector<double>> intervalValues;
    QPair<int,QString> intervalKey;
    QVector<double> intervalSum(sampleMap.first().count(),0);

    intervalSum[0] = intKey.second - intKey.first;
    intervalSum[1] = 0;

    intervalKey.first = intCount;
    intervalKey.second = checkRangeLevel(80);

    intervalValues.insert(intervalKey,intervalSum);

    return intervalValues;
}

void Activity::set_polishData()
{
    QVector<double> polishValues(3,0);
    for(QMap<int,QVector<double>>::const_iterator sampleData = sampleMap.cbegin(), end = sampleMap.cend(); sampleData != end; ++sampleData)
    {
        if(isSwim)
        {
            polishValues[0] = sampleData.value().at(1);
            polishValues[1] = sampleData.value().at(2);
            polishValues[2] = sampleData.value().at(0);
        }
        else
        {
            polishValues[0] = sampleData.value().at(1);
            polishValues[1] = sampleData.value().at(3);
            polishValues[2] = sampleData.value().at(4);
        }
        polishData.insert(sampleData.key(),polishValues);
    }
}

void Activity::set_activityHeader(QString headerName,QStringList *headerList)
{
    QStringList *tempList = settings::getHeaderMap(headerName);

    for(int i = 0; i < tempList->count(); ++i)
    {
        headerList->append(tempList->at(i));
    }
}

void Activity::extend_activityHeader()
{
    if(powerFlag)
    {
        this->set_activityHeader("averagepower",&averageHeader);
        this->set_activityHeader("powerheader",&activityHeader);
    }
    else
    {
        this->set_activityHeader("paceheader",&activityHeader);
    }
}

QString Activity::set_intervalInfo(QTreeWidgetItem *item, bool isLap)
{
    QTreeWidgetItem *parentItem = nullptr;
    QString lapName;
    QString intCount;
    QStringList intKey;

    if(isLap)
    {
        int calcPace = calc_lapPace(get_timesec(item->data(4,Qt::DisplayRole).toString()),poolLength);
        intKey = parentItem->data(0,Qt::AccessibleTextRole).toString().split("-");
        intCount = intKey.at(1).split("_").first();
        parentItem = item->parent();

        lapName = intCount+"_"+QString::number(item->data(0,Qt::AccessibleTextRole).toInt() * poolLength)+"_"+
                          checkRangeLevel(calcPace);
    }
    else
    {
        lapName = intCount+"_Int_"+QString::number(parentItem->childCount()*poolLength);
        item->setData(0,Qt::AccessibleTextRole,intKey.first()+"-"+lapName);
    }

    return lapName;
}

void Activity::set_activityData()
{
    QPair<int,QString> activityKey;
    QMap<QPair<int,QString>,QVector<double>> actIntervals;
    int intCounter = 1;
    double activityDist = 0,activityHR = 0;
    moveTime = 0;

    for(QMap<int,QPair<int,int>>::const_iterator interval = intervallMap.cbegin(), end = intervallMap.cend(); interval != end; ++interval)
    {
        activityKey.first = interval.key();

        if(isSwim)
        {
            actIntervals= this->get_swimLapData(intCounter,interval.value());
            if(actIntervals.firstKey().first == 0)
            {
                activityKey.second = actIntervals.firstKey().second;
            }
            else
            {
                activityDist = activityDist + ((actIntervals.count()*poolLength)/distFactor);
                activityKey.second = QString::number(intCounter++)+"_Int_"+QString::number(actIntervals.count()*poolLength);
            }
            activityMap.insert(activityKey, actIntervals);
        }
        else
        {
            if(isBike || isRun)
            {
                actIntervals = this->get_intervalData(intCounter++,interval.value());
                activityDist = activityDist + (actIntervals.first().at(1));
                activityHR = activityHR + actIntervals.first().at(5);
            }
            else if(isTria)
            {
                actIntervals = this->get_intervalData(intCounter++,interval.value());
                activityDist = activityDist + (actIntervals.first().at(1));
            }
            else
            {
                actIntervals = this->get_simpleData(intCounter++,interval.value());
            }

            activityKey.second = actIntervals.firstKey().second;
            activityMap.insert(activityKey,actIntervals);
        }
    }
    activityInfo.insert("Distance",QString::number(set_doubleValue(activityDist,true)));
    this->set_polishData();

    if(isSwim)
    {
        this->set_swimTimeInZone(false);
    }
    else
    {
        double avgHF = round(activityHR / activityMap.count());
        double totalCal = this->calc_totalCal(actWeight,avgHF,sampleMap.count());
        activityInfo.insert("Total Cal",QString::number(totalCal));
        activityInfo.insert("AvgHF",QString::number(avgHF));
    }
}

void Activity::prepare_baseData()
{
    this->fill_rangeLevel(usePMData);
    isUpdated = tagData.value("Updated").toInt();
    isTimeBased = tagData.value("Timebased").toInt();
    isIndoor = false;
    averageHeader.clear();
    activityHeader.clear();
    actWeight = settings::get_weightforDate(QDateTime::fromString(activityInfo.value("Date"),"yyyy/MM/dd hh:mm:ss UTC").addSecs(QDateTime::currentDateTime().offsetFromUtc()));

    this->set_activityHeader("averagepace",&averageHeader);

    if(isSwim)
    {
        distFactor = 1000;
        poolLength = tagData.value("Pool Length").toDouble();
        breakName = generalValues->value("breakname");
        swimType = settings::get_listValues("SwimStyle");
        hfMax = thresValues->value("hfmax");

        this->set_activityHeader("averageswim",&averageHeader);
        this->set_activityHeader("swimheader",&activityHeader);

        QString temp;
        double zoneLow, zoneHigh;
        int zoneCount = levels.count();

        //Set Swim HFZone low and high
        for(int i = 0; i < levels.count(); i++)
        {
            paceTimeInZone.insert(levels.at(i),0);
            hfTimeInZone.insert(levels.at(i),0);

            temp = settings::get_rangeValue("HF",levels.at(i));
            zoneLow = temp.split("-").first().toDouble();
            zoneHigh = temp.split("-").last().toDouble();

            zoneLow = ceil(hfThreshold*(zoneLow/100.0));
            zoneHigh = ceil(hfThreshold*(zoneHigh/100.0));

            if(i < zoneCount-1)
            {
                hfZoneAvg.insert(levels.at(i),ceil((zoneLow + zoneHigh) / 2));
            }
            else
            {
                hfZoneAvg.insert(levels.at(i),ceil((zoneLow + hfMax) / 2));
            }
        }

        QVector<double> hf_factor = {0.50,0.35,0.10,0.05};
        QVector<double> hFactor(zoneCount);
        hFactor.fill(0);

        for(int i = 0; i < hf_factor.count(); ++i)
        {
            hFactor[i] = hf_factor.at(i);
        }
        swimHFZoneFactor.insert(breakName,hFactor);

        int vPos = 0;
        for(int i = 0,v=0; i < zoneCount; ++i)   //paceZone
        {
            hFactor.fill(0);
            if(i % 2 == 0 && i > 0) ++v;
            for(int x = 0; x < hf_factor.count(); ++x)
            {
                vPos = x+v;
                hFactor[vPos] = hf_factor.at(x);
            }
            swimHFZoneFactor.insert(levels.at(i),hFactor);
        }
    }
    else
    {
        distFactor = 1;
        polishFactor = 0.1;

        this->set_activityHeader("baseheader",&activityHeader);

        if(isBike)
        {
            if(tagData.value("SubSport") == "home trainer") isIndoor = true;
            this->extend_activityHeader();
        }
        else if(isRun)
        {
            this->extend_activityHeader();
        }
        else if(isTria)
        {
            this->set_activityHeader("triaheader",&activityHeader);
        }
        else
        {

        }

        activityHeader.move(4,activityHeader.count()-1);
        activityHeader.move(4,activityHeader.count()-1);
    }
    activityInfo.insert("Duration",QDateTime::fromTime_t(sampleMap.count()).toUTC().toString("hh:mm:ss"));
}

void Activity::set_swimTimeInZone(bool recalc)
{
    int timeinzone;
    this->hasOverride = true;

    if(recalc)
    {
        for(QHash<QString,int>::const_iterator it = hfTimeInZone.cbegin(), end = hfTimeInZone.cend(); it != end; ++it)
        {
            hfTimeInZone.insert(it.key(),0);
        }
    }

    for(QHash<QString,QVector<double>>::const_iterator it = swimHFZoneFactor.cbegin(), end = swimHFZoneFactor.cend(); it != end; ++it)
    {
        for(int i = 0; i < levels.count(); ++i)
        {
            timeinzone = hfTimeInZone.value(levels.at(i));
            timeinzone = timeinzone + (swimHFZoneFactor.value(it.key()).at(i) * paceTimeInZone.value(it.key()));
            hfTimeInZone.insert(levels.at(i),timeinzone);
        }
    }

    int hfAvg = 0;
    double workoutTime = this->get_timesec(activityInfo.value("Duration"));

    for(QHash<QString,int>::const_iterator it = hfZoneAvg.cbegin(), end = hfZoneAvg.cend(); it != end; ++it)
    {
        hfAvg = hfAvg + ceil((it.value() * hfTimeInZone.value(it.key())) / workoutTime);
    }

    activityInfo.insert("AvgHF",QString::number(hfAvg));
    overrideData.insert("average_hr",QString::number(hfAvg));

    double totalCal = this->calc_totalCal(actWeight,hfAvg,moveTime);
    activityInfo.insert("Total Cal",QString::number(totalCal));
    overrideData.insert("total_kcalories",QString::number(totalCal));

    int hfZone = 0;

    for(QHash<QString,int>::const_iterator it = hfTimeInZone.cbegin(), end = hfTimeInZone.cend(); it != end; ++it)
    {
        hfZone = levels.indexOf(it.key())+1;
        overrideData.insert("time_in_zone_H" + QString::number(hfZone),QString::number(it.value()));
    }
}

void Activity::fill_rangeLevel(bool isPower)
{
    QPair<double,double> zoneValues;
    QString currentValues;
    double zoneLow = 0;
    double zoneHigh = 0;

    for(int i = 0; i < levels.count(); ++i)
    {
        currentValues = settings::get_rangeValue(currentSport,levels.at(i));
        zoneLow = currentValues.split("-").first().toDouble();
        zoneHigh = currentValues.split("-").last().toDouble();

        if(isPower)
        {
            zoneValues.first = ceil(thresPower*(zoneHigh/100.0));
            zoneValues.second = ceil(thresPower*(zoneLow/100.0));
        }
        else
        {
            zoneValues.first = ceil(thresPace/(zoneLow/100.0));
            zoneValues.second = ceil(thresPace/(zoneHigh/100.0));
        }
        rangeLevels.insert(levels.at(i),zoneValues);
    }
}

QString Activity::checkRangeLevel(double lapValue)
{
    for(QHash<QString,QPair<double,double>>::const_iterator it = rangeLevels.cbegin(), end = rangeLevels.cend(); it != end; ++it)
    {
        if(lapValue <= it.value().first && lapValue > it.value().second) return it.key();
    }
    return levels.first();
}

QString Activity::set_intervalName(QTreeWidgetItem *item,bool isInterval)
{
    QString lapName;

    if(isSwim)
    {
        if(isInterval)
        {
            lapName = item->data(0,Qt::AccessibleTextRole).toString().split("-").last().split("_").first()+"_Int_"+
                      item->data(3,Qt::DisplayRole).toString()+"_"+
                      checkRangeLevel(item->data(6,Qt::UserRole).toInt());
        }
        else
        {
            QTreeWidgetItem *parent = item->parent();
            int lapCount = parent->indexOfChild(item)+1;
            lapName = parent->data(0,Qt::AccessibleTextRole).toString().split("-").last().split("_").first()+"_"+
                      QString::number(lapCount * poolLength)+"_"+
                      checkRangeLevel(item->data(6,Qt::UserRole).toInt());
        }
    }
    else
    {
        if(usePMData)
        {
            lapName = item->data(0,Qt::UserRole).toString()+"_"+checkRangeLevel(item->data(7,Qt::DisplayRole).toDouble());
        }
        else
        {
            lapName = item->data(0,Qt::UserRole).toString()+"_"+checkRangeLevel(item->data(5,Qt::UserRole).toDouble());
        }

        QString label;

        if(isTimeBased)
        {
            int lapTime = item->data(1,Qt::UserRole).toInt();

            if(lapTime < 60)
            {
                label = "Sec";
                lapTime = ceil(lapTime);
            }
            else
            {
                label = "Min";
                lapTime = (ceil(lapTime/10.0)*10)/60.0;
            }

            lapName = lapName + "-"+QString::number(lapTime)+label;
        }
        else
        {
            double lapDist = item->data(activityHeader.indexOf("Distance (Int)"),Qt::DisplayRole).toDouble();

            lapDist = (round(lapDist*10)/10.0);
            if(lapDist < 1)
            {
                label = "M";
                lapDist = ceil(lapDist*1000.0);
            }
            else
            {
                label = "K";
            }

            lapName = lapName + "-"+QString::number(lapDist)+label;
        }
    }

    return lapName;
}

void Activity::prepare_save()
{
    activityMap.clear();
    intervallMap.clear();

    if(isSwim) xDataValues.clear();
}

void Activity::set_workoutContent(QString content)
{
    this->tagData.insert("Workout Content",content);
}

QPair<int,QVector<double>> Activity::set_averageMap(QTreeWidgetItem *selItem, int avgCol)
{
    QPair<int,QVector<double>> averageData;
    QVector<double> avgValues(0,0);

    if(selItem->data(avgCol,Qt::UserRole).toBool())
    {
        selItem->setData(avgCol,Qt::DisplayRole,"-");
        selItem->setData(avgCol,Qt::UserRole,false);

        averageMap.remove(selItem->data(0,Qt::DisplayRole).toString());
    }
    else
    {
        selItem->setData(avgCol,Qt::DisplayRole,"+");
        selItem->setData(avgCol,Qt::UserRole,true);

        if(isSwim)
        {
            avgValues.resize(6);
            avgValues[0] =  get_timesec(selItem->data(4,Qt::DisplayRole).toString());
            avgValues[1] =  get_timesec(selItem->data(6,Qt::DisplayRole).toString());
            avgValues[2] =  selItem->data(3,Qt::DisplayRole).toDouble();
            avgValues[3] =  selItem->data(8,Qt::DisplayRole).toDouble();
            avgValues[4] =  get_timesec(selItem->data(4,Qt::DisplayRole).toString()) + selItem->data(8,Qt::DisplayRole).toDouble();
            avgValues[5] =  get_timesec(selItem->data(4,Qt::DisplayRole).toString()) / selItem->data(8,Qt::DisplayRole).toDouble();
        }
        else
        {
            avgValues.resize(5);
            avgValues[0] =  get_timesec(selItem->data(1,Qt::DisplayRole).toString());
            avgValues[1] =  get_timesec(selItem->data(5,Qt::DisplayRole).toString());
            avgValues[2] =  selItem->data(4,Qt::DisplayRole).toDouble();
            avgValues[3] =  selItem->data(7,Qt::DisplayRole).toDouble();
            avgValues[4] =  selItem->data(8,Qt::DisplayRole).toDouble();
        }

        averageMap.insert(selItem->data(0,Qt::DisplayRole).toString(),avgValues);
    }

    if(averageMap.isEmpty())
    {
        averageData.first = 0;
        averageData.second = avgValues.fill(0);
    }
    else
    {
        avgValues.fill(0);
        avgValues.resize(averageMap.first().count());

        for(QMap<QString,QVector<double>>::const_iterator it = averageMap.cbegin(), end = averageMap.cend(); it != end; ++it)
        {
            for(int i = 0; i < it.value().count(); ++i)
            {
                avgValues[i] = avgValues.at(i) + it.value().at(i);
            }
        }

        int avgCount = averageMap.count();

        for(int i = 0; i < avgValues.count(); ++i)
        {
            avgValues[i] = avgValues.at(i) / avgCount;
        }
        averageData.first = avgCount;
        averageData.second = avgValues;
    }

    return averageData;
}

QPair<double, double> Activity::get_polishMinMax(double avgSpeed)
{
    QPair<double,double> minMax;

    minMax.first = avgSpeed - (avgSpeed * polishFactor);
    minMax.second = avgSpeed + (avgSpeed * polishFactor);

    return minMax;
}

double Activity::polish_SpeedValues(double currSpeed,double avgSpeed,bool setrand)
{
    QPair<double,double> avgLowHigh = get_polishMinMax(avgSpeed);
    double avgLow = avgLowHigh.first;
    double avgHigh = avgLowHigh.second;
    double randfact = 0.0;
    double min = 0.0;
    double max = 1.0;

    std::random_device seeder;
    std::mt19937 gen(seeder());

    if(currSpeed < avgLow)
    {
        min = currSpeed;
        max = avgLow;
    }
    else if(currSpeed > avgHigh)
    {
        min = avgHigh;
        max = currSpeed;
    }

    std::uniform_real_distribution<double> distCurr(min,max);

    if(((currSpeed >= 0.0 && currSpeed <= avgLow) || currSpeed > avgHigh) && setrand)
    {
        currSpeed = distCurr(gen);
    }

    double randNum = static_cast<double>(rand()) / static_cast<double>(RAND_MAX);

    if(isRun)
    {
        randfact = randNum / (currSpeed/((polishFactor*100)+1.0));
    }
    if(isBike)
    {
        randfact = randNum / (currSpeed/((polishFactor*1000)+1.0));
    }
    if(isTria)
    {
        randfact = randNum / (currSpeed/((0.025*100)+1.0));
    }

    if(setrand)
    {
        if(currSpeed < avgLow)
        {
            return avgLow+randfact;
        }
        if(currSpeed > avgHigh)
        {
            return avgHigh-randfact;
        }
        if(currSpeed >= avgLow && currSpeed <= avgHigh)
        {
            return currSpeed;
        }
        return currSpeed + randfact;
    }
    else
    {
        if(currSpeed < avgLow)
        {
            return avgLow;
        }
        if(currSpeed > avgHigh)
        {
            return avgHigh;
        }
        if(currSpeed > avgLow && currSpeed < avgHigh)
        {
            return currSpeed;
        }
    }
    return 0;
}

double Activity::interpolate_speed(int row,int sec,double limit)
{
    double curr_speed = sampleMap.value(sec).at(2);
    double avg_speed = 0;

    if(curr_speed >= 0 && curr_speed < limit)
    {
        curr_speed = limit;
    }

    if(row == 0 && sec < 5)
    {
        return (static_cast<double>(sec) + ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)))) * 1.2;
    }
    else
    {
        if(avg_speed >= limit)
        {
            return this->polish_SpeedValues(curr_speed,avg_speed,true);
        }
        else
        {
            return curr_speed;
        }
    }
}
