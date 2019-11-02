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
}

void schedule::add_newSaison(QStringList saisonInfo)
{
    QDate saisonStart = QDate::fromString(saisonInfo.at(1),"dd.MM.yyyy");
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
            itemList << new QStandardItem(firstday.toString("dd.MM.yyyy"));
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

QHash<int, QString> schedule::get_weekList()
{

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

QMap<int, QStringList> schedule::get_workouts(bool dayWorkouts,QString indexString)
{
    QMap<int,QStringList> workouts;
    QStringList workItems;
    QStandardItemModel *model;
    int counter = 0;
    int indexCol = 0;

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

    QModelIndex modelIndex = this->get_modelIndex(model,indexString,indexCol);

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
    QModelIndex weekIndex = this->get_modelIndex(phaseModel,weekID,1);

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
    int ltsDays = ltsValues->value("ltsdays");
    QDomElement root_lts = stressContent.firstChildElement();
    QDomNodeList lts_list;
    QDate wDate;

    if(stressContent.hasChildNodes())
    {
        lts_list = root_lts.elementsByTagName("StressLTS");
    }

    if(!lts_list.isEmpty())
    {
        for(int i = 0; i < lts_list.count(); ++i)
        {
            QDomElement stress_element;
            QDomNode stress_node = lts_list.at(i);
            stress_element = stress_node.toElement();

            wDate = QDate::fromString(stress_element.attribute("day"),"dd.MM.yyyy");

            if(!stressValues.contains(wDate))
            {
                stressValues.insert(wDate,qMakePair(stress_element.attribute("stress").toDouble(),0.0));
            }
        }

        QDate firstWork = firstdayofweek.addDays(-ltsDays);
        QDate lastWork = stressValues.lastKey();
        int total = firstWork.daysTo(lastWork)+1;

        for(int row = 0 ; row < total; ++row)
        {
            if(!stressValues.contains(firstWork.addDays(row)))
            {
                stressValues.insert(firstWork.addDays(row),qMakePair(0,0));
            }
        }
        if(QDate::currentDate() == firstdayofweek && stressValues.firstKey() < firstdayofweek.addDays(-ltsDays)) this->save_ltsValues();
    }
    else
    {
        stressValues.insert(firstdayofweek,qMakePair(0,0));
    }
}

void schedule::save_ltsFile(double ltsDays)
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement;
    xmlRoot = xmlDoc.createElement("Stress");
    xmlDoc.appendChild(xmlRoot);

    for(QMap<QDate,QPair<double,double>>::const_iterator it = stressValues.find(firstdayofweek.addDays(-ltsDays)), end = stressValues.find(firstdayofweek); it != end; ++it)
    {
        xmlElement = xmlDoc.createElement("StressLTS");
        xmlElement.setAttribute("day",it.key().toString("dd.MM.yyyy"));
        xmlElement.setAttribute("stress",QString::number(it.value().first));
        xmlRoot.appendChild(xmlElement);
    }
    this->write_XMLFile(schedulePath,&xmlDoc,ltsFile);
}

void schedule::check_workouts(QDate date)
{
    QList<QStandardItem*> itemList;
    QString dateString = date.toString("dd.MM.yyyy");
    QModelIndex index = this->get_modelIndex(scheduleModel,calc_weekID(date),0);

    if(!index.isValid())
    {
        itemList << new QStandardItem(calc_weekID(date));
        scheduleModel->invisibleRootItem()->appendRow(itemList);
        index = this->get_modelIndex(scheduleModel,calc_weekID(date),0);
        qDebug() << "Added Week" << scheduleModel->itemFromIndex(index)->data(Qt::DisplayRole).toString();
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

void schedule::save_ltsValues()
{
    double stress = 0,currStress = 0,pastStress = 0,startLTS = 0;
    startLTS = ltsValues->value("lastlts");
    int ltsDays = ltsValues->value("ltsdays");
    double factor = (double)exp(-1.0/ltsDays);
    pastStress = startLTS;

    for(QMap<QDate,QPair<double,double>>::const_iterator it = stressValues.cbegin(), end = stressValues.find(firstdayofweek.addDays(-ltsDays)); it != end; ++it)
    {
        currStress = it.value().first;
        stress = (currStress * (1.0 - factor)) + (pastStress * factor);
        pastStress = stress;
    }
    settings::set_ltsValue("lastlts",round(pastStress));

    startLTS = ltsValues->value("laststs");
    double stsDays = ltsValues->value("stsdays");
    factor = (double)exp(-1.0/stsDays);
    pastStress = startLTS;

    for(QMap<QDate,QPair<double,double>>::const_iterator it = stressValues.find(firstdayofweek.addDays((-stsDays)-7)), end = stressValues.find(firstdayofweek.addDays((-stsDays))); it != end; ++it)
    {
        currStress = it.value().first;
        stress = (currStress * (1.0 - factor)) + (pastStress * factor);
        pastStress = stress;
    }
    settings::set_ltsValue("laststs",round(pastStress));

    this->save_ltsFile(ltsDays);

    while (stressValues.firstKey() < firstdayofweek.addDays(-ltsDays))
    {
        stressValues.remove(stressValues.firstKey());
    }

    settings::autoSave();
}

void schedule::copyWeek(QString copyFrom,QString copyTo)
{
    qDebug() << copyFrom << copyTo;
}

void schedule::deleteWeek(QString weekId)
{
    qDebug() << weekId;
}

QString schedule::get_weekPhase(QDate currDate)
{
    QStringList weekMeta = this->get_weekMeta(this->calc_weekID(currDate));
    return weekMeta.at(2);
}

void schedule::set_workoutData(QHash<QString,QMap<int,QStringList>> workoutMap)
{
    QString workoutStress;
    QPair<double,double> stressMap;
    double dayStress = 0;
    double dayDura = 0;
    QModelIndex dayIndex;
    QStandardItem *dayItem = nullptr;

    for(QHash<QString,QMap<int,QStringList>>::const_iterator it =  workoutMap.cbegin(), end = workoutMap.cend(); it != end; ++it)
    {
        dayIndex = this->get_modelIndex(scheduleModel,it.key(),0);
        dayItem = scheduleModel->itemFromIndex(dayIndex);
        scheduleModel->removeRows(0,dayItem->rowCount(),dayIndex);
        this->set_compValues(true,it.key(),it.value());

        for(QMap<int,QStringList>::const_iterator vit = it.value().cbegin(), vend = it.value().cend(); vit != vend; ++vit)
        { 
            QList<QStandardItem*> itemList;
            itemList << new QStandardItem(QString::number(vit.key()));
            dayStress = dayStress + vit.value().at(7).toDouble();
            dayDura = dayDura + get_timesec(vit.value().at(5)) / 60.0;
            for(int itemValue = 0; itemValue < vit.value().count(); ++itemValue)
            {
                itemList << new QStandardItem(vit.value().at(itemValue));
            }
            dayItem->appendRow(itemList);
        }
        stressMap.first = dayStress;
        stressMap.second = dayDura;
        this->updateStress(it.key(),stressMap,0);
    }
}

void schedule::set_compValues(bool update,QString workDate,QMap<int,QStringList> valueList)
{
    QMap<QString,QVector<double>> compSum;
    QMap<int,QStringList> compValues;

    if(update)
    {
        this->update_compValues(&compSum,&valueList);
        compMap.insert(QDate::fromString(workDate,"dd.MM.yyyy"),compSum);
    }
    else
    {

        QStringList saisons = saisonValues.keys();
        QDate compDate;
        qint64 daysLeft;
        for(int saison = 0; saison < saisons.count(); ++saison)
        {
            daysLeft = firstdayofweek.daysTo(QDate().fromString(saisonValues.value(saisons.at(0)).at(1),"dd.MM.yyyy"));
            for(int day = 0; day < daysLeft; ++day)
            {
                compDate = firstdayofweek.addDays(day);
                this->check_workouts(compDate);
                compValues = this->get_workouts(true,compDate.toString("dd.MM.yyyy"));
                this->update_compValues(&compSum,&compValues);
                compMap.insert(compDate,compSum);
                compSum.clear();
            }
        }
    }
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
            comp[0] = compSum->value(compSport).at(0) + amount;
            comp[1] = compSum->value(compSport).at(1) + this->get_timesec(it.value().at(5));
            comp[3] = compSum->value(compSport).at(3) + it.value().at(6).toDouble();
            comp[4] = compSum->value(compSport).at(4) + it.value().at(7).toDouble();
            comp[5] = compSum->value(compSport).at(5) + it.value().at(8).toDouble();
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

void schedule::set_saisonValues()
{
    QString saison;
    QStringList values;

    for(int saisons = 0; saisons < phaseModel->rowCount(); ++saisons)
    {
        for(int col = 0; col < macroTags->count(); ++col)
        {
            if(col == 0)
            {
                saison = phaseModel->data(phaseModel->index(saisons,col)).toString();
            }
            else
            {
                values << phaseModel->data(phaseModel->index(saisons,col)).toString();
            }
        }
        saisonValues.insert(saison,values);
        values.clear();
    }
    this->set_compValues(false,QString(),QMap<int,QStringList>());
}

void schedule::updateStress(QString date,QPair<double,double> stressMap,int mode)
{
    double stressValue = 0;
    double duraValue = 0;
    QDate wDate = QDate::fromString(date,"dd.MM.yyyy");

    if(mode == ADD)
    {
        if(stressValues.contains(wDate))
        {
            stressValue = stressValues.value(wDate).first;
            duraValue = stressValues.value(wDate).second;
            stressValues.insert(wDate,qMakePair(stressValue+stressMap.first,duraValue+stressMap.second));
        }
        else
        {
            stressValues.insert(wDate,qMakePair(stressMap.first,stressMap.second));
        }
    }
    else if(mode == EDIT)
    {
        stressValues.insert(wDate,qMakePair(stressMap.first,stressMap.second));
    }
    else if(mode == DEL)
    {
        stressValue = stressValues.value(wDate).first;
        duraValue = stressValues.value(wDate).second;
        if(stressValue - stressMap.first == 0.0)
        {
            stressValues.insert(wDate,qMakePair(0,0));
        }
        else
        {
            stressValues.insert(wDate,qMakePair(stressValue -stressMap.first,duraValue - stressMap.second));
        }
    }

    isUpdated = true;
}
