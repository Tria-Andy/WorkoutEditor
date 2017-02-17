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

    curr_sport = this->readJsonContent(jsonfile);

    for(int i = 0; i < valueList.count();++i)
    {
        stgValue = valueList.at(i);
        ride_info.insert(stgValue,tagData.value(stgValue));
    }
    ride_info.insert("Date",rideData.value("STARTTIME"));

    if(intAct)
    {
        QMap<int,QString> mapValues;

        intModel = new QStandardItemModel;
        mapValues = settings::get_intList();
        this->init_actModel("INTERVALS",&mapValues,intModel,&intList,3);

        sampleModel = new QStandardItemModel;
        mapValues = settings::get_sampList();
        this->init_actModel("SAMPLES",&mapValues,sampleModel,&sampList,0);

        if(hasXdata)
        {
            xdataModel = new QStandardItemModel;
            this->init_xdataModel(curr_sport,xdataModel);
        }
        this->prepareData();
    }
}

void Activity::prepareData()
{
    isSwim = isBike = isRun = isTria = isStrength = false;
    itemHeader.insert(0,QStringList() << "Name:" << "Type:" << "Distance (M):" << "Duration (Sec):" << "Pace/100m:" << "Speed km/h:" << "Strokes:");
    itemHeader.insert(1,QStringList() << "Name:" << "Start:" << "Distance:" << "Duration:" << "Pace:" << "Speed km/h:");
    avgHeader.insert(0,QStringList() << "Intervalls:" << "Duration:" << "Pace:" << "Distance:");
    avgHeader.insert(1,QStringList() << "Intervalls:" << "Duration:" << "Pace:" << "Distance:" << "Watts:" << "CAD:");
    intTreeModel = new QStandardItemModel;
    selItemModel = new QStandardItemModel;
    selItemModel->setColumnCount(2);
    avgModel = new QStandardItemModel;
    avgModel->setColumnCount(1);

    if(curr_sport == settings::isSwim) isSwim = true;
    else if(curr_sport == settings::isBike) isBike = true;
    else if(curr_sport == settings::isRun) isRun = true;
    else if(curr_sport == settings::isTria) isTria = true;
    else if(curr_sport == settings::isStrength) isStrength = true;

    int sampCount = sampleModel->rowCount();
    int intModelCount = intModel->rowCount();
    int avgHF = 0;
    int posHF = sampList.indexOf("HR");
    int lapCount = 1;
    int lapKey = 1;
    int dist = 0;
    int lapIdent = 0;
    double threshold;
    bool isTimeBased = true;
    levels = settings::get_listValues("Level");
    breakName = settings::get_generalValue("breakname");
    QString lapName;
    intLabel = "_Int_";
    sampSpeed.resize(sampCount);

    for(int i = 0; i < sampCount; ++i)
    {
        sampSpeed[i] = sampleModel->data(sampleModel->index(i,2,QModelIndex())).toDouble();
        avgHF = avgHF + sampleModel->data(sampleModel->index(i,posHF,QModelIndex())).toInt();
    }

    if(isRun)
    {
        avgHF = (avgHF / sampCount);
        double totalWork = ceil(this->calc_totalWork(tagData.value("Weight").toDouble(),avgHF,sampCount));
        overrideData.insert("total_work",QString::number(totalWork));
        ride_info.insert("Total Cal",QString::number(totalWork));
        ride_info.insert("Total Work",QString::number(totalWork));
        ride_info.insert("AvgHF",QString::number(avgHF));
        hasOverride = true;
    }

    intModel->setData(intModel->index(intModelCount-1,2,QModelIndex()),sampCount-1);

    ride_info.insert("Distance",QString::number(sampleModel->data(sampleModel->index(sampCount-1,1,QModelIndex())).toDouble()));
    ride_info.insert("Duration",QDateTime::fromTime_t(sampCount).toUTC().toString("hh:mm:ss"));

    if(isSwim)
    {
        distFactor = 1000;
        swim_track = tagData.value("Pool Length").toDouble();
        double temp_cv = settings::get_thresValue("swimpace");
        hf_threshold = settings::get_thresValue("hfthres");
        hf_max = settings::get_thresValue("hfmax");

        zone_count = levels.count();
        swim_cv = (3600.0 / temp_cv) / 10.0;
        pace_cv = temp_cv;
        int intCounter = 1;
        int breakCounter = 1;
        int intStart,intStop;
        int swimLap = 0;

        swimModel = new QStandardItemModel(xdataModel->rowCount(),10);
        swimType = settings::get_listValues("SwimStyle");

        //Read current CV and HF Threshold
        threshold = settings::get_thresValue("swimpace");
        this->fillRangeLevel(threshold,true);

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

            zoneLow = this->get_zone_values(zone_low.toDouble(),hf_threshold,false);
            zoneHigh = this->get_zone_values(zone_high.toDouble(),hf_threshold,false);

            if(i < zone_count-1)
            {
                hfZoneAvg.insert(levels.at(i),ceil((zoneLow + zoneHigh) / 2));
            }
            else
            {
                hfZoneAvg.insert(levels.at(i),ceil((zoneLow + hf_max) / 2));
            }
        }

        QVector<double> hf_factor = {0.50,0.35,0.10,0.05};
        QVector<double> hFactor(zone_count);
        hFactor.fill(0);

        for(int i = 0; i < hf_factor.count(); ++i)
        {
            hFactor[i] = hf_factor.at(i);
        }
        swimHFZoneFactor.insert(breakName,hFactor);

        int vPos = 0;
        for(int i = 0,v=0; i < zone_count; ++i)   //paceZone
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

        for(int row = 0; row < intModelCount; ++row)
        {
            lapCount = this->get_swim_laps(row);
            dist = lapCount*swim_track;
            intModel->setData(intModel->index(row,3),dist);
            intModel->setData(intModel->index(row,4),lapCount);

            intStart = intModel->data(intModel->index(row,1)).toInt();
            intStop = intModel->data(intModel->index(row,2)).toInt();

            if(lapCount > 0)
            {
                lapIdent = lapCount == 1 ? 1 : 0;
                intModel->setData(intModel->index(row,5),"Int-"+QString::number(lapKey)+"_"+QString::number(lapIdent));
            }
            else
            {
                intModel->setData(intModel->index(row,5),breakName+"_"+QString::number(breakCounter));
                ++lapKey;
                ++breakCounter;
            }

            if(row == 0)
            {
                lapName = QString::number(intCounter)+intLabel+QString::number(dist);
                swimLap = this->build_swimModel(true,"Int-",intCounter,intStart,intStop,swimLap);
                ++intCounter;
            }
            else if(row != 0 && row < intModelCount-1)
            {
                if(lapCount > 0)
                {
                    lapName = QString::number(intCounter)+intLabel+QString::number(dist);
                    swimLap = this->build_swimModel(true,"Int-",intCounter,intStart,intStop,swimLap);
                    ++intCounter;
                }
                else
                {
                    lapName = breakName;
                    swimLap = this->build_swimModel(false,breakName,intCounter,intStart,intStop,swimLap);
                }
            }
            else if(row == intModelCount-1)
            {
                lapName = QString::number(intCounter)+intLabel+QString::number(dist);
                swimLap = this->build_swimModel(true,"Int-",intCounter,intStart,intStop,swimLap);
            }

            intModel->setData(intModel->index(row,0),lapName);
        }

        swimProxy = new QSortFilterProxyModel;
        swimProxy->setSourceModel(swimModel);

        xdataModel->clear();

        //Calc Swim Data
        move_time = this->get_moveTime();

        swim_pace = ceil(static_cast<double>(move_time) / (ride_info.value("Distance").toDouble()*10));
        double goal = sqrt(pow(static_cast<double>(swim_pace),3.0))/10;
        swim_sri = static_cast<double>(pace_cv) / goal;

        avgValues.resize(4);
        avgModel->setVerticalHeaderLabels(avgHeader.value(0));

        this->swimhfTimeInZone(false);
    }
    else
    {
        distFactor = 1;

        if(isBike)
        {
            threshold = settings::get_thresValue("bikepower");
            this->fillRangeLevel(threshold,false);
            isTimeBased = true;
            avgValues.resize(6);
            avgModel->setVerticalHeaderLabels(avgHeader.value(1));
        }

        if(isRun)
        {
            threshold = settings::get_thresValue("runpace");
            this->fillRangeLevel(threshold,true);
            isTimeBased = false;
            avgValues.resize(4);
            avgModel->setVerticalHeaderLabels(avgHeader.value(0));
        }

        for(int row = 0; row < intModelCount; ++row)
        {
            intModel->setData(intModel->index(row,3),this->get_int_distance(row));
            intModel->setData(intModel->index(row,4),lapCount);
            intModel->setData(intModel->index(row,5),"Int-"+QString::number(row));
        }
    }

    avgModel->setRowCount(avgValues.count());
    selItemModel->setVerticalHeaderLabels(itemHeader.value(1));

    this->reset_avgSelection();
    this->build_intTree(isTimeBased);
}

