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

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        int textMargin = option.rect.width()/50;
        QFont headFont,workFont;
        QString sport = index.data(Qt::ToolTipRole).toString();

        headFont.setBold(true);
        headFont.setPixelSize(settings::get_fontValue("fontBig"));
        workFont.setBold(false);
        workFont.setPixelSize(settings::get_fontValue("fontMedium"));

        QLinearGradient rectGradient;
        rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        rectGradient.setSpread(QGradient::RepeatSpread);
        QColor gradColor;
        gradColor.setHsv(0,0,180,200);
        QColor rectColor = settings::get_itemColor(sport).toHsv();

        rectColor.setAlpha(225);
        rectGradient.setColorAt(0,rectColor);
        rectGradient.setColorAt(1,gradColor);

        QPainterPath workPath;
        QRect workHead(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height()*0.25);
        workPath.addRoundedRect(workHead,5,5);

        QRect workIcon(option.rect.x(),option.rect.y(), workHead.height(),workHead.height());
        QRect workTitle(workIcon.right()+textMargin,option.rect.y(),(option.rect.width()-workIcon.width()-textMargin)*0.75,workHead.height());
        QRect workTime(workTitle.right(),option.rect.y(),(option.rect.width()-workIcon.width()-textMargin)*0.25,workHead.height());

        painter->setBrush(rectGradient);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->setPen(Qt::black);
        painter->setFont(headFont);
        painter->drawPath(workPath);
        painter->drawPixmap(workIcon,settings::sportIcon.value(sport));
        painter->drawText(workTitle,Qt::AlignLeft | Qt::AlignVCenter,sport + " - "+index.data(Qt::AccessibleTextRole).toString());
        painter->drawText(workTime,Qt::AlignRight | Qt::AlignVCenter,index.data(Qt::UserRole).toString());

        QPainterPath bodyPath;
        QRect workInfo(option.rect.x(),workHead.bottom(),option.rect.width(),option.rect.height()*0.725);
        bodyPath.addRoundedRect(workInfo,5,5);

        rectGradient.setColorAt(0,rectColor);
        rectGradient.setColorAt(1,gradColor);

        if(!sport.isEmpty())
        {
             QString workMeta;
             int rectHeight = workInfo.height()*0.25;
             int rectWidth = (option.rect.width()-rectHeight)/2;

             workMeta = index.data(Qt::DisplayRole).toString()+"\n"+index.data(Qt::AccessibleDescriptionRole).toString()+" - "+index.data(Qt::UserRole+5).toString();

             QRect rectMeta(option.rect.x()+textMargin,workHead.bottom(),option.rect.width(),workInfo.height()*0.5);

             QRect duraIcon(option.rect.x(),rectMeta.bottom(),rectHeight,rectHeight);
             QRect duraValue(duraIcon.right()+textMargin,rectMeta.bottom(),rectWidth,rectHeight);
             QRect distIcon(duraValue.right(),rectMeta.bottom(),rectHeight,rectHeight);
             QRect distValue(distIcon.right()+textMargin,rectMeta.bottom(),rectWidth,rectHeight);

             QRect stressIcon(option.rect.x(),duraIcon.bottom(),rectHeight,rectHeight);
             QRect stressValue(stressIcon.right()+textMargin,duraIcon.bottom(),rectWidth,rectHeight);
             QRect workIcon(stressValue.right(),duraIcon.bottom(),rectHeight,rectHeight);
             QRect workValue(workIcon.right()+textMargin,duraIcon.bottom(),rectWidth,rectHeight);

             painter->setBrush(rectGradient);
             painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
             painter->setPen(Qt::black);
             painter->setFont(workFont);
             painter->drawPath(bodyPath);
             painter->drawText(rectMeta,Qt::AlignLeft | Qt::AlignVCenter,workMeta);
             painter->drawPixmap(duraIcon,settings::sportIcon.value("Duration"));
             painter->drawText(duraValue,Qt::AlignLeft | Qt::AlignVCenter,index.data(Qt::UserRole+1).toString());
             painter->drawPixmap(distIcon,settings::sportIcon.value("Distance"));
             painter->drawText(distValue,Qt::AlignLeft | Qt::AlignVCenter,index.data(Qt::UserRole+2).toString());
             painter->drawPixmap(stressIcon,settings::sportIcon.value("TSS"));
             painter->drawText(stressValue,Qt::AlignLeft | Qt::AlignVCenter,index.data(Qt::UserRole+3).toString());
             painter->drawPixmap(workIcon,settings::sportIcon.value("KJ"));
             painter->drawText(workValue,Qt::AlignLeft | Qt::AlignVCenter,index.data(Qt::UserRole+4).toString()+" - ("+index.data(Qt::UserRole+6).toString()+")");
        }
        painter->restore();
    }
};

namespace Ui {
class day_popup;
}

class day_popup : public QDialog, public jsonHandler,public calculation
{
    Q_OBJECT

public:
    explicit day_popup(QWidget *parent = nullptr, const QDate w_date = QDate(), schedule *p_sched = nullptr,standardWorkouts *p_stdWorkout = nullptr);
    ~day_popup();

private slots:
    void on_toolButton_close_clicked();
    void on_toolButton_editMove_clicked();
    void on_toolButton_copy_clicked();
    void on_toolButton_delete_clicked();
    void on_toolButton_dayEdit_clicked(bool checked);
    void on_toolButton_upload_clicked();
    void on_toolButton_map_clicked();
    void on_dateEdit_workDate_dateChanged(const QDate &date);
    void on_comboBox_stdworkout_currentIndexChanged(int index);
    void on_tableWidget_day_itemClicked(QTableWidgetItem *item);
    void on_toolButton_title_clicked();
    void on_toolButton_comment_clicked();
    void on_toolButton_addWorkout_clicked();
    void on_toolButton_clearMask_clicked();
    void on_comboBox_workSport_currentIndexChanged(const QString &arg1);
    void on_timeEdit_workDuration_userTimeChanged(const QTime &time);
    void on_doubleSpinBox_workDistance_valueChanged(double arg1);

private:
    Ui::day_popup *ui;
    standardWorkouts *stdWorkouts;
    schedule *workSchedule;
    del_daypop daypop_del;

    QDate popupDate;
    QMap<int,QStringList> workoutMap;
    QString isSeries,isGroup;
    QStringList *workListHeader,*partTags,*stepTags;
    QStringList weekMeta;
    QIcon editIcon,addIcon;
    bool moveWorkout;
    int maxWorkouts;

    QMap<int,QStringList> reorder_workouts(QMap<int,QStringList>*);
    QVariant get_itemValue(QStandardItem*,QStringList*,QString);

    void init_dayWorkouts(QDate);
    void copy_workoutValue(QString);
    void set_comboWorkouts(QString);
    void set_controls(bool);
    void set_exportContent();
    void fill_exportValues(QStandardItem*);
    void read_stdWorkout(QString);
    void read_childValues(QStandardItem*);
    void set_result(int);
    void reset_controls();
    void set_progress(int);
};

#endif // DAY_POPUP_H
