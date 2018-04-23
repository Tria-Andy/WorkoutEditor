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
    workoutTags << "week" << "date" << "time" << "sport" << "code" << "title" << "duration" << "distance" << "stress" << "kj" << "stdid";
    metaTags << "saison" << "id" << "week" << "name" << "fdw" << "content" << "goal";
    //metaTags << "id" << "saison" << "week" << "weekid" << "phase" << "fdw";
    contentTags << "id" << "week";
    for(int i = 0; i < settings::get_listValues("Sportuse").count();++i)
    {
        workout_sport = settings::get_listValues("Sportuse").at(i);
        contentTags << workout_sport.toLower();
    }
    contentTags << "summery";
    workout_sport.clear();
    firstdayofweek = QDate::currentDate().addDays(1 - QDate::currentDate().dayOfWeek());
    schedulePath = gcValues->value("schedule");
    workoutFile = "workout_schedule.xml";
    metaFile = "workout_phase_meta.xml";
    contentFile = "workout_phase_content.xml";
    ltsFile = "longtermstress.xml";

    if(!schedulePath.isEmpty())
    {
        this->check_File(schedulePath,workoutFile);
        this->read_dayWorkouts(this->load_XMLFile(schedulePath,workoutFile));
        this->check_File(schedulePath,metaFile);
        this->check_File(schedulePath,contentFile);
        this->read_weekPlan(this->load_XMLFile(schedulePath,metaFile),this->load_XMLFile(schedulePath,contentFile));
        this->check_File(schedulePath,ltsFile);
        this->read_ltsFile(this->load_XMLFile(schedulePath,ltsFile));
    }
    isUpdated = false;
}

enum {ADD,EDIT,COPY,DEL};

void schedule::freeMem()
{
    delete saisonsModel;
    delete contestModel;
    delete scheduleProxy;
    delete week_meta;
    delete week_content;
}

void schedule::read_dayWorkouts(QDomDocument workouts)
{
    QPair<double,double> tempValues;

    QDomElement root_workouts = workouts.firstChildElement();
    QDomNodeList workout_list;
    if(workouts.hasChildNodes())
    {
        workout_list = root_workouts.elementsByTagName("workout");
    }

    workout_schedule = new QStandardItemModel(workout_list.count(),workoutTags.count());
    QDate workDate;

    if(!workout_list.isEmpty())
    {
        for(int i = 0; i < workout_list.count(); ++i)
        {
            QDomElement workout_element;
            QDomNode workout_node = workout_list.at(i);

            workout_element = workout_node.toElement();
            for(int col = 0; col < workout_schedule->columnCount(); ++col)
            {
                workout_schedule->setData(workout_schedule->index(i,col,QModelIndex()),workout_element.attribute(workoutTags.at(col)));
            }
            workDate = QDate::fromString(workout_element.attribute("date"),"dd.MM.yyyy");
            tempValues.first = workout_element.attribute("stress").toInt();
            tempValues.second = this->get_timesec(workout_element.attribute("duration"))/60.0;

            if(stressValues.contains(workDate))
            {
                tempValues.first = tempValues.first + stressValues.value(workDate).first;
                tempValues.second = tempValues.second + stressValues.value(workDate).second;
                stressValues.insert(workDate,tempValues);
            }
            else
            {
                stressValues.insert(workDate,tempValues);
            }
        }
        workout_schedule->sort(2);
    }
    scheduleProxy = new QSortFilterProxyModel();
    scheduleProxy->setSourceModel(workout_schedule);
}

void schedule::save_dayWorkouts()
{
        QModelIndex index;
        QDomDocument xmlDoc;
        QDomElement xmlRoot,xmlElement;
        xmlRoot = xmlDoc.createElement("workouts");
        xmlDoc.appendChild(xmlRoot);

        for(int i = 0; i < workout_schedule->rowCount(); ++i)
        {
            index = workout_schedule->index(i,1,QModelIndex());
            if(QDate::fromString(workout_schedule->data(index,Qt::DisplayRole).toString(),"dd.MM.yyyy") < firstdayofweek ) continue;

            xmlElement = xmlDoc.createElement("workout");

            for(int x = 0; x < workout_schedule->columnCount(); ++x)
            {
                index = workout_schedule->index(i,x,QModelIndex());
                xmlElement.setAttribute(workoutTags.at(x),workout_schedule->data(index,Qt::DisplayRole).toString());
            }
            xmlRoot.appendChild(xmlElement);
        }
        this->write_XMLFile(schedulePath,&xmlDoc,workoutFile);
}

