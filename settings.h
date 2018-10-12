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
    static QStringList table_header,header_swim,header_pm,header_pace,headerTria,header_other;
    static QStringList header_int_time,header_swim_time,triaDistance;

    static QMap<int,QString> sampList,intList;
    static QMap<int,double> weightMap;
    static QHash<QString,QString> swimRange,bikeRange,runRange,triRange,stgRange,altRange,hfRange;
    static QHash<QString,int> fontMap;
    static QVector<double> tempVector;

    //Getter
    static QColor get_colorRGB(QString,bool);
    static QStringList get_colorStringList(QStringList*);

    //Setter
    static void fill_mapList(QMap<int,QString>*,QString*);
    static void fill_mapColor(QStringList*,QString*,bool);
    static void fill_mapRange(QHash<QString,QString>*,QString*);

protected:
    static QHash<QString,double> thresholdMap,ltsMap,athleteMap,modeMap;
    static QHash<QString,QString> generalMap,gcInfo,triaMap;
    static QHash<QString,QColor> colorMap;
    static QHash<QString,QStringList> listMap,jsonTags;
    static QStringList keyList,extkeyList;


public:
    settings();
    static bool settingsUpdated;
    static void loadSettings();
    static QString isAlt,isSwim,isBike,isRun,isTria,isStrength,isOther;

    //QMap/QHash Getter
    static QHash<QString,QColor> get_colorMap() {return colorMap;}
    static QHash<QString,QVector<double>> doubleMap;
    static QColor get_itemColor(QString key) {return colorMap.value(key);}
    static QString get_rangeValue(QString,QString);
    static int get_fontValue(QString key) {return fontMap.value(key);}
    static QStringList get_listValues(QString key) {return listMap.value(key);}
    static QStringList get_triaDistance() {return triaDistance;}
    static QStringList get_jsonTags(QString key) {return jsonTags.value(key);}

    //Get Map Pointer
    enum lMap {Sample,Interval};
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
        return nullptr;
    }

    enum stingMap {GC,General,Tria};
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
        if(map == Tria)
        {
            return &triaMap;
        }
        return nullptr;
    }

    enum dMap {Threshold,Athlete,LTS};
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
        return nullptr;
    }

    //QMap/QHash Setter
    static void set_rangeValue(QString,QString,QString);
    static void set_ltsValue(QString key,double value) {ltsMap.insert(key,value);}

    //common functions
    static QStringList get_int_header(QString,bool);
    static int get_timesec(QString time);
    static double get_weightforDate(QDateTime);
    static void writeListValues(QHash<QString,QStringList> *plist);
    static void autoSave() {settings::saveSettings();}
};

#endif // SETTINGS_H