void Activity::build_intTree(bool timeBased)
{
    QStandardItem *rootItem = intTreeModel->invisibleRootItem();
    if(isSwim)
    {
        QString intKey;
        for(int i = 0; i < intModel->rowCount(); ++i)
        {
            intKey = intModel->data(intModel->index(i,5)).toString().split("_").first();
            rootItem->appendRow(setSwimLap(i,intKey));
        }
    }
    else
    {
        for(int i = 0; i < intModel->rowCount(); ++i)
        {
            rootItem->appendRow(setIntRow(i,timeBased));
        }
    }
    intTreeModel->setHorizontalHeaderLabels(settings::get_int_header(curr_sport));
    this->recalcIntTree();
}


int Activity::build_swimModel(bool isInt,QString intName,int intCount,int intStart,int intStop,int swimLap)
{
    QString swimLapName,lapKey,level;
    int lapStart = 0,lapStop = 0;
    int swimlapCount = 1;
    int type = 0;
    int lapTime;
    double lapDist,lapSpeed;
    int lapPace = 0;

    lapStart = intStart;

    while (lapStart >= intStart && lapStop < intStop)
    {
        lapStart = xdataModel->data(xdataModel->index(swimLap,0)).toInt();
        lapStop = xdataModel->data(xdataModel->index(swimLap+1,0)).toInt();

        lapTime = round(xdataModel->data(xdataModel->index(swimLap,3)).toDouble());
        type = xdataModel->data(xdataModel->index(swimLap,2)).toInt();
        lapDist = xdataModel->data(xdataModel->index(swimLap,1)).toDouble();

        if(isInt)
        {
            lapSpeed = this->calcSpeed(lapTime,swim_track,distFactor);
            swimLapName = QString::number(intCount)+"_"+QString::number(swimlapCount*swim_track);
            lapKey = intName+QString::number(intCount)+"_"+QString::number(swimlapCount);
        }
        else
        {
            lapTime = lapStop-lapStart;
            lapSpeed = 0;
            swimLapName = lapKey = intName;
        }

        lapPace = lapTime * (100.0/swim_track);
        level = this->checkRangeLevel(lapPace);

        if(isInt) swimLapName = swimLapName+"_"+level;

        swimModel->setData(swimModel->index(swimLap,0,QModelIndex()),swimLapName);
        swimModel->setData(swimModel->index(swimLap,1,QModelIndex()),lapStart);
        swimModel->setData(swimModel->index(swimLap,2,QModelIndex()),lapDist);
        swimModel->setData(swimModel->index(swimLap,3,QModelIndex()),type);
        swimModel->setData(swimModel->index(swimLap,4,QModelIndex()),swim_track);
        swimModel->setData(swimModel->index(swimLap,5,QModelIndex()),lapTime);
        swimModel->setData(swimModel->index(swimLap,6,QModelIndex()),lapPace);
        swimModel->setData(swimModel->index(swimLap,7,QModelIndex()),lapSpeed);
        swimModel->setData(swimModel->index(swimLap,8,QModelIndex()),xdataModel->data(xdataModel->index(swimLap,4,QModelIndex())));
        swimModel->setData(swimModel->index(swimLap,9,QModelIndex()),lapKey);

        this->swimTimeInZone(lapTime,lapPace);

        ++swimLap;
        ++swimlapCount;
    }
    return swimLap;
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
    }
    return currZone;
}

