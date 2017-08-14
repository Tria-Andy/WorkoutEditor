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

#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QStandardItemModel>
#include <QtXml>
#include <QMessageBox>
#include "settings.h"
#include "saisons.h"
#include "calculation.h"

class schedule : public saisons, public calculation
{

public:
    schedule();
    QStandardItemModel *workout_schedule,*week_meta,*week_content;
    QSortFilterProxyModel *metaProxy,*contentProxy;
    QHash<QModelIndex,QHash<int,QString>> itemList;
    void freeMem();
    void save_dayWorkouts();
    void save_weekPlan();
    void save_ltsFile(double);
    int check_workouts(QDate);
    QString get_weekPhase(QDate);
    void copyWeek(QString,QString);
    void deleteWeek(QString);
    QMap<QDate,QPair<double,double> > *get_StressMap() {return &stressValues;}
    //void set_stressMap(QDate key,double value) {stressValues.insert(key,value);}
    void updateStress(QString,QPair<double,double>,int);
    bool get_isUpdated() {return isUpdated;}
    void add_newSaison(QString);
    void delete_Saison(QString);
    QHash<int,QString> get_weekList();

//Workout
    //Setter
    void set_workout_date(QString w_date) {workout_date = w_date;}
    void set_isUpdated(bool updateFlag) {isUpdated = updateFlag;}

    //edit Workouts
    void set_workoutData(int);
    void delete_workout(QModelIndex);

private:
    QSortFilterProxyModel *scheduleProxy;
    QStringList workoutTags,metaTags,contentTags;
    QString schedulePath,workoutFile,metaFile,contentFile,ltsFile;
    QDate firstdayofweek;
    QMap<QDate,QPair<double,double> > stressValues;
    QHash<int,QString> weekList;
    bool fileCreated,isUpdated;
    void read_dayWorkouts(QDomDocument);
    void read_weekPlan(QDomDocument,QDomDocument);
    void read_ltsFile(QDomDocument);   
    void save_ltsValues();

    //Workout Var
    QString workout_date;   
    QString workout_sport;

};

#endif // SCHEDULE_H
