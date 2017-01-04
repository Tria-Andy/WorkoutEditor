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
    static void saveSettings();
    static QString setSettingString(QStringList);
    static QStringList setRangeString(QHash<QString,QString>*);
    static QString set_colorString(QColor);

    static QString settingFile,valueFile,valueFilePath,splitter,header_swim;
    static QStringList keyList,extkeyList,table_header,header_int,header_bike,header_int_time,header_swim_time;

    static QHash<QString,QStringList> listMap;
    static QMap<int,QString> sampList,intList;
    static QHash<QString,QString> generalMap,gcInfo,saisonInfo;
    static QHash<QString,QColor> colorMap;
    static QHash<QString,double> thresholdMap,ltsMap;
    static QHash<QString,QString> swimRange,bikeRange,runRange,stgRange,hfRange;
    static QHash<QString,int> fontMap;
    static bool act_isloaded,act_isrecalc;
    static int swimLaplen;

    //Getter
    static QColor get_colorRGB(QString,bool);
    static QStringList get_colorStringList(QStringList*);

    //Setter
    static void fill_mapList(QMap<int,QString>*,QString*);
    static void fill_mapColor(QStringList*,QString*,bool);
    static void fill_mapRange(QHash<QString,QString>*,QString*);

public:
    settings();
    static void loadSettings();
    static QString isAlt,isSwim,isBike,isRun,isTria,isStrength,isOther;

    //QMap/QHash Getter
    static QHash<QString,QStringList> get_listMap() {return listMap;}
    static QMap<int,QString> get_sampList() {return sampList;}
    static QMap<int,QString> get_intList() {return intList;}
    static QString get_saisonInfo(QString key) {return saisonInfo.value(key);}
    static QString get_gcInfo(QString key) {return gcInfo.value(key);}
    static QColor get_itemColor(QString key) {return colorMap.value(key);}
    static QHash<QString,QColor> get_colorMap() {return colorMap;}
    static QString get_rangeValue(QString,QString);
    static double get_thresValue(QString key) {return thresholdMap.value(key);}
    static double get_ltsValue(QString key) {return ltsMap.value(key);}
    static QString get_generalValue(QString key) {return generalMap.value(key);}
    static int get_fontValue(QString key) {return fontMap.value(key);}


    //QMap/QHash Setter
    static void set_saisonInfos(QString key, QString value){saisonInfo.insert(key,value);}
    static void set_gcInfo(QString key, QString value){gcInfo.insert(key,value);}
    static void set_itemColor(QString key,QColor value) {colorMap.insert(key,value);}
    static void set_thresValue(QString key,double value) {thresholdMap.insert(key,value);}
    static void set_rangeValue(QString,QString,QString);
    static void set_ltsValue(QString key,double value) {ltsMap.insert(key,value);}
    static void set_generalValue(QString key,QString value) {generalMap.insert(key,value);}

    //Lists Getter
    static QStringList get_listValues(QString key) {return listMap.value(key);}
    static QStringList get_keyList() {return keyList;}
    static QStringList get_extkeyList() {return extkeyList;}

    //common functions
    static int get_swimLaplen() {return swimLaplen;}
    static int get_timesec(QString time);
    static QStringList get_int_header(QString);
    static QStringList get_time_header() {return header_int_time;}
    static QStringList get_swimtime_header(){return header_swim_time;}
    static void set_act_isload(bool isloaded) {act_isloaded = isloaded;}
    static bool get_act_isload() {return act_isloaded;}
    static void set_act_recalc(bool isrecalc) {act_isrecalc = isrecalc;}
    static bool get_act_isrecalc() {return act_isrecalc;}
    static void set_swimLaplen(int lapLen) {swimLaplen = lapLen;}
    static void writeListValues(QHash<QString,QStringList> *plist);
    static void autoSave() {settings::saveSettings();}
};

#endif // SETTINGS_H
