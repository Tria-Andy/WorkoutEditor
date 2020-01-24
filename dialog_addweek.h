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
    QHash<QString,QString>* generalValues = settings::getStringMapPointer(settings::stingMap::General);

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        int textMargin = option.rect.width()/50;
        int iconMargin = 2;
        QFont headFont,workFont;
        QString sport = index.data(Qt::AccessibleTextRole).toString();
        QStringList sportValues = index.data(Qt::DisplayRole).toString().split("#");
        QString sportIcon = settings::sportIcon.value(sport);
        QString timeIcon = ":/images/icons/Timewatch.png";
        QString energyIcon = ":/images/icons/Battery.png";
        QString distIcon = ":/images/icons/Kilometer.png";
        QString percentIcon = ":/images/icons/Percent.png";

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

        QPainterPath sportPath;
        QRect workHead(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height()*0.3);
        sportPath.addRoundedRect(workHead,5,5);
        int headerHeight = workHead.height();

        QRect sportImage(option.rect.x(),option.rect.y(), headerHeight-iconMargin,headerHeight-iconMargin);
        QRect sportTitle(sportImage.right()+textMargin,option.rect.y(),(option.rect.width()-headerHeight-textMargin)*0.5,workHead.height());
        QRect sportCount(sportTitle.right(),option.rect.y(),(option.rect.width()-headerHeight-textMargin)*0.5,workHead.height());

        painter->setBrush(rectGradient);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->setPen(Qt::black);
        painter->setFont(headFont);
        painter->drawPath(sportPath);
        painter->drawPixmap(sportImage,sportIcon);
        painter->drawText(sportTitle,Qt::AlignLeft | Qt::AlignVCenter,sportValues.at(0));
        painter->drawText(sportCount,Qt::AlignRight | Qt::AlignVCenter,sportValues.at(1));

        QPainterPath bodyPath;
        QRect sportMeta(option.rect.x(),workHead.bottom(),option.rect.width(),option.rect.height()*0.7);
        bodyPath.addRoundedRect(sportMeta,5,5);

        rectGradient.setColorAt(0,rectColor);
        rectGradient.setColorAt(1,gradColor);

        if(!sportValues.isEmpty())
        {
         int rectHeight = sportMeta.height()*0.5;
         int rectwidth = (sportMeta.width()-rectHeight)*0.5;

         QRect rectTime(option.rect.x(),workHead.bottom(),rectHeight-iconMargin,rectHeight-iconMargin);
         QRect rectAmount(rectTime.right()+textMargin,workHead.bottom(),rectwidth,rectHeight);

         QRect rectPercentIcon(rectAmount.right(),workHead.bottom(),rectHeight-iconMargin,rectHeight-iconMargin);
         QRect rectPercent(rectPercentIcon.right()+textMargin,workHead.bottom(),rectwidth,rectHeight);

         QRect rectDistIcon(option.rect.x(),rectAmount.bottom(),rectHeight-iconMargin,rectHeight-iconMargin);
         QRect rectDist(rectDistIcon.right()+textMargin,rectAmount.bottom(),rectwidth,rectHeight);

         QRect rectEngeryIcon(rectDist.right(),rectAmount.bottom(),rectHeight-iconMargin,rectHeight-iconMargin);
         QRect rectEnergy(rectEngeryIcon.right()+textMargin,rectAmount.bottom(),rectwidth,rectHeight);

         painter->setBrush(rectGradient);
         painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
         painter->setPen(Qt::black);
         painter->setFont(workFont);
         painter->drawPath(bodyPath);

         painter->drawPixmap(rectTime,timeIcon);
         painter->drawText(rectAmount,Qt::AlignLeft | Qt::AlignVCenter,sportValues.at(2));

         painter->drawPixmap(rectDistIcon,distIcon);
         painter->drawText(rectDist,Qt::AlignLeft | Qt::AlignVCenter,sportValues.at(3));

         painter->drawPixmap(rectPercentIcon,percentIcon);
         painter->drawText(rectPercent,Qt::AlignLeft | Qt::AlignVCenter,sportValues.at(4));

         painter->drawPixmap(rectEngeryIcon,energyIcon);
         painter->drawText(rectEnergy,Qt::AlignLeft | Qt::AlignVCenter,sportValues.at(5));
        }
        painter->restore();
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
    QMap<QString, QVector<double>> compValues;
    QString editWeekID,timeFormat,empty;
    QStringList sportuseList,weekMeta;
    QStringList *weekHeader;
    int sportlistCount;
    bool update;
    QHash<QString,QString> *generalValues;
    void fill_values(QString);
    void update_values();
};

#endif // DIALOG_ADDWEEK_H
