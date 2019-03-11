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

Activity::Activity(QString jsonfile,bool intAct)
{
    this->readJsonFile(jsonfile,intAct);
}

void Activity::readJsonFile(QString jsonfile,bool intAct)
{
    QStringList valueList;
    QString stgValue;
    valueList = settings::get_listValues("JsonFile");
    this->hasPMData = false;
    curr_sport = this->readJsonContent(jsonfile);

    for(int i = 0; i < valueList.count();++i)
    {
        stgValue = valueList.at(i);
        ride_info.insert(stgValue,tagData.value(stgValue));
    }
    ride_info.insert("Date",rideData.value("STARTTIME"));

    if(intAct)
    {
        QMap<int,QString> *mapValues;

        intModel = new QStandardItemModel();
        mapValues = settings::getListMapPointer(settings::lMap::Interval);
        this->init_actModel("INTERVALS",mapValues,intModel,&intList,3);
        intList.removeAt(intList.indexOf("COLOR"));
        intList.removeAt(intList.indexOf("PTEST"));
        intListCount = intList.count();

        sampleModel = new QStandardItemModel();
        mapValues = settings::getListMapPointer(settings::lMap::Sample);
        this->init_actModel("SAMPLES",mapValues,sampleModel,&sampList,0);

        if(hasXdata)
        {
            xdataModel = new QStandardItemModel();
            this->init_xdataModel(xdataModel);
        }
        this->prepareData();
    }
}

