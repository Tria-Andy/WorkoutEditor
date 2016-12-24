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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QAbstractItemModel>
#include <QTextBrowser>
#include <QtXml>
#include "calendar_delegate.h"
#include "week_delegate.h"
#include "summery_delegate.h"
#include "del_spinbox_int.h"
#include "del_swimlap.h"
#include "del_intview.h"
#include "del_intselect.h"
#include "del_level.h"
#include "schedule.h"
#include "dialog_add.h"
#include "dialog_addweek.h"
#include "dialog_version.h"
#include "day_popup.h"
#include "week_popup.h"
#include "year_popup.h"
#include "stress_popup.h"
#include "dialog_edit.h"
#include "dialog_export.h"
#include "dialog_stresscalc.h"
#include "dialog_inteditor.h"
#include "dialog_settings.h"
#include "dialog_pacecalc.h"
#include "dialog_lapeditor.h"
#include "dialog_week_copy.h"
#include "settings.h"
#include "jsonhandler.h"
#include "activity.h"
#include "standardworkouts.h"
#include "logger.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    schedule *workSchedule;
    Activity *curr_activity;
    jsonHandler *jsonhandler;
    settings editorSettings;
    standardWorkouts *stdWorkout;
    calendar_delegate calender_del;
    week_delegate week_del;
    summery_delegate sum_del;
    del_spinbox_int time_del;
    del_swimlap swimlap_del;
    del_intview intervall_del;
    del_intselect intSelect_del;
    del_level level_del;
    QStandardItemModel *calendar_model,*sum_model;
    QStringList modus_list,cal_header,year_header,schedMode;

    //Intervall Chart
    QVector<double> secTicker,speedValues,polishValues,speedMinMax,rangeMinMax;
    void set_speedValues(int);
    void set_speedgraph(double,double);
    void set_speedPlot(double,double);
    void set_polishValues(int,double);

    int sel_count,sportUse;
    QDate selectedDate,firstdayofweek;
    QString weeknumber,phaseFilter;
    QVector<int> work_sum,dur_sum,stress_sum;
    QVector<double> dist_sum;
    int weekRange,weekpos;
    int saisonWeeks,weekDays;
    unsigned int weekCounter,sportCounter;
    bool userSetup,isWeekMode,safeFlag,graphLoaded;

    void openPreferences();
    void set_summeryInfo();
    void summery_view();
    void summery_calc(int,QModelIndex,bool);
    QString set_summeryString(int,bool);
    void workout_calendar();
    QString get_weekRange();
    void set_buttons(bool);
    void set_calender();
    void set_comboIntervall();
    //Editor
    void select_activity_file();
    void loadfile(const QString &filename);
    void set_activty_infos();
    void set_activty_intervalls();
    void set_avg_fields();
    void write_hf_infos();
    void fill_WorkoutContent();
    void unselect_intRow();
    void set_menuItems(bool,bool);
    void reset_jsontext();
    void freeMem();

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void refresh_model();

private slots:
    void on_actionEditor_triggered();
    void on_actionPlaner_triggered();
    void on_actionExit_triggered();
    void on_actionNew_triggered();
    void on_calendarWidget_clicked(const QDate &date);
    void on_tableView_cal_clicked(const QModelIndex &index);
    void on_pushButton_current_week_clicked();
    void on_pushButton_week_plus_clicked();
    void on_pushButton_week_minus_clicked();
    void on_actionExport_to_Golden_Cheetah_triggered();
    void on_actionExit_and_Save_triggered();
    void on_actionSave_Workout_Schedule_triggered();
    void on_pushButton_fourplus_clicked();
    void on_actionSelect_File_triggered();
    void on_tableView_int_clicked(const QModelIndex &index);
    void on_actionReset_triggered();
    void on_actionUnselect_all_rows_triggered();
    void on_actionEdit_Distance_triggered();
    void on_actionEdit_Undo_triggered();
    void on_actionStress_Calculator_triggered();
    void on_actionIntervall_Editor_triggered();
    void on_actionPreferences_triggered();
    void on_actionPace_Calculator_triggered();
    void on_comboBox_schedMode_currentIndexChanged(int index);
    void on_tableView_summery_clicked(const QModelIndex &index);
    void on_actionSwitch_Year_triggered();
    void on_comboBox_phasefilter_currentIndexChanged(int index);
    void on_actionVersion_triggered();
    void on_horizontalSlider_factor_valueChanged(int value);
    void on_comboBox_intervals_currentIndexChanged(int index);
    void on_actionLapEditor_triggered();
    void on_horizontalSlider_polish_valueChanged(int value);
    void on_tableView_int_times_clicked(const QModelIndex &index);
    void on_actionSave_to_GoldenCheetah_triggered();
    void on_lineEdit_workContent_textChanged(const QString &arg1);
    void on_toolButton_putInt_clicked();
    void on_toolButton_sync_clicked();
    void on_toolButton_clearContent_clicked();
    void on_actionPMC_triggered();
};

#endif // MAINWINDOW_H
