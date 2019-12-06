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
    workoutPath = settings::getStringMapPointer(settings::stingMap::GC)->value("workouts");
    fileMap = settings::getStringMapPointer(settings::stingMap::File);
    stdWorkoutsModel = new QStandardItemModel();
    selectedModel = new QStandardItemModel();

    if(!workoutPath.isEmpty())
    {
        //this->fill_treeModel(fileMap->value("standardworkoutfile"),stdWorkoutsModel);
        this->xml_toTreeModel(fileMap->value("standardworkoutfile"),stdWorkoutsModel);
        this->fill_workoutMap();
    }
}

void standardWorkouts::fill_workoutMap()
{
    QStandardItem *sportItem,*workItem;
    QHash<QString,QVector<QString>> workoutInfo;
    QVector<QString> workoutMeta(8);
    QString sportName,workID;

    workoutMapping = settings::get_xmlMapping("standardworkout");

    for(int sport = 0; sport < stdWorkoutsModel->rowCount(); ++sport)
    {        
        sportItem = stdWorkoutsModel->item(sport,0);
        sportName = sportItem->index().siblingAtColumn(xmlTagMap.value(sportItem->data(Qt::UserRole).toString()).key("sport")).data(Qt::DisplayRole).toString();

        if(sportItem->hasChildren())
        {    
            for(int work = 0; work < sportItem->rowCount(); ++work)
            {
                workItem = sportItem->child(work,0);
                workID = this->get_accessibleValue(workItem,workoutMapping.value("WorkID"));
                workoutMeta[0] = this->get_accessibleValue(workItem,workoutMapping.value("WorkCode"));
                workoutMeta[1] = this->get_accessibleValue(workItem,workoutMapping.value("WorkTitle"));
                workoutMeta[2] = this->get_accessibleValue(workItem,workoutMapping.value("WorkTime"));
                workoutMeta[3] = this->get_accessibleValue(workItem,workoutMapping.value("WorkDist"));
                workoutMeta[4] = this->get_accessibleValue(workItem,workoutMapping.value("WorkStress"));
                workoutMeta[5] = this->get_accessibleValue(workItem,workoutMapping.value("WorkKJ"));
                workoutMeta[6] = this->get_accessibleValue(workItem,workoutMapping.value("WorkBase"));
                workoutMeta[7] = this->get_accessibleValue(workItem,workoutMapping.value("WorkPic"));
                workoutIndex.insert(workID,workItem->index());
                workoutInfo.insert(workID,workoutMeta);
            }
        }
        workoutMap.insert(sportName,workoutInfo);
        workoutInfo.clear();
    }
}

QString standardWorkouts::get_accessibleValue(QStandardItem *item,QString tagName)
{
    return item->index().siblingAtColumn(xmlTagMap.value(item->data(Qt::AccessibleTextRole).toString()).key(tagName)).data(Qt::DisplayRole).toString();
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
    get_modelIndex(workID,0);

}