void schedule::read_weekPlan(QDomDocument weekMeta, QDomDocument weekContent)
{
    QString contentString;
    QString currentSaison = this->get_currSaison();
    QDate startDate = this->get_saisonInfo(currentSaison,"start").toDate();
    int saisonWeeks = this->get_saisonInfo(currentSaison,"weeks").toInt();

    QDomElement root_meta = weekMeta.firstChildElement();
    QDomElement root_content = weekContent.firstChildElement();
    QDomNodeList meta_list,content_list;
    meta_list = root_meta.elementsByTagName("phase");
    content_list = root_content.elementsByTagName("content");

    week_meta = new QStandardItemModel(meta_list.count(),metaTags.count());
    metaProxy = new QSortFilterProxyModel();
    metaProxy->setSourceModel(week_meta);
    week_content = new QStandardItemModel(content_list.count(),contentTags.count());
    contentProxy = new QSortFilterProxyModel();
    contentProxy->setSourceModel(week_content);

    //fill week_meta
    if(meta_list.count() == 0)
    {
        QString weekid;
        QString noPhase = generalValues->value("empty");

        week_meta->setRowCount(saisonWeeks);
        for(int week = 0,id = 1; week < saisonWeeks; ++week,++id)
        {
            weekid = QString::number(startDate.addDays(week*7).weekNumber()) +"_"+ QString::number(startDate.addDays(week*7).year());
            week_meta->setData(week_meta->index(week,0,QModelIndex()),currentSaison);
            week_meta->setData(week_meta->index(week,1,QModelIndex()),id);
            week_meta->setData(week_meta->index(week,2,QModelIndex()),weekid);
            week_meta->setData(week_meta->index(week,3,QModelIndex()),noPhase);
            week_meta->setData(week_meta->index(week,4,QModelIndex()),startDate.addDays(week*7).toString("dd.MM.yyyy"));
        }
        this->save_weekPlan();
    }
    else
    {
        for(int i = 0; i < meta_list.count(); ++i)
        {
            QDomElement meta_element;
            QDomNode meta_node = meta_list.at(i);
            meta_element = meta_node.toElement();
            for(int col = 0; col < week_meta->columnCount(); ++col)
            {
                if(col == 1)
                {
                    week_meta->setData(week_meta->index(i,col,QModelIndex()),meta_element.attribute(metaTags.at(col)).toInt());
                }
                else
                {
                    week_meta->setData(week_meta->index(i,col,QModelIndex()),meta_element.attribute(metaTags.at(col)));
                }
            }
        }
    }
    week_meta->sort(1);

    //fill week_content
    if(content_list.count() == 0)
    {
        QString weekid;
        week_content->setRowCount(saisonWeeks);

        for(int week = 0,id = 1; week < saisonWeeks; ++week,++id)
        {
            weekid = QString::number(startDate.addDays(week*7).weekNumber()) +"_"+ QString::number(startDate.addDays(week*7).year());
            week_content->setData(week_content->index(week,0,QModelIndex()),id);
            week_content->setData(week_content->index(week,1,QModelIndex()),weekid);
            for(int col = 2; col < contentTags.count(); ++col)
            {
                week_content->setData(week_content->index(week,col,QModelIndex()),"0-0-00:00-0");
            }
        }
        this->save_weekPlan();
    }
    else
    {
        for(int i = 0; i < content_list.count(); ++i)
        {
            QDomElement content_element;
            QDomNode content_node = content_list.at(i);
            content_element = content_node.toElement();
            for(int col = 0; col < week_content->columnCount(); ++col)
            {
                if(col == 0)
                {
                    week_content->setData(week_content->index(i,col,QModelIndex()),content_element.attribute(contentTags.at(col)).toInt());
                }
                else
                {
                    contentString = content_element.attribute(contentTags.at(col));
                    if(!contentString.isEmpty())
                    {
                        week_content->setData(week_content->index(i,col,QModelIndex()),contentString);
                    }
                    else
                    {
                        week_content->setData(week_content->index(i,col,QModelIndex()),"0-0-00:00-0");
                    }
                }
            }
        }
    }
}

