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

#ifndef DIALOG_ADDWEEK_H
#define DIALOG_ADDWEEK_H

#include "schedule.h"
#include "settings.h"
#include "calculation.h"
#include <QtGui>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTimeEdit>
#include <QDialog>
#include <QDebug>

class del_addweek : public QStyledItemDelegate, public calculation
{
    Q_OBJECT
public:
    explicit del_addweek(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QStringList *sportUse;

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont cFont;
        QString indexData;

        const QAbstractItemModel *model = index.model();
        cFont.setPixelSize(12);

        QColor rectColor;
        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        indexData = index.data().toString();
        painter->setPen(Qt::black);

        if(index.row() == sportUse->count())
        {
            rectColor = settings::get_itemColor(generalValues->value("sum")).toHsv();
            cFont.setBold(true);
        }
        else
        {
            rectColor = settings::get_itemColor(model->data(model->index(index.row(),0,QModelIndex())).toString().trimmed()).toHsv();
            cFont.setBold(false);
        }
        rectColor.setAlpha(175);
        painter->fillRect(option.rect,rectColor);
        painter->setFont(cFont);
        painter->drawText(rect_text,indexData,QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        int listCount = sportUse->count();

        if((index.column() == 1 || index.column() == 6) && index.row() != listCount)
        {
            QSpinBox *editor = new QSpinBox(parent);
            editor->setFrame(false);
            editor->setMinimum(0);
            editor->setMaximum(1000);
            return editor;
        }
        if(index.column() == 2 && index.row() != listCount)
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("hh:mm:ss");
            editor->setFrame(false);
            return editor;
        }
        if(index.column() == 4 && index.row() != listCount)
        {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(false);
            editor->setDecimals(2);
            editor->setMinimum(0.0);
            editor->setMaximum(500.0);
            return editor;
        }

        return nullptr;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        int listCount = sportUse->count();

        if((index.column() == 1 || index.column() == 6) && index.row() != listCount)
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            spinBox->setValue(index.model()->data(index, Qt::EditRole).toInt());
        }
        if(index.column() == 2 && index.row() != listCount)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            timeEdit->setTime(index.model()->data(index,Qt::EditRole).toTime());
        }
        if(index.column() == 4 && index.row() != listCount)
        {
            QDoubleSpinBox *doublespinBox = static_cast<QDoubleSpinBox*>(editor);
            doublespinBox->setValue(index.model()->data(index, Qt::EditRole).toDouble());
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        int listCount = sportUse->count();
        double factor = 1.0;
        QString sport = model->data(model->index(index.row(),0,QModelIndex())).toString();
        QModelIndex sum_index = model->index(listCount,index.column(),QModelIndex());

        if(sport == settings::isSwim) factor = thresValues->value("swimfactor");
        if(sport == settings::isBike) factor = thresValues->value("bikefactor");
        if(sport == settings::isRun) factor = thresValues->value("runfactor");

        if(index.column() == 1 && index.row() != listCount)
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            spinBox->interpretText();
            model->setData(index,spinBox->value());
            model->setData(sum_index,sum_int(model,sportUse,1));
        }
        if(index.column() == 2 && index.row() != listCount)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            QTime dura = timeEdit->time();
            timeEdit->interpretText();
            model->setData(index,dura.toString("hh:mm:ss"));
            model->setData(sum_index,sum_time(model,sportUse,2).toString("hh:mm:ss"));
            model->setData(model->index(index.row(),6,QModelIndex()),round(get_timeMin(dura)*factor));
            model->setData(model->index(listCount,6,QModelIndex()),sum_int(model,sportUse,6));
            calc_percent(sportUse,model);
            setPace(model,index.row());
        }
        if(index.column() == 4 && index.row() != listCount)
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->interpretText();
            model->setData(index, spinBox->value(), Qt::EditRole);
            model->setData(sum_index,sum_double(model,sportUse,4));
            setPace(model,index.row());
        }
        if(index.column() == 6 && index.row() != listCount)
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            int stress = spinBox->value();
            spinBox->interpretText();
            model->setData(index,stress);
            model->setData(sum_index,sum_int(model,sportUse,6));
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

    int get_timeMin(QTime time) const
    {
        int minutes;
        minutes = time.hour() * 60;
        minutes = minutes + time.minute();
        return minutes;
    }

    void setPace(QAbstractItemModel *model,int row) const
    {
        model->setData(model->index(row,5,QModelIndex()),
                       get_workout_pace(model->data(model->index(row,4,QModelIndex())).toDouble(),
                       get_timesec(model->data(model->index(row,2,QModelIndex())).toString()),
                       model->data(model->index(row,0,QModelIndex())).toString(),false));
    }

    void calc_percent(QStringList *list,QAbstractItemModel *model) const
    {
        int sum = 0;
        sum = get_timeMin(model->data(model->index(list->count(),2,QModelIndex())).toTime());
        if(sum > 0)
        {
            for(int i = 0; i < list->count(); ++i)
            {
                model->setData(model->index(i,3,QModelIndex()),
                               set_doubleValue(static_cast<double>(get_timeMin(model->data(model->index(i,2,QModelIndex())).toTime())) / static_cast<double>(sum)*100.0,false));
            }
        }
    }

    int sum_int(QAbstractItemModel *model,QStringList *list, int col) const
    {
        int sum = 0;
        for(int i = 0; i < list->count(); ++i)
        {
           sum = sum + model->data(model->index(i,col,QModelIndex())).toInt();
        }
        return sum;
    }

    double sum_double(QAbstractItemModel *model,QStringList *list, int col) const
    {
        double sum = 0;
        for(int i = 0; i < list->count(); ++i)
        {
           sum = sum + model->data(model->index(i,col,QModelIndex())).toDouble();
        }
        return sum;
    }

    QTime sum_time(QAbstractItemModel *model,QStringList *list, int col) const
    {
        QTime sum(0,0,0);
        for(int i = 0; i < list->count(); ++i)
        {
           sum = sum.addSecs(get_timesec(model->data(model->index(i,col,QModelIndex())).toTime().toString("hh:mm:ss")));
        }
        return sum;
    }
};

namespace Ui {
class Dialog_addweek;
}

class Dialog_addweek : public QDialog, public calculation
{
    Q_OBJECT

public:
    explicit Dialog_addweek(QWidget *parent = nullptr,QString sel_week = QString(), schedule *p_sched = nullptr);
    ~Dialog_addweek();

private slots:
    void on_dateEdit_selectDate_dateChanged(const QDate &date);
    void on_toolButton_update_clicked();
    void on_toolButton_close_clicked();
    void on_toolButton_copy_clicked();
    void on_toolButton_paste_clicked();

private:
    Ui::Dialog_addweek *ui;
    schedule *workSched;
    del_addweek week_del;
    QStandardItemModel *weekModel;
    QMap<QString, QVector<double>> compValues;
    QString editWeekID,timeFormat,empty;
    QStringList sportuseList,weekMeta;
    QStringList *weekHeader;
    int sportlistCount;
    bool update;

    void fill_values(QString);
    void update_values();
    void fill_weekSumRow(QAbstractItemModel*);
    QStringList create_values();
};

#endif // DIALOG_ADDWEEK_H
