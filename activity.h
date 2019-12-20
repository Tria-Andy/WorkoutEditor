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
    void fill_actMap();

    QMap<QPair<int,QString>,QMap<QPair<int,QString>,QVector<double>>> ActivityMap;
    QList<QStandardItem*> setIntRow(int);
    QString actFile, v_date,intLabel;
    QHash<QString,QString> *fileMap;

    QMap<int,QStringList> itemHeader,avgHeader;
    QHash<QString,int> paceTimeInZone,hfTimeInZone,hfZoneAvg;
    QHash<QString,QPair<double,double>> rangeLevels;
    QHash<QString,QVector<double>> swimHFZoneFactor;
    QStandardItem *rootItem;
    QStringList ride_items,swimType,levels;
    QVector<double> calcSpeed,calcCadence,swimTime,newDist;
    double thresLimit,swimTrack,polishFactor,hfThreshold,hfMax,actWeight;
    int distFactor,avgCounter,moveTime,intListCount;
    bool changeRowCount,isUpdated,selectInt,isTimeBased;
    QVector<bool> editRow;

    //Functions
    void read_gcActivities();
    void save_actvitiyFile();
    QMap<int,QVector<double>> get_xData(QPair<int,int>);
    QMap<QPair<int,QString>,QVector<double>> get_swimLapData(int,QPair<int,int>);
    QMap<QPair<int,QString>,QVector<double>> get_intervalData(int,QPair<int,int>);
    void set_activityHeader(QString,QStringList*);


    QString set_intervalName(int,int,double);
    QString build_lapName(QString,int,double);
    void updateInterval();
    void recalcIntTree();
    void updateSampleModel(int);
    void calcAvgValues();
    double interpolate_speed(int,int,double);

    //Swim Calculations
    void swimhfTimeInZone(bool);
    void swimTimeInZone(int,double);
    void fillRangeLevel(double,bool);
    int get_zone_values(double,int,bool);

public:
    explicit Activity();

    QMap<QString,QVector<QString>> gcActivtiesMap;
    QMap<QString,QString> ride_info;
    QStringList averageHeader,activityHeader;
    QVector<double> sampSpeed,sampSecond,avgValues;
    QStringList *infoHeader;
    QString breakName;
    double poolLength;


    QMap<QPair<int,QString>,QMap<QPair<int,QString>,QVector<double>>>* get_activityMap() {return &ActivityMap;}
    QString get_swimType(int key) {return swimType.at(key);}
    QString checkRangeLevel(double);
    QPair<int,int> get_intervalData(int key) {return intervallMap.value(key);}

    void clear_loadedActivity();
    bool read_jsonFile(QString);
    void set_activityData();
    void prepare_baseData();
    void set_intervalData(int,int,int);
    void set_workoutContent(QString);

    void writeChangedData();

    //Recalculation
    double get_int_speed(int);
    double polish_SpeedValues(double,double,double,bool);

    //Value Getter and Setter
    void set_polishFactor(double vFactor) {polishFactor = vFactor;}
    QString get_sport() {return currentSport;}

    //Averages
    void reset_avgSelection();
    void set_avgValues(int,int);

    bool isIndoor;
};

#endif // ACTIVITY_H
