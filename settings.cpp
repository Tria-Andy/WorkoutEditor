#include "settings.h"
#include <QApplication>
#include <QDebug>
#include <QDateTime>

settings::settings()
{
    settingFile = QApplication::applicationDirPath() + "/WorkoutEditor.ini";
    splitter = "/";
    header_int << "Interval" << "Duration" << "Distance" << "Distance (Int)" << "Pace";
    header_swim = "Swim Laps";
    header_bike = "Watt";
    header_int_time << "Interval" << "Start Sec" << "Stop Sec";
    header_swim_time << "Lap" << "Start" << "Time" << "Strokes" << "Speed";
    header_int_km << "Interval" << "Distance new";
    act_isloaded = false;
    act_isrecalc = false;

    //General Settings
    if(QFile(settingFile).exists())
    {
    QSettings *mysettings = new QSettings(settingFile,QSettings::IniFormat);

    mysettings->beginGroup("GoldenCheetah");
        gc_infos << mysettings->value("regPath").toString();
        gc_infos << mysettings->value("dir").toString();
        gc_infos << mysettings->value("athlete").toString();
        gc_infos << mysettings->value("folder").toString();
    mysettings->endGroup();

    QSettings gc_reg(gc_infos.at(0),QSettings::NativeFormat);
    QString gc_dir = gc_reg.value(gc_infos.at(1)).toString();
    gcPath = gc_dir + gc_infos.at(2) + gc_infos.at(3);

    mysettings->beginGroup("Filepath");
        schedulePath = mysettings->value("schedule").toString();
        workoutsPath = mysettings->value("workouts").toString();
        valueFile = mysettings->value("valuefile").toString();
    mysettings->endGroup();

    //Sport Value Settings
    valueFilePath = workoutsPath + "/" + valueFile;
    QSettings *myvalues = new QSettings(valueFilePath,QSettings::IniFormat);

    myvalues->beginGroup("Keylist");
        QString key_childs = myvalues->value("keys").toString();
        keyList << key_childs.split(splitter);
    myvalues->endGroup();

    myvalues->beginGroup("Saisoninfo");
        saison_year = myvalues->value("saison").toString();
        saison_weeks = myvalues->value("weeks").toInt();
        saison_start = myvalues->value("startkw").toInt();
        saisonFDW = myvalues->value("fdw").toString();
    myvalues->endGroup();

    myvalues->beginGroup("Sport");
        QString sport_childs = myvalues->value("sports").toString();
        sportList << sport_childs.split(splitter);
    myvalues->endGroup();

    myvalues->beginGroup("Threshold");
        powerList[0] = myvalues->value("swimpower").toDouble();
        powerList[1] = myvalues->value("bikepower").toDouble();
        powerList[2] = myvalues->value("runpower").toDouble();
        powerList[3] = myvalues->value("stgpower").toDouble();
    myvalues->endGroup();

    myvalues->beginGroup("Threshold");
        QString thres_childs = myvalues->value("pace").toString();
        paceList << thres_childs.split(splitter);
        thres_childs = myvalues->value("hf").toString();
        hfList << thres_childs.split(splitter);
    myvalues->endGroup();

    myvalues->beginGroup("Level");
        QString level_childs = myvalues->value("levels").toString();
        levelList << level_childs.split(splitter);
    myvalues->endGroup();

    myvalues->beginGroup("Range");
        QString range_childs = myvalues->value("swim").toString();
        swimRangeList << range_childs.split(splitter);
        range_childs = myvalues->value("bike").toString();
        bikeRangeList << range_childs.split(splitter);
        range_childs = myvalues->value("run").toString();
        runRangeList << range_childs.split(splitter);
        range_childs = myvalues->value("strength").toString();
        stgRangeList << range_childs.split(splitter);
        range_childs = myvalues->value("hf").toString();
        hfRangeList << range_childs.split(splitter);
    myvalues->endGroup();

    myvalues->beginGroup("Phase");
        QString phase_childs = myvalues->value("phases").toString();
        phaseList << phase_childs.split(splitter);
    myvalues->endGroup();

    myvalues->beginGroup("Cycle");
        QString cycle_childs = myvalues->value("cycles").toString();
        cycleList << cycle_childs.split(splitter);
    myvalues->endGroup();

    myvalues->beginGroup("WorkoutCode");
        QString work_childs = myvalues->value("codes").toString();
        codeList << work_childs.split(splitter);
    myvalues->endGroup();

    myvalues->beginGroup("JsonFile");
        QString json_childs = myvalues->value("taginfo").toString();
        jsoninfos << json_childs.split(splitter);
    myvalues->endGroup();

    myvalues->beginGroup("IntEditor");
        QString intPlaner_childs = myvalues->value("parts").toString();
        intPlanList << intPlaner_childs.split(splitter);
    myvalues->endGroup();

    isSwim = sportList.at(0);
    isBike = sportList.at(1);
    isRun = sportList.at(2);
    isStrength = sportList.at(3);
    isAlt = sportList.at(4);
    isTria = sportList.at(5);
    isOther = sportList.at(6);

    delete mysettings;
    delete myvalues;

    }
}

