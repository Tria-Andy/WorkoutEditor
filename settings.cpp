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

#include "settings.h"
#include <QApplication>
#include <QDebug>
#include <QDateTime>
#include <QDesktopWidget>

settings::settings()
{
}

QString settings::settingFile;
QString settings::splitter = "/";

QHash<QString,QString> settings::gcInfo;
QHash<QString,QColor> settings::colorMap;
QHash<QString,int> settings::generalMap;
QHash<QString,QString> settings::saisonInfo;

QString settings::valueFile;
QString settings::valueFilePath;

QString settings::isSwim;
QString settings::isBike;
QString settings::isRun;
QString settings::isStrength;
QString settings::isAlt;
QString settings::isTria;
QString settings::isOther;

QMap<int,QString> settings::sampList;
QMap<int,QString> settings::intList;
QHash<QString,double> settings::thresholdMap;
QHash<QString,double> settings::ltsMap;
QHash<QString,QString> settings::swimRange;
QHash<QString,QString> settings::bikeRange;
QHash<QString,QString> settings::runRange;
QHash<QString,QString> settings::stgRange;
QHash<QString,QString> settings::hfRange;

QStringList settings::keyList;
QStringList settings::extkeyList;
QStringList settings::sportList;
QStringList settings::sportUseList;
QStringList settings::phaseList;
QStringList settings::cycleList;
QStringList settings::codeList;
QStringList settings::levelList;
QStringList settings::intPlanList;
QStringList settings::jsoninfos;

bool settings::act_isloaded = false;
bool settings::act_isrecalc = false;

QStringList settings::header_int;
QStringList settings::header_bike;
QStringList settings::header_int_time;
QStringList settings::header_swim_time;
QStringList settings::table_header;
QString settings::header_swim;

int settings::swimLaplen;

enum {SPORT,LEVEL,PHASE,CYCLE,WCODE,JFILE,EDITOR};
enum {SPORTUSE};

void settings::fill_mapList(QMap<int,QString> *map, QString *values)
{
    QStringList list = values->split(splitter);

    for(int i = 0; i < list.count(); ++i)
    {
        map->insert(i,list.at(i));
    }
}

void settings::fill_mapColor(QStringList *stringList, QString *colorString,bool trans)
{
    for(int i = 0; i < stringList->count(); ++i)
    {
        colorMap.insert(stringList->at(i),settings::get_colorRGB(colorString->split(splitter).at(i),trans));
    }
}

void settings::fill_mapRange(QHash<QString, QString> *map, QString *values)
{
    QStringList list = values->split(splitter);
    for(int i = 0; i < levelList.count(); ++i)
    {
        map->insert(levelList.at(i),list.at(i));
    }
}


QStringList settings::get_colorStringList(QStringList *stringList)
{
    QStringList colorList;
    for(int i = 0; i < stringList->count(); ++i)
    {
        colorList << settings::set_colorString(colorMap.value(stringList->at(i)));
    }
    return colorList;
}

