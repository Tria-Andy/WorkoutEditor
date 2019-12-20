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
#include "calculation.h"
#include "xmlhandler.h"

class schedule : public xmlHandler, public calculation
{

public:
    schedule();
    bool newSaison;
    QStandardItemModel *scheduleModel,*phaseModel;
    QString dateFormat,longTime,shortTime;
    QHash<QDate,QMap<int,QStringList>> workoutUpdates;

    QString get_weekPhase(QDate);
    QString get_selSaison() {return selSaison;}
    QStringList get_weekList();
    QStringList get_weekMeta(QString);
    QStandardItem *get_phaseItem(QString);

    QHash<QString,QMap<QDate,QStringList>> get_contestMap() {return contestMap;}
    QMap<QDate,QVector<double>> *get_stressMap() {return &stressMap;}
    QMap<int,QStringList> get_workouts(int,QString);
    QMap<QString,QStringList> *get_saisonValues() {return &saisonValues;}
    QMap<QDate,QPair<double,double> > stressValues;
    QMap<QDate,int> get_linkStdWorkouts(QString key) {return linkedWorkouts.value(key);}
    QHash<QDate,QMap<QString,QVector<double> >> *get_compValues() {return &compMap;}
    QHash<QString,QMap<QString,QVector<double> >> *get_compWeekValues() {return &compWeekMap;}

    void init_scheduleData();
    void save_workouts(bool);
    void save_ltsFile();
    void copyWeek(QString,QString);
    void clearWeek(QString);
    void add_newSaison(QStringList);
    void delete_Saison(QString);
    bool get_isUpdated() {return isUpdated;}
    void set_selSaison(QString value) {selSaison = value;}

    //Workout
    //Setter
    void set_isUpdated(bool updateFlag) {isUpdated = updateFlag;}

    //edit Workouts
    void set_workoutData();
    void update_linkedWorkouts(QDate,QString,int,bool);
    void set_weekCompValues(QStringList,QMap<QString,QVector<double>>);
    void add_contest(QString,QDate,QStringList);
    void remove_contest(QString,QDate);

protected:


private:
    bool isUpdated;
    QDate firstdayofweek;
    QHash<QString,double> *doubleValues;
    QHash<QString,QString> *gcValues,*fileMap;

    QStringList sportTags; 
    QStringList *macroTags,*scheduleTags,*saisonTags;
    QString selSaison;

    QMap<QString,QStringList> saisonValues;
    QHash<QString,QMap<QDate,QStringList>> contestMap;
    QHash<QDate,QMap<QString,QVector<double>>> compMap;
    QHash<QString,QMap<QString,QVector<double>>> compWeekMap;
    QMap<QDate,QVector<double>> stressMap;
    QHash<QString,QMap<QDate,int>> linkedWorkouts;
    QModelIndex get_modelIndex(QStandardItemModel*,QString,int);

    void set_compValues(bool,QDate,QMap<int,QStringList>);
    void update_compValues(QMap<QString,QVector<double>>*,QMap<int,QStringList>*);
    void recalc_stressValues();
    void remove_WeekofPast(QDate);
    void set_stressMap();
    void set_saisonValues();
    void check_workouts(QDate);
};

#endif // SCHEDULE_H
