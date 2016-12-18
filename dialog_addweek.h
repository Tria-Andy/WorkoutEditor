/*
 * Copyright (c) 2016 Andreas Hunner (andy-atech@gmx.net)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef DIALOG_ADDWEEK_H
#define DIALOG_ADDWEEK_H

#include <QDialog>
#include <QStandardItemModel>
#include "schedule.h"
#include "del_addweek.h"
#include "settings.h"

namespace Ui {
class Dialog_addweek;
}

class Dialog_addweek : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_addweek(QWidget *parent = 0,QString sel_week = QString(), schedule *p_sched = 0);
    ~Dialog_addweek();

private slots:
    void on_pushButton_cancel_clicked();
    void on_spinBox_swim_work_valueChanged(int arg1);
    void on_spinBox_bike_work_valueChanged(int arg1);
    void on_spinBox_run_work_valueChanged(int arg1);
    void on_spinBox_stg_work_valueChanged(int arg1);
    void on_spinBox_alt_work_valueChanged(int arg1);
    void on_timeEdit_swim_dur_timeChanged(const QTime &time);
    void on_timeEdit_bike_dur_timeChanged(const QTime &time);
    void on_timeEdit_run_dur_timeChanged(const QTime &time);
    void on_timeEdit_stg_dur_timeChanged(const QTime &time);
    void on_timeEdit_alt_dur_timeChanged(const QTime &time);
    void on_doubleSpinBox_swim_dist_valueChanged(double arg1);
    void on_doubleSpinBox_bike_dist_valueChanged(double arg1);
    void on_doubleSpinBox_run_dist_valueChanged(double arg1);
    void on_doubleSpinBox_alt_dist_valueChanged(double arg1);
    void on_spinBox_swim_stress_valueChanged(int arg1);
    void on_spinBox_bike_stress_valueChanged(int arg1);
    void on_spinBox_run_stress_valueChanged(int arg1);
    void on_spinBox_stg_stress_valueChanged(int arg1);
    void on_spinBox_alt_stress_valueChanged(int arg1);
    void on_dateEdit_selectDate_dateChanged(const QDate &date);
    void on_pushButton_ok_clicked();

    void on_tableView_sportValues_clicked(const QModelIndex &index);

private:
    Ui::Dialog_addweek *ui;
    schedule *workSched;
    del_addweek week_del;
    QStandardItemModel *weekModel;
    QString openID,selYear,weekID,swimValues,bikeValues,runValues,stgValues,altValues,sumValues,empty;
    QStringList sportuseList,weekHeader,weekMeta,weekContent;
    bool update;

    void fill_values(QString);
    void store_values(int);
    void store_meta();

    void sum_workouts();
    void sum_distance();
    void sum_duration();
    void sum_stress();

    void calc_percent();
};

#endif // DIALOG_ADDWEEK_H
