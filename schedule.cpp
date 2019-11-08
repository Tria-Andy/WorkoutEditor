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
    macroTags = settings::getHeaderMap("macro");
    workoutTags = settings::getHeaderMap("workout");
    compTags = settings::getHeaderMap("comp");
    phaseTags = settings::getHeaderMap("micro");
    sportTags = settings::get_listValues("Sportuse");
    qDebug() << "schedule" << generalValues->value("dateformat");
    schedulePath = gcValues->value("schedule");
    //scheduleFile = "workouts_schedule.xml";
    scheduleFile = "scheduleTest.xml";
    scheduleModel = new QStandardItemModel();
    scheduleModel->setColumnCount(workoutTags->count());
    //phaseFile = "phase_content.xml";
    phaseFile = "phaseTest.xml";
    phaseModel = new QStandardItemModel();
    phaseModel->setColumnCount(compTags->count());
    ltsFile = "longtermstress.xml";

    if(!schedulePath.isEmpty())
    {
        //Schedule
        this->check_File(schedulePath,scheduleFile);
        xmlList = this->load_XMLFile(schedulePath,scheduleFile).firstChildElement().elementsByTagName("week");
        this->fill_treeModel(&xmlList,scheduleModel);

        //Seasion - Phase
        this->check_File(schedulePath,phaseFile);
        xmlList = this->load_XMLFile(schedulePath,phaseFile).firstChildElement().elementsByTagName("macro");
        this->fill_treeModel(&xmlList,phaseModel);

        //Last LTS
        this->read_ltsFile(this->load_XMLFile(schedulePath,ltsFile));

        this->set_saisonValues();
    }
    isUpdated = false;
}

enum {ADD,EDIT,COPY,DEL};
enum {SAISON,SCHEDULE};

void schedule::freeMem()
{
    delete saisonsModel;
    delete phaseModel;
    delete contestModel;
}

void schedule::filter_schedule(QString filterValue, int col, bool fixed)
{
    qDebug() << filterValue << col << fixed;
}

void schedule::save_workouts(bool saveModel)
{
    if(saveModel == SAISON)
    {
        this->read_treeModel(phaseModel,"saisons",phaseFile);
    }

    if(saveModel == SCHEDULE)
    {
        this->read_treeModel(scheduleModel,"schedule",scheduleFile);
    }
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

    QList<QStandardItem*> itemList;
    for(int info = 0; info < saisonInfo.count(); ++info)
    {
        itemList << new QStandardItem(saisonInfo.at(info));
    }
    phaseModel->invisibleRootItem()->appendRow(itemList);

    QStandardItem *macroItem = itemList.at(0);
    itemList.clear();

    for(int meso = 0; meso < phaseList.count(); ++meso)
    {
        itemList << new QStandardItem(QString::number(meso));
        itemList << new QStandardItem(phaseList.at(meso));

        macroItem->appendRow(itemList);
        QStandardItem *mesoItem = itemList.at(0);
        itemList.clear();

        for(int micro = 0; micro < static_cast<int>(phaseCount.at(meso)); ++micro)
        {
            firstday = saisonStart.addDays(7*weekCount++);
            itemList << new QStandardItem(QString::number(weekCount));
            itemList << new QStandardItem(this->calc_weekID(firstday));
            itemList << new QStandardItem(phaseList.at(meso)+"_X");
            itemList << new QStandardItem(firstday.toString(dateFormat));
            itemList << new QStandardItem("-");
            itemList << new QStandardItem("-");

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

                microItem->appendRow(itemList);
                itemList.clear();
            }
        }
    }
    this->save_workouts(SAISON);
}