void settings::loadSettings()
{
    header_int << "Interval" << "Duration" << "Distance" << "Distance (Int)" << "Pace";
    header_int_time << "Interval" << "Start Sec" << "Stop Sec" << "Distance";
    header_swim_time << "Lap" << "Start" << "Time" << "Strokes" << "Speed";
    header_swim = "Swim Laps";
    header_bike << "Watt" << "CAD";

    settingFile = QApplication::applicationDirPath() + QDir::separator() +"WorkoutEditor.ini";

    //General Settings
    if(QFile(settingFile).exists())
    {
        QSettings *mysettings = new QSettings(settingFile,QSettings::IniFormat);
        mysettings->beginGroup("GoldenCheetah");
            gcInfo.insert("regPath",mysettings->value("regPath").toString());
            gcInfo.insert("dir",mysettings->value("dir").toString());
            gcInfo.insert("athlete",mysettings->value("athlete").toString());
            gcInfo.insert("yob",mysettings->value("yob").toString());
            gcInfo.insert("folder",mysettings->value("folder").toString());
            gcInfo.insert("gcpath",mysettings->value("gcpath").toString());
        mysettings->endGroup();

        if(gcInfo.value("gcpath").isEmpty())
        {
            QSettings gc_reg(gcInfo.value("regPath"),QSettings::NativeFormat);
            QString gc_dir = gc_reg.value(gcInfo.value("dir")).toString();
            QString gcPath = gc_dir + QDir::separator() + gcInfo.value("athlete") + QDir::separator() + gcInfo.value("folder");
            gcInfo.insert("gcpath",QDir::toNativeSeparators(gcPath));
        }
        mysettings->beginGroup("Filepath");
            gcInfo.insert("schedule",mysettings->value("schedule").toString());
            gcInfo.insert("workouts",mysettings->value("workouts").toString());
            gcInfo.insert("valuefile",mysettings->value("valuefile").toString());
            valueFile = mysettings->value("valuefile").toString();
        mysettings->endGroup();

        //Sport Value Settings
        if(gcInfo.value("schedule").isEmpty())
        {
            valueFilePath = QApplication::applicationDirPath() + QDir::separator() + valueFile;
        }
        else
        {
            valueFilePath = gcInfo.value("workouts") + QDir::separator() + valueFile;
        }
        QSettings *myvalues = new QSettings(valueFilePath,QSettings::IniFormat);

//###########################Upgrade values ini####################################
        myvalues->beginGroup("Common");
            QString com_childs = myvalues->value("ltsdays").toString();
            if(com_childs.isEmpty()) myvalues->setValue("ltsdays","42");
            com_childs = myvalues->value("stsdays").toString();
            if(com_childs.isEmpty()) myvalues->setValue("stsdays","7");
            com_childs = myvalues->value("lastlts").toString();
            if(com_childs.isEmpty()) myvalues->setValue("lastlts","0");
            com_childs = myvalues->value("laststs").toString();
            if(com_childs.isEmpty()) myvalues->setValue("laststs","0");
        myvalues->endGroup();

        myvalues->beginGroup("Level");
        QStringList levColor,levList;
            QString lev_childs = myvalues->value("levels").toString();
            levList << lev_childs.split(splitter);
            lev_childs = myvalues->value("color").toString();
            if(levList.count() > 0 && lev_childs.isEmpty())
            {
                for(int i = 0; i < levList.count();++i)
                {
                    levColor.insert(i,"230-230-230");
                }
                myvalues->setValue("color",settings::setSettingString(levColor));
            }
        myvalues->endGroup();

        myvalues->beginGroup("Keylist");
            QString kList_childs = myvalues->value("extkeys").toString();
            if(kList_childs.isEmpty())
            {
                myvalues->setValue("extkeys","Sportuse");
            }
        myvalues->endGroup();

        myvalues->beginGroup("Sport");
            QString sportuse_childs = myvalues->value("sportuse").toString();
            if(sportuse_childs.isEmpty())
            {
                sportuse_childs = myvalues->value("sports").toString();
                QStringList tempUse(sportuse_childs.split(splitter));
                if(tempUse.count() > 5)
                {
                    for(int i = 5; i < tempUse.count();++i)
                    {
                        tempUse.removeAt(i);
                    }
                    tempUse.removeLast();
                }
                myvalues->setValue("sportuse",settings::setSettingString(tempUse));
            }
        myvalues->endGroup();

        myvalues->beginGroup("Threshold");
            QStringList thresList;
            QString thres_childs = myvalues->value("pace").toString();
            if(!thres_childs.isEmpty())
            {
                thresList << thres_childs.split(splitter);
                myvalues->setValue("swimpace",QString::number(settings::get_timesec(thresList.at(0))));
                myvalues->setValue("bikepace",QString::number(settings::get_timesec(thresList.at(1))));
                myvalues->setValue("runpace",QString::number(settings::get_timesec(thresList.at(2))));
                myvalues->remove("pace");
            }
            thresList.clear();
            thres_childs = myvalues->value("hf").toString();
            if(!thres_childs.isEmpty())
            {
                thresList << thres_childs.split(splitter);
                myvalues->setValue("hfthres",thresList.at(0));
                myvalues->setValue("hfmax",thresList.at(1));
                myvalues->remove("hf");
            }
        myvalues->endGroup();
//###########################Upgrade values ini done####################################

        myvalues->beginGroup("Common");
            ltsMap.insert("ltsdays",myvalues->value("ltsdays").toDouble());
            ltsMap.insert("stsdays",myvalues->value("stsdays").toDouble());
            ltsMap.insert("lastlts",myvalues->value("lastlts").toDouble());
            ltsMap.insert("laststs",myvalues->value("laststs").toDouble());
        myvalues->endGroup();

        myvalues->beginGroup("JsonFile");
            QString json_childs = myvalues->value("actinfo").toString();
            jsoninfos << json_childs.split(splitter);
            json_childs = myvalues->value("intInfo").toString();
            fill_mapList(&intList,&json_childs);
            json_childs = myvalues->value("sampinfo").toString();
            fill_mapList(&sampList,&json_childs);
        myvalues->endGroup();

        myvalues->beginGroup("Keylist");
            QString key_childs = myvalues->value("keys").toString();
            keyList << key_childs.split(splitter);
            key_childs = myvalues->value("extkeys").toString();
            extkeyList << key_childs.split(splitter);
        myvalues->endGroup();

        myvalues->beginGroup("Saisoninfo");
            saisonInfo.insert("saison",myvalues->value("saison").toString());
            saisonInfo.insert("startDate",myvalues->value("startDate").toString());
            saisonInfo.insert("startkw",myvalues->value("startkw").toString());
            saisonInfo.insert("endDate",myvalues->value("endDate").toString());
            saisonInfo.insert("weeks",myvalues->value("weeks").toString());
        myvalues->endGroup();

        myvalues->beginGroup("Sport");
            QString sport_childs = myvalues->value("sports").toString();
            sportList << sport_childs.split(splitter);
            sport_childs = myvalues->value("sportuse").toString();
            sportUseList << sport_childs.split(splitter);
            sport_childs = myvalues->value("color").toString();
            settings::fill_mapColor(&sportList,&sport_childs,false);
        myvalues->endGroup();

        myvalues->beginGroup("Threshold");
            thresholdMap.insert("swimpower",myvalues->value("swimpower").toDouble());
            thresholdMap.insert("bikepower",myvalues->value("bikepower").toDouble());
            thresholdMap.insert("runpower",myvalues->value("runpower").toDouble());
            thresholdMap.insert("stgpower",myvalues->value("stgpower").toDouble());
            thresholdMap.insert("swimfactor",myvalues->value("swimfactor").toDouble());
            thresholdMap.insert("bikefactor",myvalues->value("bikefactor").toDouble());
            thresholdMap.insert("runfactor",myvalues->value("runfactor").toDouble());
            thresholdMap.insert("swimpace",myvalues->value("swimpace").toDouble());
            thresholdMap.insert("bikepace",myvalues->value("bikepace").toDouble());
            thresholdMap.insert("runpace",myvalues->value("runpace").toDouble());
            thresholdMap.insert("hfthres",myvalues->value("hfthres").toDouble());
            thresholdMap.insert("hfmax",myvalues->value("hfmax").toDouble());
        myvalues->endGroup();

        myvalues->beginGroup("Level");
            QString level_childs = myvalues->value("levels").toString();
            levelList << level_childs.split(splitter);
            level_childs = myvalues->value("color").toString();
            settings::fill_mapColor(&levelList,&level_childs,true);
            gcInfo.insert("breakName",myvalues->value("breakname").toString());
        myvalues->endGroup();

        myvalues->beginGroup("Range");
            QString range_childs = myvalues->value("swim").toString();
            settings::fill_mapRange(&swimRange,&range_childs);
            range_childs = myvalues->value("bike").toString();
            settings::fill_mapRange(&bikeRange,&range_childs);
            range_childs = myvalues->value("run").toString();
            settings::fill_mapRange(&runRange,&range_childs);
            range_childs = myvalues->value("strength").toString();
            settings::fill_mapRange(&stgRange,&range_childs);
            range_childs = myvalues->value("hf").toString();
            settings::fill_mapRange(&hfRange,&range_childs);
        myvalues->endGroup();

        myvalues->beginGroup("Phase");
            QString phase_childs = myvalues->value("phases").toString();
            phaseList << phase_childs.split(splitter);
            phase_childs = myvalues->value("color").toString();
            settings::fill_mapColor(&phaseList,&phase_childs,false);
            gcInfo.insert("emptyPhase",myvalues->value("empty").toString());
            colorMap.insert("emptycolor",settings::get_colorRGB(myvalues->value("emptycolor").toString(),false));
        myvalues->endGroup();

        myvalues->beginGroup("Cycle");
            QString cycle_childs = myvalues->value("cycles").toString();
            cycleList << cycle_childs.split(splitter);
        myvalues->endGroup();

        myvalues->beginGroup("WorkoutCode");
            QString work_childs = myvalues->value("codes").toString();
            codeList << work_childs.split(splitter);
        myvalues->endGroup();

        myvalues->beginGroup("IntEditor");
            QString intPlaner_childs = myvalues->value("parts").toString();
            intPlanList << intPlaner_childs.split(splitter);
        myvalues->endGroup();

        for(int i = 0; i < sportList.count(); ++i)
        {
            if(sportList.at(i) == "Swim") isSwim = sportList.at(i);
            else if(sportList.at(i) == "Bike") isBike = sportList.at(i);
            else if(sportList.at(i) == "Run") isRun = sportList.at(i);
            else if(sportList.at(i) == "Strength" || sportList.at(i) == "Power") isStrength = sportList.at(i);
            else if(sportList.at(i) == "Alt" || sportList.at(i) == "Alternativ") isAlt = sportList.at(i);
            else if(sportList.at(i) == "Tria" || sportList.at(i) == "Triathlon") isTria = sportList.at(i);
            else if(sportList.at(i) == "Other") isOther = sportList.at(i);
        }

        QDesktopWidget desk;
        int screenHeight = desk.screenGeometry(0).height();

        if(screenHeight > 1000)
        {
            generalMap.insert("weekRange",8);
            generalMap.insert("weekOffSet",12);
            generalMap.insert("fontBig",16);
            generalMap.insert("fontMedium",14);
            generalMap.insert("fontSmall",12);
        }
        else
        {
            generalMap.insert("weekRange",6);
            generalMap.insert("weekOffSet",8);
            generalMap.insert("fontBig",14);
            generalMap.insert("fontMedium",12);
            generalMap.insert("fontSmall",10);
        }

        delete mysettings;
        delete myvalues;
    }
}

