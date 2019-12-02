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

#include "standardworkouts.h"
#include <QMessageBox>

standardWorkouts::standardWorkouts()
{
    meta_tags << "sport" << "id" << "code" << "title" << "comment" << "duration" << "distance" << "stress" << "work" << "timebase" << "pic";
    step_tags << "sport-id" << "id" << "part" << "level" << "threshold" << "int-time" << "int-dist" << "repeats" << "parent";
    workoutPath = settings::getStringMapPointer(settings::stingMap::GC)->value("workouts");

    metaFile = "standard_workouts_meta.xml";
    stepFile = "standard_workouts_steps.xml";

    stdWorkoutFile = "standard_workouts.xml";
    stdWorkoutsModel = new QStandardItemModel();
    selectedModel = new QStandardItemModel();
    if(!workoutPath.isEmpty())
    {

        this->check_File(workoutPath,metaFile);
        this->check_File(workoutPath,stepFile);
        this->read_standard_workouts(this->load_XMLFile(workoutPath,metaFile),this->load_XMLFile(workoutPath,stepFile));
        this->xml_toTreeModel(stdWorkoutFile,stdWorkoutsModel);
        this->fill_workoutMap();
    }
}

void standardWorkouts::read_standard_workouts(QDomDocument meta_doc,QDomDocument step_doc)
{
    workouts_meta = new QStandardItemModel(0,meta_tags.count());
    workouts_steps = new QStandardItemModel(0,step_tags.count());
    QDomElement rootTag;
    QDomNodeList xmlList;
    QDomElement xmlElement;

    rootTag = meta_doc.firstChildElement();
    xmlList = rootTag.elementsByTagName("workout");

    //META
    for(int row = 0; row < xmlList.count(); ++row)
    {
        xmlElement = xmlList.at(row).toElement();
        workouts_meta->insertRows(row,1,QModelIndex());
        for(int col = 0; col < workouts_meta->columnCount(); ++col)
        {
            workouts_meta->setData(workouts_meta->index(row,col,QModelIndex()),xmlElement.attribute(meta_tags.at(col)));
        }
    }

    rootTag = step_doc.firstChildElement();
    xmlList = rootTag.elementsByTagName("step");

    //Steps
    for(int row = 0; row < xmlList.count(); ++row)
    {
        xmlElement = xmlList.at(row).toElement();
        workouts_steps->insertRows(row,1,QModelIndex());
        for(int col = 0; col < workouts_steps->columnCount(); ++col)
        {
            if(col == 1)
            {
                workouts_steps->setData(workouts_steps->index(row,col,QModelIndex()),xmlElement.attribute(step_tags.at(col)).toInt());
            }
            else
            {
                workouts_steps->setData(workouts_steps->index(row,col,QModelIndex()),xmlElement.attribute(step_tags.at(col)));
            }
        }
    }

    metaProxy =  new QSortFilterProxyModel();
    metaProxy->setSourceModel(workouts_meta);
    stepProxy = new QSortFilterProxyModel();
    stepProxy->setSourceModel(workouts_steps);
    this->set_workoutIds();
}

void standardWorkouts::fill_workoutMap()
{
    QStandardItem *sportItem,*workItem;
    QHash<QString,QVector<QString>> workoutInfo;
    QVector<QString> workoutMeta(7);
    QString sportName,workID;

    for(int sport = 0; sport < stdWorkoutsModel->rowCount(); ++sport)
    {        
        sportItem = stdWorkoutsModel->item(sport,0);
        sportName = sportItem->index().siblingAtColumn(xmlTagMap.value(sportItem->data(Qt::UserRole).toString()).key("sport")).data(Qt::DisplayRole).toString();

        if(sportItem->hasChildren())
        {    
            for(int work = 0; work < sportItem->rowCount(); ++work)
            {
                workItem = sportItem->child(work,0);
                workID = this->get_modelValue(workItem,"name");
                workoutMeta[0] = this->get_modelValue(workItem,"code");
                workoutMeta[1] = this->get_modelValue(workItem,"title");
                workoutMeta[2] = this->get_modelValue(workItem,"duration");
                workoutMeta[3] = this->get_modelValue(workItem,"distance");
                workoutMeta[4] = this->get_modelValue(workItem,"stress");
                workoutMeta[5] = this->get_modelValue(workItem,"work");
                workoutMeta[6] = this->get_modelValue(workItem,"timebase");
                workoutIndex.insert(workID,workItem->index());
                workoutInfo.insert(workID,workoutMeta);
            }
        }
        workoutMap.insert(sportName,workoutInfo);
        workoutInfo.clear();
    }
}

QString standardWorkouts::get_modelValue(QStandardItem *item, QString tag)
{
    return item->index().siblingAtColumn(xmlTagMap.value(item->data(Qt::UserRole).toString()).key(tag)).data(Qt::DisplayRole).toString();
}

QStandardItemModel *standardWorkouts::get_selectedWorkout(QString workID)
{
    selectedModel->clear();
    QStandardItem *targetItem = selectedModel->invisibleRootItem();

    QModelIndex sourceIndex = workoutIndex.value(workID);
    QStandardItem *sourceItem = stdWorkoutsModel->itemFromIndex(sourceIndex);

    for(int step = 0; step < sourceItem->rowCount(); ++step)
    {
        if(sourceItem->child(step,0)->hasChildren())
        {
            read_childFromModel(sourceItem->child(step,0),this->set_childtoModel(sourceItem->child(step,0),targetItem));
        }
        else
        {
            this->set_childtoModel(sourceItem->child(step,0),targetItem);
        }
    }
    return selectedModel;
}

