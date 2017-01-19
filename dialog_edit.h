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

#ifndef DIALOG_EDIT_H
#define DIALOG_EDIT_H

#include <QDialog>
#include <QStandardItemModel>
#include "day_popup.h"
#include "schedule.h"
#include "dialog_workouts.h"
#include "settings.h"
#include "standardworkouts.h"
#include "calculation.h"

namespace Ui {
class Dialog_edit;
}

class Dialog_edit : public QDialog, public calculation
{
    Q_OBJECT

private:
    schedule *workSched;
    standardWorkouts *std_workouts;
    QSortFilterProxyModel *scheduleProxy;
    QStringList w_code;
    QModelIndex curr_index;
    void controlButtons(QDate);
    void set_result(QString,int);
    void set_workout_data(bool);

public:
    explicit Dialog_edit(QWidget *parent = 0, const QDate edit_date = QDate(), schedule *p_sched = 0, standardWorkouts *p_stdworkouts = 0);
    ~Dialog_edit();
    bool delete_workout;
    void set_workout_info(const QDate &d);
    void show_workout(int);

private slots:
    void set_edit_calweek();
    void on_pushButton_close_clicked();
    void on_comboBox_time_currentIndexChanged(int index);
    void on_pushButton_edit_clicked();
    void on_pushButton_copy_clicked();
    void on_pushButton_delete_clicked();
    void on_doubleSpinBox_distance_valueChanged(double arg1);
    void on_timeEdit_duration_timeChanged(const QTime &time);
    void on_dateEdit_edit_date_dateChanged(const QDate &date);
    void on_toolButton_workouts_clicked();
    void on_checkBox_complete_toggled(bool checked);

private:
    Ui::Dialog_edit *ui;
};

#endif // DIALOG_EDIT_H