QString settings::get_rangeValue(QString map, QString key)
{
    if(map == settings::isSwim) return swimRange.value(key);
    if(map == settings::isBike) return bikeRange.value(key);
    if(map == settings::isRun) return runRange.value(key);
    if(map == settings::isStrength) return stgRange.value(key);
    if(map == "HF") return hfRange.value(key);

    return 0;
}

void settings::set_rangeValue(QString map, QString key,QString value)
{
    if(map == settings::isSwim) swimRange.insert(key,value);
    if(map == settings::isBike) bikeRange.insert(key,value);
    if(map == settings::isRun)  runRange.insert(key,value);
    if(map == settings::isStrength) stgRange.insert(key,value);
    if(map == "HF") hfRange.insert(key,value);
}

void settings::writeListValues(QHash<QString,QStringList> *plist)
{
    sportList = plist->value(keyList.at(SPORT));
    levelList = plist->value(keyList.at(LEVEL));
    phaseList = plist->value(keyList.at(PHASE));
    cycleList = plist->value(keyList.at(CYCLE));
    codeList = plist->value(keyList.at(WCODE));
    jsoninfos = plist->value(keyList.at(JFILE));
    intPlanList = plist->value(keyList.at(EDITOR));
    sportUseList = plist->value(extkeyList.at(SPORTUSE));

    settings::saveSettings();
}

