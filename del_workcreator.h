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

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        if(hasValue)
        {
            if(index.row() == 0 || index.row() == 1)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(false);
                return editor;
            }
            if(index.row() == 2)
            {
                const QAbstractItemModel *model = index.model();
                QString level = model->data(model->index(1,0)).toString();
                QSpinBox *editor = new QSpinBox(parent);
                editor->setFrame(true);
                editor->setMinimum(get_thresPercent(sport,level,false));
                editor->setMaximum(get_thresPercent(sport,level,true));
                return editor;
            }
            if(sport == settings::isSwim)
            {
                if(index.row() == 6)
                {
                    QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
                    editor->setFrame(true);
                    editor->setDecimals(3);
                    editor->setMinimum(0.0);
                    editor->setMaximum(5.0);
                    editor->setSingleStep(0.025);
                    return editor;
                }
            }
            else
            {
                if(index.row() ==  4)
                {
                    QTimeEdit *editor = new QTimeEdit(parent);
                    editor->setDisplayFormat("mm:ss");
                    editor->setFrame(true);
                    return editor;
                }
            }
            if(index.row() ==  5)
            {
                QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
                editor->setFrame(true);
                editor->setDecimals(2);
                editor->setMinimum(0.0);
                editor->setMaximum(500.0);
                return editor;
            }
        }
        else
        {
            if(index.row() == 0)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(false);
                return editor;
            }
            if(index.row() == 1)
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setFrame(true);
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
            if(sport == settings::isSwim)
            {
                if(index.row() == 6)
                {
                    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                    spinBox->setValue(index.data(Qt::DisplayRole).toDouble());
                }
            }
            else
            {
                if(index.row() == 4)
                {
                    QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                    timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"mm:ss"));
                }
            }
            if(index.row() == 5)
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
            if(index.row() == 0 || index.row() == 1)
            {
               QComboBox *comboBox = static_cast<QComboBox*>(editor);
               QString value = comboBox->currentText();
               model->setData(index,value, Qt::EditRole);
               if(index.row() == 1)
               {
                   model->setData(model->index(2,1),comboBox->currentIndex());
                   model->setData(model->index(2,0),get_thresPercent(sport,value,false));
               }
            }
            if(index.row() == 2)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value, Qt::EditRole);
                model->setData(model->index(3,0),calc_threshold(sport,currThres,value));
                model->setData(model->index(4,0),calc_duration(sport,model->data(model->index(6,0)).toDouble(),model->data(model->index(3,0)).toString()));
                model->setData(model->index(5,0),estimate_stress(sport,model->data(model->index(3,0)).toString(),get_timesec(model->data(model->index(4,0)).toString())));
            }
            if(sport == settings::isSwim)
            {
                if(index.row() == 6)
                {
                    QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                    spinBox->interpretText();
                    double value = spinBox->value();
                    model->setData(index, value, Qt::EditRole);
                    model->setData(model->index(4,0),calc_duration(sport,value,model->data(model->index(3,0)).toString()));
                    model->setData(model->index(5,0),estimate_stress(sport,model->data(model->index(3,0)).toString(),get_timesec(model->data(model->index(4,0)).toString())));
                }
            }
            else
            {
                if(index.row() == 4)
                {
                    QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                    QTime value = timeEdit->time();
                    timeEdit->interpretText();
                    model->setData(index,value.toString("mm:ss"), Qt::EditRole);
                    model->setData(model->index(6,0),calc_distance(value.toString("mm:ss"),get_timesec(model->data(model->index(3,0)).toString())));
                    model->setData(model->index(5,0),estimate_stress(sport,model->data(model->index(3,0)).toString(),get_timesec(model->data(model->index(4,0)).toString())));
                }
            }
            if(index.row() == 5)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->interpretText();
                double value = spinBox->value();
                model->setData(index, value, Qt::EditRole);
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
};




#endif // DEL_WORKCREATOR_H
