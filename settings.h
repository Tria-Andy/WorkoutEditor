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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore>
#include <QSettings>
#include <QColor>

class settings
{
private:
    static QString splitter,header_swim;
    static QStringList table_header,header_int,header_bike,header_int_time,header_swim_time;

    static QString settingFile,gcPath,valueFile,valueFilePath,breakName;
    static QStringList keyList,gc_infos,sportList,paceList,phaseList,hfList,cycleList,codeList,levelList,intPlanList,jsoninfos,swimRangeList,bikeRangeList,runRangeList,stgRangeList,hfRangeList;
    static QMap<int,QString> sampList,intList;
    static QMap<QString,double> powerMap;
    static QMap<QString,QString> gcInfo,saisonInfo;
    static QHash<QString,QColor> colorMap;
    static QVector<double> powerList,factorList;
    static QVector<int> fontSize;
    static QString act_sport,emptyPhase,emptyPhaseColor;
    static bool act_isloaded,act_isrecalc;
    static int weekRange,weekOffSet,swimLaplen;
    static void saveSettings();
    int set_range_values(QString);
    static void fill_mapList(QMap<int,QString>*,QString*);

    static QColor get_colorRGB(QString);
    static QString set_colorString(QColor);

public:
    settings();
    static void loadSettings();

    //QMap Getter
    static QString get_saisonInfo(QString key) {return saisonInfo.value(key);}
    static QString get_gcInfo(QString key) {return gcInfo.value(key);}
    static QMap<int,QString> get_sampList() {return sampList;}
    static QMap<int,QString> get_intList() {return intList;}
    static QColor get_itemColor(QString key) {return colorMap.value(key);}

    //QMap Setter
    static void set_saisonInfos(QString key, QString value){saisonInfo.insert(key,value);}
    static void set_gcInfo(QString key, QString value){gcInfo.insert(key,value);}
    static void set_itemColor(QString key,QColor value) {colorMap.insert(key,value);}

    static QString isAlt,isSwim,isBike,isRun,isTria,isStrength,isOther;
    static QString get_emptyPhase() {return emptyPhase;}
    static QString get_breakName() {return breakName;}

    static QStringList get_keyList() {return keyList;}
    static QStringList get_phaseList() {return phaseList;}
    static QStringList get_sportList() {return sportList;}
    static QStringList get_paceList() {return paceList;}
    static QStringList get_hfList() {return hfList;}
    static QStringList get_cycleList() {return cycleList;}
    static QStringList get_codeList() {return codeList;}
    static QStringList get_levelList() {return levelList;}
    static QStringList get_intPlanerList() {return intPlanList;}
    static QStringList get_jsoninfos() {return jsoninfos;}
    static QStringList get_swimRange() {return swimRangeList;}
    static QStringList get_bikeRange() {return bikeRangeList;}
    static QStringList get_runRange() {return runRangeList;}
    static QStringList get_stgRange() {return stgRangeList;}
    static QStringList get_hfRange() {return hfRangeList;}
    static int get_weekRange() {return weekRange;}
    static int get_weekOffSet() {return weekOffSet;}
    static QVector<double> *get_powerList()
    {
        QVector<double> *p = &powerList;
        return p;
    }
    static QVector<double> *get_factorList()
    {
        QVector<double> *p = &factorList;
        return p;
    }
    static QVector<int> get_fontSize() {return fontSize;}

    static int get_swimLaplen() {return swimLaplen;}
    static double calc_totalWork(double,double,double);
    static QString set_time(int);
    static QString get_workout_pace(double, QTime, QString,bool);
    static QString get_speed(QTime,int,QString,bool);
    static double estimate_stress(QString,QString,int);
    static int get_hfvalue(QString);
    static int get_timesec(QString time);
    static double set_doubleValue(double,bool);




    static QStringList get_int_header(QString);
    static QStringList get_time_header() {return header_int_time;}
    static QStringList get_swimtime_header(){return header_swim_time;}
    static void set_act_sport(QString sport) {act_sport = sport;}
    static void set_act_isload(bool isloaded) {act_isloaded = isloaded;}
    static bool get_act_isload() {return act_isloaded;}
    static void set_act_recalc(bool isrecalc) {act_isrecalc = isrecalc;}
    static bool get_act_isrecalc() {return act_isrecalc;}
    static void set_swimLaplen(int lapLen) {swimLaplen = lapLen;}
    static void writeSettings(QString,QStringList,QStringList,QStringList);
    static QString setSettingString(QStringList);
};

#endif // SETTINGS_H
