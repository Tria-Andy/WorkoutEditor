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

#include "schedule.h"

schedule::schedule()
{
    macroTags = settings::get_xmlMapping("macro");
    scheduleTags = settings::get_xmlMapping("schedule");
    saisonTags = settings::get_xmlMapping("saisons");
    sportTags = settings::get_listValues("Sportuse");
    firstdayofweek = settings::firstDayofWeek;
    dateFormat = settings::get_format("dateformat");
    longTime = settings::get_format("longtime");
    shortTime = settings::get_format("shorttime");
    doubleValues = settings::getdoubleMapPointer(settings::dMap::Double);
    gcValues = settings::getStringMapPointer(settings::stingMap::GC);

    isUpdated = false;
}

enum {ADD,EDIT,COPY,DEL};
enum {SAISON,SCHEDULE};

void schedule::init_scheduleData()
{
    if(!gcValues->value("schedule").isEmpty())
    {
        this->set_stressMap();
        this->remove_WeekofPast(firstdayofweek.addDays(-7));
        this->set_saisonValues();
    }
}

void schedule::save_workouts(bool saveModel)
{
    this->save_data(saveModel);
    if(saveModel == SCHEDULE) this->save_ltsFile();
    isUpdated = false;
}

void schedule::add_newSaison(QStringList saisonInfo)
{
    QDate saisonStart = QDate::fromString(saisonInfo.at(1),dateFormat);
    QDate firstday;
    int weekCount = 0;
    QVector<double> phaseCount = settings::doubleVector.value("Phaseweeks");
    QStringList phaseList = settings::get_listValues("Phase");
    QStringList sportUseList = settings::get_listValues("Sportuse");
    sportUseList.removeLast();

    QList<QStandardItem*> itemList;
    for(int info = 0; info < saisonInfo.count(); ++info)
    {
        itemList << new QStandardItem(saisonInfo.at(info));
    }
    itemList.at(0)->setData(saisonTags->at(0),Qt::AccessibleTextRole);
    phaseModel->invisibleRootItem()->appendRow(itemList);

    QStandardItem *macroItem = itemList.at(0);
    itemList.clear();

    for(int meso = 0; meso < phaseList.count(); ++meso)
    {
        itemList << new QStandardItem(phaseList.at(meso));
        itemList << new QStandardItem(QString::number(meso));

        itemList.at(0)->setData(saisonTags->at(1),Qt::AccessibleTextRole);
        macroItem->appendRow(itemList);
        QStandardItem *mesoItem = itemList.at(0);
        itemList.clear();

        for(int micro = 0; micro < static_cast<int>(phaseCount.at(meso)); ++micro)
        {
            firstday = saisonStart.addDays(7*weekCount++);
            itemList << new QStandardItem(this->calc_weekID(firstday));
            itemList << new QStandardItem(QString::number(weekCount));
            itemList << new QStandardItem(phaseList.at(meso)+"_X");
            itemList << new QStandardItem(firstday.toString(dateFormat));
            itemList << new QStandardItem("-");
            itemList << new QStandardItem("-");

            itemList.at(0)->setData(saisonTags->at(2),Qt::AccessibleTextRole);
            mesoItem->appendRow(itemList);
            QStandardItem *microItem = itemList.at(0);
            itemList.clear();

            for(int comp = 0; comp < sportUseList.count(); ++comp)
            {
                itemList << new QStandardItem(QString::number(comp));
                itemList << new QStandardItem(sportUseList.at(comp));
                itemList << new QStandardItem("0");
                itemList << new QStandardItem("0.0");
                itemList << new QStandardItem("00:00");
                itemList << new QStandardItem("0");

                itemList.at(0)->setData(saisonTags->at(3),Qt::AccessibleTextRole);
                microItem->appendRow(itemList);
                itemList.clear();
            }
        }
    }
    this->set_saisonValues();
    this->save_workouts(SAISON);
}

void schedule::delete_Saison(QString saisonName)
{
   QStandardItem *saisonItem = phaseModel->itemFromIndex(get_modelIndex(phaseModel,saisonName,0));
   phaseModel->removeRows(0,saisonItem->rowCount());
   this->save_workouts(SAISON);
}

