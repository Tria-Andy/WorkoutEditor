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
         int textMargin = 2;
         QFont headFont,workFont;
         QPainterPath rectRow;
         int celloffset = 21;
         QString sport = index.data(Qt::AccessibleTextRole).toString();
         QStringList workValues = index.data(Qt::DisplayRole).toString().split("#");
         QIcon sportIcon = settings::sportIcon.value(sport);

         headFont.setBold(true);
         headFont.setPixelSize(settings::get_fontValue("fontBig"));
         workFont.setBold(false);
         workFont.setPixelSize(settings::get_fontValue("fontMedium"));

         QRect rectEntry(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height());
         rectRow.addRoundedRect(rectEntry,2,2);
         QRect rectText(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,option.rect.height());

         QLinearGradient rectGradient;
         rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
         rectGradient.setSpread(QGradient::RepeatSpread);
         QColor gradColor;
         gradColor.setHsv(0,0,180,200);
         QColor rectColor = settings::get_itemColor(sport).toHsv();

         rectColor.setAlpha(225);
         rectGradient.setColorAt(0,rectColor);
         rectGradient.setColorAt(1,gradColor);

         QPainterPath rectHead;
         QRect rect_head(option.rect.x(),option.rect.y(), option.rect.width(),20);
         rectHead.addRoundedRect(rect_head,4,4);
         QRect rect_head_text(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,20);
         //QRect rectIcon(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,20);

         painter->setPen(Qt::black);
         painter->setBrush(rectGradient);
         painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
         painter->drawPath(rectHead);
         painter->setFont(headFont);
         painter->drawText(rect_head_text,Qt::AlignLeft | Qt::AlignVCenter,workValues.at(0));

         QPainterPath rectWork;
         QRect rectWorkout(option.rect.x(),option.rect.y()+celloffset,option.rect.width(),option.rect.height()-celloffset-1);
         rectWork.addRoundedRect(rectWorkout,5,5);
         QRect rectLabel(option.rect.x()+textMargin,option.rect.y()+celloffset+textMargin,(option.rect.width()/2)-textMargin,option.rect.height()-celloffset-textMargin-1);

         rectGradient.setColorAt(0,rectColor);
         rectGradient.setColorAt(1,gradColor);

         if(!workValues.isEmpty())
         {
             QString partValue;

             for(int i = 1; i < workValues.count(); ++i)
             {
                partValue = partValue + workValues.at(i) + "\n";
             }
             partValue.chop(1);

             QRect rectValues(option.rect.x()+textMargin,option.rect.y()+celloffset+textMargin,option.rect.width()-textMargin,option.rect.height()-celloffset-textMargin-2);
             painter->setBrush(rectGradient);
             painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
             painter->setPen(Qt::black);
             painter->drawPath(rectWork);
             painter->setFont(workFont);
             painter->drawText(rectValues,Qt::AlignLeft,partValue);
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

private:
    Ui::day_popup *ui;
    standardWorkouts *stdWorkouts;
    schedule *workSchedule;
    del_daypop daypop_del;

    QDate popupDate;
    QMap<int,QStringList> workoutMap;
    QStringList *workListHeader;
    QIcon editIcon,addIcon;
    bool moveWorkout;
    int maxWorkouts;

    QMap<int,QStringList> reorder_workouts(QMap<int,QStringList>*);

    void init_dayWorkouts(QDate);
    void copy_workoutValue(QString);
    void set_comboWorkouts(QString);
    void set_controls(bool);
    void set_exportContent();
    void set_result(int);
    void reset_controls();


};

#endif // DAY_POPUP_H
