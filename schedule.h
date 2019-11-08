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
    QStandardItemModel *scheduleModel,*phaseModel;
    QHash<QString,QMap<int,QStringList>> workoutList;
    QMap<QDate,QPair<double,double> > stressValues;
    QMap<QString,QStringList> headerList;

    QString get_weekPhase(QDate);
    QMap<QDate,QVector<double>> *get_stressMap() {return &stressMap;}
    QHash<QDate,QMap<QString,QVector<double> >> *get_compValues() {return &compMap;}
    QHash<QString,QMap<QString,QVector<double> >> *get_compWeekValues() {return &compWeekMap;}
    QStringList get_weekList();
    QMap<int,QStringList> get_workouts(bool,QString);
    QStringList get_weekMeta(QString);
    QModelIndex get_modelIndex(QStandardItemModel*,QString,int);
    QStandardItem *get_phaseItem(QString);
    QMap<QString,QStringList> *get_saisonValues() {return &saisonValues;}

    void freeMem();
    void filter_schedule(QString,int,bool);
    void save_workouts(bool);
    void save_ltsFile(double);
    void check_workouts(QDate);  
    void copyWeek(QString,QString);
    void add_newSaison(QStringList);
    void delete_Saison(QString);

    bool get_isUpdated() {return isUpdated;}


//Workout
    //Setter
    void set_isUpdated(bool updateFlag) {isUpdated = updateFlag;}

    //edit Workouts
    void set_workoutData(QHash<QDate,QMap<int,QStringList>>);
    void set_weekCompValues(QStringList,QMap<QString,QVector<double>>);

private:
    QStringList sportTags;
    QStringList *macroTags,*workoutTags,*compTags,*phaseTags;
    QDomNodeList xmlList;
    QString schedulePath,scheduleFile,phaseFile,ltsFile;
    QMap<QString,QStringList> saisonValues;
    QHash<QDate,QMap<QString,QVector<double>>> compMap;
    QHash<QString,QMap<QString,QVector<double>>> compWeekMap;
    QMap<QDate,QVector<double>> stressMap;
    bool isUpdated;

    void set_compValues(bool,QDate,QMap<int,QStringList>);
    void update_compValues(QMap<QString,QVector<double>>*,QMap<int,QStringList>*);
    void recalc_stressValues();
    void set_stressMap(QDate,bool);
    void set_saisonValues();
    void read_ltsFile(QDomDocument);   

    //Workout Var  
};

#endif // SCHEDULE_H