QModelIndex schedule::get_modelIndex(QStandardItemModel *model,QString searchString, int col)
{
    QList<QStandardItem*> list;

    list = model->findItems(searchString,Qt::MatchExactly | Qt::MatchRecursive,col);

    if(list.count() > 0)
    {
        return model->indexFromItem(list.at(0));
    }
    else
    {
        return QModelIndex();
    }
}

void schedule::update_linkedWorkouts(QDate workDate, QString workID,int pos,bool add)
{
    QMap<QDate,int> linkMap = linkedWorkouts.value(workID);

    if(add)
    {
        linkMap.insert(workDate,pos);
    }
    else
    {
        linkMap.remove(workDate);
    }
    linkedWorkouts.insert(workID,linkMap);
}

QStandardItem* schedule::get_phaseItem(QString phase)
{
    return phaseModel->itemFromIndex(get_modelIndex(phaseModel,phase,0));
}

QMap<int, QStringList> schedule::get_workouts(int dataSource,QString indexString)
{
    QMap<int,QStringList> workouts;
    QStringList workItems;

    QStandardItemModel *model = nullptr;
    if(dataSource == SAISON) model = phaseModel;
    else if (dataSource == SCHEDULE) model = scheduleModel;

    QModelIndex modelIndex = this->get_modelIndex(model,indexString,0);

    if(modelIndex.isValid())
    {
        QStandardItem *parent = model->itemFromIndex(modelIndex);

        if(parent->hasChildren())
        {
            for(int work = 0; work < parent->rowCount(); ++work)
            {
                for(int x = 1; x < model->columnCount(); ++x)
                {
                   workItems << model->data(model->index(work,x,modelIndex)).toString();
                }
                workouts.insert(work,workItems);
                workItems.clear();
            }
        }
    }
    return workouts;
}

QStringList schedule::get_weekMeta(QString weekID)
{
    QStringList weekMeta;
    QModelIndex weekIndex = this->get_modelIndex(phaseModel,weekID,0);

    if(weekIndex.isValid())
    {
        for(int meta = 0; meta < phaseModel->columnCount(); ++meta)
        {
            weekMeta << phaseModel->data(weekIndex.siblingAtColumn(meta)).toString();
        }
    }
    return weekMeta;
}

void schedule::save_ltsFile()
{
    QMap<int,QStringList> saveList;
    QStringList mapList;
    int counter = 0;
    int ltsDays = settings::get_intValue("ltsdays")+1;

    for(QMap<QDate,QVector<double>>::const_iterator it = stressMap.find(firstdayofweek.addDays(-ltsDays)), end = stressMap.find(firstdayofweek.addDays(7)); it != end; ++it)
    {
        mapList << it.key().toString(dateFormat);
        for(int value = 0; value < it.value().count(); ++value)
        {
            mapList << QString::number(set_doubleValue(it.value().at(value),false));
        }
        saveList.insert(counter++,mapList);
        mapList.clear();
    }
    this->listMap_toXml(&saveList,fileMap->value("stressfile"));
}

void schedule::add_contest(QString saison,QDate contestDate, QStringList contestValues)
{
    QMap<QDate,QStringList> contest = contestMap.value(saison);
    contest.insert(contestDate,contestValues);
    contestMap.insert(saison,contest);
}

void schedule::remove_contest(QString saison, QDate contestDate)
{
    QMap<QDate,QStringList> contest = contestMap.value(saison);
    contest.remove(contestDate);
    contestMap.insert(saison,contest);

    QStandardItem *dayItem = scheduleModel->itemFromIndex(get_modelIndex(scheduleModel,contestDate.toString(dateFormat),0));
    scheduleModel->removeRows(0,dayItem->rowCount(),dayItem->index());
}