QString settings::setSettingString(QStringList list)
{
    QString setValue;

    for(int i = 0 ; i < list.count(); ++i)
    {
        setValue.append(list.at(i)+splitter);
    }
    setValue.remove(setValue.length()-1,1);
    return setValue;
}

QStringList settings::setRangeString(QHash<QString, QString> *hash)
{
    QStringList rangeList;

    for(int i = 0; i < levelList.count(); ++i)
    {
        rangeList.insert(i,hash->value(levelList.at(i)));
    }
    return rangeList;
}

void settings::saveSettings()
{
    QStringList tempColor;
    QSettings *mysettings = new QSettings(settingFile,QSettings::IniFormat);

    mysettings->beginGroup("GoldenCheetah");
        mysettings->setValue("dir",gcInfo.value("dir"));
        mysettings->setValue("athlete",gcInfo.value("athlete"));
        mysettings->setValue("yob",gcInfo.value("yob"));
        mysettings->setValue("folder",gcInfo.value("folder"));
        mysettings->setValue("gcpath",gcInfo.value("gcpath"));
    mysettings->endGroup();

    mysettings->beginGroup("Filepath");
        mysettings->setValue("schedule",gcInfo.value("schedule"));
        mysettings->setValue("workouts",gcInfo.value("workouts"));
    mysettings->endGroup();

    QSettings *myvalues = new QSettings(valueFilePath,QSettings::IniFormat);

    myvalues->beginGroup("Common");
        myvalues->setValue("ltsdays",ltsMap.value("ltsdays"));
        myvalues->setValue("stsdays",ltsMap.value("stsdays"));
        myvalues->setValue("lastlts",ltsMap.value("lastlts"));
        myvalues->setValue("laststs",ltsMap.value("laststs"));
    myvalues->endGroup();

    myvalues->beginGroup("Threshold");
        myvalues->setValue("swimpower",QString::number(thresholdMap.value("swimpower")));
        myvalues->setValue("swimpace",QString::number(thresholdMap.value("swimpace")));
        myvalues->setValue("swimfactor",QString::number(thresholdMap.value("swimfactor")));
        myvalues->setValue("bikepower",QString::number(thresholdMap.value("bikepower")));
        myvalues->setValue("bikepace",QString::number(thresholdMap.value("bikepace")));
        myvalues->setValue("bikefactor",QString::number(thresholdMap.value("bikefactor")));
        myvalues->setValue("runpower",QString::number(thresholdMap.value("runpower")));
        myvalues->setValue("runpace",QString::number(thresholdMap.value("runpace")));
        myvalues->setValue("runfactor",QString::number(thresholdMap.value("runfactor")));
        myvalues->setValue("stgpower",QString::number(thresholdMap.value("stgpower")));
        myvalues->setValue("hfthres",QString::number(thresholdMap.value("hfthres")));
        myvalues->setValue("hfmax",QString::number(thresholdMap.value("hfmax")));
    myvalues->endGroup();

    myvalues->beginGroup("Saisoninfo");
        myvalues->setValue("saison",saisonInfo.value("saison"));
        myvalues->setValue("weeks",saisonInfo.value("weeks"));
        myvalues->setValue("startkw",saisonInfo.value("startkw"));
        myvalues->setValue("startDate",saisonInfo.value("startDate"));
        myvalues->setValue("endDate",saisonInfo.value("endDate"));
    myvalues->endGroup();

    myvalues->beginGroup("Sport");
        myvalues->setValue("sports",settings::setSettingString(sportList));
        myvalues->setValue("sportuse",settings::setSettingString(sportUseList));
        tempColor = settings::get_colorStringList(&sportList);
        myvalues->setValue("color",settings::setSettingString(tempColor));
        tempColor.clear();
    myvalues->endGroup();

    myvalues->beginGroup("Phase");
        myvalues->setValue("phases",settings::setSettingString(phaseList));
        tempColor = settings::get_colorStringList(&phaseList);
        myvalues->setValue("color",settings::setSettingString(tempColor));
        tempColor.clear();
    myvalues->endGroup();

    myvalues->beginGroup("Level");
        myvalues->setValue("levels",settings::setSettingString(levelList));
        tempColor = settings::get_colorStringList(&levelList);
        myvalues->setValue("color",settings::setSettingString(tempColor));
    myvalues->endGroup();

    myvalues->beginGroup("Range");
        myvalues->setValue("swim",settings::setSettingString(settings::setRangeString(&swimRange)));
        myvalues->setValue("bike",settings::setSettingString(settings::setRangeString(&bikeRange)));
        myvalues->setValue("run",settings::setSettingString(settings::setRangeString(&runRange)));
        myvalues->setValue("strength",settings::setSettingString(settings::setRangeString(&stgRange)));
        myvalues->setValue("hf",settings::setSettingString(settings::setRangeString(&hfRange)));
    myvalues->endGroup();

    myvalues->beginGroup("Cycle");
        myvalues->setValue("cycles",settings::setSettingString(cycleList));
    myvalues->endGroup();

    myvalues->beginGroup("WorkoutCode");
        myvalues->setValue("codes",settings::setSettingString(codeList));
    myvalues->endGroup();

    myvalues->beginGroup("JsonFile");
        myvalues->setValue("actinfo",settings::setSettingString(jsoninfos));
    myvalues->endGroup();

    myvalues->beginGroup("IntEditor");
        myvalues->setValue("parts",settings::setSettingString(intPlanList));
    myvalues->endGroup();

    delete myvalues;
}