void Activity::prepareData()
{
    calculation::usePMData = hasPMData;
    isSwim = isBike = isRun = isTria = isStrength = false;
    itemHeader.insert(0,QStringList() << "Name:" << "Type:" << "Distance (M):" << "Duration (Sec):" << "Pace/100m:" << "Speed km/h:" << "Strokes:");
    itemHeader.insert(1,QStringList() << "Name:" << "Start:" << "Distance:" << "Duration:" << "Pace:" << "Speed km/h:");
    itemHeader.insert(2,QStringList() << "Name:" << "Start:" << "Distance:" << "Duration:");
    avgHeader.insert(0,QStringList() << "Intervalls:" << "Duration:" << "Pace:" << "Distance:" << "Strokes:" << "SWOLF:" << "Cycletime:");
    avgHeader.insert(1,QStringList() << "Intervalls:" << "Duration:" << "Pace:" << "Distance:" << "Watts:" << "CAD:");
    avgHeader.insert(2,QStringList() << "Intervalls:" << "Duration:" << "Pace:" << "Distance");
    intTreeModel = new QStandardItemModel();
    rootItem = intTreeModel->invisibleRootItem();
    selItemModel = new QStandardItemModel();
    selItemModel->setColumnCount(2);
    avgModel = new QStandardItemModel();
    avgModel->setColumnCount(1);
    moveTime = 0;
    if(curr_sport == settings::isSwim) isSwim = true;
    else if(curr_sport == settings::isBike) isBike = true;
    else if(curr_sport == settings::isRun) isRun = true;
    else if(curr_sport == settings::isTria) isTria = true;
    else if(curr_sport == settings::isStrength) isStrength = true;
    else if(curr_sport == settings::isAlt) isAlt = true;

    int sampCount = sampleModel->rowCount();
    int intModelCount = intModel->rowCount();
    int intStart,intStop;
    int avgHF = 0;
    int posHF = sampList.indexOf("HR");
    int dist = 0;
    double secondValue;
    isTimeBased = true;
    isIndoor = false;
    isUpdated = tagData.value("Updated").toInt();
    levels = settings::get_listValues("Level");
    breakName = generalValues->value("breakname");
    QString lapName;
    intLabel = "_Int_";
    sampSpeed.resize(sampCount);
    sampSecond.resize(sampCount);

    actWeight = settings::get_weightforDate(QDateTime::fromString(ride_info.value("Date"),"yyyy/MM/dd hh:mm:ss UTC").addSecs(QDateTime::currentDateTime().offsetFromUtc()));

    if(tagData.value("SubSport") == "home trainer") isIndoor = true;

    intModel->setData(intModel->index(intModelCount-1,2,QModelIndex()),sampCount-1);

    ride_info.insert("Distance",QString::number(sampleModel->data(sampleModel->index(sampCount-1,1,QModelIndex())).toDouble()));
    ride_info.insert("Duration",QDateTime::fromTime_t(sampCount).toUTC().toString("hh:mm:ss"));

    if(isSwim)
    {
        distFactor = 1000;
        swimTrack = tagData.value("Pool Length").toDouble();
        thresPace = thresValues->value("swimpace");

        hfThreshold = thresValues->value("hfthres");
        hfMax = thresValues->value("hfmax");

        zoneCount = levels.count();
        int intervallCount = 1;

        swimType = settings::get_listValues("SwimStyle");

        //Read current CV and HF Threshold
        this->fillRangeLevel(thresPace,true);

        QString temp,zone_low,zone_high;
        double zoneLow, zoneHigh;

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
        if(hasXdata)
        {
            QList<QStandardItem*> intItems,subItems;
            QMap<int,QList<QStandardItem*> > lapItems;
            int intTime = 0;
            int intType = 0;
            int intStrokes = 0;
            double intWork = 0;
            int intPace = 0;
            int strokeCount = 0;
            int currType = 0,lastType = 0;
            int lapPace = 0;
            int lapTime = 0;
            double lapSpeed = 0;
            double lapWork = 0;
            QString level;

            for(int row = 0; row < intModelCount; ++row)
            {
                intTime = 0;
                intStart = intModel->data(intModel->index(row,1)).toInt();
                intStop = intModel->data(intModel->index(row,2)).toInt();

                int xdataRow = xdataModel->findItems(QString::number(intStart),Qt::MatchExactly,0).at(0)->row();

                int xdataLap = 0;
                int laps = 1;

                while(xdataModel->data(xdataModel->index(xdataRow+xdataLap,0)).toInt() < intStop && xdataModel->hasIndex(xdataRow+xdataLap,0))
                {
                    lapTime = round(xdataModel->data(xdataModel->index(xdataRow+xdataLap,3)).toDouble());
                    intTime = intTime + lapTime;

                    if(!xdataModel->data(xdataModel->index(xdataRow+xdataLap,2)).toBool())
                    {
                        lapName = breakName;
                    }
                    else
                    {
                        lapName = intLabel;
                        currType = xdataModel->data(xdataModel->index(xdataRow+xdataLap,2)).toInt();
                        strokeCount = xdataModel->data(xdataModel->index(xdataRow+xdataLap,4)).toInt();
                        intStrokes = intStrokes + strokeCount;
                        lapSpeed = this->calc_Speed(lapTime,swimTrack,distFactor);
                        lapPace = lapTime * (100.0/swimTrack);
                        level = this->checkRangeLevel(lapPace);
                        lapWork = this->calc_totalWork(curr_sport,lapPace,lapTime,currType);
                        intWork = intWork + lapWork;
                        moveTime = moveTime + lapTime;


                        subItems << new QStandardItem(QString::number(intervallCount)+"_"+QString::number(laps*swimTrack)+"_"+level);
                        subItems << new QStandardItem(swimType.at(currType));
                        subItems << new QStandardItem("-");
                        subItems << new QStandardItem(QString::number(swimTrack));
                        subItems << new QStandardItem(this->set_time(lapTime));
                        subItems << new QStandardItem("-");
                        subItems << new QStandardItem(this->set_time(lapPace));
                        subItems << new QStandardItem(QString::number(this->set_doubleValue(lapSpeed,true)));
                        subItems << new QStandardItem(QString::number(strokeCount));
                        subItems << new QStandardItem(QString::number(this->set_doubleValue(lapWork,false)));

                        lapItems.insert(laps,subItems);
                        subItems.clear();
                        ++laps;
                        intType = currType == lastType ? currType : 6;
                        lastType = currType;
                    }
                    ++xdataLap;
                }
                --laps;
                dist = laps*swimTrack;
                intModel->setData(intModel->index(row,intListCount),dist);
                intModel->setData(intModel->index(row,intListCount+1),laps);

                if(laps == 1) intType = currType;

                intPace = this->get_int_pace(row,lapName);

                if(laps > 0)
                {
                    level = this->checkRangeLevel(intPace);
                    intItems << new QStandardItem(QString::number(intervallCount)+intLabel+QString::number(dist));
                    intItems << new QStandardItem(swimType.at(intType));
                    intItems << new QStandardItem(QString::number(laps));
                    intItems << new QStandardItem(QString::number(dist));
                    intItems << new QStandardItem(this->set_time(intTime));
                    intItems << new QStandardItem(this->set_time(intStart));
                    intItems << new QStandardItem(this->set_time(intPace));
                    intItems << new QStandardItem(QString::number(this->set_doubleValue(this->get_int_speed(row),true)));
                    intItems << new QStandardItem(QString::number(intStrokes));
                    intItems << new QStandardItem(QString::number(this->set_doubleValue(intWork,false)));
                    intItems << new QStandardItem("-");
                    ++intervallCount;
                }
                else
                {
                    currType = 0;
                    intItems << new QStandardItem(lapName);
                    intItems << new QStandardItem(swimType.at(intType));
                    intItems << new QStandardItem(QString::number(laps));
                    intItems << new QStandardItem(QString::number(dist));
                    intItems << new QStandardItem(this->set_time(intTime));
                    intItems << new QStandardItem(this->set_time(intStart));
                    intItems << new QStandardItem(this->set_time(intPace));
                    intItems << new QStandardItem(QString::number(this->set_doubleValue(this->get_int_speed(row),true)));
                    intItems << new QStandardItem(QString::number(intStrokes));
                    intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(curr_sport,intPace,intTime,currType),false)));
                    intItems << new QStandardItem("-");
                }

                for(int i = 1; i <= lapItems.count(); ++i)
                {
                    intItems.first()->appendRow(lapItems.value(i));
                }

                lapItems.clear();
                rootItem->appendRow(intItems);
                intItems.clear();

                this->swimTimeInZone(intTime,intPace);

                intTime = 0;
                intStrokes = 0;
                intWork = 0;
                intType = 0;
            }
            xdataModel->clear();
        }
        else
        {
            intModel->setData(intModel->index(0,intListCount),overrideData.value("total_distance"));
            intModel->setData(intModel->index(0,intListCount+1),1);
            intModel->setData(intModel->index(0,intListCount+2),"Workout_1");
        }

        for(int i = 0; i < sampCount; ++i)
        {
            secondValue = sampleModel->data(sampleModel->index(i,3,QModelIndex())).toDouble();
            sampSpeed[i] = sampleModel->data(sampleModel->index(i,2,QModelIndex())).toDouble();
            sampSecond[i] = secondValue;
        }

        selItemModel->setVerticalHeaderLabels(itemHeader.value(0));
        avgValues.resize(7);
        avgModel->setVerticalHeaderLabels(avgHeader.value(0));

        this->swimhfTimeInZone(false);
    }
    else
    {
        distFactor = 1;
        polishFactor = 0.1;

        if(isBike)
        {
            thresPower = thresValues->value("bikepower");
            thresPace = thresValues->value("bikepace");
            thresSpeed = thresValues->value("bikespeed");
            thresLimit = thresValues->value("bikelimit");
            if(hasPMData)
            {
                this->fillRangeLevel(thresPower,false);
            }
            else
            {
                this->fillRangeLevel(thresPace,true);
            }
            isTimeBased = true;
            avgValues.resize(avgHeader.value(hasPMData).count());
            avgModel->setVerticalHeaderLabels(avgHeader.value(hasPMData));
            selItemModel->setVerticalHeaderLabels(itemHeader.value(1));
        }
        else if(isRun)
        {
            thresPower = thresValues->value("runpower");
            thresPace = thresValues->value("runpace");
            thresLimit = thresValues->value("runlimit");

            this->fillRangeLevel(thresPace,true);

            isTimeBased = false;
            avgValues.resize(avgHeader.value(hasPMData).count());
            avgModel->setVerticalHeaderLabels(avgHeader.value(hasPMData));
            selItemModel->setVerticalHeaderLabels(itemHeader.value(1));
        }
        else if(isTria)
        {
            isTimeBased = false;
        }
        else
        {
            isTimeBased = true;
            avgValues.resize(avgHeader.value(hasPMData).count());
            avgModel->setVerticalHeaderLabels(avgHeader.value(0));
            selItemModel->setVerticalHeaderLabels(itemHeader.value(0));
        }

        for(int row = 0; row < intModelCount; ++row)
        {
            intModel->setData(intModel->index(row,intListCount),this->get_int_distance(row,0));
            intModel->setData(intModel->index(row,intListCount+1),1);
            intModel->setData(intModel->index(row,intListCount+2),"Int-"+QString::number(row));
        }

        for(int i = 0; i < sampCount; ++i)
        {
            if(isIndoor)
            {
                secondValue = sampleModel->data(sampleModel->index(i,4,QModelIndex())).toDouble();
                sampSpeed[i] = this->wattToSpeed(thresPower,thresSpeed,secondValue);
                sampSecond[i] = secondValue;
                avgHF = avgHF + sampleModel->data(sampleModel->index(i,posHF,QModelIndex())).toDouble();
            }
            else
            {
                sampSpeed[i] = sampleModel->data(sampleModel->index(i,2,QModelIndex())).toDouble();

                if(hasPMData)
                {
                    sampSecond[i] = sampleModel->data(sampleModel->index(i,4,QModelIndex())).toDouble();
                    avgHF = avgHF + sampleModel->data(sampleModel->index(i,posHF,QModelIndex())).toDouble();
                }
                else
                {
                    secondValue = sampleModel->data(sampleModel->index(i,posHF,QModelIndex())).toDouble();
                    sampSecond[i] = secondValue;
                    avgHF = avgHF + static_cast<int>(secondValue);
                }
            }
        }

        avgHF = (avgHF / sampCount);
        double totalCal = ceil(this->calc_totalCal(actWeight,avgHF,sampCount));
        ride_info.insert("Total Cal",QString::number(totalCal));
        ride_info.insert("AvgHF",QString::number(avgHF));
    }

    avgModel->setRowCount(avgValues.count());

    this->reset_avgSelection();

    intTreeModel->setHorizontalHeaderLabels(settings::get_int_header(curr_sport,hasPMData));
    if(curr_sport != settings::isSwim) this->build_intTree();
    this->recalcIntTree();
}

