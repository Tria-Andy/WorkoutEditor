#ifndef DIALOG_INTEDITOR_H
#define DIALOG_INTEDITOR_H

#include <QDialog>
#include <QStandardItemModel>
#include <QtGui>
#include <QtCore>
#include <QTreeWidget>
#include <QWidget>
#include "settings.h"
#include "standardworkouts.h"

namespace Ui {
class Dialog_inteditor;
}

class Dialog_inteditor : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_inteditor(QWidget *parent = 0, settings *p_setting = 0,standardWorkouts *p_workouts = 0);
    ~Dialog_inteditor();

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_add_clicked();

    void on_pushButton_clear_clicked();

    void on_comboBox_sport_currentIndexChanged(const QString &arg1);

    void on_pushButton_delete_clicked();

    void on_pushButton_edit_clicked();

    void on_timeEdit_int_time_timeChanged(const QTime &time);

    void on_doubleSpinBox_int_dist_valueChanged(double arg1);

    void on_comboBox_part_currentIndexChanged(const QString &arg1);

    void on_treeWidget_planer_itemClicked(QTreeWidgetItem *item, int column);

    void on_pushButton_addtop_clicked();

    void refresh_model();

    void on_pushButton_save_std_clicked();

    void on_spinBox_threshold_valueChanged(int arg1);

    void on_comboBox_level_currentIndexChanged(int index);

    void on_pushButton_up_clicked();

    void on_pushButton_down_clicked();

    void on_listView_workouts_clicked(const QModelIndex &index);

    void on_lineEdit_workoutname_textChanged(const QString &arg1);

    void on_treeWidget_planer_itemSelectionChanged();


    void on_pushButton_delete_std_clicked();

    void on_pushButton_copy_std_clicked();

private:
    Ui::Dialog_inteditor *ui;
    settings *pop_settings;
    standardWorkouts *stdWorkouts;
    QStandardItemModel *plot_model,*workout_model;
    QTreeWidgetItem *select_item;
    QStringList model_header;
    QString current_sport,isSeries,isGroup,current_workID;
    int threshold_pace,current_pace;
    int threshold_power,current_power;
    double *powerlist,*powerfactor;
    double time_sum,dist_sum,stress_sum,speedfactor;
    bool clearFlag;

    QStringList add_int_values();
    int get_series_count();
    int get_group_count();
    void add_topItem(QString);
    void add_interval();
    void open_stdWorkout(QString);
    void move_item(bool);
    void set_pushbutton(bool);
    void get_workouts(QString);
    void save_workout();
    void save_workout_values(QStringList,QStandardItemModel *workmodel);
    QString get_treeValue(int,int,int,int,int);

    void set_sport_threshold(QString);
    QString calc_threshold(double);
    void calc_distance(QString);
    void set_components(QString);
    void clearIntTree();
    void reset_values();
    void reset_workoutInfo();
    void set_min_max(int,QString);
    void init_model();
    void set_plot_model();

    void add_to_plot(QTreeWidgetItem *item);
    void set_plot_graphic(int);
    void load_item(QTreeWidgetItem *item);
    void edit_item(QTreeWidgetItem *item);
    int get_xaxis_values();
    int get_x2axis_values();
    int get_yaxis_values(bool);
};

#endif // DIALOG_INTEDITOR_H