QList<QStandardItem *> Activity::setIntRow(int pInt,bool timeBased)
{
    QList<QStandardItem*> intItems;
    QModelIndex data_index = sampleModel->index(intModel->data(intModel->index(pInt,2)).toInt()-1,1);
    QString lapName = intModel->data(intModel->index(pInt,0)).toString();
    int lapPace = this->get_int_pace(pInt,lapName);
    int lapTime = this->get_int_duration(pInt);
    double lapDist = this->set_doubleValue(this->get_int_distance(pInt),true);

    intItems << new QStandardItem(lapName);
    intItems << new QStandardItem(this->set_time(lapTime));
    intItems << new QStandardItem(this->set_time(intModel->data(intModel->index(pInt,1)).toInt()));
    intItems << new QStandardItem(QString::number(this->set_doubleValue(sampleModel->data(data_index).toDouble(),true)));
    intItems << new QStandardItem(QString::number(lapDist));
    intItems << new QStandardItem(this->set_time(lapPace));
    intItems << new QStandardItem(QString::number(this->set_doubleValue(this->get_int_speed(pInt),false)));

    if(isBike)
    {
        double watts = round(this->get_int_value(pInt,4));
        lapName = QString::number(pInt+1)+"_"+this->checkRangeLevel(watts);
        intItems << new QStandardItem(QString::number(watts));
        intItems << new QStandardItem(QString::number(round(this->get_int_value(pInt,3))));
    }
    else
    {
        lapName = QString::number(pInt+1)+"_"+this->checkRangeLevel(lapPace);
    }

    if(timeBased)
    {
        QString timeLabel;

        if(lapTime < 60)
        {
            timeLabel = "Sec";
            lapTime = ceil(lapTime);
        }
        else
        {
            timeLabel = "Min";
            lapTime = (ceil(lapTime/10.0)*10)/60.0;
        }

        lapName = lapName + "-"+QString::number(lapTime)+timeLabel;
    }
    else
    {
        lapDist = (round(lapDist*10)/10.0);
        QString distLabel;
        if(lapDist < 1)
        {
            distLabel = "M";
            lapDist = ceil(lapDist*1000.0);
        }
        else
        {

            distLabel = "K";
        }

        lapName = lapName + "-"+QString::number(lapDist)+distLabel;
    }

    intItems.at(0)->setData(lapName,Qt::EditRole);
    intItems << new QStandardItem("-");

    return intItems;
}