QStringList settings::get_int_header(QString vSport)
{
    table_header.clear();
    if(vSport == isSwim) return table_header << header_int << header_swim;
    if(vSport == isBike) return table_header << header_int << header_bike;

    return header_int;
}

QString settings::set_time(int sec)
{
    if(sec < 3600)
    {
        return QDateTime::fromTime_t(sec).toUTC().toString("mm:ss");
    }
    if(sec >= 3600 && sec < 86400)
    {
        return QDateTime::fromTime_t(sec).toUTC().toString("hh:mm:ss");
    }
    if(sec >=86400)
    {
        QString timeVal;
        int hours, minutes;
        hours = sec / 60 / 60;
        minutes = (sec - (hours*60*60)) / 60;
        timeVal = QString::number(hours) + ":" + QString::number(minutes);
        return timeVal;
    }
    return 0;
}

int settings::get_timesec(QString time)
{
    int sec = 0;
    if(time.length() == 8)
    {
        QTime durtime = QTime::fromString(time,"hh:mm:ss");
        sec = durtime.hour()*60*60;
        sec = sec + durtime.minute()*60;
        sec = sec + durtime.second();
    }
    if(time.length() == 5)
    {
        QTime durtime = QTime::fromString(time,"mm:ss");
        sec = durtime.minute()*60;
        sec = sec + durtime.second();
    }

    return sec;
}

