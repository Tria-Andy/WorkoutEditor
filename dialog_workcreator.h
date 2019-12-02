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
    QStringList groupList;

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        const QAbstractItemModel *model = index.model();

        QString levelName;
        if(index.parent().isValid())
        {
            //levelName = index.parent().child(index.row(),1).data().toString();
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
    void set_workoutModel(QDate);
    void update_workouts();
    void on_pushButton_sync_clicked();
    void on_checkBox_timebased_clicked(bool checked);
    void on_toolButton_map_clicked();
    void on_listWidget_workouts_itemClicked(QListWidgetItem *item);
    void on_spinBox_level_valueChanged(int arg1);
    void on_comboBox_levelName_currentTextChanged(const QString &arg1);

private:
    Ui::Dialog_workCreator *ui;

    schedule *worksched;
    standardWorkouts *stdWorkouts;
    QString isSeries,isGroup,currentWorkID,isBreak,viewBackground,workMap;
    int thresValue;
    double workFactor;
    QString sportMark;
    QHash<QString,QHash<QString,QVector<QString>>> *workoutMap;
    QHash<QString,QMap<int,QString>> *xmlTagMap;

    void load_selectedWorkout(QString);
    void read_selectedChild(QStandardItem*,QTreeWidgetItem*);
    QTreeWidgetItem* set_selectedData(QStandardItem*,QTreeWidgetItem*);
    QVariant get_modelValue(QStandardItem*,QString);
    void set_sportData(QString);
    QString calc_threshold(double);
    int get_swimStyleID(QString);
    void edit_selectedStep(QTreeWidgetItem*);
    void refresh_editStep();

    QStandardItemModel *plotModel,*valueModel,*workoutModel;
    QStandardItemModel *selworkModel;
    QSortFilterProxyModel *metaProxy,*stepProxy, *schedProxy, *proxyFilter;
    QMap<int,QString> dataPoint;
    QStringList modelHeader,phaseList,groupList,levelList;
    QTreeWidgetItem *currentItem;
    del_workcreator workTree_del;
    del_mousehover mousehover_del;
    double timeSum,distSum,stressSum,workSum;
    QVector<bool> editRow;
    bool clearFlag;
    QString dateFormat,longTime,shortTime;

    QDialog *updateDialog;
    QDateTimeEdit *updateFrom,*updateTo;
    QRadioButton *updateAll, *updateRange;
    QPushButton *updateClose;
    QToolButton *updateOk;
    QListView *workoutView;
    QProgressBar *updateProgess;

    QString get_treeValue(int,int,int,int,int);

    void control_editPanel(bool);
    void resetAxis();
    void set_itemData(QTreeWidgetItem *item);
    void set_selectData(QTreeWidgetItem *item);
    QTreeWidgetItem* move_item(bool,QTreeWidgetItem*);
    void set_defaultData(QTreeWidgetItem *item,bool);
    void get_workouts(QString);
    QString get_workoutTime(double);
    void clearIntTree();
    void set_plotModel();
    void add_to_plot(QTreeWidgetItem *item,int);
    void set_plotGraphic(int);
    void save_workout();
    void save_workout_values(QStringList,QStandardItemModel *);
    void update_workoutsSchedule(int,QDate,QPair<double,double>,int);
    void set_metaFilter(QString,int,bool);
};

#endif // DIALOG_WORKCREATOR_H
