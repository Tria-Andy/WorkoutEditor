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

void Activity::clear_loadedActivity()
{
    xDataMap.clear();
    intervallMap.clear();
    sampleMap.clear();
    ActivityMap.clear();
    moveTime = 0;
    ride_info.clear();
}

void Activity::save_actvitiyFile()
{
    QStringList listValues;
    int counter = 0;
    mapList.clear();

    for(QMap<QString,QVector<QString>>::const_iterator it = gcActivtiesMap.cbegin(), end = gcActivtiesMap.cend(); it != end; ++it)
    {
        listValues << it.key();
        for(int value = 0; value < it.value().count(); ++value)
        {
            listValues << it.value().at(value);
        }
        mapList.insert(counter++,listValues);
        listValues.clear();
    }
    this->listMap_toXml(fileMap->value("activityfile"));
}

void Activity::read_gcActivities()
{
    QString jsonFile;
    QString filePath;
    bool newActivity = false;
    int maxFileCount = generalValues->value("filecount").toInt();
    QDir directory(gcValues->value("actpath"));
    directory.setSorting(QDir::Name | QDir::Reversed);
    directory.setFilter(QDir::Files);
    QFileInfoList fileList = directory.entryInfoList();
    maxFileCount = fileList.count() > maxFileCount ? maxFileCount : fileList.count();

    for(int fileCount = 0; fileCount < maxFileCount; ++fileCount)
    {
        filePath = fileList.at(fileCount).path()+QDir::separator()+fileList.at(fileCount).fileName();
        jsonFile = fileList.at(fileCount).fileName();

         if(!gcActivtiesMap.contains(jsonFile))
         {
            gcActivtiesMap.insert(jsonFile,this->read_activityMeta(filePath,gcActivtiesMap.count()));
            newActivity = true;
         }
    }

    if(newActivity) save_actvitiyFile();
}

void Activity::fill_actMap()
{
    QVector<QString> actValues;
    QString jsonFile;
    for(QMap<int,QStringList>::const_iterator it = mapList.cbegin(), end = mapList.cend(); it != end; ++it)
    {
        jsonFile = it.value().at(0);
        actValues.insert(0,it.value().at(1));
        actValues.insert(1,it.value().at(2));
        actValues.insert(2,it.value().at(3));
        actValues.insert(3,it.value().at(4));
        actValues.insert(4,QString::number(it.key()));
        gcActivtiesMap.insert(jsonFile,actValues);
        actValues.clear();
    }
    this->read_gcActivities();
}


bool Activity::read_jsonFile(QString jsonfile)
{
    QFileInfo fileinfo(jsonfile);

    if(fileinfo.suffix() == "json")
    {
        this->set_currentSport(this->read_jsonContent(jsonfile));

        QStringList valueList;
        QString stgValue;
        valueList = settings::get_listValues("JsonFile");

        for(int i = 0; i < valueList.count();++i)
        {
            stgValue = valueList.at(i);
            ride_info.insert(stgValue,tagData.value(stgValue));
        }
        ride_info.insert("Date",rideData.value("STARTTIME"));

        return true;
    }
    return false;
}

QMap<int,QVector<double>> Activity::get_xData(QPair<int,int> intKey)
{
    QMap<int,QVector<double>> xDataValues;


    for(QMap<int,QVector<double>>::const_iterator xdata = xDataMap.lowerBound(intKey.first), end = xDataMap.lowerBound(intKey.second); xdata != end; ++xdata)
    {
        xDataValues.insert(xdata.key(),xdata.value());
    }

    return xDataValues;
}

QMap<QPair<int,QString>,QVector<double>> Activity::get_swimLapData(int intCount,QPair<int, int> intKey)
{
    QMap<QPair<int,QString>,QVector<double>> xDataValues;
    QPair<int,QString> xDataKey;
    int lapCount = 1;
    QString lapLevel;
    int timezone = 0;

    for(QMap<int,QVector<double>>::const_iterator xdata = xDataMap.lowerBound(intKey.first), end = xDataMap.lowerBound(intKey.second); xdata != end; ++xdata)
    {
        if(xdata.value().at(1) != 0)
        {
            lapLevel = this->checkRangeLevel(this->calc_lapPace(xdata.value().at(2),poolLength));
            timezone = paceTimeInZone.value(lapLevel);
            xDataKey.first = lapCount;
            xDataKey.second = QString::number(intCount)+"_"+QString::number((lapCount++)*poolLength)+"_"+lapLevel;
            xDataValues.insert(xDataKey,xdata.value());
            paceTimeInZone.insert(lapLevel,timezone+xdata.value().at(2));
            moveTime = moveTime + xdata.value().at(2);
        }
        else
        {
            timezone = paceTimeInZone.value(levels.at(0));
            xDataKey.first = 0;
            xDataKey.second = breakName;
            xDataValues.insert(xDataKey,xdata.value());
            paceTimeInZone.insert(levels.at(0),timezone+xdata.value().at(2));
        }
    }

    return xDataValues;
}