void schedule::delete_Saison(QString saisonName)
{
    qDebug() << saisonName;
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

QStandardItem* schedule::get_phaseItem(QString phase)
{
    return phaseModel->itemFromIndex(get_modelIndex(phaseModel,phase,0));
}

QMap<int, QStringList> schedule::get_workouts(bool dayWorkouts,QString indexString)
{
    QMap<int,QStringList> workouts;
    QStringList workItems;
    QStandardItemModel *model;
    int counter = 0;

    if(dayWorkouts)
    {
        model = scheduleModel;
        counter = workoutTags->count();
    }
    else
    {
        model = phaseModel;
        counter = compTags->count();
    }

    QModelIndex modelIndex = this->get_modelIndex(model,indexString,0);

    if(modelIndex.isValid())
    {
        QStandardItem *parent = model->itemFromIndex(modelIndex);

        if(parent->hasChildren())
        {
            for(int work = 0; work < parent->rowCount(); ++work)
            {
                for(int x = 1; x < counter; ++x)
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
        for(int meta = 0; meta < phaseTags->count(); ++meta)
        {
            weekMeta << phaseModel->data(weekIndex.siblingAtColumn(meta)).toString();
        }
    }
    return weekMeta;
}

void schedule::read_ltsFile(QDomDocument stressContent)
{
    QDomElement root_lts = stressContent.firstChildElement();
    QDomNodeList lts_list;

    QVector<double> stress(4);
    stress.fill(0);

    if(stressContent.hasChildNodes())
    {
        lts_list = root_lts.elementsByTagName("StressLTS");

        for(int i = 0; i < lts_list.count(); ++i)
        {
            QDomElement stress_element;
            QDomNode stress_node = lts_list.at(i);
            stress_element = stress_node.toElement();

            stress[0] = stress_element.attribute("stress").toDouble();
            stressMap.insert(QDate::fromString(stress_element.attribute("day"),dateFormat),stress);
        }
    }
}

void schedule::save_ltsFile(double ltsDays)
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement;
    xmlRoot = xmlDoc.createElement("Stress");
    xmlDoc.appendChild(xmlRoot);
    QStringList *stressheader = settings::getHeaderMap("stresslts");


    for(QMap<QDate,QVector<double>>::const_iterator it = stressMap.find(firstdayofweek.addDays(-ltsDays)), end = stressMap.find(firstdayofweek); it != end; ++it)
    {
        xmlElement = xmlDoc.createElement("stresslts");

        xmlElement.setAttribute(stressheader->at(0),it.key().toString(dateFormat));
        xmlElement.setAttribute(stressheader->at(1),QString::number(it.value().at(0)));
        xmlElement.setAttribute(stressheader->at(2),QString::number(it.value().at(1)));
        xmlElement.setAttribute(stressheader->at(3),QString::number(it.value().at(2)));
        xmlElement.setAttribute(stressheader->at(4),QString::number(it.value().at(3)));

        xmlRoot.appendChild(xmlElement);
    }
    this->write_XMLFile(schedulePath,&xmlDoc,ltsFile);
}

void schedule::check_workouts(QDate date)
{
    QList<QStandardItem*> itemList;
    QString dateString = date.toString(dateFormat);
    QModelIndex index = this->get_modelIndex(scheduleModel,calc_weekID(date),0);

    if(!index.isValid())
    {
        itemList << new QStandardItem(calc_weekID(date));
        scheduleModel->invisibleRootItem()->appendRow(itemList);
        index = this->get_modelIndex(scheduleModel,calc_weekID(date),0);
    }
    itemList.clear();
    QStandardItem *weekItem = scheduleModel->itemFromIndex(index);

    index = this->get_modelIndex(scheduleModel,dateString,0);

    if(!index.isValid())
    {
        itemList << new QStandardItem(dateString);
        weekItem->appendRow(itemList);
        index = this->get_modelIndex(scheduleModel,dateString,0);
        qDebug() << "Added Day" << scheduleModel->itemFromIndex(index)->data(Qt::DisplayRole).toString() << "to" << weekItem->data(Qt::DisplayRole).toString();
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

void schedule::set_workoutData(QHash<QDate,QMap<int,QStringList>> workoutMap)
{
    QString workoutStress;
    QModelIndex dayIndex;
    QStandardItem *dayItem = nullptr;

    for(QHash<QDate,QMap<int,QStringList>>::const_iterator it =  workoutMap.cbegin(), end = workoutMap.cend(); it != end; ++it)
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
            dayItem->appendRow(itemList);
        }
    }
    isUpdated = true;
}


void schedule::copyWeek(QString copyFrom,QString copyTo)
{
    QStandardItem *weekItem = scheduleModel->itemFromIndex(get_modelIndex(scheduleModel,copyFrom,0));
    QHash<QDate,QMap<int,QStringList>> workoutMap;
    QMap<int,QStringList> workouts;
    QDate workDate = QDate::fromString(this->get_weekMeta(copyFrom).at(4),dateFormat);

    for(int day = 0; day < weekItem->rowCount();++day)
    {
        workouts = get_workouts(true,workDate.addDays(day).toString(dateFormat));
        workoutMap.insert(workDate.addDays(day),workouts);
        workouts.clear();
    }
    qDebug() << workoutMap;
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

    for(QMap<QString, QVector<double>>::const_iterator it = compValues.cbegin(), vend = compValues.cend(); it != vend; ++it)
    {
        QList<QStandardItem*> itemList;
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
            comp[1] = compSum->value(compSport).at(1) + this->get_timesec(it.value().at(5));    //Duration
            comp[3] = compSum->value(compSport).at(3) + it.value().at(6).toDouble();            //Distance
            comp[4] = compSum->value(compSport).at(4) + it.value().at(7).toDouble();            //Stress
            comp[5] = compSum->value(compSport).at(5) + it.value().at(8).toDouble();            //Work KJ
        }
        else
        {
            comp[0] = amount;
            comp[1] = this->get_timesec(it.value().at(5));
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
    double lte = static_cast<double>(exp(-1.0/ltsValues->value("ltsdays")));
    double ste = static_cast<double>(exp(-1.0/ltsValues->value("stsdays")));
    double ltsStress = 0,stsStress = 0;
    QVector<double> stressValue(4);

    stsStress = ltsValues->value("laststs");
    ltsStress = ltsValues->value("lastlts");

    for(QMap<QDate,QVector<double>>::const_iterator it = stressMap.cbegin(), end = stressMap.cend(); it != end; ++it)
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
        QVector<double> sportValues(4);
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
                                for(int values = 1; values < compTags->count(); ++values)
                                {
                                    if(values == 1)
                                    {
                                        sportKey = saisonItem->child(phase,0)->child(week,0)->child(comp,values)->data(Qt::DisplayRole).toString();
                                    }
                                    else if(values == 4)
                                    {
                                        sportValues[values-2] = get_timesec(saisonItem->child(phase,0)->child(week,0)->child(comp,values)->data(Qt::DisplayRole).toString())*60.0;
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
