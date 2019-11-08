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

#ifndef DAY_POPUP_H
#define DAY_POPUP_H

#include "jsonhandler.h"
#include "standardworkouts.h"
#include "schedule.h"
#include "settings.h"
#include "dialog_map.h"
#include <QtGui>
#include <QDialog>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>
#include <QTimeEdit>
#include <QDebug>

class del_daypop: public QStyledItemDelegate, public calculation
{
    Q_OBJECT

public:
    explicit del_daypop(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    int selCol;

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
         painter->save();
         const QAbstractItemModel *model = index.model();
         int textMargin = 2;
         QPainterPath rectRow;
         QString sport = model->data(model->index(1,index.column())).toString();
         QString workEntry = index.data(Qt::DisplayRole).toString();
         QRect rectEntry(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height());
         rectRow.addRoundedRect(rectEntry,2,2);
         QRect rectText(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,option.rect.height());

         QLinearGradient rectGradient;
         rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
         rectGradient.setSpread(QGradient::RepeatSpread);
         QColor gradColor;
         gradColor.setHsv(0,0,180,200);
         QColor rectColor;

         if(option.state & QStyle::State_Selected)
         {
             rectColor.setHsv(240,255,150,200);
             rectColor.setAlpha(200);
             rectGradient.setColorAt(0,rectColor);
             rectGradient.setColorAt(1,gradColor);
             painter->setPen(Qt::white);
         }
         else
         {
             if(sport.isEmpty())
             {
                rectColor.setHsv(0,0,220);
             }
             else
             {
                rectColor = settings::get_itemColor(sport).toHsv();
             }
             rectColor.setAlpha(200);
             rectGradient.setColorAt(0,rectColor);
             rectGradient.setColorAt(1,gradColor);
             painter->setPen(Qt::black);
         }

         painter->setBrush(rectGradient);
         painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
         painter->drawPath(rectRow);
         painter->drawText(rectText,Qt::AlignLeft | Qt::AlignVCenter, workEntry);
         painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        int row = index.row();
        int col = index.column();

        if(row == 0 && col == selCol)    //Time
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("hh:mm");
            editor->setFrame(true);
            return editor;
        }
        if((row == 1 || row == 2) && col == selCol)    //Sport || Code
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setFrame(false);
            return editor;
        }
        if((row == 3 || row == 4) && col == selCol)    //Title || Comment
        {
            QLineEdit *editor = new QLineEdit(parent);
            editor->setFrame(true);
            return editor;
        }
        if(row ==  5 && col == selCol)
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("hh:mm:ss");
            editor->setFrame(true);
            return editor;
        }
        if(row == 6 && col == selCol)
        {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(true);
            editor->setDecimals(3);
            editor->setMinimum(0.0);
            editor->setMaximum(500.0);
            editor->setSingleStep(0.100);
            return editor;
        }
        if(row == 7 && col == selCol)
        {
            QSpinBox *editor = new QSpinBox(parent);
            editor->setFrame(true);
            editor->setMinimum(0);
            editor->setMaximum(500);
            return editor;
        }
        if(row == 8 && col == selCol)
        {
            QSpinBox *editor = new QSpinBox(parent);
            editor->setFrame(true);
            editor->setMinimum(0);
            editor->setMaximum(5000);
            return editor;
        }
        return nullptr;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        int row = index.row();
        if(row == 0 || row == 5)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            if(row == 0)
            {
                timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"hh:mm"));
            }
            else
            {
                timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"hh:mm:ss"));
            }
        }
        if(row == 1 || row == 2)
        {
            QString value = index.data(Qt::DisplayRole).toString();
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            if(row == 1)
            {
                comboBox->addItems(settings::get_listValues("Sport"));
            }
            else
            {
                comboBox->addItems(settings::get_listValues("WorkoutCode"));
            }
            comboBox->setCurrentText(value);
        }
        if(row == 3 || row == 4)
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            lineEdit->setText(index.data().toString());
        }
        if(row == 6)
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->setValue(index.data(Qt::DisplayRole).toDouble());
        }
        if(row == 7 || row == 8)
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            spinBox->setValue(index.data(Qt::DisplayRole).toInt());
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        int col = index.column();
        int row = index.row();

        if(row == 0 || row == 5) //Time || Duration
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            QTime value = timeEdit->time();
            timeEdit->interpretText();
            if(row == 0)
            {
                model->setData(index,value.toString("hh:mm"), Qt::EditRole);
            }
            else
            {
                model->setData(index,value.toString("hh:mm:ss"), Qt::EditRole);
                model->setData(model->index(10,col),get_workout_pace(model->data(model->index(6,col)).toDouble(),value,model->data(model->index(1,col)).toString(),true));
            }
        }
        if(row == 1 || row == 2) //Sport || Code
        {
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            QString value = comboBox->currentText();
            model->setData(index,value, Qt::EditRole);
        }
        if(row == 3 || row == 4) //Title || Comment
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            QString value = lineEdit->text();
            model->setData(index,value);
        }
        if(row == 6) //Distance
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->interpretText();
            double value = spinBox->value();
            model->setData(index, value, Qt::EditRole);
            model->setData(model->index(10,col),get_workout_pace(value,QTime::fromString(model->data(model->index(5,col)).toString(),"hh:mm:ss"),model->data(model->index(1,col)).toString(),true));
        }
        if(row == 7 || row == 8) //Stress && Work(kj)
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            int value = spinBox->value();
            spinBox->interpretText();
            model->setData(index,value, Qt::EditRole);
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }
};

namespace Ui {
class day_popup;
}

class day_popup : public QDialog, public jsonHandler, public standardWorkouts
{
    Q_OBJECT

public:
    explicit day_popup(QWidget *parent = nullptr, const QDate w_date = QDate(), schedule *p_sched = nullptr);
    ~day_popup();

private slots:
    void on_toolButton_close_clicked();
    void on_toolButton_editMove_clicked();
    void on_toolButton_copy_clicked();
    void on_toolButton_delete_clicked();
    void load_workoutData(int);
    void setNextEditRow();
    void read_workValues();
    void on_tableView_day_clicked(const QModelIndex &index);
    void on_toolButton_dayEdit_clicked(bool checked);
    void on_toolButton_upload_clicked();
    void on_comboBox_stdworkout_activated(int index);
    void on_toolButton_map_clicked();
    void on_dateEdit_workDate_dateChanged(const QDate &date);

private:
    Ui::day_popup *ui;
    schedule *workSched;
    del_daypop daypop_del;
    QStandardItemModel *dayModel,*stdlistModel,*intExport, *sampExport;
    QHash<QDate,QMap<int,QStringList>> workoutMap;
    QMap<int,QStringList> dayWorkouts;
    QHash<int,QString> stdworkData;
    QDate popupDate;
    QStringList *workListHeader;
    QIcon editIcon,addIcon;
    int selWorkout;
    bool editMode,moveWorkout;

    void init_dayWorkouts(QDate);
    void update_workouts();
    void set_comboWorkouts(QString,QString);
    void set_controlButtons(bool);
    void set_exportContent();
    void set_result(int);
    void set_proxyFilter(QString,int,bool);

};

#endif // DAY_POPUP_H