QString settings::get_workout_pace(double dist, QTime duration,QString sport,bool full_label)
{
    QStringList speedLabel;
    speedLabel << " min/km - " << " km/h" << " min/km" << "no Speed";
    int nr=0;
    double speed = 0;

    int sec = duration.hour()*60*60;
    sec = sec + duration.minute()*60;
    sec = sec + duration.second();

    int min = duration.hour()*60;
    min = min + duration.minute();

    if(dist != 0.0 || min != 0)
    {
        if(sport == isSwim) speed = sec/dist, nr=0;
        if(sport == isBike)
        {
            if(min > 0)
            {
                speed = (dist/min)*60, nr=1;
            }
            else
            {
                speed = (dist/sec)*3600, nr=1;
            }
        }
        if(sport == isRun)  speed = sec/dist, nr=2;
        if(sport == isAlt || sport == isStrength) speed = 0.0, nr=3;

        speed = settings::set_doubleValue(speed,false);

        if(full_label)
        {
            if(nr == 0) return (QDateTime::fromTime_t(speed).toUTC().toString("mm:ss") + speedLabel.at(nr) + QDateTime::fromTime_t(speed/10.0).toUTC().toString("mm:ss") +" min/100m");
            if(nr == 1) return (QString::number(speed) + speedLabel.at(nr));
            if(nr == 2) return (QDateTime::fromTime_t(speed).toUTC().toString("mm:ss") + speedLabel.at(nr));
            if(nr == 3) return speedLabel.at(nr);
        }
        else
        {
            if(nr == 0) return (QDateTime::fromTime_t(speed/10.0).toUTC().toString("mm:ss") +" min/100m");
            if(nr == 1) return (QString::number(speed) + speedLabel.at(nr));
            if(nr == 2) return (QDateTime::fromTime_t(speed).toUTC().toString("mm:ss") + speedLabel.at(nr));
            if(nr == 3) return speedLabel.at(nr);
        }
    }
    else
    {
        speed = 0.0;
    }

    return speedLabel.at(3);
}