void standardWorkouts::save_selectedWorkout(QString workoutID)
{
    QModelIndex workIndex = workoutIndex.value(workoutID);
    QStandardItem *workoutItem = stdWorkoutsModel->itemFromIndex(workIndex);
    QList<QStandardItem*> itemList;

    stdWorkoutsModel->removeRows(0,workoutItem->rowCount(),workIndex);

    for(int row = 0; row < selectedModel->rowCount(); ++row)
    {

    }
}

QString standardWorkouts::get_newWorkoutID(QString sport)
{
    QHash<QString,QVector<QString>> sportID = workoutMap.value(sport);
    QString workID;
    int counter;

    for(counter = 1; counter < sportID.count()+1; ++counter)
    {
        if(!sportID.contains(sport+"_"+QString::number(counter)))
        {
            workID = sport+"_"+QString::number(counter);
        }
    }
    if(workID.isEmpty()) workID = sport+"_"+QString::number(counter);

    return workID;
}

void standardWorkouts::read_childFromModel(QStandardItem *sourceItem,QStandardItem *targetItem)
{
    QStandardItem *childSourceItem;

    if(sourceItem->hasChildren())
    {
        for(int row = 0; row < sourceItem->rowCount(); ++row)
        {
            childSourceItem = sourceItem->child(row,0);
            if(childSourceItem->hasChildren())
            {
                this->read_childFromModel(childSourceItem,this->set_childtoModel(childSourceItem,targetItem));
            }
            else
            {
                this->set_childtoModel(childSourceItem,targetItem);
            }
        }
    }
}

QStandardItem* standardWorkouts::set_childtoModel(QStandardItem *source,QStandardItem *target)
{
    QList<QStandardItem*> itemList;
    int attCount = xmlTagMap.value(source->data(Qt::UserRole).toString()).count();

    for(int att = 0; att < attCount; ++att)
    {
        itemList << source->model()->itemFromIndex(source->index().siblingAtColumn(att))->clone();
    }

    target->appendRow(itemList);
    return itemList.at(0);
}

void standardWorkouts::write_standard_workouts()
{
    QModelIndex index;
    QDomDocument xmlDoc;

    QDomElement xmlRoot,xmlElement;
    xmlRoot = xmlDoc.createElement("workouts");
    xmlDoc.appendChild(xmlRoot);

    //Meta
    for(int i = 0; i < workouts_meta->rowCount(); ++i)
    {
        index = workouts_meta->index(i,2,QModelIndex());
        xmlElement = xmlDoc.createElement("workout");

        for(int x = 0; x < workouts_meta->columnCount(); ++x)
        {
            index = workouts_meta->index(i,x,QModelIndex());
            xmlElement.setAttribute(meta_tags.at(x),workouts_meta->data(index,Qt::DisplayRole).toString());
        }
        xmlRoot.appendChild(xmlElement);
    }
    this->write_XMLFile(workoutPath,&xmlDoc,metaFile);
    xmlDoc.clear();

    //Steps
    xmlRoot = xmlDoc.createElement("steps");
    xmlDoc.appendChild(xmlRoot);

    for(int i = 0; i < workouts_steps->rowCount(); ++i)
    {
        index = workouts_steps->index(i,2,QModelIndex());
        xmlElement = xmlDoc.createElement("step");

        for(int x = 0; x < workouts_steps->columnCount(); ++x)
        {
            index = workouts_steps->index(i,x,QModelIndex());
            xmlElement.setAttribute(step_tags.at(x),workouts_steps->data(index,Qt::DisplayRole).toString());
        }
        xmlRoot.appendChild(xmlElement);
    }
    this->write_XMLFile(workoutPath,&xmlDoc,stepFile);
}

void standardWorkouts::set_workoutIds()
{
    workoutIDs = QStringList();
    for(int i = 0; i < workouts_meta->rowCount(); ++i)
    {
        workoutIDs << workouts_meta->data(workouts_meta->index(i,1,QModelIndex())).toString();
    }
}

void standardWorkouts::filter_steps(QString workID,bool fixed)
{
    stepProxy->invalidate();
    if(fixed)
    {
        stepProxy->setFilterFixedString(workID);
    }
    else
    {
        stepProxy->setFilterRegExp("\\b"+workID+"\\b");
    }
    stepProxy->setFilterKeyColumn(0);
    stepProxy->sort(1);
}

void standardWorkouts::filter_workout(QString filterValue,int col,bool fixed)
{
    metaProxy->invalidate();
    if(fixed)
    {
        metaProxy->setFilterFixedString(filterValue);
    }
    else
    {
        metaProxy->setFilterRegExp("\\b"+filterValue+"\\b");
    }

    metaProxy->setFilterKeyColumn(col);
}

QModelIndex standardWorkouts::get_modelIndex(QString searchString,int col)
{
    QList<QStandardItem*> list;

    list = stdWorkoutsModel->findItems(searchString,Qt::MatchExactly | Qt::MatchRecursive,col);

    if(list.count() > 0)
    {
        return stdWorkoutsModel->indexFromItem(list.at(0));
    }
    else
    {
        return QModelIndex();
    }
}

QStandardItem *standardWorkouts::get_modelItem(QString searchString, int col)
{
    QList<QStandardItem*> list;

    list = stdWorkoutsModel->findItems(searchString,Qt::MatchExactly | Qt::MatchRecursive,col);

    return list.at(0);
}

void standardWorkouts::delete_stdWorkout(QString workID,bool isdelete)
{
    this->filter_workout(workID,1,false);
    metaProxy->removeRow(0,QModelIndex());

    this->filter_steps(workID,false);
    stepProxy->removeRows(0,stepProxy->rowCount(),QModelIndex());

    if(isdelete) this->set_workoutIds();
    this->write_standard_workouts();
}