void schedule::save_weekPlan()
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement;
    xmlRoot = xmlDoc.createElement("phases");
    xmlDoc.appendChild(xmlRoot);

    for(int i = 0; i < week_meta->rowCount(); ++i)
    {
        xmlElement = xmlDoc.createElement("phase");
        for(int col = 0; col < week_meta->columnCount(); ++col)
        {
            xmlElement.setAttribute(metaTags.at(col) ,week_meta->data(week_meta->index(i,col,QModelIndex())).toString());
        }
        xmlRoot.appendChild(xmlElement);
    }
    this->write_XMLFile(schedulePath,&xmlDoc,metaFile);
    xmlDoc.clear();

    xmlRoot = xmlDoc.createElement("contents");
    xmlDoc.appendChild(xmlRoot);

    for(int i = 0; i < week_content->rowCount(); ++i)
    {
        xmlElement = xmlDoc.createElement("content");
        for(int col = 0; col < week_content->columnCount(); ++col)
        {
            xmlElement.setAttribute(contentTags.at(col) ,week_content->data(week_content->index(i,col,QModelIndex())).toString());
        }
        xmlRoot.appendChild(xmlElement);
    }
    this->write_XMLFile(schedulePath,&xmlDoc,contentFile);
}

void schedule::add_newSaison(QString saisonName)
{
    int saisonWeeks = this->get_saisonInfo(saisonName,"weeks").toInt();
    QDate startDay = this->get_saisonInfo(saisonName,"start").toDate();
    QDate firstDay;
    QString phase = generalValues->value("empty");
    QString weekID;
    QString emptyContent = "0-0-00:00-0";
    int rowCount;

    for(int i = 0; i < saisonWeeks; ++i)
    {
        firstDay = startDay.addDays(i*7);
        weekID = QString::number(firstDay.weekNumber())+"_"+QString::number(firstDay.year());
        rowCount = week_meta->rowCount();

        week_meta->insertRow(rowCount,QModelIndex());
        week_meta->setData(week_meta->index(rowCount,0),saisonName);
        week_meta->setData(week_meta->index(rowCount,1),i+1);
        week_meta->setData(week_meta->index(rowCount,2),weekID);
        week_meta->setData(week_meta->index(rowCount,3),phase);
        week_meta->setData(week_meta->index(rowCount,4),firstDay.toString("dd.MM.yyyy"));

        week_content->insertRow(rowCount,QModelIndex());
        week_content->setData(week_content->index(rowCount,0),i+1);
        week_content->setData(week_content->index(rowCount,1),weekID);

        for(int col = 2; col < contentTags.count(); ++col)
        {
            week_content->setData(week_content->index(rowCount,col),emptyContent);
        }
    }
}

void schedule::delete_Saison(QString saisonName)
{
   metaProxy->invalidate();
   week_meta->sort(1);
   metaProxy->setFilterRegExp("\\b"+saisonName+"\\b");
   metaProxy->setFilterKeyColumn(0);
   QString weekID;

   for(int i = 0; i < metaProxy->rowCount(); ++i)
   {
       weekID = metaProxy->data(metaProxy->index(i,2)).toString();
       contentProxy->setFilterRegExp("\\b"+weekID+"\\b");
       contentProxy->setFilterKeyColumn(1);
       contentProxy->removeRow(0);
       contentProxy->invalidate();
   }
   metaProxy->removeRows(0,metaProxy->rowCount());
}

