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

class Activity : public jsonHandler
{

private:
    QList<QStandardItem*> setIntRow(int);
    QStandardItem *rootItem;
    QString v_date,intLabel,breakName;
    QMap<int,QStringList> itemHeader,avgHeader;
    QHash<QString,int> paceTimeInZone,hfTimeInZone,hfZoneAvg;
    QHash<QString,QPair<double,double>> rangeLevels;
    QHash<QString,QVector<double>> swimHFZoneFactor;
    QStringList ride_items,swimType,levels;
    QVector<double> calcSpeed,calcCadence,swimTime,newDist;
    double thresLimit,swimTrack,polishFactor,hfThreshold,hfMax,actWeight;
    int distFactor,avgCounter,zoneCount,moveTime,intListCount;
    bool changeRowCount,isUpdated,selectInt,isTimeBased;
    QVector<bool> editRow;

    //Functions
    void readJsonFile(QString,bool);
    void prepareData();
    void build_intTree();
    QString build_lapName(QString,int,double);
    void updateSwimLap();
    void updateSwimInt(QModelIndex,QItemSelectionModel*);
    void updateSwimBreak(QModelIndex,QItemSelectionModel*,int);
    void updateInterval();
    void recalcIntTree();
    void updateSampleModel(int);
    void calcAvgValues();
    double get_int_value(int,int);
    double interpolate_speed(int,int,double);

    //Swim Calculations
    void swimhfTimeInZone(bool);
    void swimTimeInZone(int,double);
    void fillRangeLevel(double,bool);
    QString checkRangeLevel(double);
    int get_zone_values(double,int,bool);

public:
    explicit Activity(QString jsonfile = QString(),bool intAct = false);

    void set_selectedItem(QItemSelectionModel*);
    void set_editRow(QString,bool);
    void showSwimLap(bool);
    void showInterval(bool);
    void updateIntModel(int,int);
    void set_workoutContent(QString);
    void updateXDataModel();
    void writeChangedData();
    QHash<int,QModelIndex> selItem,avgItems;
    QHash<QString,int> swimPace,swimHF;
    QStandardItemModel *intModel,*sampleModel,*xdataModel,*intTreeModel,*selItemModel,*avgModel;
    QMap<QString,QString> ride_info;
    QVector<double> sampSpeed,sampSecond,avgValues;

    //Recalculation
    void updateRow_intTree(QItemSelectionModel *);
    void addRow_intTree(QItemSelectionModel *);
    void removeRow_intTree(QItemSelectionModel *);
    double get_int_distance(int,int);
    int get_int_duration(int);
    int get_int_pace(int,QString);
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
