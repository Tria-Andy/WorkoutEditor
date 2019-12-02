#ifndef CALCULATION_H
#define CALCULATION_H

#include "settings.h"
#include "qcustomplot.h"

class calculation
{
public:
    calculation();

private:

protected:
    QString currentSport;
    int thresPower,thresPace,thresSpeed;
    bool usePMData,isSwim,isBike,isRun,isStrength,isAlt,isOther,isTria;

    int calc_thresPace(double) const;
    int get_hfvalue(QString) const;
    int get_secFromTime(QTime) const;

    double calc_totalCal(double,double,double) const;
    double calc_stressScore(double,int) const;
    double calc_swim_xpower(double,double,double,double) const;    
    double calc_thresPower(double) const;
    double calc_thresSpeed(double) const;
    double get_corrected_MET(double,int) const;
    double calc_totalWork(QString,double,double,int) const;
    QString calc_duration(double,QString) const;


    QString set_time(int) const;
    QString get_workout_pace(double, double, QString,bool) const;
    double get_speed(QTime,int,QString,bool) const;
    double calc_Speed(double,double,double) const;
    QString calc_lapSpeed(double) const;
    int calc_lapPace(QString,int,double) const;


    QString threstopace(double,double) const;
    double wattToSpeed(double,double) const;
    int get_timesec(QString time) const;
    double set_doubleValue(double,bool) const;
    int get_thresPercent(QString,QString,bool) const;
    double calc_distance(QString, double) const;
    double estimate_stress(QString,int,int,bool) const;

    double calc_lnp(double,double,double) const;
    double current_dayCalories(QDateTime) const;
    QString calc_weekID(QDate);
    QHash<QString,double> *thresValues, *athleteValues;
    QHash<QString,QString> *generalValues;
};

#endif // CALCULATION_H
