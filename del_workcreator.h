#ifndef DEL_WORKCREATOR_H
#define DEL_WORKCREATOR_H

#include <QtGui>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTimeEdit>
#include <QDebug>
#include "calculation.h"

class del_workcreator : public QStyledItemDelegate, public calculation
{
    Q_OBJECT
public:
    explicit del_workcreator(QObject *parent = 0) : QStyledItemDelegate(parent) {}
    bool hasValue;
    QString sport;
    double currThres;
    double thresPace;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QFont eFont;
        eFont.setPixelSize(settings::get_fontValue("fontSmall"));

        Q_UNUSED(option)
        if(hasValue)
        {
            if(index.row() == 0 || index.row() == 1)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(false);
                editor->setFont(eFont);
                return editor;
            }
            if(index.row() == 2)
            {
                const QAbstractItemModel *model = index.model();
                QString level = model->data(model->index(1,0)).toString();
                QSpinBox *editor = new QSpinBox(parent);
                editor->setFrame(true);
                editor->setFont(eFont);
                editor->setMinimum(get_thresPercent(sport,level,false));
                editor->setMaximum(get_thresPercent(sport,level,true));
                return editor;
            }
            if(sport != settings::isSwim)
            {
                if(index.row() ==  4)
                {
                    QTimeEdit *editor = new QTimeEdit(parent);
                    editor->setDisplayFormat("mm:ss");
                    editor->setFrame(true);
                    editor->setFont(eFont);
                    return editor;
                }
            }

            if(index.row() == 6)
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
            if(index.row() == 0)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(true);
                editor->setFont(eFont);
                return editor;
            }
            if(index.row() == 1)
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
            if(sport != settings::isSwim)
            {
                if(index.row() == 4)
                {
                    QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                    timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"mm:ss"));
                }
            }
            if(index.row() == 6)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->setValue(index.data(Qt::DisplayRole).toDouble());
            }
        }
        else
        {
            if(index.row() == 0)
            {
                QString value = index.data(Qt::DisplayRole).toString();
                QComboBox *comboBox = static_cast<QComboBox*>(editor);
                comboBox->addItem("Group");
                comboBox->addItem("Series");
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
            if(index.row() == 2) //RangeValue Percent
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value, Qt::EditRole);
                rangeChanged(model,value);
            }
            if(sport != settings::isSwim)
            {
                if(index.row() == 4) //Duration
                {
                    QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                    QTime value = timeEdit->time();
                    timeEdit->interpretText();
                    model->setData(model->index(4,0),value.toString("mm:ss"), Qt::EditRole);
                    set_distance(model,value);
                    set_stressValue(model);
                }
            }
            if(index.row() == 6) //Distance
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->interpretText();
                double value = spinBox->value();
                model->setData(index, value, Qt::EditRole);
                set_duration(model);
                set_stressValue(model);
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
        model->setData(model->index(3,0),calc_threshold(sport,currThres,value));

        if(sport == settings::isBike)
        {
            set_distance(model,QTime::fromString(model->data(model->index(4,0)).toString(),"mm:ss"));
        }
        else
        {
            set_duration(model);
            set_speed(model,static_cast<double>(get_timesec(model->data(model->index(3,0)).toString())));
        }

        set_stressValue(model);
    }

    void set_duration(QAbstractItemModel *model) const
    {
        if(sport != settings::isBike) model->setData(model->index(4,0),calc_duration(sport,model->data(model->index(6,0)).toDouble(),model->data(model->index(3,0)).toString()));
    }

    void set_stressValue(QAbstractItemModel *model) const
    {
        model->setData(model->index(5,0),estimate_stress(sport,model->data(model->index(3,0)).toString(),get_timesec(model->data(model->index(4,0)).toString())));
    }

    void set_distance(QAbstractItemModel *model, QTime value) const
    {
        if(sport == settings::isBike)
        {
            int pace = get_timesec(threstopace(thresPace,model->data(model->index(2,0)).toDouble()));
            model->setData(model->index(6,0),calc_distance(value.toString("mm:ss"),pace));
            set_speed(model,static_cast<double>(pace));
        }
        else
        {
            model->setData(model->index(6,0),calc_distance(value.toString("mm:ss"),get_timesec(model->data(model->index(3,0)).toString())));
        }
    }

    void set_speed(QAbstractItemModel *model,double sec) const
    {
        model->setData(model->index(7,0),calc_lapSpeed(sport,sec));
    }
};




#endif // DEL_WORKCREATOR_H