void Activity::build_intTree()
{
    QList<QStandardItem*> intItems;
    QModelIndex data_index;

    if(isBike || isRun)
    {
        for(int i = 0; i < intModel->rowCount(); ++i)
        {
            rootItem->appendRow(setIntRow(i));
        }
    }
    else if(isTria)
    {
        for(int i = 0; i < intModel->rowCount(); ++i)
        {
            rootItem->appendRow(setIntRow(i));
        }
    }
    else if(isAlt || isStrength)
    {
        QString lapName = intModel->data(intModel->index(0,0)).toString();
        int worktime = sampleModel->rowCount();
        double pValue = tagData.value("CommonRI").toDouble();

        intItems << new QStandardItem(lapName);
        intItems << new QStandardItem(this->set_time(worktime));
        intItems << new QStandardItem(this->set_time(intModel->data(intModel->index(0,1)).toInt()));
        intItems << new QStandardItem(QString::number(this->set_doubleValue(sampleModel->data(data_index).toDouble(),true)));
        intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(curr_sport,pValue,worktime,0),false)));
        intItems << new QStandardItem("-");

        rootItem->appendRow(intItems);
        intItems.clear();
    }
    else
    {
        for(int i = 0; i < intModel->rowCount(); ++i)
        {
            data_index = sampleModel->index(intModel->data(intModel->index(i,2)).toInt()-1,1);
            QString lapName = intModel->data(intModel->index(i,0)).toString();
            int lapTime = this->get_int_duration(i);

            intItems << new QStandardItem(lapName);
            intItems << new QStandardItem(this->set_time(lapTime));
            intItems << new QStandardItem(this->set_time(intModel->data(intModel->index(i,1)).toInt()));
            intItems << new QStandardItem(QString::number(this->set_doubleValue(sampleModel->data(data_index).toDouble(),true)));
            intItems << new QStandardItem("-");
            intItems << new QStandardItem("-");

            rootItem->appendRow(intItems);
            intItems.clear();
        }
    }
    intTreeModel->setHorizontalHeaderLabels(settings::get_int_header(curr_sport,hasPMData));
    this->recalcIntTree();
}

QList<QStandardItem *> Activity::setIntRow(int pInt)
{
    QList<QStandardItem*> intItems;
    QString lapName = intModel->data(intModel->index(pInt,0)).toString();
    int lapPace = this->get_int_pace(pInt,lapName);
    int lapTime = this->get_int_duration(pInt);
    double lapDist = this->set_doubleValue(this->get_int_distance(pInt,1),true);
    double lapSpeed = this->get_int_speed(pInt);
    double workDist = 0;

    if(pInt == 0)
    {
        workDist = intModel->data(intModel->index(pInt,intListCount+1)).toDouble();
    }
    else
    {
        for(int i = 0; i <= pInt; ++i)
        {
            workDist = workDist + intModel->data(intModel->index(i,intListCount)).toDouble();
        }
    }

    intItems << new QStandardItem(lapName);
    intItems << new QStandardItem(this->set_time(lapTime));
    intItems << new QStandardItem(this->set_time(intModel->data(intModel->index(pInt,1)).toInt()));
    intItems << new QStandardItem(QString::number(this->set_doubleValue(workDist,true)));
    intItems << new QStandardItem(QString::number(lapDist));
    intItems << new QStandardItem(this->set_time(lapPace));
    intItems << new QStandardItem(QString::number(this->set_doubleValue(lapSpeed,false)));

    if(isBike || isRun)
    {
        if(hasPMData)
        {
            double watts = round(this->get_int_value(pInt,4));

            intItems << new QStandardItem(QString::number(watts));
            intItems << new QStandardItem(QString::number(round(this->get_int_value(pInt,3))));
            if(isBike)
            {
                intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(curr_sport,watts,lapTime,0),false)));
                lapName = QString::number(pInt+1)+"_"+this->checkRangeLevel(watts);
            }
            else
            {           
                intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(curr_sport,lapSpeed,lapTime,0),false)));
                lapName = QString::number(pInt+1)+"_"+this->checkRangeLevel(lapPace);
            }


            if(isIndoor)
            {
                double wattSpeed = this->wattToSpeed(thresPower,thresSpeed,watts);             
                intItems.at(1)->setData(this->set_time(lapTime),Qt::EditRole);
                intItems.at(4)->setData(QString::number(this->calc_distance(this->set_time(lapTime),3600.0/wattSpeed)),Qt::EditRole);
                intItems.at(6)->setData(QString::number(wattSpeed),Qt::EditRole);
            }
        }
        else
        {
            lapName = QString::number(pInt+1)+"_"+this->checkRangeLevel(lapPace);
            intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(curr_sport,lapSpeed,lapTime,0),false)));
        }

    }
    else if(isTria)
    {
        if(pInt == 0)
        {
            lapName = settings::isSwim;
            intItems << new QStandardItem("-");
            intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(lapName,lapPace,lapTime,8),false)));
        }
        else if(pInt == 1)
        {
            lapName = "T1";
            intItems << new QStandardItem("-");
            intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(settings::isRun,lapSpeed,lapTime,0),false)));
        }
        else if(pInt == 2)
        {
            lapName = settings::isBike;
            double watts = round(this->get_int_value(pInt,intListCount+1));
            intItems << new QStandardItem(QString::number(watts));
            intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(lapName,watts,lapTime,0),false)));
        }
        else if(pInt == 3)
        {
            lapName = "T2";
            intItems << new QStandardItem("-");
            intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(settings::isRun,lapSpeed,lapTime,0),false)));
        }
        else if(pInt == 4)
        {
            lapName = settings::isRun;
            intItems << new QStandardItem("-");
            intItems << new QStandardItem(QString::number(this->set_doubleValue(this->calc_totalWork(lapName,lapSpeed,lapTime,0),false)));
        }
    }

    lapName = this->build_lapName(lapName,lapTime,lapDist);

    intItems.at(0)->setData(lapName,Qt::EditRole);
    intItems << new QStandardItem("-");

    return intItems;
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

