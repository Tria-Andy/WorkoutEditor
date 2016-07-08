#ifndef SETTINGS_H
#define SETTINGS_H

#include <QtCore>
#include <QSettings>

class settings
{

private:
    QString settingFile,valueFile,valueFilePath,splitter;
    QStringList keyList,gc_infos,phaseList,sportList,paceList,hfList,cycleList,codeList,levelList,intPlanList,jsoninfos,swimRangeList,bikeRangeList,runRangeList,stgRangeList,hfRangeList;
    double powerList[4];
    QStringList table_header,header_int,header_int_time, header_int_km;
    QString header_swim,header_bike,saisonFDW;
    QString gcPath,schedulePath,workoutsPath,act_sport,saison_year;
    bool act_isloaded,act_isrecalc;
    void loadSettings();
    void saveSettings();
    int set_range_values(QString);
    int saison_weeks,saison_start;

public:
    settings();
    QString isAlt,isSwim,isBike,isRun,isTria,isStrength,isOther;
    QString get_gcPath() {return gcPath;}
    QString get_schedulePath() {return schedulePath;}
    QString get_workoutsPath() {return workoutsPath;}
    QString get_saisonYear() {return saison_year;}
    QString get_saisonFDW() {return saisonFDW;}
    QStringList get_keyList() {return keyList;}
    QStringList get_gcInfo() {return gc_infos;}
    QStringList get_phaseList() {return phaseList;}
    QStringList get_sportList() {return sportList;}
    QStringList get_paceList() {return paceList;}
    QStringList get_hfList() {return hfList;}
    QStringList get_cycleList() {return cycleList;}
    QStringList get_codeList() {return codeList;}
    QStringList get_levelList() {return levelList;}
    QStringList get_intPlanerList() {return intPlanList;}
    QStringList get_jsoninfos() {return jsoninfos;}
    QStringList get_swimRange() {return swimRangeList;}
    QStringList get_bikeRange() {return bikeRangeList;}
    QStringList get_runRange() {return runRangeList;}
    QStringList get_stgRange() {return stgRangeList;}
    QStringList get_hfRange() {return hfRangeList;}
    int get_saisonWeeks() {return saison_weeks;}
    int get_saisonStart() {return saison_start;}

    double *get_powerList()
    {
        double *pw = powerList;
        return pw;
    }

    void set_saisonInfos(QString v_saison,QDate v_fdw ,int v_weeks,int v_start)
    {
        saison_year = v_saison;
        saison_weeks = v_weeks;
        saisonFDW = v_fdw.toString("dd.MM.yyyy");
        saison_start = v_start;
    }

    QString set_time(int);
    QString get_workout_pace(double, QTime, QString,bool);
    QString get_speed(QTime,int,QString,bool);
    double estimate_stress(QString,QString,QTime);
    int get_hfvalue(QString);
    int get_timesec(QString time);
    double set_doubleValue(double);

    QStringList get_int_header();
    QStringList get_time_header() {return header_int_time;}
    QStringList get_km_header() {return header_int_km;}
    void set_act_sport(QString sport) {act_sport = sport;}
    void set_act_isload(bool isloaded) {act_isloaded = isloaded;}
    bool get_act_isload() {return act_isloaded;}
    void set_act_recalc(bool isrecalc) {act_isrecalc = isrecalc;}
    bool get_act_isrecalc() {return act_isrecalc;}
    void writeSettings(QString,QStringList,QStringList,QStringList);
    QString setSettingString(QStringList);
};

#endif // SETTINGS_H