QHash<int, QString> schedule::get_weekList()
{
    metaProxy->sort(1);

    return weekList;
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

int schedule::check_workouts(QDate date)
{
    int workCount;
    scheduleProxy->setSourceModel(workout_schedule);
    scheduleProxy->setFilterRegExp("\\b"+date.toString("dd.MM.yyyy")+"\\");
    scheduleProxy->setFilterKeyColumn(1);
    workCount = scheduleProxy->rowCount();
    scheduleProxy->setFilterRegExp("");

    return workCount;
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
    scheduleProxy->setFilterRegExp("\\b"+copyFrom+"\\b");
    scheduleProxy->setFilterKeyColumn(0);
    this->deleteWeek(copyTo);
    QHash<int,QString> valueList;
    QString fromWeek,toWeek,fromYear,toYear,workdate;
    int fromWeek_int,fromYear_int,toWeek_int,toYear_int,addfactor = 0;
    int days = 7;
    fromWeek = copyFrom.split("_").first();
    fromYear = copyFrom.split("_").last();
    toWeek = copyTo.split("_").first();
    toYear = copyTo.split("_").last();

    QDate lastDay(fromYear.toInt(),12,31);
    QDate workoutDate;
    fromWeek_int = fromWeek.toInt();
    fromYear_int = fromYear.toInt();
    toWeek_int = toWeek.toInt();
    toYear_int = toYear.toInt();

    if(toYear_int == fromYear_int)
    {
        addfactor = toWeek_int - fromWeek_int;
    }
    if(toYear_int > fromYear_int)
    {
       addfactor = (lastDay.weekNumber() - fromWeek_int) + toWeek_int;
    }

    for(int row = 0; row < scheduleProxy->rowCount(); ++row)
    {
        workdate = scheduleProxy->data(scheduleProxy->index(row,1)).toString();
        valueList.insert(0,copyTo);
        valueList.insert(1,workoutDate.fromString(workdate,"dd.MM.yyyy").addDays(days*addfactor).toString("dd.MM.yyyy"));

        for(int col = 2; col < scheduleProxy->columnCount();++col)
        {
            valueList.insert(col,scheduleProxy->data(scheduleProxy->index(row,col)).toString());
        }
        itemList.insert(scheduleProxy->index(row,0),valueList);
    }
    this->set_workoutData(COPY);
    scheduleProxy->setFilterRegExp("");
}

void schedule::delete_workout(QModelIndex index)
{
    QPair<double,double> stressMap;
    QString wDate = workout_schedule->data(workout_schedule->index(index.row(),1)).toString();
    stressMap.first = workout_schedule->data(workout_schedule->index(index.row(),8)).toDouble();
    stressMap.second = get_timesec(workout_schedule->data(workout_schedule->index(index.row(),8)).toString())/60.0;
    workout_schedule->removeRow(index.row(),QModelIndex());
    this->updateStress(wDate,stressMap,3);
}

void schedule::deleteWeek(QString deleteWeek)
{
    QList<QStandardItem*> deleteList = workout_schedule->findItems(deleteWeek,Qt::MatchExactly,0);
    for(int i = 0; i < deleteList.count(); ++i)
    {
        this->delete_workout(workout_schedule->indexFromItem(deleteList.at(i)));
    }
}

QString schedule::get_weekPhase(QDate currDate,bool full)
{
    metaProxy->invalidate();
    QString weekID = QString::number(currDate.weekNumber()) +"_"+ QString::number(currDate.addDays(1 - currDate.dayOfWeek()).year());
    metaProxy->setFilterRegExp("\\b"+weekID+"\\b");
    metaProxy->setFilterKeyColumn(2);
    QString phaseString = metaProxy->data(metaProxy->index(0,3)).toString();

    if(metaProxy->rowCount() == 1 && full)
    {
        return phaseString +"#"+metaProxy->data(metaProxy->index(0,5)).toString()+"#"+metaProxy->data(metaProxy->index(0,6)).toString();
    }
    if(metaProxy->rowCount() == 1 && !full)
    {
        return phaseString;
    }

    return 0;
}

void schedule::set_workoutData(int mode)
{
    QString workoutStress;
    QPair<double,double> stressMap;
    double currStress = 0;
    double currDura = 0;
    int row = 0;
    int col = 0;

    if(mode == EDIT) //EDIT
    {
        for(QHash<QModelIndex,QHash<int,QString>>::const_iterator it =  itemList.cbegin(), end = itemList.cend(); it != end; ++it)
        {
            row = it.key().row();
            currStress = workout_schedule->data(workout_schedule->index(row,8)).toDouble();
            currDura = get_timesec(workout_schedule->data(workout_schedule->index(row,6)).toString())/60.0;

            for(QHash<int,QString>::const_iterator vStart = it.value().cbegin(), vEnd = it.value().cend(); vStart != vEnd; ++vStart,++col)
            {
                workout_schedule->setData(workout_schedule->index(row,col,QModelIndex()),it.value().value(col));
            }

            workout_date = it.value().value(1);
            workoutStress = it.value().value(8);
            stressMap.first = workoutStress.toDouble()-currStress;
            stressMap.second = (get_timesec(it.value().value(6))/60.0)-currDura;
            this->updateStress(workout_date,stressMap,0);
        }
    }
    else if(mode == ADD || mode == COPY) //ADD and COPY
    {
        int rowCount = workout_schedule->rowCount();
        workout_schedule->insertRows(rowCount,itemList.count(),QModelIndex());

        for(QHash<QModelIndex,QHash<int,QString>>::const_iterator it =  itemList.cbegin(), end = itemList.cend(); it != end; ++it,++rowCount)
        {
            for(QHash<int,QString>::const_iterator vStart = it.value().cbegin(), vEnd = it.value().cend(); vStart != vEnd; ++vStart,++col)
            {
                workout_schedule->setData(workout_schedule->index(rowCount,col,QModelIndex()),it.value().value(col));
            }
            workout_date = it.value().value(1);
            workoutStress = it.value().value(8);
            stressMap.first = workoutStress.toDouble();
            stressMap.second = get_timesec(it.value().value(6))/60.0;
            this->updateStress(workout_date,stressMap,0);
            col = 0;
        }
    }
    itemList.clear();
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
        if(stressValue - stressMap.first == 0)
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