QList<QStandardItem *> Activity::setSwimLap(int pInt,QString intKey)
{
    QList<QStandardItem*> intItems,subItems;
    int strokeCount = 0;
    int currType = 0,lastType = 0;
    int lapType = 0;
    int lapCount = intModel->data(intModel->index(pInt,4)).toInt();
    QString lapName = intModel->data(intModel->index(pInt,0)).toString();

    swimProxy->setFilterRegExp("^"+intKey+"_");
    swimProxy->setFilterKeyColumn(9);

    intItems << new QStandardItem(lapName);
    intItems << new QStandardItem(swimType.at(currType));
    intItems << new QStandardItem(intModel->data(intModel->index(pInt,4)).toString());
    intItems << new QStandardItem(intModel->data(intModel->index(pInt,3)).toString());
    intItems << new QStandardItem(this->set_time(this->get_int_duration(pInt)));
    intItems << new QStandardItem(this->set_time(intModel->data(intModel->index(pInt,1)).toInt()));
    intItems << new QStandardItem(this->set_time(this->get_int_pace(pInt,lapName)));
    intItems << new QStandardItem(QString::number(this->get_int_speed(pInt)));
    intItems << new QStandardItem(QString::number(strokeCount));
    intItems << new QStandardItem("-");

    if(lapCount > 1)
    {
        int intTime = 0;
        int lapTime = 0;

        for(int i = 0; i < swimProxy->rowCount();++i)
        {
            lapTime = swimProxy->data(swimProxy->index(i,5)).toInt();
            currType = swimProxy->data(swimProxy->index(i,3)).toInt();
            lapType = currType == lastType ? currType : 6;
            strokeCount = strokeCount+swimProxy->data(swimProxy->index(i,8)).toInt();

            subItems << new QStandardItem(swimProxy->data(swimProxy->index(i,0)).toString());
            subItems << new QStandardItem(swimType.at(currType));
            subItems << new QStandardItem("-");
            subItems << new QStandardItem(swimProxy->data(swimProxy->index(i,4)).toString());
            subItems << new QStandardItem(this->set_time(lapTime));
            subItems << new QStandardItem("-");
            subItems << new QStandardItem(this->set_time(swimProxy->data(swimProxy->index(i,6)).toInt()));
            subItems << new QStandardItem(QString::number(swimProxy->data(swimProxy->index(i,7)).toDouble()));
            subItems << new QStandardItem(swimProxy->data(swimProxy->index(i,8)).toString());

            lastType = currType;
            intTime = intTime + lapTime;
            intItems.first()->appendRow(subItems);
            subItems.clear();
        }

        intItems.at(1)->setData(swimType.at(lapType),Qt::EditRole);
        intItems.at(4)->setData(this->set_time(intTime),Qt::EditRole);
        intItems.at(8)->setData(strokeCount,Qt::EditRole);
    }
    else
    {
        intItems.at(1)->setData(swimType.at(swimProxy->data(swimProxy->index(0,3)).toInt()),Qt::EditRole);
        intItems.at(8)->setData(swimProxy->data(swimProxy->index(0,8)).toString(),Qt::EditRole);
    }

    return intItems;
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
                editRow << 1 << 1 << 0 << 1 << 0 << 0 << 1;
            }
        }
    }
    else
    {
        if(isInt) editRow << 1 << 1 << 1 << 0 << 0 << 0;
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
    QStandardItem *parentItem = intTreeModel->itemFromIndex(selItem.value(0))->parent();
    QList<QStandardItem*> subItems;

    if(selItem.value(0).isValid())
    {
        subItems << new QStandardItem(selItem.value(0).data().toString());
        subItems << new QStandardItem(selItem.value(1).data().toString());
        subItems << new QStandardItem("-");
        subItems << new QStandardItem(QString::number(swim_track));
        subItems << new QStandardItem("00:00");
        subItems << new QStandardItem("-");
        subItems << new QStandardItem("00:00");
        subItems << new QStandardItem(QString::number(0.0));
        subItems << new QStandardItem(QString::number(0));

        parentItem->insertRow(treeSelect->currentIndex().row(),subItems);
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
            intTreeModel->setData(treeSelect->selectedRows(6).at(0),this->set_time(this->calc_lapPace(curr_sport,delDura,swim_track)));
            intTreeModel->setData(treeSelect->selectedRows(7).at(0),this->set_doubleValue(this->calcSpeed(delDura,swim_track,distFactor),true));
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
    }
    else
    {
        intTreeModel->setData(selItem.value(0),selItemModel->data(selItemModel->index(0,0)));
        intTreeModel->setData(selItem.value(1),selItemModel->data(selItemModel->index(3,0)));
        intTreeModel->setData(selItem.value(2),this->set_time(selItemModel->data(selItemModel->index(1,0)).toInt()));
        intTreeModel->setData(selItem.value(4),selItemModel->data(selItemModel->index(2,0)));
        intTreeModel->setData(selItem.value(5),selItemModel->data(selItemModel->index(4,0)));
        intTreeModel->setData(selItem.value(6),this->set_doubleValue(selItemModel->data(selItemModel->index(5,0)).toDouble(),false));
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
    int levelTime = 0;

    intTreeModel->setData(selItem.value(0),selItemModel->data(selItemModel->index(0,0)));
    intTreeModel->setData(selItem.value(1),selItemModel->data(selItemModel->index(1,0)));
    intTreeModel->setData(selItem.value(4),this->set_time(selItemModel->data(selItemModel->index(3,0)).toInt()));
    intTreeModel->setData(selItem.value(6),selItemModel->data(selItemModel->index(4,0)));
    intTreeModel->setData(selItem.value(7),this->set_doubleValue(selItemModel->data(selItemModel->index(5,0)).toDouble(),true));
    intTreeModel->setData(selItem.value(8),selItemModel->data(selItemModel->index(6,0)));

    if(oldLevel != newLevel)
    {
        levelTime = paceTimeInZone.value(oldLevel);
        paceTimeInZone.insert(oldLevel,levelTime - oldTime);

        levelTime = paceTimeInZone.value(newLevel);
        paceTimeInZone.insert(newLevel,levelTime + newTime);

        levelTime = (oldTime - newTime) + paceTimeInZone.value(breakName);
        paceTimeInZone.insert(breakName,levelTime);
        this->swimhfTimeInZone(true);
    }
}

void Activity::updateSwimInt(QModelIndex parentIndex,QItemSelectionModel *treeSelect)
{
    QVector<double> intValue(5);
    intValue.fill(0);
    QString lapName,level;
    int swimLap = 0;
    int lapPace;
    if(intTreeModel->itemFromIndex(parentIndex)->hasChildren())
    {
        swimLap = 0;
        lapName = intTreeModel->data(parentIndex).toString().split("Int").first();
        do
        {
            lapPace = this->get_timesec(intTreeModel->itemFromIndex(parentIndex)->child(swimLap,6)->text());
            level = this->checkRangeLevel(lapPace);
            intTreeModel->itemFromIndex(parentIndex)->child(swimLap,0)->setData(lapName+QString::number((swimLap+1)*swim_track)+"_"+level,Qt::EditRole);
            intValue[0] = intValue[0] + intTreeModel->itemFromIndex(parentIndex)->child(swimLap,3)->text().toDouble();
            intValue[1] = intValue[1] + this->get_timesec(intTreeModel->itemFromIndex(parentIndex)->child(swimLap,4)->text());
            intValue[2] = intValue[2] + lapPace; //this->get_timesec(intTreeModel->itemFromIndex(parentIndex)->child(swimLap,6)->text());
            intValue[3] = intValue[3] + intTreeModel->itemFromIndex(parentIndex)->child(swimLap,7)->text().toDouble();
            intValue[4] = intValue[4] + intTreeModel->itemFromIndex(parentIndex)->child(swimLap,8)->text().toDouble();
            ++swimLap;
        }
        while(intTreeModel->itemFromIndex(parentIndex)->child(swimLap,0) != 0);
        intValue[2] = intValue[2] / swimLap;
        intValue[3] = intValue[3] / swimLap;
    }
    treeSelect->select(parentIndex,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    intTreeModel->setData(treeSelect->selectedRows(0).at(0),lapName+"Int_"+QString::number(intValue[0]));
    intTreeModel->setData(treeSelect->selectedRows(2).at(0),swimLap);
    intTreeModel->setData(treeSelect->selectedRows(3).at(0),intValue[0]);
    intTreeModel->setData(treeSelect->selectedRows(4).at(0),this->set_time(intValue[1]));
    intTreeModel->setData(treeSelect->selectedRows(6).at(0),this->set_time(intValue[2]));
    intTreeModel->setData(treeSelect->selectedRows(7).at(0),this->set_doubleValue(intValue[3],true));
    intTreeModel->setData(treeSelect->selectedRows(8).at(0),intValue[4]);

    this->updateSwimBreak(parentIndex,treeSelect,intValue[1]);
    this->recalcIntTree();
}

void Activity::updateSwimBreak(QModelIndex intIndex,QItemSelectionModel *treeSelect,int value)
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

    treeSelect->clearSelection();
}

void Activity::recalcIntTree()
{
    int startTime = 0;
    int intTime = 0;
    double workDist = 0;
    int rowCount = intTreeModel->rowCount();
    if(isSwim)
    {
        int intCounter = 1;
        int lapDist;
        int intPace;
        QString lapName,level;

        for(int row = 0; row < rowCount; ++row)
        {
           intTime = this->get_timesec(intTreeModel->data(intTreeModel->index(row,4)).toString());
           lapDist = intTreeModel->data(intTreeModel->index(row,3)).toDouble();
           intPace = this->get_timesec(intTreeModel->data(intTreeModel->index(row,6)).toString());
           workDist = workDist + lapDist;
           lapName = intTreeModel->data(intTreeModel->index(row,0)).toString();
           if(!lapName.contains(breakName))
           {
                level = this->checkRangeLevel(intPace);
                lapName = QString::number(intCounter)+intLabel+QString::number(lapDist)+"_"+level;
                ++intCounter;
           }
           intTreeModel->setData(intTreeModel->index(row,0),lapName);
           intTreeModel->setData(intTreeModel->index(row,4),this->set_time(intTime));
           intTreeModel->setData(intTreeModel->index(row,5),this->set_time(startTime));
           startTime = startTime+intTime;
        }
    }
    else
    {
        for(int row = 0; row < rowCount; ++row)
        {
           startTime = this->get_timesec(intTreeModel->data(intTreeModel->index(row,2)).toString());
           intTime = this->get_timesec(intTreeModel->data(intTreeModel->index(row,1)).toString());
           workDist = workDist + intTreeModel->data(intTreeModel->index(row,4)).toDouble();

           intTreeModel->setData(intTreeModel->index(row,1),this->set_time(intTime));
           intTreeModel->setData(intTreeModel->index(row,2),this->set_time(startTime));
           intTreeModel->setData(intTreeModel->index(row,3),this->set_doubleValue(workDist,true));
        }
    }
    ride_info.insert("Distance",QString::number(workDist/distFactor));
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

    for(int row = 0; row < intTreeModel->rowCount(); ++row)
    {
        intStart = this->get_timesec(intTreeModel->data(intTreeModel->index(row,startCol)).toString());
        intStop = intStart + this->get_timesec(intTreeModel->data(intTreeModel->index(row,duraCol)).toString());

        intModel->setData(intModel->index(row,0),intTreeModel->data(intTreeModel->index(row,0)).toString().trimmed());
        intModel->setData(intModel->index(row,1),intStart);
        intModel->setData(intModel->index(row,2),intStop);
        intModel->setData(intModel->index(row,3),intTreeModel->data(intTreeModel->index(row,4)).toDouble());
    }
    this->updateSampleModel(++intStop);
}

void Activity::set_workoutContent(QString content)
{
    this->tagData.insert("Workout Content",content);
}

void Activity::updateSampleModel(int rowcount)
{
    int sampRowCount = rowcount;
    new_dist.resize(sampRowCount);
    calc_speed.resize(sampRowCount);
    calc_cadence.resize(sampRowCount);
    double msec = 0.0;
    int intStart,intStop,sportpace = 0;
    double lowLimit = 0.0,limitFactor = 0.0;
    double swimPace,swimSpeed,swimCycle;
    QString lapName;

    if(!isSwim && !isTria)
    {
        if(isBike)
        {
            sportpace = settings::get_thresValue("bikepace");
            limitFactor = 0.35;
        }
        if(isRun)
        {
            sportpace = settings::get_thresValue("runpace");
            limitFactor = 0.20;
        }
        lowLimit = this->get_speed(QTime::fromString(this->set_time(sportpace),"mm:ss"),0,curr_sport,true);
        lowLimit = lowLimit - (lowLimit*limitFactor);
    }
    if(isTria)
    {
        lowLimit = 1.0;
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
                overDist = overDist + (swim_track / distFactor);
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
                msec = (swim_track / swimPace) / distFactor;
            }

            for(int lapsec = intStart; lapsec <= intStop; ++lapsec)
            {
                if(lapsec == 0)
                {
                    new_dist[0] = 0.001;
                }
                else
                {
                    if(lapsec == intStart && isBreak)
                    {
                        new_dist[lapsec] = overDist;
                    }
                    else
                    {
                        new_dist[lapsec] = currDist + msec;
                    }
                    if(lapsec == intStop)
                    {
                        new_dist[intStop] = overDist;
                    }
                }
                currDist = new_dist[lapsec];
                calc_speed[lapsec] = swimSpeed;
                calc_cadence[lapsec] = swimCycle;
            }
        }

        for(int row = 0; row < sampRowCount;++row)
        {
            sampleModel->setData(sampleModel->index(row,0,QModelIndex()),row);
            sampleModel->setData(sampleModel->index(row,1,QModelIndex()),new_dist[row]);
            sampleModel->setData(sampleModel->index(row,2,QModelIndex()),calc_speed[row]);
            sampleModel->setData(sampleModel->index(row,3,QModelIndex()),calc_cadence[row]);
        }
    }
    else
    {
        double workDist = 0;
        for(int intRow = 0; intRow < intModel->rowCount(); ++intRow)
        {
            intStart = intModel->data(intModel->index(intRow,1)).toInt();
            intStop = intModel->data(intModel->index(intRow,2)).toInt();
            workDist = workDist + intModel->data(intModel->index(intRow,3)).toDouble();
            msec = floor((intModel->data(intModel->index(intRow,3)).toDouble() / (intStop-intStart))*100000.0)/100000.0;

            if(isBike || isRun)
            {
                for(int intSec = intStart;intSec <= intStop; ++intSec)
                {
                    if(intSec == 0)
                    {
                        new_dist[0] = 0.0000;
                    }
                    else
                    {
                        calc_speed[intSec] = this->interpolate_speed(intRow,intSec,lowLimit);
                        new_dist[intSec] = new_dist[intSec-1] + msec;
                        if(isBike) calc_cadence[intSec] = sampleModel->data(sampleModel->index(intSec,3,QModelIndex())).toDouble();
                    }
                }
                new_dist[intStop] = workDist;
            }
        }
    }

    if(isBike)
    {
        for(int row = 0; row < sampRowCount;++row)
        {
            sampleModel->setData(sampleModel->index(row,0,QModelIndex()),row);
            sampleModel->setData(sampleModel->index(row,1,QModelIndex()),new_dist[row]);
            sampleModel->setData(sampleModel->index(row,2,QModelIndex()),calc_speed[row]);
            sampleModel->setData(sampleModel->index(row,3,QModelIndex()),calc_cadence[row]);
        }
    }

    if(isRun)
    {
        for(int row = 0; row < sampRowCount;++row)
        {
            sampleModel->setData(sampleModel->index(row,0,QModelIndex()),row);
            sampleModel->setData(sampleModel->index(row,1,QModelIndex()),new_dist[row]);
            sampleModel->setData(sampleModel->index(row,2,QModelIndex()),calc_speed[row]);
        }
    }

    if(isTria)
    {
        double triValue = 0,sportValue = 0;

        for(int row = 0; row < sampRowCount;++row)
        {
          sampleModel->setData(sampleModel->index(row,0,QModelIndex()),row);
          sampleModel->setData(sampleModel->index(row,1,QModelIndex()),new_dist[row]);
          sampleModel->setData(sampleModel->index(row,2,QModelIndex()),calc_speed[row]);
          sampleModel->setData(sampleModel->index(row,3,QModelIndex()),calc_cadence[row]);
        }
        this->hasOverride = true;
        sportValue = round(this->estimate_stress(settings::isSwim,this->set_time(this->get_int_pace(0,settings::isSwim)/10),this->get_int_duration(0)));
        this->overrideData.insert("swimscore",QString::number(sportValue));
        triValue = triValue + sportValue;
        sportValue = round(this->estimate_stress(settings::isBike,QString::number(this->get_int_value(2,4)),this->get_int_duration(2)));
        this->overrideData.insert("skiba_bike_score",QString::number(sportValue));
        triValue = triValue + sportValue;
        sportValue = round(this->estimate_stress(settings::isRun,this->set_time(this->get_int_pace(4,settings::isRun)),this->get_int_duration(4)));
        this->overrideData.insert("govss",QString::number(sportValue));
        triValue = triValue + sportValue;
        this->overrideData.insert("triscore",QString::number(triValue));
    }
}

