#ifndef DEL_INTSELECT_H
#define DEL_INTSELECT_H

#include <QtGui>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTimeEdit>
#include <QDebug>
#include "calculation.h"

class del_intselect : public QStyledItemDelegate, public calculation
{
    Q_OBJECT

public:
    explicit del_intselect(QObject *parent = 0) : QStyledItemDelegate(parent) {}
    enum{SwimLap,Interval};
    int intType;
    QString sport;

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
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
        setEdit = model->data(model->index(index.row(),1,QModelIndex())).toBool();

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
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->drawPath(rectValue);
        painter->drawText(rectText,Qt::AlignLeft | Qt::AlignVCenter,index.data().toString());
        painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        const QAbstractItemModel *model = index.model();
        bool setEdit = model->data(model->index(index.row(),1,QModelIndex())).toBool();

        if(index.row() == 0 && setEdit)
        {
            QLineEdit *editor = new QLineEdit(parent);
            editor->setFrame(true);
            return editor;
        }

        if(intType == SwimLap)
        {
            if(index.row() == 1 && setEdit)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(true);
                return editor;
            }
            if((index.row() == 3 || index.row() == 6) && setEdit)
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setMinimum(0);
                editor->setMaximum(200);
                editor->setFrame(true);
                return editor;
            }
        }
        if(intType == Interval)
        {
            if(index.row() == 1 && setEdit)
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setMinimum(0);
                editor->setMaximum(10000);
                editor->setFrame(true);
                return editor;
            }
            if(index.row() == 2 && setEdit)
            {
                QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
                editor->setMinimum(0.0);
                editor->setFrame(true);
                editor->setDecimals(3);
                return editor;
            }
        }

        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        if(index.row() == 0)
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            lineEdit->setText(index.data().toString());
        }

        if(intType == SwimLap)
        {
            if(index.row() == 1)
            {
                QString value = index.data().toString();
                QComboBox *comboBox = static_cast<QComboBox*>(editor);
                comboBox->addItems(settings::get_listValues("SwimStyle"));
                comboBox->setCurrentText(value);
            }
            if(index.row() == 3 || index.row() == 6)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data().toInt());
            }
        }
        if(intType == Interval)
        {
            if(index.row() == 1)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data().toInt());
            }
            if(index.row() == 2)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->setValue(index.data().toDouble());
            }
        }
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        if(index.row() == 0)
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            QString value = lineEdit->text();
            model->setData(index,value);
            if(value.contains(settings::get_generalValue("breakname")))
            {
                model->setData(model->index(3,0),"00:30");
                model->setData(model->index(2,0),0);
            }
        }

        if(intType == SwimLap)
        {
            if(index.row() == 1)
            {
                QComboBox *comboBox = static_cast<QComboBox*>(editor);
                model->setData(index,comboBox->currentText());
            }
            if(index.row() == 3 || index.row() == 6)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value);
                if(index.row() == 3) setPace(model,value);
            }
        }
        if(intType == Interval)
        {
            if(index.row() == 1)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value);
            }
            if(index.row() == 2)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->interpretText();
                double value = spinBox->value();
                model->setData(index, value);
                setPace(model,get_timesec(model->data(model->index(3,0)).toString()));
            }
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

    void setPace(QAbstractItemModel *model,int sec) const
    {
        double dist = model->data(model->index(2,0)).toDouble();
        model->setData(model->index(4,0),set_time(calc_lapPace(sport,sec,dist)));
        setSpeed(model,sec);
    }

    void setSpeed(QAbstractItemModel *model,int sec) const
    {
        double factor = sport == settings::isSwim ? 1000.0 : 1.0;
        double dist = model->data(model->index(2,0)).toDouble();
        model->setData(model->index(5,0),set_doubleValue(calcSpeed(sec,dist,factor),true));
    }

    void set_duration(QAbstractItemModel *model) const
    {
        model->data(model->index(1,0)).toInt();
    }

};
#endif // DEL_INTSELECT_H