QMap<QPair<int, QString>, QVector<double> > Activity::get_intervalData(int intCount, QPair<int, int> intKey)
{
    QMap<QPair<int,QString>,QVector<double>> intervalValues;
    QPair<int,QString> intervalKey;
    QVector<double> intervalSum(6,0);

    if(intKey.first < 0) intKey.first = 0;

    intervalSum[0] = intKey.second - intKey.first;
    intervalSum[1] = sampleMap.value(intKey.second).at(0) - sampleMap.value(intKey.first).at(0);

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

void Activity::set_activityHeader(QString headerName,QStringList *headerList)
{
    QStringList *tempList = settings::getHeaderMap(headerName);

    for(int i = 0; i < tempList->count(); ++i)
    {
        headerList->append(tempList->at(i));
    }
}

void Activity::set_activityData()
{
    QPair<int,QString> activityKey;
    QMap<QPair<int,QString>,QVector<double>> actIntervals;
    int intCounter = 1;
    double activityDist = 0;

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
            ActivityMap.insert(activityKey, actIntervals);
        }
        else
        {
            actIntervals = this->get_intervalData(intCounter++,interval.value());
            activityDist = activityDist + (actIntervals.first().at(1));
            activityKey.second = actIntervals.firstKey().second;
            ActivityMap.insert(activityKey,actIntervals);
        }
    }
    ride_info.insert("Distance",QString::number(activityDist));

    if(isSwim) this->swimhfTimeInZone(false);
}

void Activity::prepare_baseData()
{
    usePMData = hasPMData;
    isUpdated = tagData.value("Updated").toInt();
    isTimeBased = tagData.value("Timebased").toInt();
    isIndoor = false;
    averageHeader.clear();
    activityHeader.clear();
    actWeight = settings::get_weightforDate(QDateTime::fromString(ride_info.value("Date"),"yyyy/MM/dd hh:mm:ss UTC").addSecs(QDateTime::currentDateTime().offsetFromUtc()));

    this->set_activityHeader("averagepace",&averageHeader);

    if(isSwim)
    {
        distFactor = 1000;
        poolLength = tagData.value("Pool Length").toDouble();
        breakName = generalValues->value("breakname");
        swimType = settings::get_listValues("SwimStyle");
        hfThreshold = thresValues->value("hfthres");
        hfMax = thresValues->value("hfmax");

        this->fillRangeLevel(thresPace,true);
        this->set_activityHeader("averageswim",&averageHeader);
        this->set_activityHeader("swimheader",&activityHeader);

        QString temp,zone_low,zone_high;
        double zoneLow, zoneHigh;
        int zoneCount = levels.count();

        //Set Swim HFZone low and high
        for(int i = 0; i < levels.count(); i++)
        {
            paceTimeInZone.insert(levels.at(i),0);
            hfTimeInZone.insert(levels.at(i),0);

            temp = settings::get_rangeValue("HF",levels.at(i));
            zone_low = temp.split("-").first();
            zone_high = temp.split("-").last();

            zoneLow = this->get_zone_values(zone_low.toDouble(),static_cast<int>(hfThreshold),false);
            zoneHigh = this->get_zone_values(zone_high.toDouble(),static_cast<int>(hfThreshold),false);

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
        }
        else if(isRun)
        {

        }
        else if(isTria)
        {
            this->set_activityHeader("triaheader",&activityHeader);
        }
        else
        {

        }

        if(hasPMData)
        {
            this->fillRangeLevel(thresPower,false);
            this->set_activityHeader("averagepower",&averageHeader);
            this->set_activityHeader("powerheader",&activityHeader);
        }
        else
        {
            this->fillRangeLevel(thresPace,true);
            this->set_activityHeader("paceheader",&activityHeader);
        }
        activityHeader.move(4,activityHeader.count()-1);
        activityHeader.move(4,activityHeader.count()-1);
    }

    ride_info.insert("Duration",QDateTime::fromTime_t(sampleMap.count()).toUTC().toString("hh:mm:ss"));
}