void Activity::recalcIntTree()
{
    int startTime = 0;
    int intTime = 0;
    int lastStart = 0;
    int lastTime = 0;
    double workDist = 0;
    double totalWork = 0.0;
    double totalCal = 0.0;
    int rowCount = intTreeModel->rowCount();
    QString lapName,level;
    moveTime = 0;

    if(isSwim)
    {
        int intCounter = 1;
        int lapDist;
        int intPace;

        for(int row = 0; row < rowCount; ++row)
        {
           intTime = this->get_timesec(intTreeModel->data(intTreeModel->index(row,4)).toString());
           lapDist = intTreeModel->data(intTreeModel->index(row,3)).toDouble();
           intPace = this->get_timesec(intTreeModel->data(intTreeModel->index(row,6)).toString());
           workDist = workDist + lapDist;
           totalWork = totalWork + intTreeModel->data(intTreeModel->index(row,9)).toDouble();
           lapName = intTreeModel->data(intTreeModel->index(row,0)).toString();
           if(!lapName.contains(breakName))
           {
                level = this->checkRangeLevel(intPace);
                lapName = QString::number(intCounter)+intLabel+QString::number(lapDist)+"_"+level;
                ++intCounter;
                moveTime = moveTime + intTime;
           }
           intTreeModel->setData(intTreeModel->index(row,0),lapName);
           intTreeModel->setData(intTreeModel->index(row,4),this->set_time(intTime));
           intTreeModel->setData(intTreeModel->index(row,5),this->set_time(startTime));
           startTime = startTime+intTime;
        }

        this->hasOverride = true;
    }
    else
    {
        double lapDist = 0;
        for(int row = 0; row < rowCount; ++row)
        {
           startTime = this->get_timesec(intTreeModel->data(intTreeModel->index(row,2)).toString());
           intTime = this->get_timesec(intTreeModel->data(intTreeModel->index(row,1)).toString());
           lapDist = intTreeModel->data(intTreeModel->index(row,4)).toDouble();

           workDist = workDist + lapDist;

           if(row > 0)
           {
               lastTime = this->get_timesec(intTreeModel->data(intTreeModel->index(row-1,1)).toString());
               lastStart = this->get_timesec(intTreeModel->data(intTreeModel->index(row-1,2)).toString());
           }

           if(isBike)
           {
               totalWork = totalWork + intTreeModel->data(intTreeModel->index(row,9)).toDouble();
               startTime = lastStart + lastTime;
           }
           else if(isRun)
           {
               if(hasPMData)
               {
                   totalWork = totalWork + intTreeModel->data(intTreeModel->index(row,9)).toDouble();
               }
               else
               {
                   totalWork = totalWork + intTreeModel->data(intTreeModel->index(row,7)).toDouble();
                   totalCal = ceil((totalWork*4)/4.184);
                   ride_info.insert("Total Cal",QString::number(totalCal));
                   this->hasOverride = true;
               }
           }
           else if(isTria)
           {
               totalWork = totalWork + intTreeModel->data(intTreeModel->index(row,8)).toDouble();
               this->hasOverride = true;
           }
           else if(isAlt || isStrength)
           {
               totalWork = intTreeModel->data(intTreeModel->index(0,4)).toDouble();
               ride_info.insert("Total Cal",QString::number(ceil(totalWork)));
               workDist = 0;
           }

           intTreeModel->setData(intTreeModel->index(row,1),this->set_time(intTime));
           intTreeModel->setData(intTreeModel->index(row,2),this->set_time(startTime));
           intTreeModel->setData(intTreeModel->index(row,3),this->set_doubleValue(workDist,true));
        }
    }

    ride_info.insert("Total Work",QString::number(ceil(totalWork)));
    overrideData.insert("total_work",QString::number(ceil(totalWork)));
    ride_info.insert("Distance",QString::number(workDist/distFactor));
}

void Activity::swimTimeInZone(int lapTime,double lapPace)
{
    int timeZone = 0;
    QString currZone = checkRangeLevel(lapPace);

    timeZone = paceTimeInZone.value(currZone) + lapTime;
    paceTimeInZone.insert(currZone,timeZone);
}

