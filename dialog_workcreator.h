#ifndef DIALOG_WORKCREATOR_H
#define DIALOG_WORKCREATOR_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTimeEdit>
#include <QtGui>
#include <QtCore>
#include <QTreeWidget>
#include <QWidget>
#include "settings.h"
#include "standardworkouts.h"
#include "del_mousehover.h"
#include "schedule.h"
#include "dialog_map.h"

class del_workcreator : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit del_workcreator(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        const QAbstractItemModel *model = index.model();

        QString levelName;        
        if(index.parent().isValid())
        {
            levelName = model->index(index.row(),1,index.parent()).data().toString();
        }
        else
        {
            levelName = model->data(model->index(index.row(),1,QModelIndex())).toString();
        }

        QLinearGradient itemGradient(option.rect.topLeft(),option.rect.bottomLeft());
        itemGradient.setSpread(QGradient::ReflectSpread);
        QColor itemColor,gradColor;

        gradColor.setHsv(0,0,200,150);

        if(option.state & (QStyle::State_Selected | QStyle::State_MouseOver))
        {
            itemColor.setHsv(240,255,255,180);
            painter->setPen(Qt::white);
        }
        else
        {
            itemColor = settings::get_itemColor(levelName).toHsv();
            itemColor.setAlpha(180);
            if(levelName.isEmpty()) itemColor.setHsv(0,0,255,200);
            painter->setPen(Qt::black);
        }

        itemGradient.setColorAt(0,gradColor);
        itemGradient.setColorAt(1,itemColor);
        painter->fillRect(option.rect,itemGradient);

        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }
};

namespace Ui {
class Dialog_workCreator;
}

class Dialog_workCreator : public QDialog, public calculation
{
    Q_OBJECT

public:
    explicit Dialog_workCreator(QWidget *parent = nullptr,standardWorkouts *pworkouts = nullptr, schedule *psched = nullptr);
    ~Dialog_workCreator();

private slots:
    void on_treeWidget_workoutTree_itemChanged(QTreeWidgetItem *item, int column);
    void on_treeWidget_workoutTree_itemClicked(QTreeWidgetItem *item, int column);
    void on_comboBox_sport_currentTextChanged(const QString &arg1);
    void on_pushButton_clear_clicked();
    void on_toolButton_update_clicked();
    void on_toolButton_remove_clicked();
    void on_toolButton_up_clicked();
    void on_toolButton_down_clicked();
    void on_treeWidget_workoutTree_itemSelectionChanged();
    void on_toolButton_save_clicked();
    void on_toolButton_copy_clicked();
    void on_toolButton_delete_clicked();
    void on_lineEdit_workoutname_textChanged(const QString &arg1);
    void on_toolButton_close_clicked();
    void on_toolButton_cancel_clicked();
    void set_controlButtons(bool);
    void set_updateDates(bool);
    void update_workouts();
    void on_pushButton_sync_clicked();
    void on_checkBox_timebased_clicked(bool checked);
    void on_toolButton_map_clicked();
    void on_listWidget_workouts_itemClicked(QListWidgetItem *item);
    void on_spinBox_level_valueChanged(int arg1);
    void on_comboBox_levelName_currentTextChanged(const QString &arg1);
    void on_doubleSpinBox_distance_valueChanged(double arg1);
    void on_checkBox_timebased_stateChanged(int arg1);
    void on_timeEdit_lapTime_userTimeChanged(const QTime &time);

private:
    Ui::Dialog_workCreator *ui;

    schedule *worksched;
    standardWorkouts *stdWorkouts;
    QString isSeries,isGroup,currentWorkID,isBreak,viewBackground;
    int thresValue;
    double workFactor;
    QString sportMark;
    QHash<QString,QMap<int,QString>> *xmlTagMap;
    QMap<int,QPair<QString,QVector<double>>> plotMap;
    QMap<QString,QPair<int,int>> plotRange;
    QMap<QString,int> stdWorkoutMapping;
    QVector<double> lastPlotValues;
    QSet<QString> workoutTitle;

    QTreeWidgetItem* set_itemToWidget(QStandardItem*,QTreeWidgetItem*);
    QVariant get_modelValue(QStandardItem*,QString);
    QVariant get_itemValue(QStandardItem*,QStringList*,QString);

    void load_selectedWorkout(QString);
    void read_selectedChild(QStandardItem*,QTreeWidgetItem*);
    void set_sportData(QString);
    int get_swimStyleID(QString);
    void edit_selectedStep(QTreeWidgetItem*);
    void update_selectedStep();
    void refresh_editStep();
    void read_currentWorkTree();
    void read_currentChild(QTreeWidgetItem*,QString,int);
    void read_currentData(QTreeWidgetItem*);
    void save_selectedWorkout();
    void read_childToWorkout(QTreeWidgetItem*,QStandardItem*,int);
    QStandardItem* add_itemToWorkout(QTreeWidgetItem*,QStandardItem*,int);
    void draw_plotGraphic(int);
    void set_itemDataRange(QTreeWidgetItem *item);
    void get_workouts(QString);

    QStringList groupList,levelList;
    QStringList *workoutHeader,*workoutTags,*stdWorkTags,*partTags,*stepTags;
    QHash<QString,QString> modelMapping;
    QTreeWidgetItem *currentItem;
    del_workcreator workTree_del;
    del_mousehover mousehover_del;
    bool clearFlag,isWorkLoaded;
    QString dateFormat,longTime,shortTime;

    QDialog *updateDialog;
    QDateTimeEdit *updateFrom,*updateTo;
    QRadioButton *updateAll, *updateRange;
    QPushButton *updateClose;
    QToolButton *updateOk;
    QListView *workoutView;
    QProgressBar *updateProgess;

    void control_editPanel(bool);
    void resetAxis();

    QTreeWidgetItem* move_item(int,QTreeWidgetItem*);
    void set_defaultData(QTreeWidgetItem *item,bool);

    void clearIntTree();
    void update_workoutsSchedule(int,QDate,QPair<double,double>,int);
};

#endif // DIALOG_WORKCREATOR_H
