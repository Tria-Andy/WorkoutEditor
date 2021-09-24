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
    fileMap = settings::getStringMapPointer(settings::stingMap::File);
    stdWorkoutsModel = new QStandardItemModel();

    if(!settings::getStringMapPointer(settings::stingMap::GC)->value("workouts").isEmpty())
    {
        this->xml_toTreeModel(fileMap->value("standardworkoutfile"),stdWorkoutsModel);
        this->fill_workoutMap();
    }
}

void standardWorkouts::fill_workoutMap()
{
    workoutUpdate = false;
    QStandardItem *sportItem;
    QString sportName;
    QVector<double> loadValues(settings::get_listValues("Sportlevel").count(),0);

    for(int sport = 0; sport < stdWorkoutsModel->rowCount(); ++sport)
    {        
        sportItem = stdWorkoutsModel->item(sport,0);
        sportName = sportItem->index().siblingAtColumn(1).data(Qt::DisplayRole).toString();

        if(sportItem->hasChildren())
        {    
            for(int work = 0; work < sportItem->rowCount(); ++work)
            {
                workoutIndex.insert(this->add_workoutToMap(sportItem->child(work,0),sportName),sportItem->child(work,0)->index());
                levelLoadMap.insert(sportItem->child(work,1)->data(Qt::DisplayRole).toString(),loadValues);
                this->read_childFromModel(sportItem->child(work,0),sportItem->child(work,1)->data(Qt::DisplayRole).toString(),1);
                loadValues.fill(0);
            }
        }
    }
}

QString standardWorkouts::add_workoutToMap(QStandardItem *workitem,QString sport)
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

    workoutInfo.insert(workID,workoutMeta);
    workoutMap.insert(sport,workoutInfo);

    return workID;
}

QStandardItem *standardWorkouts::get_selectedWorkout(QString workID)
{
    return stdWorkoutsModel->itemFromIndex(workoutIndex.value(workID));
}

QString standardWorkouts::get_workoutCount(QString workID)
{
    return stdWorkoutsModel->itemFromIndex(workoutIndex.value(workID))->data(Qt::DisplayRole).toString();
}

void standardWorkouts::update_selectedWorkout(QString workoutID,QList<QStandardItem*> metaList)
{
    QModelIndex workIndex = workoutIndex.value(workoutID);
    QStandardItem *workoutItem = stdWorkoutsModel->itemFromIndex(workIndex);

    if(workoutItem->hasChildren())
    {
        stdWorkoutsModel->removeRows(0,workoutItem->rowCount(),workIndex);
    }

    QStandardItem *sportItem = workoutItem->parent();

    for(int item = 0; item < metaList.count(); ++item)
    {
        sportItem->setChild(workIndex.row(),item,metaList.at(item));
    }
    this->add_workoutToMap(sportItem->child(workIndex.row(),0),sportItem->index().siblingAtColumn(1).data(Qt::DisplayRole).toString());
    workoutUpdate = true;
}

void standardWorkouts::read_childFromModel(QStandardItem *item,QString workID,int reps)
{
    if(item->hasChildren())
    {
        for(int row = 0; row < item->rowCount(); ++row)
        {
            if(item->child(row,0)->hasChildren())
            {
                this->read_childFromModel(item->child(row,0),workID,item->child(row,2)->data(Qt::DisplayRole).toInt());
            }
            else
            {
                this->set_levelLoadValues(item->child(row,0),workID,reps);
            }
        }
    }
}

void standardWorkouts::set_levelLoadValues(QStandardItem *item,QString workID,int reps)
{
    QVector<double> levelValues = levelLoadMap.value(workID);

    int setPos = settings::get_listValues("Sportlevel").indexOf(settings::get_levelMap(item->index().siblingAtColumn(4).data(Qt::DisplayRole).toString()));
    double levelValue = levelValues.at(setPos) + (item->index().siblingAtColumn(3).data(Qt::DisplayRole).toDouble() * reps);

    levelValues[setPos] = levelValue;
    levelLoadMap.insert(workID,levelValues);
}

void standardWorkouts::check_workouts()
{
    QStandardItem *sportItem;
    QModelIndex workIndex;
    for(int sport = 0; sport < stdWorkoutsModel->rowCount(); ++sport)
    {
        sportItem = stdWorkoutsModel->item(sport,0);
        for(int work = 0; work < sportItem->rowCount(); ++work)
        {
            workIndex = sportItem->child(work,0)->index();
            for(int item = 0; item < 9; ++item)
            {
                qDebug() << workIndex.siblingAtColumn(item).data(Qt::DisplayRole);
            }
        }
    }
}

QPair<int,QString> standardWorkouts::create_newWorkout(QString sport)
{
    QHash<QString,QVector<QString>> sportID = workoutMap.value(sport);
    QString workID;
    QList<QStandardItem*> metaList;

    int counter;

    for(counter = 1; counter < sportID.count()+1; ++counter)
    {
        if(!sportID.contains(sport+"_"+QString::number(counter)))
        {
            workID = sport+"_"+QString::number(counter);
        }
    }

    if(workID.isEmpty()) workID = sport+"_"+QString::number(counter);

    metaList.insert(0,new QStandardItem(QString::number(counter-1)));
    metaList.insert(1,new QStandardItem(workID));

    for(int item = 2; item < 8; ++item)
    {
        metaList.insert(item, new QStandardItem(""));
    }

    QStandardItem *sportItem = stdWorkoutsModel->item(this->get_modelIndex(sport,1).row(),0);
    sportItem->appendRow(metaList);

    workoutIndex.insert(workID,sportItem->child(sportItem->rowCount()-1,0)->index());

    return qMakePair(counter-1,workID);
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

void standardWorkouts::delete_stdWorkout(QString sport,QString workID)
{
    QModelIndex index = get_modelIndex(workID,1);
    QStandardItem *item = get_modelItem(workID,1);

    stdWorkoutsModel->removeRows(0,item->rowCount(),index);
    stdWorkoutsModel->removeRow(item->row(),index.parent());

    QHash<QString,QVector<QString>> workoutInfo = workoutMap.value(sport);
    workoutInfo.remove(workID);
    workoutIndex.remove(workID);
    workoutMap.insert(sport,workoutInfo);
    workoutUpdate = true;
    this->save_workouts();
}

void standardWorkouts::save_workouts()
{
    if(workoutUpdate)
    {
        this->treeModel_toXml(stdWorkoutsModel,fileMap->value("standardworkoutfile"));
    }
    workoutUpdate = false;
}
