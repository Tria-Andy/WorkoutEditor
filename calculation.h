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
    double calc_totalWork(double,double,double);
    static QString set_time(int);
    static QString get_workout_pace(double, QTime, QString,bool);
    static QString get_speed(QTime,int,QString,bool);
    double estimate_stress(QString,QString,int);
    int get_hfvalue(QString);
    static int get_timesec(QString time);
    static double set_doubleValue(double,bool);

    QCPGraph *get_QCPLine(QCustomPlot*,QString,QColor,QVector<double> &ydata,bool);
    void set_itemTracer(QCustomPlot*,QCPGraph*,QColor,int);
    void set_itemText(QFont,QVector<double> &ydata,int,bool);
};

#endif // CALCULATION_H
