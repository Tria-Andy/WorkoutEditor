#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore>
#include <QSettings>

class settings
{
private:
    static QString version,builddate,splitter,header_swim,header_bike;
    static QStringList table_header,header_int,header_int_time,header_swim_time;

    static QString settingFile,valueFile,valueFilePath,saisonFDW,breakName;
    static QStringList keyList,gc_infos,sportList,paceList,phaseList,hfList,cycleList,codeList,levelList,intPlanList,jsoninfos,swimRangeList,bikeRangeList,runRangeList,stgRangeList,hfRangeList;
    static QStringList sportColor,phaseColor;
    static QMap<int,QString> sampList,intList;
    static QVector<double> powerList,factorList;
    static QVector<int> fontSize;
    static QString gcPath,schedulePath,workoutsPath,act_sport,saison_year,emptyPhase,emptyPhaseColor;
    static bool act_isloaded,act_isrecalc;
    static int saison_weeks,saison_start,athleteYOB;
    static int weekRange,weekOffSet,swimLaplen;
    static void saveSettings();
    int set_range_values(QString);
    static void fill_mapList(QMap<int,QString>*,QString*);

public:
    settings();
    static void loadSettings();

    static QString isAlt,isSwim,isBike,isRun,isTria,isStrength,isOther;
    static QString get_version() {return version;}
    static QString get_builddate() {return builddate;}
    static QString get_gcPath() {return gcPath;}
    static QString get_schedulePath() {return schedulePath;}
    static QString get_workoutsPath() {return workoutsPath;}
    static QString get_saisonYear() {return saison_year;}
    static QString get_saisonFDW() {return saisonFDW;}
    static QString get_emptyPhaseColor() {return emptyPhaseColor;}
    static QString get_emptyPhase() {return emptyPhase;}
    static QString get_breakName() {return breakName;}

    static QMap<int,QString> get_sampList() {return sampList;}
    static QMap<int,QString> get_intList() {return intList;}

    static QStringList get_keyList() {return keyList;}
    static QStringList get_gcInfo() {return gc_infos;}
    static QStringList get_phaseList() {return phaseList;}
    static QStringList get_phaseColor() {return phaseColor;}
    static QStringList get_sportList() {return sportList;}
    static QStringList get_sportColor() {return sportColor;}
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
    static int get_saisonWeeks() {return saison_weeks;}
    static int get_saisonStart() {return saison_start;}
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

    static void set_saisonInfos(QString v_saison,QDate v_fdw ,int v_weeks,int v_start)
    {
        saison_year = v_saison;
        saison_weeks = v_weeks;
        saisonFDW = v_fdw.toString("dd.MM.yyyy");
        saison_start = v_start;
    }

    static int get_swimLaplen() {return swimLaplen;}
    static double calc_totalWork(double,double,double);
    static QString set_time(int);
    static QString get_workout_pace(double, QTime, QString,bool);
    static QString get_speed(QTime,int,QString,bool);
    static double estimate_stress(QString,QString,int);
    static int get_hfvalue(QString);
    static int get_timesec(QString time);
    static double set_doubleValue(double,bool);


    static QColor get_color(QString);
    static QString get_colorValues(QColor);

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
