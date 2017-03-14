#ifndef DIALOG_WORKCREATOR_H
#define DIALOG_WORKCREATOR_H

#include <QDialog>
#include <QStandardItemModel>
#include <QtGui>
#include <QtCore>
#include <QTreeWidget>
#include <QWidget>
#include "settings.h"
#include "standardworkouts.h"
#include "del_workcreator.h"
#include "del_workcreatoredit.h"
#include "del_mousehover.h"
#include "calculation.h"

namespace Ui {
class Dialog_workCreator;
}

class Dialog_workCreator : public QDialog, public calculation, public standardWorkouts
{
    Q_OBJECT

public:
    explicit Dialog_workCreator(QWidget *parent = 0);
    ~Dialog_workCreator();

private slots:
    void on_treeWidget_intervall_itemChanged(QTreeWidgetItem *item, int column);
    void on_treeWidget_intervall_itemClicked(QTreeWidgetItem *item, int column);
    void on_comboBox_sport_currentTextChanged(const QString &arg1);
    void on_listView_workouts_clicked(const QModelIndex &index);
    void on_pushButton_clear_clicked();
    void on_toolButton_update_clicked();
    void on_toolButton_remove_clicked();
    void on_toolButton_up_clicked();
    void on_toolButton_down_clicked();
    void on_treeWidget_intervall_itemSelectionChanged();
    void on_toolButton_save_clicked();
    void on_toolButton_copy_clicked();
    void on_toolButton_delete_clicked();
    void on_lineEdit_workoutname_textChanged(const QString &arg1);
    void on_toolButton_close_clicked();

    void on_toolButton_cancel_clicked();

private:
    Ui::Dialog_workCreator *ui;

    QString isSeries,isGroup,current_sport,current_workID,isBreak,buttonStyle,viewBackground;
    QStandardItemModel *plotModel,*valueModel,*listModel;
    QSortFilterProxyModel *metaProxy,*stepProxy;
    QMap<QString,QString> workoutMap;
    QMap<int,QString> dataPoint;
    QStringList modelHeader,phaseList,groupList,levelList;
    QTreeWidgetItem *currentItem;
    del_workcreator workTree_del;
    del_workcreatoredit edit_del;
    del_mousehover mousehover_del;
    double time_sum,dist_sum,stress_sum;
    int currThres,threshold_pace,current_pace,threshold_power,current_power;
    QVector<bool> editRow;

    bool clearFlag;

    QString get_treeValue(int,int,int,int,int);

    void control_editPanel(bool);
    void resetAxis();
    void set_editRow(QString);
    void set_sport_threshold(QString);
    void set_itemData(QTreeWidgetItem *item);
    void show_editItem(QTreeWidgetItem *item);
    void set_selectData(QTreeWidgetItem *item);
    QTreeWidgetItem* move_item(bool,QTreeWidgetItem*);
    void set_defaultData(QTreeWidgetItem *item,bool);
    void get_workouts(QString);
    void clearIntTree();
    void open_stdWorkout(QString);
    void set_plotModel();
    void add_to_plot(QTreeWidgetItem *item,int);
    void set_plotGraphic(int);
    void save_workout();
    void save_workout_values(QStringList,QStandardItemModel *);
};

#endif // DIALOG_WORKCREATOR_H
