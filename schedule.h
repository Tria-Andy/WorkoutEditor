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
#include <QtCharts>
#include <QMessageBox>
#include "settings.h"
#include "logger.h"

QT_CHARTS_USE_NAMESPACE

class schedule
{

public:
    schedule();
    QStandardItemModel *workout_schedule,*week_meta,*week_content;
    void save_dayWorkouts();
    void save_weekPlan();
    void save_ltsFile(double);
    void changeYear();
    QString get_weekPhase(QDate);
    void copyWeek(QString,QString);
    void deleteWeek(QString);
    QMap<QDate,double> *get_StressMap() {return &stressValues;}
    void set_stressMap(QDate key,double value) {stressValues.insert(key,value);}

//Workout
    //Setter
    void set_workout_date(QString w_date) {workout_date = w_date;}
    void set_workout_time(QString w_time) {workout_time = w_time;}
    void set_workout_calweek(QString w_calweek) {workout_calweek = w_calweek;}
    void set_workout_sport(QString sport) {workout_sport = sport;}
    void set_workout_code(QString wcode) {workout_code = wcode;}
    void set_workout_title(QString wtitle) {workout_title = wtitle;}
    void set_workout_duration(QString wduration) {workout_duration = wduration;}
    void set_workout_distance(double wdistance) {workout_distance = wdistance;}
    void set_workout_stress(int wstress) {workout_stress_score = wstress;}

    //edit Workouts
    void add_workout();
    void edit_workout(QModelIndex);
    void delete_workout(QModelIndex);

private:
    logger *logFile;
    QStringList workoutTags,metaTags,contentTags;
    QString schedulePath;
    QDate firstdayofweek;
    QMap<QDate,double> stressValues;
    void check_workoutFiles();
    void load_workoutsFiles();
    void read_dayWorkouts(QDomDocument);
    void read_weekPlan(QDomDocument,QDomDocument);
    void read_ltsFile(QDomDocument);
    void updateStress(QString,double,bool);
    void save_ltsValues();
    void saveXML(QDomDocument,QString);

    //Workout Var
    QString workout_date;
    QString workout_time;
    QString workout_calweek;
    QString workout_phase;
    QString workout_sport;
    QString workout_code;
    QString workout_title;
    QString workout_duration;
    float workout_distance;
    int workout_stress_score;

};

#endif // SCHEDULE_H
