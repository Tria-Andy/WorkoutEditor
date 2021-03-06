#ifndef STRESS_POPUP_H
#define STRESS_POPUP_H

#include <QDialog>
#include "schedule.h"
#include "settings.h"
#include "qcustomplot.h"

namespace Ui {
class stress_popup;
}

class stress_popup : public QDialog
{
    Q_OBJECT

public:
    explicit stress_popup(QWidget *parent = nullptr,schedule *p_sched = nullptr,const QDate startDate = QDate());
    ~stress_popup();

private slots:
    void on_toolButton_close_clicked();
    void on_dateEdit_start_dateChanged(const QDate &date);
    void on_dateEdit_end_dateChanged(const QDate &date);
    void on_pushButton_values_toggled(bool checked);
    void on_pushButton_reset_clicked();
    void selectionChanged();

private:
    Ui::stress_popup *ui;

    schedule *workSched;
    QDate startDay,firstDay;
    QIcon showNum,hideNum;
    double stressMax;
    QVector<double> xDate,yLTS,ySTS,yTSB,yStress,yDura;
    QVector<double> tsbMinMax;
    int dateRange;
    bool isLoad;
    QCPGraph *get_QCPLine(QString,QColor,QVector<double> &ydata,bool);
    QCPBars *get_QCPBar(QColor,int,bool);
    void set_itemTracer(QString,QCPGraph*,QColor,int);
    void set_itemText(QString,QFont,QVector<double> &ydata,int,bool);
    double calc_stress(double,double,double);
    void set_graph();
    void set_stressValues(QDate,QDate);
    void set_stressplot(QDate,QDate,bool);
};

#endif // STRESS_POPUP_H
