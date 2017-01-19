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
    workoutTags << "week" << "date" << "time" << "sport" << "code" << "title" << "duration" << "distance" << "stress";
    metaTags << "id" << "week" << "name" << "fdw";
    contentTags << "id" << "week";
    for(int i = 0; i < settings::get_listValues("Sportuse").count();++i)
    {
        workout_sport = settings::get_listValues("Sportuse").at(i);
        contentTags << workout_sport.toLower();
    }
    contentTags << "summery";
    workout_sport.clear();
    firstdayofweek = QDate::currentDate().addDays(1 - QDate::currentDate().dayOfWeek());
    schedulePath = settings::get_gcInfo("schedule");
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
}

void schedule::freeMem()
{
    delete scheduleProxy;
    delete workout_schedule;
    delete week_meta;
    delete week_content;
}

void schedule::read_dayWorkouts(QDomDocument workouts)
{

    QDomElement root_workouts = workouts.firstChildElement();
    QDomNodeList workout_list;
    if(workouts.hasChildNodes())
    {
        workout_list = root_workouts.elementsByTagName("workout");
    }

    workout_schedule = new QStandardItemModel(workout_list.count(),9);
    QDate workDate;
    int stress = 0;
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
            stress = workout_element.attribute("stress").toInt();

            if(stressValues.contains(workDate))
            {
                stress = stress + stressValues.value(workDate);
                stressValues.insert(workDate,stress);
            }
            else
            {
                stressValues.insert(workDate,stress);
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
    QDomElement root_meta = weekMeta.firstChildElement();
    QDomElement root_content = weekContent.firstChildElement();
    QDomNodeList meta_list,content_list;
    meta_list = root_meta.elementsByTagName("phase");
    content_list = root_content.elementsByTagName("content");

    week_meta = new QStandardItemModel(meta_list.count(),4);
    week_content = new QStandardItemModel(content_list.count(),contentTags.count());

    //fill week_meta
    if(meta_list.count() == 0)
    {
        QDate startDate = QDate::fromString(settings::get_saisonInfo("startDate"),"dd.MM.yyyy");
        QString weekid;
        QString noPhase = settings::get_generalValue("empty");
        int saisonWeeks = settings::get_saisonInfo("weeks").toInt();

        week_meta->setRowCount(saisonWeeks);
        for(int week = 0,id = 1; week < saisonWeeks; ++week,++id)
        {
            weekid = QString::number(startDate.addDays(week*7).weekNumber()) +"_"+ QString::number(startDate.addDays(week*7).year());
            week_meta->setData(week_meta->index(week,0,QModelIndex()),id);
            week_meta->setData(week_meta->index(week,1,QModelIndex()),weekid);
            week_meta->setData(week_meta->index(week,2,QModelIndex()),noPhase);
            week_meta->setData(week_meta->index(week,3,QModelIndex()),startDate.addDays(week*7).toString("dd.MM.yyyy"));
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
                if(col == 0)
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
    week_meta->sort(0);

    //fill week_content
    if(content_list.count() == 0)
    {
        QDate startDate = QDate::fromString(settings::get_saisonInfo("startDate"),"dd.MM.yyyy");
        QString weekid;
        int saisonWeeks = settings::get_saisonInfo("weeks").toInt();

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

void schedule::read_ltsFile(QDomDocument stressContent)
{
    int ltsDays = settings::get_ltsValue("ltsdays");
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
                stressValues.insert(wDate,stress_element.attribute("stress").toDouble());
            }
        }

        QDate firstWork = firstdayofweek.addDays(-ltsDays);
        QDate lastWork = stressValues.lastKey();
        int total = firstWork.daysTo(lastWork)+1;

        for(int row = 0 ; row < total; ++row)
        {
            if(!stressValues.contains(firstWork.addDays(row)))
            {
                stressValues.insert(firstWork.addDays(row),0);
            }
        }
        if(QDate::currentDate() == firstdayofweek && stressValues.firstKey() < firstdayofweek.addDays(-ltsDays)) this->save_ltsValues();
    }
    else
    {
        stressValues.insert(firstdayofweek,0);
    }
}

void schedule::save_ltsFile(double ltsDays)
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement;
    xmlRoot = xmlDoc.createElement("Stress");
    xmlDoc.appendChild(xmlRoot);

    for(QMap<QDate,double>::const_iterator it = stressValues.find(firstdayofweek.addDays(-ltsDays)), end = stressValues.find(firstdayofweek); it != end; ++it)
    {
        xmlElement = xmlDoc.createElement("StressLTS");
        xmlElement.setAttribute("day",it.key().toString("dd.MM.yyyy"));
        xmlElement.setAttribute("stress",QString::number(it.value()));
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
    startLTS = settings::get_ltsValue("lastlts");
    int ltsDays = settings::get_ltsValue("ltsdays");
    double factor = (double)exp(-1.0/ltsDays);
    pastStress = startLTS;

    for(QMap<QDate,double>::const_iterator it = stressValues.cbegin(), end = stressValues.find(firstdayofweek.addDays(-ltsDays)); it != end; ++it)
    {
        currStress = it.value();
        stress = (currStress * (1.0 - factor)) + (pastStress * factor);
        pastStress = stress;
    }
    settings::set_ltsValue("lastlts",round(pastStress));

    startLTS = settings::get_ltsValue("laststs");
    double stsDays = settings::get_ltsValue("stsdays");
    factor = (double)exp(-1.0/stsDays);
    pastStress = startLTS;

    for(QMap<QDate,double>::const_iterator it = stressValues.find(firstdayofweek.addDays((-stsDays)-7)), end = stressValues.find(firstdayofweek.addDays((-stsDays))); it != end; ++it)
    {
        currStress = it.value();
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
    QStringList copyValues;
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
        copyValues.clear();
        workdate = scheduleProxy->data(scheduleProxy->index(row,1)).toString();
        copyValues << copyTo << workoutDate.fromString(workdate,"dd.MM.yyyy").addDays(days*addfactor).toString("dd.MM.yyyy");
        for(int col = 2; col < scheduleProxy->columnCount();++col)
        {
            copyValues << scheduleProxy->data(scheduleProxy->index(row,col)).toString();
        }
        itemList.insert(scheduleProxy->index(row,0),copyValues);
    }
    this->set_workoutData(1);
    scheduleProxy->setFilterRegExp("");
}

void schedule::delete_workout(QModelIndex index)
{
    QString wDate = workout_schedule->data(workout_schedule->index(index.row(),1)).toString();
    double stress = workout_schedule->data(workout_schedule->index(index.row(),8)).toDouble();
    workout_schedule->removeRow(index.row(),QModelIndex());
    this->updateStress(wDate,stress,false);
}

void schedule::deleteWeek(QString deleteWeek)
{
    QList<QStandardItem*> deleteList = workout_schedule->findItems(deleteWeek,Qt::MatchExactly,0);

    for(int i = 0; i < deleteList.count(); ++i)
    {
        this->delete_workout(workout_schedule->indexFromItem(deleteList.at(i)));
    }
}

QString schedule::get_weekPhase(QDate currDate)
{
    QSortFilterProxyModel *metaProxy = new QSortFilterProxyModel();
    metaProxy->setSourceModel(week_meta);
    QString weekID = QString::number(currDate.weekNumber()) +"_"+ QString::number(currDate.addDays(1 - currDate.dayOfWeek()).year());
    metaProxy->setFilterRegExp("\\b"+weekID+"\\b");
    metaProxy->setFilterKeyColumn(1);

    if(metaProxy->rowCount() == 1) return metaProxy->data(metaProxy->index(0,2)).toString();

    return 0;
}

void schedule::changeYear()
{
    QDate startDate = QDate::fromString(settings::get_saisonInfo("startDate"),"dd.MM.yyyy");
    QString weekid;
    int id = 0;
    int saisonWeeks = settings::get_saisonInfo("weeks").toInt();

    week_meta->sort(0);
    week_content->sort(0);

    for(int week = 0; week < saisonWeeks; ++week)
    {
        id = week_meta->data(week_meta->index(week,0,QModelIndex())).toInt();

        if(id == week+1)
        {
            weekid = QString::number(startDate.addDays(week*7).weekNumber()) +"_"+ QString::number(startDate.addDays(week*7).year());
            week_meta->setData(week_meta->index(week,1,QModelIndex()),weekid);
            week_meta->setData(week_meta->index(week,3,QModelIndex()),startDate.addDays(week*7).toString("dd.MM.yyyy"));
            week_content->setData(week_content->index(week,1,QModelIndex()),weekid);
        }
    }
    if(week_meta->rowCount() > saisonWeeks)
    {
        week_meta->removeRow(week_meta->rowCount()-1,QModelIndex());
        week_content->removeRow(week_content->rowCount()-1,QModelIndex());
    }
    this->save_weekPlan();
}

void schedule::set_workoutData(int mode)
{
    QString workoutStress;
    double currStress = 0;
    int row = 0;
    if(mode == 0) //EDIT
    {
        for(QMap<QModelIndex,QStringList>::const_iterator it =  itemList.cbegin(), end = itemList.cend(); it != end; ++it)
        {
            row = it.key().row();
            currStress = workout_schedule->data(workout_schedule->index(row,8)).toDouble();
            for(int i = 0; i < it.value().count(); ++i)
            {
                workout_schedule->setData(workout_schedule->index(row,i,QModelIndex()),it.value().at(i));
            }
            workout_date = it.value().at(1);
            workoutStress = it.value().last();
            this->updateStress(workout_date,workoutStress.toDouble()-currStress,true);
        }
    }
    else if(mode == 1) //ADD and COPY
    {
        int row = 0;
        workout_schedule->insertRows(workout_schedule->rowCount(),itemList.count(),QModelIndex());
        for(QMap<QModelIndex,QStringList>::const_iterator it =  itemList.cbegin(), end = itemList.cend(); it != end; ++it,++row)
        {
            for(int i = 0; i < it.value().count(); ++i)
            {
                workout_schedule->setData(workout_schedule->index(row,i,QModelIndex()),it.value().at(i));
            }
            workout_date = it.value().at(1);
            workoutStress = it.value().last();
            this->updateStress(workout_date,workoutStress.toDouble(),true);
        }
    }
    itemList.clear();
}

void schedule::updateStress(QString date,double addStress,bool add)
{
    double stressValue = 0;
    QDate wDate = QDate::fromString(date,"dd.MM.yyyy");

    if(add)
    {
        if(stressValues.contains(wDate))
        {
            stressValue = stressValues.value(wDate);
            stressValues.insert(wDate,stressValue+addStress);
        }
        else
        {
            stressValues.insert(wDate,addStress);
        }
    }
    else
    {
        stressValue = stressValues.value(wDate);
        if(stressValue - addStress == 0)
        {
            stressValues.insert(wDate,0);
        }
        else
        {
            stressValues.insert(wDate,stressValue -addStress);
        }
    }
}