void schedule::check_workouts(QDate date)
{
    QList<QStandardItem*> itemList;
    QString dateString = date.toString(dateFormat);
    QModelIndex index = this->get_modelIndex(scheduleModel,calc_weekID(date),0);


    //Check if Week and Date available
    if(!index.isValid())
    {
        itemList << new QStandardItem(calc_weekID(date));
        scheduleModel->invisibleRootItem()->appendRow(itemList);
        index = this->get_modelIndex(scheduleModel,calc_weekID(date),0);
    }
    itemList.clear();
    QStandardItem *item = scheduleModel->itemFromIndex(index);

    index = this->get_modelIndex(scheduleModel,dateString,0);

    if(!index.isValid())
    {
        itemList << new QStandardItem(dateString);
        item->appendRow(itemList);
    }

    //Read Contest/Races
    index = this->get_modelIndex(scheduleModel,dateString,0);
    QStandardItem *dayItem = scheduleModel->itemFromIndex(index);

    if(dayItem->hasChildren())
    {
        for(int work = 0; work < dayItem->rowCount(); ++work)
        {
            if(dayItem->child(work,3)->data(Qt::DisplayRole).toString() == "Race")
            {
                QStringList contestValues;
                index = get_modelIndex(phaseModel,calc_weekID(date),0);
                item = get_phaseItem(index.parent().parent().data(Qt::DisplayRole).toString());

                for(int value = 0; value < scheduleModel->columnCount(); ++value)
                {
                    contestValues << dayItem->child(work,value)->data(Qt::DisplayRole).toString();
                }
                add_contest(item->data(Qt::DisplayRole).toString(),date,contestValues);
            }

            this->update_linkedWorkouts(date,dayItem->child(work,11)->data(Qt::DisplayRole).toString(),dayItem->child(work,0)->data(Qt::DisplayRole).toInt(),true);
        }
    }
}

QString schedule::get_weekPhase(QDate currDate)
{
    QStringList weekMeta = this->get_weekMeta(this->calc_weekID(currDate));
    return weekMeta.at(2);
}

QStringList schedule::get_weekList()
{
    QStringList weekList = compWeekMap.keys();
    std::sort(weekList.begin(),weekList.end());
    return weekList;
}

void schedule::remove_WeekofPast(QDate dayOfWeek)
{
    QModelIndex weekIndex = get_modelIndex(scheduleModel,calc_weekID(dayOfWeek),0);

    if(weekIndex.isValid())
    {
        QStandardItem *weekItem = scheduleModel->itemFromIndex(weekIndex);
        scheduleModel->removeRows(0,weekItem->rowCount(),weekIndex);
        scheduleModel->removeRow(weekIndex.row(),weekIndex.parent());
    }
}

void schedule::set_workoutData()
{
    QString workoutStress;
    QModelIndex dayIndex;
    QStandardItem *dayItem = nullptr;

    for(QHash<QDate,QMap<int,QStringList>>::const_iterator it =  workoutUpdates.cbegin(), end = workoutUpdates.cend(); it != end; ++it)
    {
        dayIndex = this->get_modelIndex(scheduleModel,it.key().toString(dateFormat),0);
        dayItem = scheduleModel->itemFromIndex(dayIndex);
        scheduleModel->removeRows(0,dayItem->rowCount(),dayIndex);
        this->set_compValues(true,it.key(),it.value());

        for(QMap<int,QStringList>::const_iterator vit = it.value().cbegin(), vend = it.value().cend(); vit != vend; ++vit)
        { 
            QList<QStandardItem*> itemList;
            itemList << new QStandardItem(QString::number(vit.key()));
            for(int itemValue = 0; itemValue < vit.value().count(); ++itemValue)
            {
                itemList << new QStandardItem(vit.value().at(itemValue));
            }
            itemList.at(0)->setData(scheduleTags->at(2),Qt::AccessibleTextRole);
            this->update_linkedWorkouts(it.key(),vit.value().last(),vit.key(),true);
            dayItem->appendRow(itemList);
        }
    }

    isUpdated = true;
}


void schedule::copyWeek(QString copyFrom,QString copyTo)
{
    QStandardItem *fromItem = scheduleModel->itemFromIndex(get_modelIndex(scheduleModel,copyFrom,0));

    QDate fromDate = QDate::fromString(this->get_weekMeta(copyFrom).at(3),dateFormat);
    QDate toDate = QDate::fromString(this->get_weekMeta(copyTo).at(3),dateFormat);

    for(int day = 0; day < fromItem->rowCount();++day)
    {
        workoutUpdates.insert(toDate.addDays(day),get_workouts(true,fromDate.addDays(day).toString(dateFormat)));
    }
    this->set_workoutData();
}

void schedule::clearWeek(QString weekID)
{
    QStandardItem *clearItem = scheduleModel->itemFromIndex(get_modelIndex(scheduleModel,weekID,0));

    QDate date = QDate::fromString(this->get_weekMeta(weekID).at(3),dateFormat);

    for(int day = 0; day < clearItem->rowCount();++day)
    {
        workoutUpdates.insert(date.addDays(day),QMap<int,QStringList>());
    }
    this->set_workoutData();
}

