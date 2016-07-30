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
#include "del_spinbox_double.h"
#include "del_spinbox_int.h"
#include "schedule.h"
#include "dialog_add.h"
#include "dialog_addweek.h"
#include "dialog_version.h"
#include "day_popup.h"
#include "week_popup.h"
#include "year_popup.h"
#include "dialog_edit.h"
#include "dialog_export.h"
#include "dialog_stresscalc.h"
#include "dialog_inteditor.h"
#include "dialog_settings.h"
#include "dialog_pacecalc.h"
#include "dialog_week_copy.h"
#include "settings.h"
#include "activity.h"
#include "standardworkouts.h"

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
    settings *editorSettings;
    standardWorkouts *stdWorkout;
    calendar_delegate calender_del;
    week_delegate week_del;
    summery_delegate sum_del;
    del_spinbox_double dist_del;
    del_spinbox_int time_del;
    QStandardItemModel *calendar_model,*sum_model;
    QStringList modus_list,cal_header,work_list,sum_name,sum_list,year_header,schedMode,sum_header;

    int fontSize,sel_count;
    QDate selectedDate,firstdayofweek;
    QString weeknumber,phaseFilter;
    int *work_sum;
    int *dur_sum;
    double *dist_sum;
    int *stress_sum;
    int weekRange,weekpos;
    int weekDays;
    unsigned int weekCounter;
    bool isWeekMode;

    void summery_view();
    void summery_calc(int,QModelIndex,bool);
    QString set_summeryString(int,bool);
    void workout_calendar();
    QString get_weekRange();
    void set_buttons(bool);
    void set_calender();

    //Editor
    void select_activity_file();
    void loadfile(const QString &filename);
    void set_activty_infos();
    void set_activty_intervalls();
    void set_avg_fields();
    void set_add_swim_values();
    void write_int_infos();
    void write_hf_infos();
    void write_samp_infos();

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
    void on_pushButton_calcHF_clicked();
    void on_actionEdit_Distance_triggered();
    void on_actionEdit_Undo_triggered();
    void on_actionCopy_new_Distance_triggered();
    void on_pushButton_clear_ovr_clicked();
    void on_pushButton_copy_ovr_clicked();
    void on_pushButton_clear_int_clicked();
    void on_pushButton_copy_int_clicked();
    void on_pushButton_clear_samp_clicked();
    void on_pushButton_copy_samp_clicked();
    void on_actionStress_Calculator_triggered();
    void on_actionIntervall_Editor_triggered();
    void on_actionPreferences_triggered();
    void on_actionPace_Calculator_triggered();
    void on_comboBox_schedMode_currentIndexChanged(int index);
    void on_tableView_summery_clicked(const QModelIndex &index);
    void on_actionSwitch_Year_triggered();
    void on_comboBox_phasefilter_currentIndexChanged(int index);
    void on_actionVersion_triggered();
};

#endif // MAINWINDOW_H
