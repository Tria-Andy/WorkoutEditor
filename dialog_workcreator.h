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
    explicit del_workcreator(QObject *parent = 0) : QStyledItemDelegate(parent) {}
    QStringList groupList;

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        const QAbstractItemModel *model = index.model();

        QString levelName;
        if(index.parent().isValid())
        {
            levelName = index.parent().child(index.row(),1).data().toString();
        }
        else
        {
            levelName = model->data(model->index(index.row(),1,QModelIndex())).toString();;
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
class del_workcreatoredit : public QStyledItemDelegate, public calculation
{
    Q_OBJECT
public:
    explicit del_workcreatoredit(QObject *parent = 0) : QStyledItemDelegate(parent) {}
    bool hasValue,timeBased;
    QString sport;
    int thresBase;
    QString groupName;
    double thresPace;
    double thresPower;
    double thresSpeed;

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        const QAbstractItemModel *model = index.model();
        QColor rectColor,gradColor;
        gradColor.setHsv(0,0,180,200);
        bool setEdit;

        QLinearGradient rectGradient;
        rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        rectGradient.setSpread(QGradient::RepeatSpread);

        QPainterPath rectValue;
        QRect rectBack(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height());
        rectValue.addRoundedRect(rectBack,1,1);
        QRect rectText(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        setEdit = model->data(model->index(index.row(),2,QModelIndex())).toBool();

        if(setEdit)
        {
            rectColor.setHsv(120,180,200,200);
            painter->setPen(Qt::black);
        }
        else
        {
            rectColor.setHsv(0,180,220,200);
            painter->setPen(Qt::darkRed);
        }
        rectGradient.setColorAt(0,rectColor);
        rectGradient.setColorAt(1,gradColor);

        painter->setBrush(rectGradient);
        painter->drawPath(rectValue);
        painter->drawText(rectText,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
       return option.fontMetrics.boundingRect(option.rect, Qt::TextWordWrap, index.data().toString()).size();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        QFont eFont;
        eFont.setPixelSize(settings::get_fontValue("fontSmall"));
        const QAbstractItemModel *model = index.model();
        bool setEdit = model->data(model->index(index.row(),2,QModelIndex())).toBool();

        if(hasValue)
        {
            if((index.row() == 0 || index.row() == 1) && setEdit)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(false);
                editor->setFont(eFont);
                return editor;
            }
            if(index.row() == 2 && setEdit)
            {
                QString level = model->data(model->index(1,0)).toString();
                QSpinBox *editor = new QSpinBox(parent);
                editor->setFrame(true);
                editor->setFont(eFont);
                editor->setMinimum(get_thresPercent(sport,level,false));
                editor->setMaximum(get_thresPercent(sport,level,true));
                return editor;
            }
            if((index.row() ==  5 || index.row() == 3) && setEdit)
            {
                QTimeEdit *editor = new QTimeEdit(parent);
                editor->setDisplayFormat("mm:ss");
                editor->setFrame(true);
                editor->setFont(eFont);
                return editor;
            }
            if((index.row() == 4 || index.row() == 6 || index.row() == 7) && setEdit)
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setFrame(true);
                editor->setFont(eFont);
                editor->setMaximum(999);
                return editor;
            }
            if(index.row() == 8 && setEdit)
            {
                QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
                editor->setFrame(true);
                editor->setFont(eFont);
                editor->setDecimals(3);
                if(sport == settings::isSwim)
                {
                    editor->setMinimum(0.0);
                    editor->setMaximum(10.0);
                    editor->setSingleStep(0.025);
                }
                else
                {
                    editor->setMinimum(0.0);
                    editor->setMaximum(500.0);
                    editor->setSingleStep(0.100);
                }
                return editor;
            }
        }
        else
        {
            if(index.row() == 0 && setEdit)
            {
                QComboBox *editor = new QComboBox(parent);

                editor->setFrame(true);
                editor->setFont(eFont);
                return editor;
            }
            if(index.row() == 1 && setEdit)
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setFrame(true);
                editor->setFont(eFont);
                editor->setMinimum(2);
                editor->setMaximum(20);
                return editor;
            }
        }
        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        if(hasValue)
        {
            if(index.row() == 0 || index.row() == 1)
            {
                QString value = index.data(Qt::DisplayRole).toString();
                QComboBox *comboBox = static_cast<QComboBox*>(editor);
                if(index.row() == 0)
                {
                    comboBox->addItems(settings::get_listValues("IntEditor"));
                }
                else
                {
                    comboBox->addItems(settings::get_listValues("Level"));
                }
                comboBox->setCurrentText(value);
            }
            if(index.row() == 2)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data(Qt::DisplayRole).toInt());
            }
            if(index.row() == 5 || index.row() == 3)
            {
                QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"mm:ss"));
            }
            if(index.row() == 4 || index.row() == 6 || index.row() == 7)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data(Qt::DisplayRole).toDouble());
            }
            if(index.row() == 8)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->setValue(index.data(Qt::DisplayRole).toDouble());
            }
        }
        else
        {
            if(index.row() == 0)
            {
                QString comboValue;
                QString value = index.data(Qt::DisplayRole).toString();
                QComboBox *comboBox = static_cast<QComboBox*>(editor);
                if(groupName.contains("Group"))
                {
                    comboValue = "Group-";
                }
                else
                {
                    comboValue = "Series-";
                }
                for(int i = 1; i < 10; ++i)
                {
                    comboBox->addItem(comboValue+QString::number(i));
                }
                comboBox->setCurrentText(value);
            }
            if(index.row() == 1)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data(Qt::DisplayRole).toInt());
            }
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        if(hasValue)
        {
            if(index.row() == 0 || index.row() == 1) //Phase and Level
            {
               QComboBox *comboBox = static_cast<QComboBox*>(editor);
               QString value = comboBox->currentText();
               model->setData(index,value, Qt::EditRole);
               if(index.row() == 1)
               {
                   levelChanged(model,value,comboBox->currentIndex());
                   rangeChanged(model,get_thresPercent(sport,value,false));
               }
            }
            if(index.row() == 2 || index.row() == 4 || index.row() == 6 || index.row() == 7) //RangeValue Percent - Pace - Stress - Work
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value, Qt::EditRole);
                if(index.row() == 2) rangeChanged(model,value);
            }
            if(index.row() == 3) //Pace
            {
                    QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                    QTime value = timeEdit->time();
                    timeEdit->interpretText();
                    model->setData(model->index(3,0),value.toString("mm:ss"), Qt::EditRole);
            }
            if(index.row() == 5) //Duration
            {
                    QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                    QTime value = timeEdit->time();
                    timeEdit->interpretText();
                    model->setData(model->index(5,0),value.toString("mm:ss"), Qt::EditRole);
                    set_distance(model,value);
                    set_stressValue(model);
                    set_work(model);
            }
            if(index.row() == 8) //Distance
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->interpretText();
                double value = spinBox->value();
                model->setData(index, value, Qt::EditRole);
                set_duration(model);
                set_stressValue(model);
                set_work(model);
            }
        }
        else
        {
            if(index.row() == 0)
            {
               QComboBox *comboBox = static_cast<QComboBox*>(editor);
               QString value = comboBox->currentText();
               model->setData(index,value, Qt::EditRole);
            }
            if(index.row() == 1)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value, Qt::EditRole);
            }
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

    void levelChanged(QAbstractItemModel *model,QString level,int index) const
    {
        model->setData(model->index(2,1),index);
        model->setData(model->index(2,0),get_thresPercent(sport,level,false));
    }

    void rangeChanged(QAbstractItemModel *model,double value) const
    {
        model->setData(model->index(3,0),calc_thresPace(thresPace,value));
        model->setData(model->index(4,0),calc_thresPower(thresPower,value));

        if(sport == settings::isBike)
        {
            set_distance(model,QTime::fromString(model->data(model->index(5,0)).toString(),"mm:ss"));
        }
        else if(sport == settings::isSwim || sport == settings::isRun)
        {
            set_duration(model);
            set_speed(model,static_cast<double>(get_timesec(model->data(model->index(3,0)).toString())));
        }

        set_stressValue(model);
        set_work(model);
    }

    void set_duration(QAbstractItemModel *model) const
    {
        if(sport == settings::isSwim || sport == settings::isRun || (sport == settings::isBike && !timeBased))
        {
            model->setData(model->index(5,0),calc_duration(sport,model->data(model->index(8,0)).toDouble(),model->data(model->index(3,0)).toString()));
        }
    }

    void set_stressValue(QAbstractItemModel *model) const
    {
        if(sport == settings::isBike || sport == settings::isStrength || sport == settings::isAlt)
        {
            model->setData(model->index(6,0),estimate_stress(sport,model->data(model->index(4,0)).toString(),get_timesec(model->data(model->index(5,0)).toString()),1));
        }
        else if(sport == settings::isRun || sport == settings::isSwim)
        {
            model->setData(model->index(6,0),estimate_stress(sport,model->data(model->index(3,0)).toString(),get_timesec(model->data(model->index(5,0)).toString()),0));
        }
    }

    void set_distance(QAbstractItemModel *model, QTime value) const
    {
        if(sport == settings::isBike)
        {
            double pace = 3600.0/wattToSpeed(thresPower,thresSpeed,model->data(model->index(4,0)).toDouble());
            model->setData(model->index(8,0),calc_distance(value.toString("mm:ss"),pace));
            set_speed(model,pace);
        }
        else if(sport == settings::isRun)
        {
            model->setData(model->index(8,0),calc_distance(value.toString("mm:ss"),get_timesec(model->data(model->index(3,0)).toString())));
        }
    }

    void set_speed(QAbstractItemModel *model,double sec) const
    {
        model->setData(model->index(9,0),calc_lapSpeed(sport,sec));
    }

    void set_work(QAbstractItemModel *model) const
    {
        double pValue = 0;
        double workFactor = 1.0;
        double dura = get_timesec(model->data(model->index(5,0)).toString());
        double pFactor = 10.0;

        if(sport == settings::isSwim)
        {
            pValue = get_timesec(model->data(model->index(3,0)).toString());
            if(timeBased) workFactor = generalValues->value("workfactor").toDouble();
        }
        if(sport == settings::isBike)
        {
            pValue = model->data(model->index(4,0)).toDouble();
        }
        if(sport == settings::isRun)
        {
            pValue = get_speed(QTime::fromString(model->data(model->index(3,0)).toString(),"mm:ss"),0,sport,true);
        }
        if(sport == settings::isStrength) pValue = model->data(model->index(2,0)).toDouble() / pFactor;
        if(sport == settings::isAlt) pValue = model->data(model->index(2,0)).toDouble() / pFactor;

        if(model->data(model->index(0,0)).toString().contains(generalValues->value("breakname")) && sport == settings::isSwim)
        {
            model->setData(model->index(7,0),set_doubleValue(calc_totalWork(sport,pValue,dura,0)*workFactor,false));
        }
        else
        {
            model->setData(model->index(7,0),set_doubleValue(calc_totalWork(sport,pValue,dura,6)*workFactor,false));
        }
    }
};