void schedule::set_weekCompValues(QStringList weekMeta,QMap<QString, QVector<double> > compValues)
{
    QModelIndex weekIndex = this->get_modelIndex(phaseModel,weekMeta.at(0),0);
    QStandardItem *weekItem = phaseModel->itemFromIndex(weekIndex);
    compWeekMap.insert(weekMeta.at(0),compValues);

    phaseModel->removeRows(0,weekItem->rowCount(),weekIndex);

    for(int meta = 0; meta < weekMeta.count(); ++meta)
    {
       phaseModel->setData(weekIndex.siblingAtColumn(meta),weekMeta.at(meta));
    }

    int sportID = 0;
    for(QMap<QString, QVector<double>>::const_iterator it = compValues.cbegin(), vend = compValues.cend(); it != vend; ++it)
    {
        QList<QStandardItem*> itemList;
        itemList << new QStandardItem(QString::number(sportID++));
        itemList << new QStandardItem(it.key());
        for(int itemValue = 0; itemValue < it.value().count(); ++itemValue)
        {
            itemList << new QStandardItem(QString::number(it.value().at(itemValue)));
        }
        weekItem->appendRow(itemList);
    }
    isUpdated = true;
}

void schedule::set_compValues(bool update,QDate workDate,QMap<int,QStringList> valueList)
{
    QMap<QString,QVector<double>> compSum;
    QMap<int,QStringList> compValues;
    QVector<double> stressValue(4);
    stressValue.fill(0);

    if(update)
    {
        this->update_compValues(&compSum,&valueList);
        compMap.insert(workDate,compSum);
        compSum.clear();

        stressValue[1] = stressMap.value(workDate).at(1);
        stressValue[2] = stressMap.value(workDate).at(2);
        compSum = compMap.value(workDate);

        for(QMap<QString,QVector<double>>::const_iterator it = compSum.cbegin(), end = compSum.cend(); it != end; ++it)
        {
            stressValue[0] = stressValue.at(0) + it.value().at(4);
            stressValue[3] = stressValue.at(3) + it.value().at(1);
        }
        stressMap.insert(workDate,stressValue);
    }
    else
    {
        QStringList saisons = saisonValues.keys();
        QDate compDate;
        qint64 daysLeft;
        for(int saison = 0; saison < saisons.count(); ++saison)
        {
            daysLeft = firstdayofweek.daysTo(QDate().fromString(saisonValues.value(saisons.at(0)).at(1),dateFormat));
            for(int day = 0; day < daysLeft; ++day)
            {
                compDate = firstdayofweek.addDays(day);
                this->check_workouts(compDate);
                compValues = this->get_workouts(true,compDate.toString(dateFormat));
                this->update_compValues(&compSum,&compValues);
                compMap.insert(compDate,compSum);
                compSum.clear();

                stressValue.fill(0);
                compSum = compMap.value(compDate);
                for(QMap<QString,QVector<double>>::const_iterator it = compSum.cbegin(), end = compSum.cend(); it != end; ++it)
                {
                    stressValue[0] = stressValue.at(0) + it.value().at(4);
                    stressValue[3] = stressValue.at(3) + it.value().at(1);
                }
                compSum.clear();
                stressMap.insert(compDate,stressValue);
            }
        }
    }

    this->recalc_stressValues();
}

void schedule::update_compValues(QMap<QString,QVector<double>> *compSum,QMap<int,QStringList> *compValues)
{
    QVector<double> comp(settings::getHeaderMap("summery")->count());
    comp.fill(0);
    QString compSport;
    int amount = 1;

    for(QMap<int,QStringList>::const_iterator it = compValues->cbegin(), end = compValues->cend(); it != end; ++it)
    {
        compSport = it.value().at(1);

        if(compSum->contains(compSport))
        {
            comp[0] = compSum->value(compSport).at(0) + amount;                                 //Workouts
            comp[1] = compSum->value(compSport).at(1) + it.value().at(5).toDouble();            //Duration
            comp[3] = compSum->value(compSport).at(3) + it.value().at(6).toDouble();            //Distance
            comp[4] = compSum->value(compSport).at(4) + it.value().at(7).toDouble();            //Stress
            comp[5] = compSum->value(compSport).at(5) + it.value().at(8).toDouble();            //Work KJ
        }
        else
        {
            comp[0] = amount;
            comp[1] = it.value().at(5).toDouble();
            comp[3] = it.value().at(6).toDouble();
            comp[4] = it.value().at(7).toDouble();
            comp[5] = it.value().at(8).toDouble();
        }

        compSum->insert(compSport,comp);
        comp.fill(0);
    }
}

