#ifndef DIALOG_ADD_H
#define DIALOG_ADD_H

#include <QtGui>
#include <QDialog>
#include <QTimeEdit>
#include "workout.h"
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
    settings *add_settings;
    standardWorkouts *stdWorkouts;
    schedule *workSched;
    workout *curr_workout;
    Dialog_workouts *std_workouts;
    QDate *current_date;
    QDate workout_date;
    QString w_date;
    QString w_time;
    QString w_cal_week;
    QString w_duration;

    double w_distance;
    int w_stress;

    QString get_weekPhase(QDate);
public:
    explicit Dialog_add(QWidget *parent = 0,schedule *p_sched = 0, workout *p_workout = 0, settings *p_settings = 0, standardWorkouts *p_stdworkouts = 0);
    ~Dialog_add();

private slots:

    void set_workout_date();
    void set_workout_time();
    void set_workout_calweek();
    void set_workout_duration();
    void set_workout_distance();
    void set_workout_stress();

    void on_pushButton_cancel_clicked();
    void on_pushButton_add_clicked();
    void on_doubleSpinBox_dist_valueChanged(double arg1);
    void on_timeEdit_duration_timeChanged(const QTime &time);
    void on_comboBox_sport_currentTextChanged(const QString &arg1);

    void on_pushButton_workouts_clicked();

private:
    Ui::Dialog_add *ui;
};

#endif // DIALOG_ADD_H
