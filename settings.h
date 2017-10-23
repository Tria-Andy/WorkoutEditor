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

    static QString settingFile,valueFile,valueFilePath,splitter;
    static QStringList table_header,header_swim,header_bike,header_run,headerTria,header_other;
    static QStringList keyList,extkeyList,header_int_time,header_swim_time,triaDistance;

    static QHash<QString,QStringList> listMap;
    static QMap<int,QString> sampList,intList;
    static QHash<QString,QString> generalMap,gcInfo,triaMap;
    static QMap<int,double> weightMap;
    static QHash<QString,QColor> colorMap;
    static QHash<QString,double> thresholdMap,ltsMap,athleteMap;
    static QHash<QString,QString> swimRange,bikeRange,runRange,stgRange,altRange,hfRange;
    static QHash<QString,int> fontMap;

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

    enum dMap {Threshold,Athlete,LTS};
    enum stingMap {GC,General};
    enum lMap {Sample,Interval};

    //QMap/QHash Getter
    static QHash<QString,QStringList> get_listMap() {return listMap;}
    static QHash<QString,QString> get_triaMap() {return triaMap;}
    static QHash<QString,QColor> get_colorMap() {return colorMap;}
    static QColor get_itemColor(QString key) {return colorMap.value(key);}
    static QString get_rangeValue(QString,QString);
    static int get_fontValue(QString key) {return fontMap.value(key);}

    static QMap<int,QString>* getListMapPointer(int map)
    {
        if(map == Sample)
        {
            return &sampList;
        }
        if(map == Interval)
        {
            return &intList;
        }
        return 0;
    }

    static QHash<QString,QString>* getStringMapPointer(int map)
    {
        if(map == GC)
        {
            return &gcInfo;
        }
        if(map == General)
        {
            return &generalMap;
        }
        return 0;
    }

    static QHash<QString,double>* getdoubleMapPointer(int map)
    {
        if(map == Threshold)
        {
            return &thresholdMap;
        }
        if(map == Athlete)
        {
            return &athleteMap;
        }
        if(map == LTS)
        {
            return &ltsMap;
        }
        return 0;
    }


    //QMap/QHash Setter
    static void set_gcInfo(QString key, QString value){gcInfo.insert(key,value);}
    static void set_itemColor(QString key,QColor value) {colorMap.insert(key,value);}
    static void set_thresValue(QString key,double value) {thresholdMap.insert(key,value);}
    static void set_rangeValue(QString,QString,QString);
    static void set_ltsValue(QString key,double value) {ltsMap.insert(key,value);}
    static void set_generalValue(QString key,QString value) {generalMap.insert(key,value);}
    static void set_athleteValue(QString key,double value) {athleteMap.insert(key,value);}

    //Lists Getter
    static QStringList get_listValues(QString key) {return listMap.value(key);}
    static QStringList get_keyList() {return keyList;}
    static QStringList get_extkeyList() {return extkeyList;}

    //common functions
    static int get_timesec(QString time);
    static double get_weightforDate(QDateTime);
    static QStringList get_int_header(QString);
    static QStringList get_time_header() {return header_int_time;}
    static QStringList get_swimtime_header(){return header_swim_time;}
    static QStringList get_triaDistance() {return triaDistance;}
    static void writeListValues(QHash<QString,QStringList> *plist);
    static void autoSave() {settings::saveSettings();}
};

#endif // SETTINGS_H