void Activity::writeChangedData()
{
    this->init_jsonFile();
    this->write_actModel("INTERVALS",intModel,&intList);
    this->write_actModel("SAMPLES",sampleModel,&sampList);
    this->write_xdataModel(curr_sport,xdataModel);
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
        for(int i = 0; i < zone_count; ++i)
        {
            timeinzone = hfTimeInZone.value(levels.at(i));
            timeinzone = timeinzone + (swimHFZoneFactor.value(it.key()).at(i) * paceTimeInZone.value(it.key()));
            hfTimeInZone.insert(levels.at(i),timeinzone);
        }
    }

    hf_avg = 0;
    double worktime = this->get_timesec(ride_info.value("Duration"));

    for(QHash<QString,int>::const_iterator it = hfZoneAvg.cbegin(), end = hfZoneAvg.cend(); it != end; ++it)
    {
        hf_avg = hf_avg + ceil((it.value() * hfTimeInZone.value(it.key())) / worktime);
    }

    //Calc Total Work and Calories
    move_time = this->get_moveTime();
    double totalWork = ceil(this->calc_totalWork(tagData.value("Weight").toDouble(),hf_avg,move_time) * swim_sri);
    double totalCal = ceil((totalWork*4)/4.184);

    ride_info.insert("Total Cal",QString::number(totalCal));
    ride_info.insert("Total Work",QString::number(totalWork));
    ride_info.insert("AvgHF",QString::number(hf_avg));

    overrideData.insert("average_hr",QString::number(hf_avg));
    overrideData.insert("total_work",QString::number(totalWork));
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
        return ceil(max/(value/100));
    }
    else
    {
        return ceil(max*(value/100));
    }

    return 0;
}

