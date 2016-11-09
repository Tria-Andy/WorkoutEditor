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

QString settings::version;
QString settings::builddate;
QString settings::valueFile;
QString settings::valueFilePath;
QString settings::saisonFDW;
QString settings::gcPath;
QString settings::schedulePath;
QString settings::workoutsPath;
QString settings::act_sport;
QString settings::saison_year;
QString settings::emptyPhase;
QString settings::emptyPhaseColor;
QString settings::breakName;

QString settings::isSwim;
QString settings::isBike;
QString settings::isRun;
QString settings::isStrength;
QString settings::isAlt;
QString settings::isTria;
QString settings::isOther;

QMap<int,QString> settings::sampList;
QMap<int,QString> settings::intList;

QStringList settings::keyList;
QStringList settings::gc_infos;
QStringList settings::sportList;
QStringList settings::paceList;
QStringList settings::phaseList;
QStringList settings::hfList;
QStringList settings::cycleList;
QStringList settings::codeList;
QStringList settings::levelList;
QStringList settings::intPlanList;
QStringList settings::jsoninfos;
QStringList settings::swimRangeList;
QStringList settings::bikeRangeList;
QStringList settings::runRangeList;
QStringList settings::stgRangeList;
QStringList settings::hfRangeList;
QStringList settings::sportColor;
QStringList settings::phaseColor;

QVector<double>  settings::powerList;
QVector<double>  settings::factorList;
QVector<int> settings::fontSize;

bool settings::act_isloaded = false;
bool settings::act_isrecalc = false;

QStringList settings::header_int;
QStringList settings::header_int_time;
QStringList settings::header_swim_time;
QStringList settings::table_header;
QString settings::header_swim;
QString settings::header_bike;

int settings::saison_weeks;
int settings::saison_start;
int settings::weekRange;
int settings::weekOffSet;
int settings::swimLaplen;
int settings::athleteYOB;


void settings::fill_mapList(QMap<int,QString> *map, QString *values)
{
    QStringList list = values->split(splitter);

    for(int i = 0; i < list.count(); ++i)
    {
        map->insert(i,list.at(i));
    }
}

void settings::loadSettings()
{
    header_int << "Interval" << "Duration" << "Distance" << "Distance (Int)" << "Pace";
    header_int_time << "Interval" << "Start Sec" << "Stop Sec" << "Distance";
    header_swim_time << "Lap" << "Start" << "Time" << "Strokes" << "Speed";
    header_swim = "Swim Laps";
    header_bike = "Watt";

    powerList.resize(4);
    factorList.resize(3);
    fontSize.resize(3);

    settingFile = QApplication::applicationDirPath() + "/WorkoutEditor.ini";

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
        myvalues->beginGroup("Athlete");
            athleteYOB = myvalues->value("yob").toInt();
        myvalues->endGroup();

        myvalues->beginGroup("Version");
            version = myvalues->value("version").toString();
            builddate = myvalues->value("build").toString();
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
            sport_childs = myvalues->value("color").toString();
            sportColor << sport_childs.split(splitter);
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
            factorList[0] = myvalues->value("swimfactor").toDouble();
            factorList[1] = myvalues->value("bikefactor").toDouble();
            factorList[2] = myvalues->value("runfactor").toDouble();
            thres_childs = myvalues->value("hf").toString();
            hfList << thres_childs.split(splitter);

        myvalues->endGroup();

        myvalues->beginGroup("Level");
            QString level_childs = myvalues->value("levels").toString();
            levelList << level_childs.split(splitter);
            level_childs = myvalues->value("breakname").toString();
            breakName = level_childs;
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
            phase_childs = myvalues->value("color").toString();
            phaseColor << phase_childs.split(splitter);
            emptyPhase = myvalues->value("empty").toString();
            emptyPhaseColor = myvalues->value("emptycolor").toString();
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

        isSwim = sportList.at(0);
        isBike = sportList.at(1);
        isRun = sportList.at(2);
        isStrength = sportList.at(3);
        isAlt = sportList.at(4);
        isTria = sportList.at(5);
        isOther = sportList.at(6);

        QDesktopWidget desk;
        //int screenWight = desk.screenGeometry(0).width();
        int screenHeight = desk.screenGeometry(0).height();

        if(screenHeight > 1000)
        {
            fontSize[0] = 16;
            fontSize[1] = 14;
            fontSize[2] = 12;
            weekRange = 8;
            weekOffSet = 12;
        }
        else
        {
            fontSize[0] = 14;
            fontSize[1] = 12;
            fontSize[2] = 10;
            weekRange = 6;
            weekOffSet = 8;
        }

        delete mysettings;
        delete myvalues;
    }
}

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

