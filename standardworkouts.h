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
#include "settings.h"
#include "xmlhandler.h"

class standardWorkouts : public xmlHandler
{
public:
    standardWorkouts();

    QStandardItemModel *stdWorkoutsModel;
    QHash<QString,QHash<QString,QVector<QString>>> *get_workoutMap() {return &workoutMap;}
    QStandardItem *get_selectedWorkout(QString);
    QString get_workoutCount(QString);

    QPair<int,QString> create_newWorkout(QString);
    QVector<double> get_workLevelLoad(QString key) {return levelLoadMap.value(key);}
    QString get_workoutImage() {return workoutImage;}
    void set_image(QString imageName) {workoutImage = imageName;}
    void update_selectedWorkout(QString,QList<QStandardItem*>);
    void save_selectedWorkout(QString,QString);
    void delete_stdWorkout(QString,QString);
    void save_workouts();

protected:

private:
    QString workoutImage;
    QHash<QString,QString> *fileMap;
    QHash<QString,QStringList> sportLevelMap;
    QHash<QString,QHash<QString,QVector<QString>>> workoutMap;
    QHash<QString,QModelIndex> workoutIndex;
    QHash<QString,QString> workoutMapping;
    QHash<QString,QVector<double>> levelLoadMap;
    bool workoutUpdate;

    QModelIndex get_modelIndex(QString,int);
    QStandardItem *get_modelItem(QString,int);
    QString add_workoutToMap(QStandardItem*,QString);

    void fill_workoutMap();
    void read_levelLoad();
    void read_childFromModel(QStandardItem*,QString,int);
    void set_levelLoadValues(QStandardItem*,QString,int);
    void check_workouts();
};

#endif // STANDARDWORKOUTS_H