namespace Ui {
class Dialog_workCreator;
}

class Dialog_workCreator : public QDialog, public calculation, public standardWorkouts
{
    Q_OBJECT

public:
    explicit Dialog_workCreator(QWidget *parent = 0,schedule *psched = 0);
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
    void set_controlButtons(bool);
    void set_updateDates(bool);
    void set_workoutModel(QDate);
    void update_workouts();
    void on_pushButton_sync_clicked();
    void on_checkBox_timebased_clicked(bool checked);
    void on_toolButton_map_clicked();

private:
    Ui::Dialog_workCreator *ui;

    schedule *worksched;
    QString isSeries,isGroup,currentSport,currentWorkID,isBreak,viewBackground,workMap;
    QStandardItemModel *plotModel,*valueModel,*listModel,*workoutModel;
    QSortFilterProxyModel *metaProxy,*stepProxy, *schedProxy, *proxyFilter;
    QMap<QString,QString> workoutMap;
    QMap<int,QString> dataPoint;
    QStringList modelHeader,phaseList,groupList,levelList;
    QTreeWidgetItem *currentItem;
    del_workcreator workTree_del;
    del_workcreatoredit edit_del;
    del_mousehover mousehover_del;
    double timeSum,distSum,stressSum,workSum,thresSpeed;
    int thresPace,thresPower;
    QVector<bool> editRow;
    bool isSwim,isBike,isRun,isStrength,isAlt,isOther,isTria;
    bool clearFlag;

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
    void show_editItem(QTreeWidgetItem *item);
    void set_selectData(QTreeWidgetItem *item);
    QTreeWidgetItem* move_item(bool,QTreeWidgetItem*);
    void set_defaultData(QTreeWidgetItem *item,bool);
    void get_workouts(QString);
    QString get_workoutTime(double);
    void clearIntTree();
    void open_stdWorkout(QString);
    void set_plotModel();
    void add_to_plot(QTreeWidgetItem *item,int);
    void set_plotGraphic(int);
    void save_workout();
    void save_workout_values(QStringList,QStandardItemModel *);
    void update_workoutsSchedule(int,QDate,QPair<double,double>,int);
    void set_metaFilter(QString,int,bool);
};

#endif // DIALOG_WORKCREATOR_H