int Activity::get_moveTime()
{
    int moveTime = 0;

    for(QHash<QString,int>::const_iterator it = paceTimeInZone.cbegin(), end = paceTimeInZone.cend(); it != end; ++it)
    {
        if(it.key() != breakName)
        {
            moveTime = moveTime + it.value();
        }
    }

    return moveTime;
}

int Activity::get_int_duration(int row)
{
    int duration;

    duration = intModel->data(intModel->index(row,2,QModelIndex()),Qt::DisplayRole).toInt() - intModel->data(intModel->index(row,1,QModelIndex()),Qt::DisplayRole).toInt();

    return duration;
}

double Activity::get_int_distance(int row)
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
        int_start = intModel->data(intModel->index(row,1,QModelIndex()),Qt::DisplayRole).toInt();
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
            pace = this->get_int_duration(row) / (this->get_int_distance(row)*10);
        }
    }
    else
    {
        pace = this->get_int_duration(row) / this->get_int_distance(row);
    }

    return pace;
}

int Activity::get_swim_laps(int row)
{
    return round((this->get_int_distance(row)*distFactor)/swim_track);
}

double Activity::get_int_value(int row,int col)
{
    double value = 0.0;
    int int_start,int_stop;
    int_start = intModel->data(intModel->index(row,1,QModelIndex())).toInt();
    int_stop = intModel->data(intModel->index(row,2,QModelIndex())).toInt();

    for(int i = int_start; i < int_stop; ++i)
    {
        value = value + sampleModel->data(sampleModel->index(i,col,QModelIndex())).toDouble();
    }
    value = value / (int_stop - int_start);

    return value;
}