void Activity::fillRangeLevel(double threshold,bool isPace)
{
    QPair<double,double> zoneValues;
    QString temp,zoneLow,zoneHigh;

    for(int i = 0; i < levels.count(); ++i)
    {
        temp = settings::get_rangeValue(curr_sport,levels.at(i));
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

void Activity::set_selectedItem(QItemSelectionModel *treeSelect)
{
    selItem.clear();
    for(int i = 0; i < treeSelect->selectedIndexes().count();++i)
    {
        selItem.insert(i,treeSelect->selectedRows(i).at(0));
    }
}
void Activity::set_editRow(QString lapIdent,bool isInt)
{
    editRow.clear();

    if(isSwim)
    {
        if(isInt && lapIdent.contains(breakName))
        {
           editRow << 1 << 1 << 1 << 0 << 0 << 0;
        }
        else
        {
            if(isInt)
            {
                editRow << 1 << 1 << 0 << 0 << 0 << 0;
            }
            else
            {
                editRow << 1 << 1 << 1 << 1 << 0 << 0 << 1;
            }
        }
    }
    else
    {
        if(isInt) editRow << 1 << 1 << 1 << 1 << 0 << 0;
    }
}

void Activity::showSwimLap(bool isInt)
{
    selItemModel->setVerticalHeaderLabels(itemHeader.value(isInt));
    selectInt = isInt;
    int rowCount = 0;

    if(isInt)
    {
        int intStart,dura;
        double dist;
        rowCount = 6;
        selItemModel->setRowCount(rowCount);
        intStart = this->get_timesec(intTreeModel->data(selItem.value(5)).toString());
        dura = this->get_timesec(intTreeModel->data(selItem.value(4)).toString());
        dist = intTreeModel->data(selItem.value(3)).toDouble();

        selItemModel->setData(selItemModel->index(0,0),intTreeModel->data(selItem.value(0)).toString());
        selItemModel->setData(selItemModel->index(1,0),intStart);
        selItemModel->setData(selItemModel->index(2,0),dist);
        selItemModel->setData(selItemModel->index(3,0),intTreeModel->data(selItem.value(4)).toString());
        selItemModel->setData(selItemModel->index(4,0),this->set_time(this->calc_lapPace(curr_sport,dura,dist)));
        selItemModel->setData(selItemModel->index(5,0),intTreeModel->data(selItem.value(7)).toDouble());
    }
    else
    {
        rowCount = 7;
        selItemModel->setRowCount(rowCount);
        selItemModel->setData(selItemModel->index(0,0), intTreeModel->data(selItem.value(0)).toString());
        selItemModel->setData(selItemModel->index(1,0), intTreeModel->data(selItem.value(1)).toString());
        selItemModel->setData(selItemModel->index(2,0), intTreeModel->data(selItem.value(3)).toString());
        selItemModel->setData(selItemModel->index(3,0), this->get_timesec(intTreeModel->data(selItem.value(4)).toString()));
        selItemModel->setData(selItemModel->index(4,0), intTreeModel->data(selItem.value(6)).toString());
        selItemModel->setData(selItemModel->index(5,0), intTreeModel->data(selItem.value(7)).toDouble());
        selItemModel->setData(selItemModel->index(6,0), intTreeModel->data(selItem.value(8)).toString());
    }

    for(int i = 0; i < editRow.count(); ++i)
    {
        selItemModel->setData(selItemModel->index(i,1),editRow[i]);
    }
}

void Activity::showInterval(bool isInt)
{
    selItemModel->setVerticalHeaderLabels(itemHeader.value(isInt));
    selectInt = isInt;
    int rowCount = 6;
    selItemModel->setRowCount(rowCount);
    selItemModel->setData(selItemModel->index(0,0),intTreeModel->data(selItem.value(0)).toString());
    selItemModel->setData(selItemModel->index(1,0),this->get_timesec(intTreeModel->data(selItem.value(2)).toString()));
    selItemModel->setData(selItemModel->index(2,0),intTreeModel->data(selItem.value(4)).toDouble());
    selItemModel->setData(selItemModel->index(3,0),intTreeModel->data(selItem.value(1)).toString());
    selItemModel->setData(selItemModel->index(4,0),intTreeModel->data(selItem.value(5)).toString());
    selItemModel->setData(selItemModel->index(5,0),intTreeModel->data(selItem.value(6)).toString());

    for(int i = 0; i < editRow.count(); ++i)
    {
        selItemModel->setData(selItemModel->index(i,1),editRow[i]);
    }
}

void Activity::updateRow_intTree(QItemSelectionModel *treeSelect)
{
    if(intTreeModel->itemFromIndex(selItem.value(0))->parent() != nullptr)
    {
        this->updateSwimLap();
        this->updateSwimInt(intTreeModel->itemFromIndex(selItem.value(0))->parent()->index(),treeSelect);
    }
    else
    {
        if(selectInt)
        {
            this->updateInterval();
        }
        else
        {
            this->updateSwimLap();
            this->updateSwimBreak(selItem.value(0),treeSelect,selItemModel->data(selItemModel->index(3,0)).toInt());
        }
    }
}

void Activity::addRow_intTree(QItemSelectionModel *treeSelect)
{

    if(intTreeModel->itemFromIndex(selItem.value(0))->parent() != nullptr)
    {
        QStandardItem *parentItem = intTreeModel->itemFromIndex(selItem.value(0))->parent();
        QList<QStandardItem*> subItems;

        if(selItem.value(0).isValid())
        {
            subItems << new QStandardItem(selItem.value(0).data().toString());
            subItems << new QStandardItem(selItem.value(1).data().toString());
            subItems << new QStandardItem("-");
            subItems << new QStandardItem(QString::number(swimTrack));
            subItems << new QStandardItem("00:01");
            subItems << new QStandardItem("-");
            subItems << new QStandardItem("00:00");
            subItems << new QStandardItem(QString::number(0.0));
            subItems << new QStandardItem(QString::number(1));

            parentItem->insertRow(treeSelect->currentIndex().row(),subItems);
        }
    }
    else
    {
        QList<QStandardItem*> intItems;
        int intStart = this->get_timesec(intTreeModel->data(intTreeModel->index(treeSelect->currentIndex().row(),4)).toString())+this->get_timesec(intTreeModel->data(intTreeModel->index(treeSelect->currentIndex().row(),5)).toString())+1;

        intItems << new QStandardItem("New_Lap");
        intItems << new QStandardItem(swimType.at(0));
        intItems << new QStandardItem("0");
        intItems << new QStandardItem("0");
        intItems << new QStandardItem("00:01");
        intItems << new QStandardItem(this->set_time(intStart));
        intItems << new QStandardItem("00:00");
        intItems << new QStandardItem(QString::number(0.0));

        intTreeModel->invisibleRootItem()->insertRow(treeSelect->currentIndex().row(),intItems);
    }
}

void Activity::removeRow_intTree(QItemSelectionModel *treeSelect)
{
    if(selectInt)
    {
        if(selItem.value(0).isValid())
        {
            QModelIndex index = treeSelect->currentIndex();
            intTreeModel->removeRow(index.row(),index.parent());
        }
        this->recalcIntTree();
    }
    else
    {
        if(intTreeModel->itemFromIndex(selItem.value(0))->parent() != nullptr)
        {
            int delDura = this->get_timesec(treeSelect->selectedRows(4).at(0).data().toString());
            int strokes = treeSelect->selectedRows(8).at(0).data().toInt();

            if(selItem.value(0).isValid())
            {
                QModelIndex index = treeSelect->currentIndex();
                intTreeModel->removeRow(index.row(),index.parent());
            }

            delDura = delDura + this->get_timesec(treeSelect->selectedRows(4).at(0).data().toString());
            strokes = strokes + treeSelect->selectedRows(8).at(0).data().toInt();

            intTreeModel->setData(treeSelect->selectedRows(4).at(0),this->set_time(delDura));
            intTreeModel->setData(treeSelect->selectedRows(6).at(0),this->set_time(this->calc_lapPace(curr_sport,delDura,swimTrack)));
            intTreeModel->setData(treeSelect->selectedRows(7).at(0),this->set_doubleValue(this->calc_Speed(delDura,swimTrack,distFactor),true));
            intTreeModel->setData(treeSelect->selectedRows(8).at(0),strokes);

            this->updateSwimInt(intTreeModel->itemFromIndex(treeSelect->selectedRows(0).at(0))->parent()->index(),treeSelect);
        }
        else
        {
            QModelIndex index = treeSelect->currentIndex();
            intTreeModel->removeRow(index.row(),index.parent());
            this->recalcIntTree();
        }
    }
}

void Activity::updateInterval()
{
    if(isSwim)
    {
        //Only SwimBreak
        QString lapName = swimType.at(0);
        intTreeModel->setData(selItem.value(0),selItemModel->data(selItemModel->index(0,0)));
        intTreeModel->setData(selItem.value(1),lapName);
        intTreeModel->setData(selItem.value(2),0);
        intTreeModel->setData(selItem.value(3),selItemModel->data(selItemModel->index(2,0)));
        intTreeModel->setData(selItem.value(4),selItemModel->data(selItemModel->index(3,0)));
        intTreeModel->setData(selItem.value(5),selItemModel->data(selItemModel->index(1,0)));
        intTreeModel->setData(selItem.value(6),selItemModel->data(selItemModel->index(4,0)));
        intTreeModel->setData(selItem.value(7),0);
        intTreeModel->setData(selItem.value(8),"");
    }
    else
    {
        QString lapName = selItemModel->data(selItemModel->index(0,0)).toString().split("-").first();
        double lapDist = selItemModel->data(selItemModel->index(2,0)).toDouble();
        int lapTime = this->get_timesec(selItemModel->data(selItemModel->index(3,0)).toString());
        double lapSpeed = selItemModel->data(selItemModel->index(5,0)).toDouble();
        lapName = this->build_lapName(lapName,lapTime,lapDist);
        selItemModel->setData(selItemModel->index(0,0),lapName);

        intTreeModel->setData(selItem.value(0),lapName);
        intTreeModel->setData(selItem.value(1),selItemModel->data(selItemModel->index(3,0)));
        intTreeModel->setData(selItem.value(2),this->set_time(selItemModel->data(selItemModel->index(1,0)).toInt()));
        intTreeModel->setData(selItem.value(4),selItemModel->data(selItemModel->index(2,0)));
        intTreeModel->setData(selItem.value(5),selItemModel->data(selItemModel->index(4,0)));
        intTreeModel->setData(selItem.value(6),this->set_doubleValue(lapSpeed,false));

        if(isRun && !hasPMData)
        {
            intTreeModel->setData(selItem.value(7),this->set_doubleValue(this->calc_totalWork(curr_sport,lapSpeed,lapTime,0),false));
        }
        if(isTria)
        {
            QString pName = selItemModel->data(selItemModel->index(0,0)).toString().split("-").first();

            if(pName == settings::isSwim)
            {
                intTreeModel->setData(selItem.value(8),this->set_doubleValue(this->calc_totalWork(settings::isSwim,lapSpeed,lapTime,0),false));
            }
            else if(pName == "T1" || pName == "T2")
            {
                intTreeModel->setData(selItem.value(8),this->set_doubleValue(this->calc_totalWork(settings::isRun,lapSpeed,lapTime,0),false));
            }
            else if(pName == settings::isBike)
            {
                double watts = intTreeModel->data(selItem.value(7)).toDouble();
                intTreeModel->setData(selItem.value(8),this->set_doubleValue(this->calc_totalWork(settings::isBike,watts,lapTime,0),false));
            }
            else if(pName == settings::isRun)
            {
                intTreeModel->setData(selItem.value(8),this->set_doubleValue(this->calc_totalWork(settings::isRun,lapSpeed,lapTime,0),false));
            }
        }
    }
    this->recalcIntTree();
}

void Activity::updateSwimLap()
{
    double oldPace = this->get_timesec(intTreeModel->data(selItem.value(6)).toString());
    QString oldLevel = this->checkRangeLevel(oldPace);
    int oldTime = this->get_timesec(intTreeModel->data(selItem.value(4)).toString());

    double newPace = this->get_timesec(selItemModel->data(selItemModel->index(4,0)).toString());
    QString newLevel = this->checkRangeLevel(newPace);
    int newTime = selItemModel->data(selItemModel->index(3,0)).toInt();
    int newStyle = swimType.indexOf(selItemModel->data(selItemModel->index(1,0)).toString());

    int levelTime = 0;

    intTreeModel->setData(selItem.value(0),selItemModel->data(selItemModel->index(0,0)));
    intTreeModel->setData(selItem.value(1),selItemModel->data(selItemModel->index(1,0)));
    intTreeModel->setData(selItem.value(4),this->set_time(selItemModel->data(selItemModel->index(3,0)).toInt()));
    intTreeModel->setData(selItem.value(6),selItemModel->data(selItemModel->index(4,0)));
    intTreeModel->setData(selItem.value(7),this->set_doubleValue(selItemModel->data(selItemModel->index(5,0)).toDouble(),true));
    intTreeModel->setData(selItem.value(8),selItemModel->data(selItemModel->index(6,0)));
    intTreeModel->setData(selItem.value(9),this->set_doubleValue(this->calc_totalWork(curr_sport,newPace,newTime,newStyle),false));

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

    if(intTreeModel->itemFromIndex(parentIndex)->hasChildren())
    {
        swimLap = 0;
        lapName = intTreeModel->data(parentIndex).toString().split("Int").first();
        do
        {
            currType = intTreeModel->itemFromIndex(parentIndex)->child(swimLap,1)->text();
            lapPace = this->get_timesec(intTreeModel->itemFromIndex(parentIndex)->child(swimLap,6)->text());
            level = this->checkRangeLevel(lapPace);
            intTreeModel->itemFromIndex(parentIndex)->child(swimLap,0)->setData(lapName+QString::number((swimLap+1)*swimTrack)+"_"+level,Qt::EditRole);
            intValue[0] = intValue[0] + intTreeModel->itemFromIndex(parentIndex)->child(swimLap,3)->text().toDouble();
            intValue[1] = intValue[1] + this->get_timesec(intTreeModel->itemFromIndex(parentIndex)->child(swimLap,4)->text());
            intValue[2] = intValue[2] + lapPace;
            intValue[3] = intValue[3] + intTreeModel->itemFromIndex(parentIndex)->child(swimLap,7)->text().toDouble();
            intValue[4] = intValue[4] + intTreeModel->itemFromIndex(parentIndex)->child(swimLap,8)->text().toDouble();
            intValue[5] = intValue[5] + intTreeModel->itemFromIndex(parentIndex)->child(swimLap,9)->text().toDouble();
            ++swimLap;    
            intType = currType == lastType ? currType : swimType.at(6);;
            lastType = currType;
        }
        while(intTreeModel->itemFromIndex(parentIndex)->child(swimLap,0) != 0);
        intValue[2] = intValue[2] / swimLap;
        intValue[3] = intValue[3] / swimLap;
        if(swimLap == 1) intType = currType;
    }
    treeSelect->select(parentIndex,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    intTreeModel->setData(treeSelect->selectedRows(0).at(0),lapName+"Int_"+QString::number(intValue[0]));
    intTreeModel->setData(treeSelect->selectedRows(1).at(0),intType);
    intTreeModel->setData(treeSelect->selectedRows(2).at(0),swimLap);
    intTreeModel->setData(treeSelect->selectedRows(3).at(0),intValue[0]);
    intTreeModel->setData(treeSelect->selectedRows(4).at(0),this->set_time(intValue[1]));
    intTreeModel->setData(treeSelect->selectedRows(6).at(0),this->set_time(intValue[2]));
    intTreeModel->setData(treeSelect->selectedRows(7).at(0),this->set_doubleValue(intValue[3],true));
    intTreeModel->setData(treeSelect->selectedRows(8).at(0),intValue[4]);
    intTreeModel->setData(treeSelect->selectedRows(9).at(0),this->set_doubleValue(intValue[5],false));

    this->updateSwimBreak(parentIndex,treeSelect,intValue[1]);
    this->recalcIntTree();
}

void Activity::updateSwimBreak(QModelIndex intIndex,QItemSelectionModel *treeSelect,int value)
{
    if(intIndex.sibling(intIndex.row()+1,0).isValid())
    {
        QStandardItem *nextBreak = intTreeModel->item(intIndex.row()+1);
        int breakStart,breakStop,breakDura;

        breakDura = this->get_timesec(intTreeModel->data(treeSelect->selectedRows(5).at(0)).toString());
        breakStart = breakDura + value;

        treeSelect->select(intTreeModel->indexFromItem(nextBreak),QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
        breakDura = this->get_timesec(intTreeModel->data(treeSelect->selectedRows(4).at(0)).toString());
        breakStop = this->get_timesec(intTreeModel->data(treeSelect->selectedRows(5).at(0)).toString());

        if((breakStop-breakStart)+breakDura >= 0)
        {
            breakDura = breakDura + (breakStop-breakStart);
        }
        else
        {
            breakDura = 0;
        }

        intTreeModel->setData(treeSelect->selectedRows(4).at(0),this->set_time(breakDura));
        intTreeModel->setData(treeSelect->selectedRows(5).at(0),this->set_time(breakStart));
        intTreeModel->setData(treeSelect->selectedRows(9).at(0),this->set_doubleValue(this->calc_totalWork(curr_sport,1,breakDura,0),false));
    }

    treeSelect->clearSelection();
}

void Activity::updateXDataModel()
{
    int child;
    double dist = 0;
    double lapSpeed = 0;
    int time = 0;
    int swimStyle = 0;
    int lapTime = 0;
    int strokes = 0;
    QString lapName;

    xdataModel->clear();
    xdataModel->setColumnCount(7);

    for(int row = 0,xdataRow = 0; row < intTreeModel->rowCount(); ++row)
    {
        if(intTreeModel->item(row,0)->hasChildren())
        {
            child = 0;
            do
            {
                xdataModel->insertRow(xdataRow,QModelIndex());
                lapName = intTreeModel->item(row,0)->child(child,0)->text();
                swimStyle = swimType.indexOf(intTreeModel->item(row,0)->child(child,1)->text());
                lapTime = this->get_timesec(intTreeModel->item(row,0)->child(child,4)->text());
                strokes = intTreeModel->item(row,0)->child(child,8)->text().toInt();
                lapSpeed = intTreeModel->item(row,0)->child(child,7)->text().toDouble();

                xdataModel->setData(xdataModel->index(xdataRow,0),lapName);
                xdataModel->setData(xdataModel->index(xdataRow,1),time);
                xdataModel->setData(xdataModel->index(xdataRow,2),dist);
                xdataModel->setData(xdataModel->index(xdataRow,3),swimStyle);
                xdataModel->setData(xdataModel->index(xdataRow,4),lapTime);
                xdataModel->setData(xdataModel->index(xdataRow,5),strokes);
                xdataModel->setData(xdataModel->index(xdataRow,6),lapSpeed);

                time = time + lapTime;
                dist = dist + (intTreeModel->item(row,0)->child(child,3)->text().toDouble() / distFactor);
                ++child;
                ++xdataRow;
            }
            while(intTreeModel->item(row,0)->child(child,0) != 0);
        }
        else
        {
            xdataModel->insertRow(xdataRow,QModelIndex());
            lapName = intTreeModel->item(row,0)->text();
            swimStyle = swimType.indexOf(intTreeModel->item(row,1)->text());
            lapTime = this->get_timesec(intTreeModel->item(row,4)->text());
            strokes = intTreeModel->item(row,8)->text().toInt();
            lapSpeed = intTreeModel->item(row,7)->text().toDouble();

            xdataModel->setData(xdataModel->index(xdataRow,0),lapName);
            xdataModel->setData(xdataModel->index(xdataRow,1),time);
            xdataModel->setData(xdataModel->index(xdataRow,2),dist);
            xdataModel->setData(xdataModel->index(xdataRow,3),swimStyle);
            xdataModel->setData(xdataModel->index(xdataRow,4),lapTime);
            xdataModel->setData(xdataModel->index(xdataRow,5),strokes);
            xdataModel->setData(xdataModel->index(xdataRow,6),lapSpeed);

            time = time + lapTime;
            dist = dist + (intTreeModel->item(row,3)->text().toDouble() / distFactor);
            ++xdataRow;
        }
    }
    this->updateIntModel(5,4);
}

void Activity::updateIntModel(int startCol,int duraCol)
{
    int intStart = 0;
    int intStop = 0;
    int lastStop = 0;

    for(int row = 0; row < intTreeModel->rowCount(); ++row)
    {
        intStart = this->get_timesec(intTreeModel->data(intTreeModel->index(row,startCol)).toString());
        intStop = intStart + this->get_timesec(intTreeModel->data(intTreeModel->index(row,duraCol)).toString());
        if(isSwim || (isBike && isIndoor)) --intStop;

        if(row > 0)
        {
            if(lastStop == intStart) ++intStart;
        }

        intModel->setData(intModel->index(row,0),intTreeModel->data(intTreeModel->index(row,0)).toString().trimmed());
        intModel->setData(intModel->index(row,1),intStart);
        intModel->setData(intModel->index(row,2),intStop);
        intModel->setData(intModel->index(row,intListCount),intTreeModel->data(intTreeModel->index(row,4)).toDouble());
        lastStop = intStop;
    }

    lastStop = sampleModel->rowCount();

    intModel->setData(intModel->index(intModel->rowCount()-1,2),lastStop-1);

    this->updateSampleModel(lastStop);
}

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

    if(!isSwim && !isTria)
    {
        lowLimit = this->get_speed(QTime::fromString(this->set_time(thresPace),"mm:ss"),0,curr_sport,true);
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
                                calcSpeed[intSec] = this->wattToSpeed(thresPower,thresSpeed,sampleModel->data(sampleModel->index(intSec,4,QModelIndex())).toDouble());
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
                        calcSpeed[intStop] = this->wattToSpeed(thresPower,thresSpeed,sampleModel->data(sampleModel->index(intStop,2,QModelIndex())).toDouble());
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
                        lowLimit = this->get_speed(QTime::fromString(this->set_time(sportpace),"mm:ss"),0,settings::isSwim,true);
                        lowLimit = lowLimit - (lowLimit*thresLimit);
                        calcSpeed[intSec] = this->interpolate_speed(intRow,intSec,lowLimit);
                    }
                    else if(intRow == 4)
                    {
                        sportpace = thresValues->value("runpace");
                        lowLimit = this->get_speed(QTime::fromString(this->set_time(sportpace),"mm:ss"),0,settings::isRun,true);
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
        sportValue = round(this->estimate_stress(settings::isSwim,this->set_time(this->get_int_pace(0,settings::isSwim)/10),this->get_int_duration(0),hasPMData));
        this->overrideData.insert("swimscore",QString::number(sportValue));
        triValue = triValue + sportValue;
        sportValue = round(this->estimate_stress(settings::isBike,QString::number(this->get_int_value(2,4)),this->get_int_duration(2),hasPMData));
        this->overrideData.insert("skiba_bike_score",QString::number(sportValue));
        triValue = triValue + sportValue;
        sportValue = round(this->estimate_stress(settings::isRun,this->set_time(this->get_int_pace(4,settings::isRun)),this->get_int_duration(4),hasPMData));
        this->overrideData.insert("govss",QString::number(sportValue));
        triValue = triValue + sportValue;
        this->overrideData.insert("triscore",QString::number(triValue));
    }
}

void Activity::writeChangedData()
{
    tagData.insert("Updated","1");
    this->init_jsonFile();
    this->write_actModel("INTERVALS",intModel,&intList);
    this->write_actModel("SAMPLES",sampleModel,&sampList);
    if(hasXdata) this->write_xdataModel(curr_sport,xdataModel);
    this->write_jsonFile();
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
        for(int i = 0; i < zoneCount; ++i)
        {
            timeinzone = hfTimeInZone.value(levels.at(i));
            timeinzone = timeinzone + (swimHFZoneFactor.value(it.key()).at(i) * paceTimeInZone.value(it.key()));
            hfTimeInZone.insert(levels.at(i),timeinzone);
        }
    }

    int hfAvg = 0;
    double workoutTime = this->get_timesec(ride_info.value("Duration"));

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

int Activity::get_int_duration(int row)
{
    int duration;

    duration = intModel->data(intModel->index(row,2,QModelIndex()),Qt::DisplayRole).toInt() - intModel->data(intModel->index(row,1,QModelIndex()),Qt::DisplayRole).toInt();

    if(isIndoor) return duration+1;
    if(isUpdated && isBike) return duration+1;

    return duration;
}

double Activity::get_int_distance(int row,int offset)
{
    double dist,dist_start,dist_stop;
    int int_start,int_stop;

    if(row == 0)
    {
        int_stop = intModel->data(intModel->index(row,2,QModelIndex()),Qt::DisplayRole).toInt();
        dist = sampleModel->data(sampleModel->index(int_stop,1,QModelIndex()),Qt::DisplayRole).toDouble();
    }
    else
    {
        int_start = intModel->data(intModel->index(row,1,QModelIndex()),Qt::DisplayRole).toInt()-offset;
        int_stop = intModel->data(intModel->index(row,2,QModelIndex()),Qt::DisplayRole).toInt();
        dist_start = sampleModel->data(sampleModel->index(int_start,1,QModelIndex()),Qt::DisplayRole).toDouble();
        dist_stop = sampleModel->data(sampleModel->index(int_stop,1,QModelIndex()),Qt::DisplayRole).toDouble();
        dist = dist_stop - dist_start;
    }
    return dist;
}

int Activity::get_int_pace(int row,QString lapName)
{
    int pace;

    if(isSwim)
    {
        if(lapName.contains(breakName))
        {
            pace = 0;
        }
        else
        {
            pace = this->get_int_duration(row) / ((intModel->data(intModel->index(row,intListCount+1)).toInt()*swimTrack)/100);
        }
    }
    else
    {
        pace = this->get_int_duration(row) / this->get_int_distance(row,1);
    }

    return pace;
}

double Activity::get_int_value(int row,int col)
{
    double value = 0.0;
    int int_start,int_stop;
    int_start = intModel->data(intModel->index(row,1,QModelIndex())).toInt();
    int_stop = intModel->data(intModel->index(row,2,QModelIndex())).toInt();

    for(int i = int_start; i <= int_stop; ++i)
    {
        value = value + sampleModel->data(sampleModel->index(i,col,QModelIndex())).toDouble();
    }

    value = value / ((int_stop - int_start)+1);

    return value;
}

double Activity::get_int_speed(int row)
{
    double speed,pace;

    if(isSwim)
    {
        pace = this->get_int_duration(row) / (intModel->data(intModel->index(row,intListCount,QModelIndex())).toDouble() / distFactor);
    }
    else
    {
        pace = this->get_int_duration(row) / (intModel->data(intModel->index(row,intListCount,QModelIndex())).toDouble()); // *10.0
    }
    speed = 3600.0 / pace;

    return speed;
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

    if(curr_sport == settings::isRun)
    {
        randfact = randNum / (currSpeed/((factor*100)+1.0));
    }
    if(curr_sport == settings::isBike)
    {
        randfact = randNum / (currSpeed/((factor*1000)+1.0));
    }
    if(curr_sport == settings::isTria)
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

    for(int i = 0; i < avgModel->rowCount();++i)
    {
        avgModel->setData(avgModel->index(i,0),"-");
    }

    for(QHash<int,QModelIndex>::const_iterator it =  avgItems.cbegin(), end = avgItems.cend(); it != end; ++it)
    {
        intTreeModel->setData(it.value(),"-");
        intTreeModel->setData(it.value(),0,Qt::UserRole+1);
    }
    avgItems.clear();
}

double Activity::interpolate_speed(int row,int sec,double limit)
{
    double curr_speed = sampSpeed[sec];
    double avg_speed = this->get_int_speed(row);

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

    return 0;
}

void Activity::set_avgValues(int counter,int factor)
{
    avgCounter = counter;

    if(counter != 0)
    {
        if(isSwim)
        {
            avgValues[1] = avgValues[1] + (static_cast<double>(this->get_timesec(intTreeModel->data(selItem.value(4)).toString()))*factor);
            avgValues[2] = avgValues.at(2) + (static_cast<double>(this->get_timesec(intTreeModel->data(selItem.value(6)).toString()))*factor);
            avgValues[3] = avgValues.at(3) + (intTreeModel->data(selItem.value(3)).toDouble()*factor);
            avgValues[4] = avgValues.at(4) + (intTreeModel->data(selItem.value(8)).toDouble());
            avgValues[5] = avgValues.at(5) + (this->get_timesec(intTreeModel->data(selItem.value(4)).toString())+intTreeModel->data(selItem.value(8)).toInt());
            avgValues[6] = avgValues.at(6) + (avgValues.at(1)/avgValues.at(4));
            avgModel->setData(avgModel->index(4,0),round(avgValues[4]/avgCounter));
            avgModel->setData(avgModel->index(5,0),round(avgValues[5]/avgCounter));
            avgModel->setData(avgModel->index(6,0),this->set_doubleValue(avgValues[6]/avgCounter,false));
        }
        else
        {
            avgValues[1] = avgValues.at(1) + (static_cast<double>(this->get_timesec(intTreeModel->data(selItem.value(1)).toString()))*factor);
            avgValues[2] = avgValues.at(2) + (static_cast<double>(this->get_timesec(intTreeModel->data(selItem.value(5)).toString()))*factor);
            avgValues[3] = avgValues.at(3) + (intTreeModel->data(selItem.value(4)).toDouble()*factor);

            if(hasPMData)
            {
                avgValues[4] = avgValues.at(4) + (intTreeModel->data(selItem.value(7)).toDouble()*factor);
                avgValues[5] = avgValues.at(5) + (intTreeModel->data(selItem.value(8)).toDouble()*factor);
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
}
