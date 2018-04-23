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
    explicit del_daypop(QObject *parent = 0) : QStyledItemDelegate(parent) {}
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

        if(index.row() == 0 && index.column() == selCol)
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("hh:mm");
            editor->setFrame(true);
            return editor;
        }
        if(index.row() == 1 && index.column() == selCol)
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setFrame(false);
            return editor;
        }
        if(index.row() == 2 && index.column() == selCol)
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setFrame(false);
            return editor;
        }
        if(index.row() == 3 && index.column() == selCol)
        {
            QLineEdit *editor = new QLineEdit(parent);
            editor->setFrame(true);
            return editor;
        }
        if(index.row() ==  4 && index.column() == selCol)
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("hh:mm:ss");
            editor->setFrame(true);
            return editor;
        }
        if(index.row() == 5 && index.column() == selCol)
        {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(true);
            editor->setDecimals(3);
            editor->setMinimum(0.0);
            editor->setMaximum(500.0);
            editor->setSingleStep(0.100);
            return editor;
        }
        if(index.row() == 6 && index.column() == selCol)
        {
            QSpinBox *editor = new QSpinBox(parent);
            editor->setFrame(true);
            editor->setMinimum(0);
            editor->setMaximum(500);
            return editor;
        }
        if(index.row() == 7 && index.column() == selCol)
        {
            QSpinBox *editor = new QSpinBox(parent);
            editor->setFrame(true);
            editor->setMinimum(0);
            editor->setMaximum(5000);
            return editor;
        }
        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        if(index.row() == 0)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"hh:mm"));
        }
        if(index.row() == 1)
        {
            QString value = index.data(Qt::DisplayRole).toString();
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            comboBox->addItems(settings::get_listValues("Sport"));
            comboBox->setCurrentText(value);
        }
        if(index.row() == 2)
        {
            QString value = index.data(Qt::DisplayRole).toString();
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            comboBox->addItems(settings::get_listValues("WorkoutCode"));
            comboBox->setCurrentText(value);
        }
        if(index.row() == 3)
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            lineEdit->setText(index.data().toString());
        }
        if(index.row() == 4)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"hh:mm:ss"));
        }
        if(index.row() == 5)
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->setValue(index.data(Qt::DisplayRole).toDouble());
        }
        if(index.row() == 6 || index.row() == 7)
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            spinBox->setValue(index.data(Qt::DisplayRole).toInt());
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        int col = index.column();

        if(index.row() == 0) //Phase and Level
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            QTime value = timeEdit->time();
            timeEdit->interpretText();
            model->setData(index,value.toString("hh:mm"), Qt::EditRole);
        }
        if(index.row() == 1) //Sport
        {
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            QString value = comboBox->currentText();
            model->setData(index,value, Qt::EditRole);
        }
        if(index.row() == 2) //WorkoutCode
        {
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            QString value = comboBox->currentText();
            model->setData(index,value, Qt::EditRole);
        }
        if(index.row() == 3) //Title
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            QString value = lineEdit->text();
            model->setData(index,value);
        }
        if(index.row() == 4) //Duration
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            QTime value = timeEdit->time();
            timeEdit->interpretText();
            model->setData(index,value.toString("hh:mm:ss"), Qt::EditRole);
            model->setData(model->index(9,col),get_workout_pace(model->data(model->index(5,col)).toDouble(),value,model->data(model->index(1,col)).toString(),true));
        }
        if(index.row() == 5) //Distance
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->interpretText();
            double value = spinBox->value();
            model->setData(index, value, Qt::EditRole);
            model->setData(model->index(9,col),get_workout_pace(value,QTime::fromString(model->data(model->index(4,col)).toString(),"hh:mm:ss"),model->data(model->index(1,col)).toString(),true));
        }
        if(index.row() == 6 || index.row() == 7) //Stress && Work(kj)
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
    explicit day_popup(QWidget *parent = 0, const QDate w_date = QDate(), schedule *p_sched = 0);
    ~day_popup();

private slots:
    void on_toolButton_close_clicked();
    void on_toolButton_editMove_clicked();
    void on_toolButton_copy_clicked();
    void on_toolButton_delete_clicked();
    void edit_workoutDate(QDate);
    void load_workoutData(int);
    void setNextEditRow();
    void update_workValues();
    void on_tableView_day_clicked(const QModelIndex &index);
    void on_toolButton_dayEdit_clicked(bool checked);
    void on_toolButton_upload_clicked();
    void on_comboBox_stdworkout_activated(int index);
    void on_toolButton_map_clicked();

private:
    Ui::day_popup *ui;
    schedule *workSched;
    del_daypop daypop_del;
    QStandardItemModel *dayModel,*stdlistModel,*intExport, *sampExport;
    QSortFilterProxyModel *scheduleProxy,*stdProxy;
    QHash<QString,QString> currWorkout;
    QHash<int,QString> stdworkData;
    QDate popupDate,newDate;
    QModelIndex selIndex;
    QStringList workListHeader;
    QIcon editIcon,addIcon;
    int selWorkout;
    bool editMode,addWorkout;

    void init_dayWorkouts(QDate);
    void set_comboWorkouts(QString,QString);
    void set_controlButtons(bool);
    void set_exportContent();
    void set_result(int);
    void set_proxyFilter(QString,int);

};

#endif // DAY_POPUP_H
