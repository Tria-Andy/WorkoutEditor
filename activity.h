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

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QFile>
#include <QFileDialog>
#include <QStandardItemModel>
#include "settings.h"
#include "jsonhandler.h"
#include "xmlhandler.h"
#include "calculation.h"

class Activity : public jsonHandler, public xmlHandler, public calculation
{

private:
    QMap<QPair<int,QString>,QMap<QPair<int,QString>,QVector<double>>> activityMap;
    QHash<QString,QString> *fileMap;
    QMap<QString,QVector<double>> averageMap;
    QMap<int,QVector<double>> polishData;
    QMap<int,QStringList> itemHeader,avgHeader;
    QHash<QString,int> paceTimeInZone,hfTimeInZone,hfZoneAvg;
    QMap<QString,QPair<double,double>> rangeLevels;
    QHash<QString,QVector<double>> swimHFZoneFactor;
    QStringList levels;
    double polishFactor,hfMax,actWeight;
    int distFactor,moveTime;
    bool isUpdated,isTimeBased;


    //Functions
    QMap<int,QVector<double>> get_xData(QPair<int,int>);
    QMap<QPair<int,QString>,QVector<double>> get_swimLapData(int,QPair<int,int>);
    QMap<QPair<int,QString>,QVector<double>> get_intervalData(int,QPair<int,int>);
    QMap<QPair<int,QString>,QVector<double>> get_simpleData(int,QPair<int,int>);
    QString set_intervalInfo(QTreeWidgetItem *,bool);
    QVector<double> calc_avgValues(QTreeWidgetItem*);
    double interpolate_speed(int,int,double);

    void set_activityHeader(QString,QStringList*);
    void set_polishData();
    void extend_activityHeader();
    void fill_rangeLevel(bool);

public:
    explicit Activity();

    QMap<QDateTime,QVector<QString>> gcActivtiesMap;
    QMap<QString,QString> activityInfo;
    QStringList averageHeader,activityHeader,swimType;
    QVector<double> sampSpeed,sampSecond;
    QStringList *infoHeader;
    QString breakName;
    double poolLength;
    bool isIndoor;

    QPair<int,QVector<double>> set_averageMap(QTreeWidgetItem*,int);
    QPair<double,double> get_polishMinMax(double);
    QString checkRangeLevel(double);
    QString set_intervalName(QTreeWidgetItem*,bool);
    double polish_SpeedValues(double,double,bool);
    double polish_powerValues(double,double,int);

    bool clear_loadedActivity();
    bool read_jsonFile(QString,bool);
    void check_activityFiles();

    void update_activityMap(QPair<int,QString>,QMap<QPair<int,QString>,QVector<double>>);
    void update_intervalMap(int,QString,QPair<int,int>);
    void update_xDataMap(int,QVector<double>);
    void update_paceInZone(QPair<QString,QString>,int);
    void update_moveTime(int value) {moveTime = value;}
    void set_swimTimeInZone(bool);
    void prepare_save();
    void set_activityData();
    void prepare_baseData();
    void set_intervalData(int,int,int);
    void set_overrideData(QString key, QString value) {overrideData.insert(key,value);}
    void set_workoutContent(QString);
    void prepare_mapToJson();
    void set_polishFactor(double vFactor) {polishFactor = 0.1-vFactor/100;}

    QMap<QPair<int,QString>,QMap<QPair<int,QString>,QVector<double>>>* get_activityMap() {return &activityMap;}
    QMap<int,QVector<double>>* get_polishData() {return &polishData;}
    QMap<QString,QVector<double>>* get_averageMap() {return &averageMap;}
    QString get_swimType(int key) {return swimType.at(key);}
    QPair<int,int> get_intervalData(int key) {return intervallMap.value(key);}
    QVector<double> get_sampleData(int key) {return sampleMap.value(key);}

};

#endif // ACTIVITY_H