QString settings::get_speed(QTime pace,int dist,QString sport,bool fixdist)
{
    int sec = pace.minute()*60;
    sec = sec + pace.second();
    double speed;

    if(sec > 0)
    {
        if(fixdist)
        {
            if(sport == settings::isSwim)
            {
                speed = 360.0/sec;
            }
            else
            {
                speed = 3600.0/sec;

            }
        }
        else
        {
            speed = (3600.0/sec) / (1000.0/dist);
        }
        return QString::number(speed);
    }

    return "--";
}

int settings::get_hfvalue(QString percent)
{
    double value = percent.toDouble();

    return static_cast<int>(round(thresholdMap.value("hfthres") * (value / 100.0)));
}

double settings::calc_totalWork(double weight,double avgHF, double moveTime)
{
    int age = QDate::currentDate().year() - gcInfo.value("yob").toInt();

    return ceil(((-55.0969 + (0.6309 * avgHF) + (0.1988 * weight) + (0.2017 * age))/4.184) * moveTime/60);
}

double settings::estimate_stress(QString sport, QString p_goal, int duration)
{
    double goal = 0;
    double est_stress = 0;
    double est_power = 0;
    double raw_effort = 0;
    double cv_effort = 0;
    double thresPower = 0;
    if(sport == settings::isSwim)
    {
        goal = settings::get_timesec(p_goal);
    }
    if(sport == settings::isBike)
    {
        goal = p_goal.toDouble();
    }
    if(sport == settings::isRun)
    {
        goal = settings::get_timesec(p_goal);
    }
    if(sport == settings::isStrength)
    {
        goal = p_goal.toDouble();
    }


    if(goal > 0)
    {
        if(sport == settings::isSwim)
        {
            thresPower = thresholdMap.value("swimpower");
            goal = thresholdMap.value("swimpace") / goal;
            goal = pow(goal,3.0);
            est_power = thresPower * goal;
            raw_effort = (duration * est_power) * (est_power / thresPower);
            cv_effort = thresPower * 3600;

        }
        if(sport == settings::isBike)
        {
            thresPower = thresholdMap.value("bikepower");
            raw_effort = (duration * goal) * (goal / thresPower);
            cv_effort = thresPower * 3600;
        }
        if(sport == settings::isRun)
        {
            thresPower = thresholdMap.value("runpower");
            est_power = thresPower * (thresholdMap.value("runpace")/goal);
            raw_effort = (duration * est_power) * (est_power / thresPower);
            cv_effort = thresPower * 3600;

        }
        if(sport == settings::isStrength)
        {
            thresPower = thresholdMap.value("stgpower");
            raw_effort = (duration * goal) * (goal / thresPower);
            cv_effort = thresPower * 3600;
        }
        est_stress = (raw_effort / cv_effort) * 100;
        return settings::set_doubleValue(est_stress,false);
    }
    return 0;
}

double settings::set_doubleValue(double value, bool isthree)
{
    if(isthree)
    {
        return value = round( value * 1000.0 ) / 1000.0;
    }
    else
    {
        return ((static_cast<int>(value *100 +.5)) / 100.0);
    }
    return 0;
}

QColor settings::get_colorRGB(QString colorValue,bool trans)
{
    QColor color;
    QString cRed,cGreen,cBlue;
    int aValue = 0;
    cRed = colorValue.split("-").at(0);
    cGreen = colorValue.split("-").at(1);
    cBlue = colorValue.split("-").at(2);
    if(trans)
    {
        aValue = 125;
    }
    else
    {
        aValue = 255;
    }

    color.setRgb(cRed.toInt(),cGreen.toInt(),cBlue.toInt(),aValue);

    return color;
}

QString settings::set_colorString(QColor color)
{
    return QString::number(color.red())+"-"+QString::number(color.green())+"-"+QString::number(color.blue());;
}