//QStringList settings::phaseList;

void settings::writeSettings(QString selection, QStringList plist, QStringList p_paceList,QStringList p_hfList)
{    
    if(selection == keyList.at(0))
    {
        sportList = plist;
    }
    if(selection == keyList.at(1))
    {
        levelList = plist;
    }
    if(selection == keyList.at(2))
    {
        phaseList = plist;
    }
    if(selection == keyList.at(3))
    {
        cycleList = plist;
    }
    if(selection == keyList.at(4))
    {
        codeList = plist;
    }
    if(selection == keyList.at(5))
    {
        jsoninfos = plist;
    }
    if(selection == keyList.at(6))
    {
        intPlanList = plist;
    }

    paceList = p_paceList;
    hfList = p_hfList;
    saveSettings();
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

void settings::saveSettings()
{
    QSettings *myvalues = new QSettings(valueFilePath,QSettings::IniFormat);

    myvalues->beginGroup("Threshold");
        myvalues->setValue("swimpower",QString::number(powerList[0]));
        myvalues->setValue("bikepower",QString::number(powerList[1]));
        myvalues->setValue("runpower",QString::number(powerList[2]));
    myvalues->endGroup();

    myvalues->beginGroup("Saisoninfo");
        myvalues->setValue("saison",saison_year);
        myvalues->setValue("weeks",QString::number(saison_weeks));
        myvalues->setValue("startkw",QString::number(saison_start));
        myvalues->setValue("fdw",saisonFDW);
    myvalues->endGroup();

    myvalues->beginGroup("Sport");
        myvalues->setValue("sports",this->setSettingString(sportList));
    myvalues->endGroup();

    myvalues->beginGroup("Threshold");
        myvalues->setValue("pace",this->setSettingString(paceList));
        myvalues->setValue("hf",this->setSettingString(hfList));
    myvalues->endGroup();

    myvalues->beginGroup("Phase");
        myvalues->setValue("phases",this->setSettingString(phaseList));
    myvalues->endGroup();

    myvalues->beginGroup("Level");
        myvalues->setValue("levels",this->setSettingString(levelList));
    myvalues->endGroup();

    myvalues->beginGroup("Cycle");
        myvalues->setValue("cycles",this->setSettingString(cycleList));
    myvalues->endGroup();

    myvalues->beginGroup("WorkoutCode");
        myvalues->setValue("codes",this->setSettingString(codeList));
    myvalues->endGroup();

    myvalues->beginGroup("JsonFile");
        myvalues->setValue("taginfo",this->setSettingString(jsoninfos));
    myvalues->endGroup();

    myvalues->beginGroup("IntEditor");
        myvalues->setValue("parts",this->setSettingString(intPlanList));
    myvalues->endGroup();

    delete myvalues;
}

QStringList settings::get_int_header()
{
    table_header.clear();
    if(act_sport == "Swim ") return table_header << header_int << header_swim;
    if(act_sport == "Bike ") return table_header << header_int << header_bike;

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

        speed = this->set_doubleValue(speed);

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
            if(sport == this->isSwim)
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
    QString hfThres = hfList.at(0);

    return static_cast<int>(round(hfThres.toDouble() * (value / 100.0)));
}

double settings::estimate_stress(QString sport, QString p_goal, QTime duration)
{
    int sport_index;
    double goal = 0;
    double est_stress = 0;
    double est_power = 0;
    double raw_effort = 0;
    double cv_effort = 0;
    if(sport == this->isSwim)
    {
        goal = this->get_timesec(p_goal);
        sport_index = 0;
    }
    if(sport == this->isBike)
    {
        goal = p_goal.toDouble();
        sport_index = 1;
    }
    if(sport == this->isRun)
    {
        goal = this->get_timesec(p_goal);
        sport_index = 2;
    }
    if(sport == this->isStrength)
    {
        goal = p_goal.toDouble();
        sport_index = 3;
    }


    if(goal > 0)
    {
        if(sport == this->isSwim)
        {
            est_power = powerList[sport_index] * ((this->get_timesec(paceList.at(sport_index)) / (goal / this->get_timesec(paceList.at(sport_index))))/100.0);
            raw_effort = (this->get_timesec(duration.toString("mm:ss")) * est_power) * (est_power / powerList[sport_index]);
            cv_effort = powerList[sport_index] * 3600;

        }
        if(sport == this->isBike || sport == this->isStrength)
        {
            raw_effort = (this->get_timesec(duration.toString("mm:ss")) * goal) * (goal / powerList[sport_index]);
            cv_effort = powerList[sport_index] * 3600;
        }
        if(sport == this->isRun)
        {
            est_power = powerList[sport_index] * (this->get_timesec(paceList.at(sport_index)) / goal);
            raw_effort = (this->get_timesec(duration.toString("mm:ss")) * est_power) * (est_power / powerList[sport_index]);
            cv_effort = powerList[sport_index] * 3600;

        }
        est_stress = (raw_effort / cv_effort) * 100;
        return this->set_doubleValue(est_stress);
    }
    return 0;
}

double settings::set_doubleValue(double value)
{
    return ((static_cast<int>(value *100 +.5)) / 100.0);
}





