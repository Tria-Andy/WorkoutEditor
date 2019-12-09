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

#ifndef STANDARDWORKOUTS_H
#define STANDARDWORKOUTS_H

#include <QStandardItemModel>
#include <QtXml>
#include "settings.h"
#include "calculation.h"
#include "xmlhandler.h"

class standardWorkouts : public xmlHandler
{
public:
    standardWorkouts();

    QStandardItemModel *stdWorkoutsModel,*selectedModel;
    QHash<QString,QHash<QString,QVector<QString>>> *get_workoutMap() {return &workoutMap;}
    QStandardItem *get_selectedWorkout(QString);
    void update_selectedWorkout(QString);
    QString create_newWorkout(QString,QList<QStandardItem*>);

    void save_selectedWorkout(QString,QString);
    void delete_stdWorkout(QString,bool);
    void write_standard_workouts();

protected:
    void filter_steps(QString,bool);
    void filter_workout(QString,int,bool);

private:
    QString workoutPath;
    QHash<QString,QString> *fileMap;
    QHash<QString,QMap<int,QString>> modelOrder;
    QHash<QString,QHash<QString,QVector<QString>>> workoutMap;
    QHash<QString,QModelIndex> workoutIndex;
    QHash<QString,QString> workoutMapping;

    QModelIndex get_modelIndex(QString,int);
    QStandardItem *get_modelItem(QString,int);

    QStandardItem* set_childtoModel(QStandardItem*,QStandardItem*);

    void fill_workoutMap();
    void add_workoutToMap(QStandardItem*,QString);
    void read_childFromModel(QStandardItem*,QStandardItem*);
    void read_standard_workouts(QDomDocument,QDomDocument);
};

#endif // STANDARDWORKOUTS_H
