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
    QStandardItemModel *workouts_meta, *workouts_steps;
    QSortFilterProxyModel *metaProxy,*stepProxy;

    void delete_stdWorkout(QString,bool);
    QStringList get_workoutIds() {return workoutIDs;}

protected:
    void write_standard_workouts();
    void filter_steps(QString,bool);
    void filter_workout(QString,int,bool);

private:
    QStringList meta_tags,step_tags,workoutIDs;
    QString workoutPath,metaFile,stepFile;
    void check_workoutFiles();
    void read_standard_workouts(QDomDocument,QDomDocument);
    void set_workoutIds();
};

#endif // STANDARDWORKOUTS_H