void settings::saveSettings()
{
    QSettings *myvalues = new QSettings(valueFilePath,QSettings::IniFormat);

    myvalues->beginGroup("Threshold");
        myvalues->setValue("swimpower",QString::number(powerList[0]));
        myvalues->setValue("bikepower",QString::number(powerList[1]));
        myvalues->setValue("runpower",QString::number(powerList[2]));
        myvalues->setValue("swimfactor",QString::number(factorList[0]));
        myvalues->setValue("bikefactor",QString::number(factorList[1]));
        myvalues->setValue("runfactor",QString::number(factorList[2]));
    myvalues->endGroup();

    myvalues->beginGroup("Saisoninfo");
        myvalues->setValue("saison",saison_year);
        myvalues->setValue("weeks",QString::number(saison_weeks));
        myvalues->setValue("startkw",QString::number(saison_start));
        myvalues->setValue("fdw",saisonFDW);
    myvalues->endGroup();

    myvalues->beginGroup("Sport");
        myvalues->setValue("sports",settings::setSettingString(sportList));
    myvalues->endGroup();

    myvalues->beginGroup("Threshold");
        myvalues->setValue("pace",settings::setSettingString(paceList));
        myvalues->setValue("hf",settings::setSettingString(hfList));
    myvalues->endGroup();

    myvalues->beginGroup("Phase");
        myvalues->setValue("phases",settings::setSettingString(phaseList));
    myvalues->endGroup();

    myvalues->beginGroup("Level");
        myvalues->setValue("levels",settings::setSettingString(levelList));
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
    QString hfThres = hfList.at(0);

    return static_cast<int>(round(hfThres.toDouble() * (value / 100.0)));
}

double settings::calc_totalWork(double weight,double avgHF, double moveTime)
{
    int age = QDate::currentDate().year() - athleteYOB;

    return ceil(((-55.0969 + (0.6309 * avgHF) + (0.1988 * weight) + (0.2017 * age))/4.184) * moveTime/60);
}

double settings::estimate_stress(QString sport, QString p_goal, int duration)
{
    int sport_index;
    double goal = 0;
    double est_stress = 0;
    double est_power = 0;
    double raw_effort = 0;
    double cv_effort = 0;
    if(sport == settings::isSwim)
    {
        goal = settings::get_timesec(p_goal);
        sport_index = 0;
    }
    if(sport == settings::isBike)
    {
        goal = p_goal.toDouble();
        sport_index = 1;
    }
    if(sport == settings::isRun)
    {
        goal = settings::get_timesec(p_goal);
        sport_index = 2;
    }
    if(sport == settings::isStrength)
    {
        goal = p_goal.toDouble();
        sport_index = 3;
    }


    if(goal > 0)
    {
        if(sport == settings::isSwim)
        {
            goal = settings::get_timesec(paceList.at(sport_index)) / goal;
            goal = pow(goal,3.0);
            est_power = powerList[sport_index] * goal;
            raw_effort = (duration * est_power) * (est_power / powerList[sport_index]);
            cv_effort = powerList[sport_index] * 3600;

        }
        if(sport == settings::isBike || sport == settings::isStrength)
        {
            raw_effort = (duration * goal) * (goal / powerList[sport_index]);
            cv_effort = powerList[sport_index] * 3600;
        }
        if(sport == settings::isRun)
        {
            est_power = powerList[sport_index] * (settings::get_timesec(paceList.at(sport_index))/goal);
            raw_effort = (duration * est_power) * (est_power / powerList[sport_index]);
            cv_effort = powerList[sport_index] * 3600;

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

QColor settings::get_color(QString colorValue)
{
    QColor color;
    QString cRed,cGreen,cBlue;
    cRed = colorValue.split("-").at(0);
    cGreen = colorValue.split("-").at(1);
    cBlue = colorValue.split("-").at(2);
    color.setRgb(cRed.toInt(),cGreen.toInt(),cBlue.toInt());

    return color;
}

QString settings::get_colorValues(QColor color)
{
    return QString::number(color.red())+"-"+QString::number(color.green())+"-"+QString::number(color.blue());;
}
