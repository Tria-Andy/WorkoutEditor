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
    double calc_totalCal(double,double,double);    
    int get_hfvalue(QString);
    QHash<QString,double> *ltsValues,*doubleValues;
    QHash<QString,QString> *gcValues,*generalValues;

    static bool usePMData;
    static double calc_totalWork(QString,double,double,int);
    static QString set_time(int);
    static QString get_workout_pace(double, QTime, QString,bool);
    static double get_speed(QTime,int,QString,bool);
    static double calc_Speed(double,double,double);
    static QString calc_lapSpeed(QString,double);
    static int calc_lapPace(QString,int,double);
    static QString calc_thresPace(double,double);
    static double calc_thresPower(double,double);
    static QString calc_duration(QString,double,QString);
    static QString threstopace(double,double);
    static double wattToSpeed(double,double);
    static int get_timesec(QString time);
    static double set_doubleValue(double,bool);
    static double get_thresPercent(QString,QString,bool);
    static double calc_distance(QString, double);
    static double estimate_stress(QString,QString,int,int);
    static double calc_swim_xpower(double,double,double,double);
    static double get_corrected_MET(double,int);
    static double calc_lnp(double,double,double);
    static double current_dayCalories(QDateTime);
    static QString calc_weekID(QDate);
    static QHash<QString,double> *thresValues, *athleteValues;

};

#endif // CALCULATION_H