void Activity::swimhfTimeInZone(bool recalc)
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
    double workoutTime = this->get_timesec(ride_info.value("Duration"));

    qDebug() << hfZoneAvg;
    qDebug() << hfTimeInZone;

    for(QHash<QString,int>::const_iterator it = hfZoneAvg.cbegin(), end = hfZoneAvg.cend(); it != end; ++it)
    {
        hfAvg = hfAvg + ceil((it.value() * hfTimeInZone.value(it.key())) / workoutTime);
    }

    ride_info.insert("AvgHF",QString::number(hfAvg));
    overrideData.insert("average_hr",QString::number(hfAvg));

    double totalCal = this->calc_totalCal(actWeight,hfAvg,moveTime);
    ride_info.insert("Total Cal",QString::number(totalCal));
    overrideData.insert("total_kcalories",QString::number(totalCal));

    int hfZone = 0;

    for(QHash<QString,int>::const_iterator it = hfTimeInZone.cbegin(), end = hfTimeInZone.cend(); it != end; ++it)
    {
        hfZone = levels.indexOf(it.key())+1;
        overrideData.insert("time_in_zone_H" + QString::number(hfZone),QString::number(it.value()));
    }
}

QString Activity::build_lapName(QString lapName,int lapTime, double lapDist)
{
    QString label;

    if(isTimeBased)
    {
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

    return lapName;
}
/*
void Activity::recalcIntTree()
{
    int startTime = 0;
    int intTime = 0;
    int lastStart = 0;
    int lastTime = 0;
    double workDist = 0;
    double totalWork = 0.0;
    double totalCal = 0.0;
    int rowCount = activityModel->rowCount();
    QString lapName,level;
    moveTime = 0;

    if(isSwim)
    {
        int intCounter = 1;
        int lapDist;
        int intPace;

        for(int row = 0; row < rowCount; ++row)
        {
           intTime = this->get_timesec(activityModel->data(activityModel->index(row,4)).toString());
           lapDist = activityModel->data(activityModel->index(row,3)).toDouble();
           intPace = this->get_timesec(activityModel->data(activityModel->index(row,6)).toString());
           workDist = workDist + lapDist;
           totalWork = totalWork + activityModel->data(activityModel->index(row,9)).toDouble();
           lapName = activityModel->data(activityModel->index(row,0)).toString();
           if(!lapName.contains(breakName))
           {
                level = this->checkRangeLevel(intPace);
                lapName = QString::number(intCounter)+intLabel+QString::number(lapDist)+"_"+level;
                ++intCounter;
                moveTime = moveTime + intTime;
           }
           activityModel->setData(activityModel->index(row,0),lapName);
           activityModel->setData(activityModel->index(row,4),this->set_time(intTime));
           activityModel->setData(activityModel->index(row,5),this->set_time(startTime));
           startTime = startTime+intTime;
        }

        this->hasOverride = true;
    }
    else
    {
        double lapDist = 0;
        for(int row = 0; row < rowCount; ++row)
        {
           startTime = this->get_timesec(activityModel->data(activityModel->index(row,2)).toString());
           intTime = this->get_timesec(activityModel->data(activityModel->index(row,1)).toString());
           lapDist = activityModel->data(activityModel->index(row,4)).toDouble();

           workDist = workDist + lapDist;

           if(row > 0)
           {
               lastTime = this->get_timesec(activityModel->data(activityModel->index(row-1,1)).toString());
               lastStart = this->get_timesec(activityModel->data(activityModel->index(row-1,2)).toString());
           }

           if(isBike)
           {
               totalWork = totalWork + activityModel->data(activityModel->index(row,9)).toDouble();
               startTime = lastStart + lastTime;
           }
           else if(isRun)
           {
               if(hasPMData)
               {
                   totalWork = totalWork + activityModel->data(activityModel->index(row,9)).toDouble();
               }
               else
               {
                   totalWork = totalWork + activityModel->data(activityModel->index(row,7)).toDouble();
                   totalCal = ceil((totalWork*4)/4.184);
                   ride_info.insert("Total Cal",QString::number(totalCal));
                   this->hasOverride = true;
               }
           }
           else if(isTria)
           {
               totalWork = totalWork + activityModel->data(activityModel->index(row,8)).toDouble();
               this->hasOverride = true;
           }
           else if(isAlt || isStrength)
           {
               totalWork = activityModel->data(activityModel->index(0,4)).toDouble();
               ride_info.insert("Total Cal",QString::number(ceil(totalWork)));
               workDist = 0;
           }

           activityModel->setData(activityModel->index(row,1),this->set_time(intTime));
           activityModel->setData(activityModel->index(row,2),this->set_time(startTime));
           activityModel->setData(activityModel->index(row,3),this->set_doubleValue(workDist,true));
        }
    }

    ride_info.insert("Total Work",QString::number(ceil(totalWork)));
    overrideData.insert("total_work",QString::number(ceil(totalWork)));
    ride_info.insert("Distance",QString::number(workDist/distFactor));
}
*/

void Activity::fillRangeLevel(double threshold,bool isPace)
{
    QPair<double,double> zoneValues;
    QString temp,zoneLow,zoneHigh;

    for(int i = 0; i < levels.count(); ++i)
    {
        temp = settings::get_rangeValue(currentSport,levels.at(i));
        zoneLow = temp.split("-").first();
        zoneHigh = temp.split("-").last();

        if(isPace)
        {
            zoneValues.first = this->get_zone_values(zoneHigh.toDouble(),threshold,isPace);
            zoneValues.second = this->get_zone_values(zoneLow.toDouble(),threshold,isPace);
        }
        else
        {
            zoneValues.first = this->get_zone_values(zoneLow.toDouble(),threshold,isPace);
            zoneValues.second = this->get_zone_values(zoneHigh.toDouble(),threshold,isPace);
        }
        rangeLevels.insert(levels.at(i),zoneValues);
    }
}

QString Activity::checkRangeLevel(double lapValue)
{
    QString currZone = breakName;

    for(QHash<QString,QPair<double,double>>::const_iterator it = rangeLevels.cbegin(), end = rangeLevels.cend(); it != end; ++it)
    {
        if(lapValue >= it.value().first && lapValue < it.value().second)
        {
            currZone = it.key();
        }

        if(lapValue < rangeLevels.value(levels.last()).second && !hasPMData)
        {
            currZone = levels.last();
        }
    }
    return currZone;
}
/*
void Activity::updateSwimLap()
{
    double oldPace = this->get_timesec(activityModel->data(selItem.value(6)).toString());
    QString oldLevel = this->checkRangeLevel(oldPace);
    int oldTime = this->get_timesec(activityModel->data(selItem.value(4)).toString());

    double newPace = this->get_timesec(selItemModel->data(selItemModel->index(4,0)).toString());
    QString newLevel = this->checkRangeLevel(newPace);
    int newTime = selItemModel->data(selItemModel->index(3,0)).toInt();
    int newStyle = swimType.indexOf(selItemModel->data(selItemModel->index(1,0)).toString());

    int levelTime = 0;

    activityModel->setData(selItem.value(0),selItemModel->data(selItemModel->index(0,0)));
    activityModel->setData(selItem.value(1),selItemModel->data(selItemModel->index(1,0)));
    activityModel->setData(selItem.value(4),this->set_time(selItemModel->data(selItemModel->index(3,0)).toInt()));
    activityModel->setData(selItem.value(6),selItemModel->data(selItemModel->index(4,0)));
    activityModel->setData(selItem.value(7),this->set_doubleValue(selItemModel->data(selItemModel->index(5,0)).toDouble(),true));
    activityModel->setData(selItem.value(8),selItemModel->data(selItemModel->index(6,0)));
    activityModel->setData(selItem.value(9),this->set_doubleValue(this->calc_totalWork(newPace,newTime,newStyle),false));

    if(oldLevel != newLevel)
    {
        levelTime = paceTimeInZone.value(oldLevel);
        paceTimeInZone.insert(oldLevel,levelTime - oldTime);

        levelTime = paceTimeInZone.value(newLevel);
        paceTimeInZone.insert(newLevel,levelTime + newTime);
    }

    levelTime = (oldTime - newTime) + paceTimeInZone.value(breakName);
    paceTimeInZone.insert(breakName,levelTime);
    this->swimhfTimeInZone(true);
}

void Activity::updateSwimInt(QModelIndex parentIndex,QItemSelectionModel *treeSelect)
{
    QVector<double> intValue(6);
    intValue.fill(0);
    QString lapName,level,lastType,currType,intType;
    int swimLap = 0;
    int lapPace;
    int lapSplit = 0;
    int lapTime = 0;

    if(activityModel->itemFromIndex(parentIndex)->hasChildren())
    {
        swimLap = 0;
        lapName = activityModel->data(parentIndex).toString().split("Int").first();
        do
        {
            currType = activityModel->itemFromIndex(parentIndex)->child(swimLap,1)->text();
            lapPace = this->get_timesec(activityModel->itemFromIndex(parentIndex)->child(swimLap,6)->text());
            lapTime = this->get_timesec(activityModel->itemFromIndex(parentIndex)->child(swimLap,4)->text());
            level = this->checkRangeLevel(lapPace);
            activityModel->itemFromIndex(parentIndex)->child(swimLap,0)->setData(lapName+QString::number((swimLap+1)*swimTrack)+"_"+level,Qt::EditRole);
            intValue[0] = intValue[0] + activityModel->itemFromIndex(parentIndex)->child(swimLap,3)->text().toDouble();
            intValue[1] = intValue[1] + lapTime;
            intValue[2] = intValue[2] + lapPace;
            intValue[3] = intValue[3] + activityModel->itemFromIndex(parentIndex)->child(swimLap,7)->text().toDouble();
            intValue[4] = intValue[4] + activityModel->itemFromIndex(parentIndex)->child(swimLap,8)->text().toDouble();
            intValue[5] = intValue[5] + activityModel->itemFromIndex(parentIndex)->child(swimLap,9)->text().toDouble();

            lapSplit = lapSplit + lapTime;
            activityModel->itemFromIndex(parentIndex)->child(swimLap,5)->setData(this->set_time(lapSplit),Qt::EditRole);
            ++swimLap;    

            intType = currType == lastType ? currType : swimType.at(6);;
            lastType = currType;
        }
        while(activityModel->itemFromIndex(parentIndex)->child(swimLap,0) != nullptr);
        intValue[2] = intValue[2] / swimLap;
        intValue[3] = intValue[3] / swimLap;
        if(swimLap == 1) intType = currType;
    }
    treeSelect->select(parentIndex,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    activityModel->setData(treeSelect->selectedRows(0).at(0),lapName+"Int_"+QString::number(intValue[0]));
    activityModel->setData(treeSelect->selectedRows(1).at(0),intType);
    activityModel->setData(treeSelect->selectedRows(2).at(0),swimLap);
    activityModel->setData(treeSelect->selectedRows(3).at(0),intValue[0]);
    activityModel->setData(treeSelect->selectedRows(4).at(0),this->set_time(intValue[1]));
    activityModel->setData(treeSelect->selectedRows(6).at(0),this->set_time(intValue[2]));
    activityModel->setData(treeSelect->selectedRows(7).at(0),this->set_doubleValue(intValue[3],true));
    activityModel->setData(treeSelect->selectedRows(8).at(0),intValue[4]);
    activityModel->setData(treeSelect->selectedRows(9).at(0),this->set_doubleValue(intValue[5],false));

    this->updateSwimBreak(parentIndex,treeSelect,intValue[1]);
    this->recalcIntTree();
}

void Activity::updateSwimBreak(QModelIndex intIndex,QItemSelectionModel *treeSelect,int value)
{
    if(intIndex.sibling(intIndex.row()+1,0).isValid())
    {
        QStandardItem *nextBreak = activityModel->item(intIndex.row()+1);
        int breakStart,breakStop,breakDura;

        breakDura = this->get_timesec(activityModel->data(treeSelect->selectedRows(5).at(0)).toString());
        breakStart = breakDura + value;

        treeSelect->select(activityModel->indexFromItem(nextBreak),QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        breakDura = this->get_timesec(activityModel->data(treeSelect->selectedRows(4).at(0)).toString());
        breakStop = this->get_timesec(activityModel->data(treeSelect->selectedRows(5).at(0)).toString());

        if((breakStop-breakStart)+breakDura >= 0)
        {
            breakDura = breakDura + (breakStop-breakStart);
        }
        else
        {
            breakDura = 0;
        }

        activityModel->setData(treeSelect->selectedRows(4).at(0),this->set_time(breakDura));
        activityModel->setData(treeSelect->selectedRows(5).at(0),this->set_time(breakStart));
        activityModel->setData(treeSelect->selectedRows(9).at(0),this->set_doubleValue(this->calc_totalWork(1,breakDura,0),false));
    }

    treeSelect->clearSelection();
}
*/

void Activity::set_workoutContent(QString content)
{
    this->tagData.insert("Workout Content",content);
}

void Activity::updateSampleModel(int sampRowCount)
{
    newDist.resize(sampRowCount);
    calcSpeed.resize(sampRowCount);
    calcCadence.resize(sampRowCount);
    double msec = 0.0;
    int intStart,intStop = 0,sportpace = 0, currPower = 0;
    double lowLimit = 0.0;
    double swimPace,swimSpeed,swimCycle;
    QString lapName;
    /*
    if(!isSwim && !isTria)
    {
        lowLimit = this->get_speed(QTime::fromString(this->set_time(thresPace),"mm:ss"),0,true);
        lowLimit = lowLimit - (lowLimit*thresLimit);
    }

    if(isSwim)
    {
        sampleModel->setRowCount(sampRowCount);
        double overDist = 0;
        double currDist = 0;
        bool isBreak = false;
        int xdataRowCount = xdataModel->rowCount();

        for(int sLap = 0; sLap < xdataRowCount;++sLap)
        {
            lapName = xdataModel->data(xdataModel->index(sLap,0)).toString();
            intStart = xdataModel->data(xdataModel->index(sLap,1)).toInt();
            swimPace = xdataModel->data(xdataModel->index(sLap,4)).toInt();
            swimSpeed = xdataModel->data(xdataModel->index(sLap,6)).toDouble();
            swimCycle = xdataModel->data(xdataModel->index(sLap,5)).toInt();

            if(sLap == xdataRowCount-1)
            {
                intStop = sampRowCount-1;
                overDist = overDist + (swimTrack / distFactor);
            }
            else
            {
                intStop = xdataModel->data(xdataModel->index(sLap+1,1)).toInt();
                overDist = xdataModel->data(xdataModel->index(sLap+1,2)).toDouble();
            }

            if(lapName.contains(breakName))
            {
                isBreak = true;
                msec = 0;
            }
            else
            {
                isBreak = false;
                msec = (swimTrack / swimPace) / distFactor;
            }

            for(int lapsec = intStart; lapsec <= intStop; ++lapsec)
            {
                if(lapsec == 0)
                {
                    newDist[0] = 0.001;
                }
                else
                {
                    if(lapsec == intStart && isBreak)
                    {
                        newDist[lapsec] = overDist;
                    }
                    else
                    {
                        newDist[lapsec] = currDist + msec;
                    }
                    if(lapsec == intStop)
                    {
                        newDist[intStop] = overDist;
                    }
                }
                currDist = newDist[lapsec];
                calcSpeed[lapsec] = swimSpeed;
                calcCadence[lapsec] = swimCycle;
            }
        }

        for(int row = 0; row < sampRowCount;++row)
        {
            sampleModel->setData(sampleModel->index(row,0,QModelIndex()),row);
            sampleModel->setData(sampleModel->index(row,1,QModelIndex()),newDist[row]);
            sampleModel->setData(sampleModel->index(row,2,QModelIndex()),calcSpeed[row]);
            sampleModel->setData(sampleModel->index(row,3,QModelIndex()),calcCadence[row]);
        }
    }
    else
    {
        int firstLap = 0;
        double firstLapWatt = this->get_int_value(0,4);

        for(int intRow = 0; intRow < intModel->rowCount(); ++intRow)
        {
            firstLap = intRow == 0 ? 0 : 1;
            intStart = intModel->data(intModel->index(intRow,1)).toInt();
            intStop = intModel->data(intModel->index(intRow,2)).toInt();
            msec = intModel->data(intModel->index(intRow,intListCount)).toDouble() / (intStop-intStart+1);

            if(isBike)
            {
                intStart = intStart-firstLap;

                for(int intSec = intStart;intSec < intStop; ++intSec)
                {
                    if(intSec == 0)
                    {
                        newDist[0] = 0.0000;
                    }
                    else
                    {
                        if(isIndoor)
                        {
                            if(usePMData)
                            {
                                calcSpeed[intSec] = this->wattToSpeed(thresPower,sampleModel->data(sampleModel->index(intSec,4,QModelIndex())).toDouble());
                            }
                            else
                            {
                                calcSpeed[intSec] = this->interpolate_speed(intRow,intSec,lowLimit);
                            }
                        }
                        else
                        {
                            calcSpeed[intSec] = sampleModel->data(sampleModel->index(intSec,2)).toDouble();
                        }
                        newDist[intSec] = newDist[intSec-1] + msec;
                        calcCadence[intSec] = sampleModel->data(sampleModel->index(intSec,3,QModelIndex())).toDouble();
                    }
                }
                if(isIndoor)
                {
                    if(usePMData)
                    {
                        calcSpeed[intStop] = this->wattToSpeed(thresPower,sampleModel->data(sampleModel->index(intStop,2,QModelIndex())).toDouble());
                    }
                    else
                    {
                        calcSpeed[intStop] = this->interpolate_speed(intRow,intStop,lowLimit);
                    }
                }
                else
                {
                    calcSpeed[intStop] = sampleModel->data(sampleModel->index(intStop,4,QModelIndex())).toDouble();
                }

                calcCadence[intStop] = sampleModel->data(sampleModel->index(intStop,3,QModelIndex())).toDouble();
            }
            if(isRun)
            {
                for(int intSec = intStart;intSec < intStop; ++intSec)
                {
                    currPower = sampleModel->data(sampleModel->index(intSec,4)).toInt();

                    if(intSec == 0)
                    {
                        newDist[0] = 0.0000;
                    }
                    else
                    {
                        if(intRow == 0 && currPower < firstLapWatt*0.5)
                        {
                            sampleModel->setData(sampleModel->index(intSec,4),round(pow(intSec,2.5)+firstLapWatt*0.5));
                        }

                        calcSpeed[intSec] = this->interpolate_speed(intRow,intSec,lowLimit);
                        newDist[intSec] = newDist[intSec-1] + msec;
                        if(hasPMData) calcCadence[intSec] = sampleModel->data(sampleModel->index(intSec,3,QModelIndex())).toDouble();
                    }

                }
                calcSpeed[intStop] = calcSpeed[intStop-1];
            }
            else if(isTria)
            {
                for(int intSec = intStart;intSec <= intStop; ++intSec)
                {
                    if(intSec == 0)
                    {
                        newDist[0] = 0.0000;
                    }
                    else
                    {
                        newDist[intSec] = newDist[intSec-1] + msec;
                    }

                    if(intRow == 0)
                    {
                        sportpace = thresValues->value("swimpace");
                        lowLimit = this->get_speed(QTime::fromString(this->set_time(sportpace),"mm:ss"),0,true);
                        lowLimit = lowLimit - (lowLimit*thresLimit);
                        calcSpeed[intSec] = this->interpolate_speed(intRow,intSec,lowLimit);
                    }
                    else if(intRow == 4)
                    {
                        sportpace = thresValues->value("runpace");
                        lowLimit = this->get_speed(QTime::fromString(this->set_time(sportpace),"mm:ss"),0,true);
                        lowLimit = lowLimit - (lowLimit*thresLimit);
                        calcSpeed[intSec] = this->interpolate_speed(intRow,intSec,lowLimit);
                    }
                    else
                    {
                        calcSpeed[intSec] = sampleModel->data(sampleModel->index(intSec,2,QModelIndex())).toDouble();
                    }
                    if(intRow == 2) calcCadence[intSec] = sampleModel->data(sampleModel->index(intSec,3,QModelIndex())).toDouble();
                }
            }
            newDist[intStop] = newDist[intStop-1]+msec;
        }
        newDist[sampRowCount-1] = newDist[intStop]+msec;
    }

    if(isBike)
    {
        for(int row = 0; row < sampRowCount;++row)
        {
            sampleModel->setData(sampleModel->index(row,0,QModelIndex()),row);
            sampleModel->setData(sampleModel->index(row,1,QModelIndex()),newDist[row]);
            sampleModel->setData(sampleModel->index(row,2,QModelIndex()),calcSpeed[row]);
            sampleModel->setData(sampleModel->index(row,3,QModelIndex()),calcCadence[row]);
        }
    }

    if(isRun)
    {
        for(int row = 0; row < sampRowCount;++row)
        {
            sampleModel->setData(sampleModel->index(row,0,QModelIndex()),row);
            sampleModel->setData(sampleModel->index(row,1,QModelIndex()),newDist[row]);
            sampleModel->setData(sampleModel->index(row,2,QModelIndex()),calcSpeed[row]);
        }
    }

    if(isTria)
    {
        double triValue = 0,sportValue = 0;

        for(int row = 0; row < sampRowCount;++row)
        {
          sampleModel->setData(sampleModel->index(row,0,QModelIndex()),row);
          sampleModel->setData(sampleModel->index(row,1,QModelIndex()),newDist[row]);
          sampleModel->setData(sampleModel->index(row,2,QModelIndex()),calcSpeed[row]);
          sampleModel->setData(sampleModel->index(row,3,QModelIndex()),calcCadence[row]);
        }
        this->hasOverride = true;
        sportValue = round(this->estimate_stress(settings::SwimLabel,this->get_int_pace(0,settings::SwimLabel)/10,this->get_int_duration(0),hasPMData));
        this->overrideData.insert("swimscore",QString::number(sportValue));
        triValue = triValue + sportValue;
        sportValue = round(this->estimate_stress(settings::BikeLabel,this->get_int_value(2,4),this->get_int_duration(2),hasPMData));
        this->overrideData.insert("skiba_bike_score",QString::number(sportValue));
        triValue = triValue + sportValue;
        sportValue = round(this->estimate_stress(settings::RunLabel,this->get_int_pace(4,settings::RunLabel),this->get_int_duration(4),hasPMData));
        this->overrideData.insert("govss",QString::number(sportValue));
        triValue = triValue + sportValue;
        this->overrideData.insert("triscore",QString::number(triValue));
    }
    */
}

void Activity::writeChangedData()
{
    tagData.insert("Updated","1");
    this->init_jsonFile();

    this->write_jsonFile();
}

int Activity::get_zone_values(double value, int max, bool ispace)
{
    if(ispace)
    {
        return static_cast<int>(ceil(max/(value/100)));
    }
    else
    {
        return static_cast<int>(ceil(max*(value/100)));
    }
}


double Activity::polish_SpeedValues(double currSpeed,double avgSpeed,double factor,bool setrand)
{
    double avgLow = avgSpeed-(avgSpeed*factor);
    double avgHigh = avgSpeed+(avgSpeed*factor);
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

    if(currentSport == settings::RunLabel)
    {
        randfact = randNum / (currSpeed/((factor*100)+1.0));
    }
    if(currentSport == settings::BikeLabel)
    {
        randfact = randNum / (currSpeed/((factor*1000)+1.0));
    }
    if(currentSport == settings::TriaLabel)
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

void Activity::reset_avgSelection()
{
    avgCounter = 0;
    avgValues.fill(0);
    /*
    for(int i = 0; i < avgModel->rowCount();++i)
    {
        avgModel->setData(avgModel->index(i,0),"-");
    }

    for(QHash<int,QModelIndex>::const_iterator it =  avgItems.cbegin(), end = avgItems.cend(); it != end; ++it)
    {
        activityModel->setData(it.value(),"-");
        activityModel->setData(it.value(),0,Qt::UserRole+1);
    }
    avgItems.clear();
    */
}

double Activity::interpolate_speed(int row,int sec,double limit)
{
    double curr_speed = sampSpeed[sec];
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
            return this->polish_SpeedValues(curr_speed,avg_speed,polishFactor,true);
        }
        else
        {
            return curr_speed;
        }
    }
}