void schedule::recalc_stressValues()
{
    int ltsDays = settings::get_intValue("ltsdays");
    int stsDays = settings::get_intValue("stsdays");

    double lte = static_cast<double>(exp(-1.0/ltsDays));
    double ste = static_cast<double>(exp(-1.0/stsDays));
    double ltsStress = 0,stsStress = 0;
    QVector<double> stressValue(4);

    stsStress = stressMap.value(firstdayofweek.addDays(-stsDays-1)).at(1);
    ltsStress = stressMap.value(firstdayofweek.addDays(-ltsDays-1)).at(2);

    for(QMap<QDate,QVector<double>>::const_iterator it = stressMap.cbegin(), end = stressMap.cend(); it != end; ++it)
    {

        if(it.key() != firstdayofweek.addDays(-ltsDays-1))
        {
            stressValue[0] = it.value().at(0);
            stressValue[1] = (it.value().at(0) * (1.0 - ste)) + (stsStress * ste);
            stressValue[2] = (it.value().at(0) * (1.0 - lte)) + (ltsStress * lte);
            stressValue[3] = it.value().at(3);
            stsStress = stressValue[1];
            ltsStress = stressValue[2];
            stressMap.insert(it.key(),stressValue);
        }
    }
}

void schedule::set_stressMap()
{
    QVector<double> stressValue(4,0);

    for(QMap<int,QStringList>::const_iterator it = mapList.cbegin(), end = mapList.cend(); it != end; ++it)
    {
        stressValue[0] = it.value().at(1).toDouble();
        stressValue[1] = it.value().at(2).toDouble();
        stressValue[2] = it.value().at(3).toDouble();
        stressValue[3] = it.value().at(4).toDouble();
        stressMap.insert(QDate::fromString(it.value().at(0),dateFormat),stressValue);
        stressValue.fill(0);
    }
}

void schedule::set_saisonValues()
{
    QString mapKey;
    QStringList mapValues;

    for(int saisons = 0; saisons < phaseModel->rowCount(); ++saisons)
    {
        for(int col = 0; col < macroTags->count(); ++col)
        {
            if(col == 0)
            {
                mapKey = phaseModel->data(phaseModel->index(saisons,col)).toString();
            }
            else
            {
                mapValues << phaseModel->data(phaseModel->index(saisons,col)).toString();
            }
        }
        saisonValues.insert(mapKey,mapValues);
        mapValues.clear();

        //Fill compWeekMap
        QStandardItem *saisonItem;
        saisonItem = phaseModel->item(saisons,0);
        QVector<double> sportValues(5,0);
        QString sportKey;
        QMap<QString,QVector<double>> compValues;

        if(saisonItem->hasChildren())
        {
            for(int phase = 0; phase < saisonItem->rowCount(); ++phase) //Phases
            {
                if(saisonItem->child(phase,0)->hasChildren())   //Check Weeks within Phases
                {
                    for(int week = 0; week < saisonItem->child(phase,0)->rowCount(); ++week) //Weeks
                    {
                        mapKey = saisonItem->child(phase,0)->child(week,0)->data(Qt::DisplayRole).toString();

                        if(saisonItem->child(phase,0)->child(week,0)->hasChildren())    //Check Entries within Weeks
                        {
                            for(int comp = 0; comp < saisonItem->child(phase,0)->child(week,0)->rowCount(); ++comp)
                            {
                                for(int values = 1; values < phaseModel->columnCount(); ++values)
                                {
                                    if(values == 1)
                                    {
                                        sportKey = saisonItem->child(phase,0)->child(week,0)->child(comp,values)->data(Qt::DisplayRole).toString();
                                    }
                                    else
                                    {
                                        sportValues[values-2] = saisonItem->child(phase,0)->child(week,0)->child(comp,values)->data(Qt::DisplayRole).toDouble();
                                    }
                                }
                                compValues.insert(sportKey,sportValues);
                            }
                        }
                        compWeekMap.insert(mapKey,compValues);
                    }
                }
            }
        }
    }
    this->set_compValues(false,QDate(),QMap<int,QStringList>());
}
