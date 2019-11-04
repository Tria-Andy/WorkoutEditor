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
    QStandardItemModel *scheduleModel,*phaseModel,*week_meta,*week_content;
    QHash<QString,QMap<int,QStringList>> workoutList;
    QMap<QDate,QPair<double,double> > stressValues;
    QMap<QString,QStringList> headerList;
    void freeMem();
    void filter_schedule(QString,int,bool);
    void save_workouts(bool);
    void save_ltsFile(double);
    void check_workouts(QDate);
    QString get_weekPhase(QDate);
    void copyWeek(QString,QString);
    void deleteWeek(QString);
    QMap<QDate,QPair<double,double> > *get_StressMap() {return &stressValues;}
    QHash<QDate,QMap<QString,QVector<double> >> *get_compValues() {return &compMap;}
    QHash<QString,QMap<QString,QVector<double> >> *get_compWeekValues() {return &compWeekMap;}
    void updateStress(QString,QPair<double,double>,int);
    bool get_isUpdated() {return isUpdated;}
    void add_newSaison(QStringList);
    void delete_Saison(QString);
    QHash<int,QString> get_weekList();
    QMap<int,QStringList> get_workouts(bool,QString);
    QStringList get_weekMeta(QString);
    QModelIndex get_modelIndex(QStandardItemModel*,QString,int);
    QStandardItem *get_phaseItem(QString);
    QMap<QString,QStringList> *get_saisonValues() {return &saisonValues;}

//Workout
    //Setter
    void set_isUpdated(bool updateFlag) {isUpdated = updateFlag;}

    //edit Workouts
    void set_workoutData(QHash<QString,QMap<int,QStringList>>);

private:
    QStringList sportTags;
    QStringList *macroTags,*workoutTags,*compTags,*phaseTags;
    QDomNodeList xmlList;
    QString schedulePath,scheduleFile,phaseFile,ltsFile;
    QMap<QString,QStringList> saisonValues;
    void set_compValues(bool,QString,QMap<int,QStringList>);
    void update_compValues(QMap<QString,QVector<double>>*,QMap<int,QStringList>*);
    void set_saisonValues();
    QHash<QDate,QMap<QString,QVector<double>>> compMap;
    QHash<QString,QMap<QString,QVector<double>>> compWeekMap;
    bool isUpdated;
    void read_ltsFile(QDomDocument);   
    void save_ltsValues();

    //Workout Var  
};

#endif // SCHEDULE_H
