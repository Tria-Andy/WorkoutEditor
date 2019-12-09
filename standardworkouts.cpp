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
        this->xml_toTreeModel(fileMap->value("standardworkoutfile"),stdWorkoutsModel);
        this->fill_workoutMap();
    }
}

void standardWorkouts::fill_workoutMap()
{
    QStandardItem *sportItem;
    QString sportName;

    for(int sport = 0; sport < stdWorkoutsModel->rowCount(); ++sport)
    {        
        sportItem = stdWorkoutsModel->item(sport,0);
        sportName = sportItem->index().siblingAtColumn(1).data(Qt::DisplayRole).toString();

        if(sportItem->hasChildren())
        {    
            for(int work = 0; work < sportItem->rowCount(); ++work)
            {
                this->add_workoutToMap(sportItem->child(work,0),sportName);
            }
        }
    }
}

void standardWorkouts::add_workoutToMap(QStandardItem *workitem,QString sport)
{
    QHash<QString,QVector<QString>> workoutInfo = workoutMap.value(sport);
    QVector<QString> workoutMeta(9);
    QString workID;

    workID = workitem->index().siblingAtColumn(1).data(Qt::DisplayRole).toString();
    workoutMeta[0] = workitem->index().siblingAtColumn(2).data(Qt::DisplayRole).toString();
    workoutMeta[1] = workitem->index().siblingAtColumn(3).data(Qt::DisplayRole).toString();
    workoutMeta[2] = workitem->index().siblingAtColumn(4).data(Qt::DisplayRole).toString();
    workoutMeta[3] = workitem->index().siblingAtColumn(5).data(Qt::DisplayRole).toString();
    workoutMeta[4] = workitem->index().siblingAtColumn(6).data(Qt::DisplayRole).toString();
    workoutMeta[5] = workitem->index().siblingAtColumn(7).data(Qt::DisplayRole).toString();
    workoutMeta[6] = workitem->index().siblingAtColumn(8).data(Qt::DisplayRole).toString();
    workoutMeta[7] = workitem->index().siblingAtColumn(9).data(Qt::DisplayRole).toString();
    workoutMeta[8] = workitem->index().siblingAtColumn(0).data(Qt::DisplayRole).toString();

    workoutIndex.insert(workID,workitem->index());
    workoutInfo.insert(workID,workoutMeta);
    workoutMap.insert(sport,workoutInfo);
}

QStandardItem *standardWorkouts::get_selectedWorkout(QString workID)
{
    return stdWorkoutsModel->itemFromIndex(workoutIndex.value(workID));
}

void standardWorkouts::update_selectedWorkout(QString workoutID)
{
    QModelIndex workIndex = workoutIndex.value(workoutID);
    QStandardItem *workoutItem = stdWorkoutsModel->itemFromIndex(workIndex);
    if(workoutItem->hasChildren())
    {
        stdWorkoutsModel->removeRows(0,workoutItem->rowCount(),workIndex);
    }
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
    int attCount = settings::get_xmlMapping(source->data(Qt::AccessibleTextRole).toString())->count();

    for(int att = 0; att < attCount; ++att)
    {
        itemList << source->model()->itemFromIndex(source->index().siblingAtColumn(att))->clone();
    }

    target->appendRow(itemList);
    return itemList.at(0);
}

QString standardWorkouts::create_newWorkout(QString sport,QList<QStandardItem*> metaList)
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


    //Append new Workout to TreeModel
    metaList.at(0)->setData("standardworkout",Qt::AccessibleTextRole);
    metaList.at(0)->setData(QString::number(workoutMap.value(sport).count()),Qt::DisplayRole);
    metaList.at(1)->setData(workID,Qt::DisplayRole);

    QStandardItem *sportItem = stdWorkoutsModel->findItems(sport,Qt::MatchExactly | Qt::MatchRecursive,1).at(0);

    sportItem->appendRow(metaList);
    this->add_workoutToMap(stdWorkoutsModel->findItems(workID,Qt::MatchExactly | Qt::MatchRecursive,1).at(0),sport);

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