void Activity::set_avgValues(int counter,int factor)
{
    avgCounter = counter;

    /*
    if(counter != 0)
    {
        if(isSwim)
        {
            avgValues[1] = avgValues[1] + (static_cast<double>(this->get_timesec(activityModel->data(selItem.value(4)).toString()))*factor);
            avgValues[2] = avgValues.at(2) + (static_cast<double>(this->get_timesec(activityModel->data(selItem.value(6)).toString()))*factor);
            avgValues[3] = avgValues.at(3) + (activityModel->data(selItem.value(3)).toDouble()*factor);
            avgValues[4] = avgValues.at(4) + (activityModel->data(selItem.value(8)).toDouble());
            avgValues[5] = avgValues.at(5) + (this->get_timesec(activityModel->data(selItem.value(4)).toString())+activityModel->data(selItem.value(8)).toInt());
            avgValues[6] = avgValues.at(6) + (avgValues.at(1)/avgValues.at(4));
            avgModel->setData(avgModel->index(4,0),round(avgValues[4]/avgCounter));
            avgModel->setData(avgModel->index(5,0),round(avgValues[5]/avgCounter));
            avgModel->setData(avgModel->index(6,0),this->set_doubleValue(avgValues[6]/avgCounter,false));
        }
        else
        {
            avgValues[1] = avgValues.at(1) + (static_cast<double>(this->get_timesec(activityModel->data(selItem.value(1)).toString()))*factor);
            avgValues[2] = avgValues.at(2) + (static_cast<double>(this->get_timesec(activityModel->data(selItem.value(5)).toString()))*factor);
            avgValues[3] = avgValues.at(3) + (activityModel->data(selItem.value(4)).toDouble()*factor);

            if(hasPMData)
            {
                avgValues[4] = avgValues.at(4) + (activityModel->data(selItem.value(7)).toDouble()*factor);
                avgValues[5] = avgValues.at(5) + (activityModel->data(selItem.value(8)).toDouble()*factor);
                avgModel->setData(avgModel->index(4,0),round(avgValues[4]/avgCounter));
                avgModel->setData(avgModel->index(5,0),round(avgValues[5]/avgCounter));
            }
        }

        avgModel->setData(avgModel->index(0,0),avgCounter);
        avgModel->setData(avgModel->index(1,0),this->set_time(round(avgValues[1]/avgCounter)));
        avgModel->setData(avgModel->index(2,0),this->set_time(round(avgValues[2]/avgCounter)));
        avgModel->setData(avgModel->index(3,0),this->set_doubleValue(avgValues[3]/avgCounter,true));
    }
    else
    {
        this->reset_avgSelection();
    }
    */
}
