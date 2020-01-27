#ifndef CALCULATION_H
#define CALCULATION_H

#include "settings.h"
#include "qcustomplot.h"

class calculation
{
public:
    calculation();
    bool usePMData,isSwim,isBike,isRun,isStrength,isAlt,isOther,isTria;
    QString calc_lapSpeed(double) const;
    int calc_lapPace(int,double) const;
    double calc_totalWork(double,double,int) const;
    double get_speed(QTime,double,bool) const;
    QString get_currentSport() {return currentSport;}
    //QString currentSport;

private:

protected:
    QString sportMark,currentSport;
    int thresPower,thresPace,thresSpeed,hfThreshold;
    double workFactor,stressFactor;
    void set_currentSport(QString);

    int calc_thresPace(double) const;
    int calc_thresPower(double) const;
    int get_baseValue(double);
    int get_hfvalue(QString) const;
    int get_secFromTime(QTime) const;

    double calc_totalCal(double,double,double) const;
    double calc_stressScore(double,int) const;
    double calc_swim_xpower(double,double,double,double) const;    
    double calc_thresSpeed(double) const;
    double get_corrected_MET(double,int) const;

    double calc_distance(int, double) const;

    QTime calc_duration(double,int) const;
    QTime set_sectoTime(int);
    QString set_time(int) const;
    QString get_workout_pace(double, double,bool) const;

    double wattToSpeed(double,double) const;
    int get_timesec(QString time) const;
    double set_doubleValue(double,bool) const;
    int get_thresPercent(QString,bool) const;

    double estimate_stress(QString,int,int,bool) const;

    double calc_lnp(double,double,double) const;
    double current_dayCalories(QDateTime) const;
    QString calc_weekID(QDate);
    QHash<QString,double> *thresValues, *athleteValues;


    //QCP Objects
    QCPGraph *create_QCPLine(QCustomPlot*, QString,QColor,QVector<double> &xdata,QVector<double> &ydata,bool);
    QCPBars *create_QCPBar(QCustomPlot*,QColor,QVector<double> &xdata,QVector<double> &ydata,bool);
    void create_itemTracer(QCustomPlot *,QString,QCPGraph*,QVector<double> &xdata,QColor,int);
    void create_itemLineText(QCustomPlot *,QString,QFont,QVector<double> &xdata,QVector<double> &ydata,int,bool);
    void create_itemBarText(QCustomPlot *plot,QString,QFont,QColor,QVector<double> &xdata,QVector<double> &ydata,int,int,bool);

};

#endif // CALCULATION_H