double Activity::get_int_speed(int row)
{
    double speed,pace;

    if(isSwim)
    {
        pace = this->get_int_duration(row) / (intModel->data(intModel->index(row,3,QModelIndex())).toDouble() / distFactor);
    }
    else
    {
        pace = this->get_int_duration(row) / (intModel->data(intModel->index(row,3,QModelIndex())).toDouble()); // *10.0
    }
    speed = 3600.0 / pace;

    return speed;
}

double Activity::polish_SpeedValues(double currSpeed,double avgSpeed,double factor,bool setrand)
{
    double randfact = 0;
    if(curr_sport == settings::isRun)
    {
        randfact = ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) / (currSpeed/((factor*100)+1.0)));
    }
    if(curr_sport == settings::isBike)
    {
        randfact = ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) / (currSpeed/((factor*1000)+1.0)));
    }

    double avgLow = avgSpeed-(avgSpeed*factor);
    double avgHigh = avgSpeed+(avgSpeed*factor);

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
        if(currSpeed > avgLow && currSpeed < avgHigh)
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

    if(curr_speed == 0)
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
        }
        else
        {
            avgValues[1] = avgValues.at(1) + (static_cast<double>(this->get_timesec(intTreeModel->data(selItem.value(1)).toString()))*factor);
            avgValues[2] = avgValues.at(2) + (static_cast<double>(this->get_timesec(intTreeModel->data(selItem.value(5)).toString()))*factor);
            avgValues[3] = avgValues.at(3) + (intTreeModel->data(selItem.value(4)).toDouble()*factor);

            if(curr_sport == settings::isBike)
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
