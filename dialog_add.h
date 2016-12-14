#ifndef DIALOG_ADD_H
#define DIALOG_ADD_H

#include <QtGui>
#include <QDialog>
#include <QTimeEdit>
#include "schedule.h"
#include "dialog_workouts.h"
#include "settings.h"
#include "standardworkouts.h"

namespace Ui {
class Dialog_add;
}

class Dialog_add : public QDialog
{
    Q_OBJECT

private:
    Ui::Dialog_add *ui;
    standardWorkouts *stdWorkouts;
    schedule *workSched;
    QDate workout_date;
    QString w_date,w_time,w_cal_week,w_duration;

    double w_distance;
    int w_stress;

    QString get_weekPhase(QDate);

public:
    explicit Dialog_add(QWidget *parent = 0,schedule *p_sched = 0, standardWorkouts *p_stdworkouts = 0);
    ~Dialog_add();

private slots:
    void on_pushButton_cancel_clicked();
    void on_pushButton_add_clicked();
    void on_doubleSpinBox_dist_valueChanged(double arg1);
    void on_timeEdit_duration_timeChanged(const QTime &time);
    void on_comboBox_sport_currentTextChanged(const QString &arg1);
    void on_pushButton_workouts_clicked();
    void on_dateEdit_wdate_dateChanged(const QDate &date);
    void on_timeEdit_time_timeChanged(const QTime &time);
    void on_spinBox_stress_score_valueChanged(int arg1);

};

#endif // DIALOG_ADD_H
