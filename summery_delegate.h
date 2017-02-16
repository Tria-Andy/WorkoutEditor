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

#ifndef SUMMERY_DELEGATE
#define SUMMERY_DELEGATE

#include <QtGui>
#include <QItemDelegate>
#include <QTableView>
#include "settings.h"

class summery_delegate : public QItemDelegate
{
    Q_OBJECT

public:
    summery_delegate(QTableView *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont phase_font,date_font, work_font;
        QString temp_value;
        QStringList sum_values;
        QStringList sportList = settings::get_listValues("Sport");
        QString delimiter = "-";
        QColor rect_color,gradColor;
        gradColor.setHsv(0,0,180,200);
        int textMargin = 2;
        int celloffset = 21;
        phase_font.setBold(true);
        phase_font.setPixelSize(settings::get_fontValue("fontBig"));
        date_font.setBold(true);
        date_font.setPixelSize(settings::get_fontValue("fontMedium"));
        work_font.setBold(false);
        work_font.setPixelSize(settings::get_fontValue("fontSmall"));

        QLinearGradient rectGradient;
        rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        rectGradient.setSpread(QGradient::RepeatSpread);

        temp_value = index.data(Qt::DisplayRole).toString();
        sum_values = temp_value.split(delimiter);

        for(int i = 0; i < sportList.count(); ++i)
        {
            if(sum_values.at(0) == sportList.at(i))
            {
                rect_color = settings::get_itemColor(sportList.at(i)).toHsv();
                break;
            }
            else
            {
                //Summery
                rect_color = settings::get_itemColor(settings::get_generalValue("sum")).toHsv();
            }
        }

        rect_color.setAlpha(225);
        rectGradient.setColorAt(0,rect_color);
        rectGradient.setColorAt(1,gradColor);

        QPainterPath rectHead;
        QRect rect_head(option.rect.x(),option.rect.y(), option.rect.width(),20);
        rectHead.addRoundedRect(rect_head,4,4);
        QRect rect_head_text(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,20);

        painter->setPen(Qt::black);
        painter->setBrush(rectGradient);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->drawPath(rectHead);
        painter->setFont(date_font);
        painter->drawText(rect_head_text,Qt::AlignLeft,sum_values.at(0));

        QString labels;
        labels = "Workouts:\n";
        labels = labels + "Duration(Hours):\n";
        labels = labels + "Amount(%):\n";
        labels = labels + "Distance(Km):\n";
        labels = labels + "Stress(TSS):";

        QPainterPath rectSummery;
        QRect rectSportSum(option.rect.x(),option.rect.y()+celloffset,option.rect.width(),option.rect.height()-celloffset-1);
        rectSummery.addRoundedRect(rectSportSum,5,5);
        QRect rectLabel(option.rect.x()+textMargin,option.rect.y()+celloffset+textMargin,(option.rect.width()/2)-textMargin,option.rect.height()-celloffset-textMargin-1);

        rectGradient.setColorAt(0,gradColor);
        rectGradient.setColorAt(1,rect_color);

        if(!sum_values.isEmpty())
        {

            QString partValue;
            partValue = sum_values.at(1) + "\n";
            partValue = partValue + sum_values.at(2) + "\n";
            partValue = partValue + sum_values.at(3) + "\n";
            partValue = partValue + sum_values.at(4) + "\n";
            partValue = partValue + sum_values.at(5) + "\n";

            QRect rectValues(option.rect.x()+(option.rect.width()/2),option.rect.y()+celloffset+textMargin,(option.rect.width()/2)-textMargin-1,option.rect.height()-+celloffset-textMargin-2);

            painter->setBrush(rectGradient);
            painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
            painter->setPen(Qt::black);
            painter->drawPath(rectSummery);
            painter->setFont(work_font);
            painter->drawText(rectLabel,Qt::AlignLeft,labels);
            painter->drawText(rectValues,Qt::AlignLeft,partValue);
        }
        painter->restore();
    }
};

#endif // SUMMERY_DELEGATE

