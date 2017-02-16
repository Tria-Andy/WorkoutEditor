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

#ifndef CALENDAR_DELEGATE
#define CALENDAR_DELEGATE
#include <QtGui>
#include <QItemDelegate>
#include <QTableView>
#include <QDebug>
#include <QDate>
#include "settings.h"

class calendar_delegate : public QItemDelegate
{
    Q_OBJECT

public:
    calendar_delegate(QTableView *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QPen rectPen;
        rectPen.setColor(Qt::black);
        rectPen.setWidth(1);
        QFont phase_font,date_font, work_font;
        QString temp_value,dayDate;
        QStringList phaseList = settings::get_listValues("Phase");
        QStringList sportList = settings::get_listValues("Sport");
        QStringList calendar_values;
        QString delimiter = "#";
        QColor rect_color,gradColor;
        gradColor.setHsv(0,0,180,200);
        int textMargin = 2;
        int celloffset = 21;
        phase_font.setBold(true);
        phase_font.setPixelSize(settings::get_fontValue("fontBig"));
        date_font.setBold(true);
        date_font.setPixelSize(settings::get_fontValue("fontMedium"));
        work_font.setBold(false);
        work_font.setPixelSize(settings::get_fontValue("fontSmall")-1);

        temp_value = index.data(Qt::DisplayRole).toString();
        calendar_values = temp_value.split(delimiter);
        dayDate = calendar_values.at(0);
        dayDate = dayDate.left(7);

        QLinearGradient rectGradient;
        rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        rectGradient.setSpread(QGradient::RepeatSpread);

        QPainterPath rectHead;

        QRect rect_head(option.rect.x(),option.rect.y(), option.rect.width(),20);
        rectHead.addRoundedRect(rect_head,3,3);
        QRect rect_head_text(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,20);

        if(QDate::fromString(calendar_values.at(0),"dd MMM yy").addYears(100) ==(QDate::currentDate()))
        {
            rect_color.setHsv(0,255,255,225);
        }
        else
        {
            rect_color.setHsv(360,0,85,200);
        }

        rectGradient.setColorAt(0,rect_color);
        rectGradient.setColorAt(1,gradColor);

        painter->setPen(rectPen);
        painter->setFont(date_font);
        painter->setBrush(rectGradient);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->drawPath(rectHead);

        painter->setPen(Qt::white);
        painter->drawText(rect_head_text,Qt::AlignLeft | Qt::AlignVCenter, dayDate);

        if(index.column() != 0)
        {
            QString workout;
            temp_value = index.data(Qt::DisplayRole).toString();
            calendar_values = temp_value.split(delimiter,QString::SkipEmptyParts);
            calendar_values.removeFirst();

            if(!calendar_values.isEmpty())
            {
                int height = floor((option.rect.height()- celloffset) / calendar_values.count());
                int yPos = (option.rect.y() + celloffset);

                for(int i = 0; i < calendar_values.count(); ++i)
                {
                    workout = calendar_values.at(i);
                    QPainterPath rectWork;
                    QRect rect_work(option.rect.x(),yPos,option.rect.width(),height-1);
                    rectWork.addRoundedRect(rect_work,4,4);
                    QRect rect_work_text(option.rect.x()+textMargin,yPos,option.rect.width()-textMargin,height-1);
                    yPos += height;

                    for(int pos = 0; pos < sportList.count();++pos)
                    {
                        if(workout.contains(sportList.at(pos)))
                        {
                            rect_color = settings::get_itemColor(sportList.at(pos)).toHsv();
                            break;
                        }
                    }

                    rect_color.setAlpha(225);
                    rectGradient.setColorAt(0,gradColor);
                    rectGradient.setColorAt(1,rect_color);

                    painter->setPen(rectPen);
                    painter->setFont(work_font);
                    painter->setBrush(rectGradient);
                    painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
                    painter->drawPath(rectWork);
                    painter->setPen(Qt::black);
                    painter->drawText(rect_work_text,Qt::AlignLeft,workout);
                }
            }
            else
            {
                QPainterPath rectDay;
                QRect rectEmpty(option.rect.x(),option.rect.y()+celloffset,option.rect.width(),option.rect.height()-celloffset-1);
                rectDay.addRoundedRect(rectEmpty,4,4);

                painter->setPen(rectPen);
                painter->setBrush(QBrush(gradColor));
                painter->setRenderHint(QPainter::Antialiasing);
                painter->drawPath(rectDay);
            }
        }
        else
        {
            QPainterPath rectPhase;
            QString phase = index.data(Qt::DisplayRole).toString();
            phase = phase.remove(0,phase.indexOf(delimiter)+1);

            if(!phase.isEmpty())
            {
                for(int pos = 0; pos < phaseList.count();++pos)
                {
                    if(phase.contains(phaseList.at(pos)))
                    {
                        rect_color = settings::get_itemColor(phaseList.at(pos)).toHsv();
                        break;
                    }
                    else
                    {
                        rect_color = settings::get_itemColor(settings::get_generalValue("empty")).toHsv();
                    }
                }

                QRect rect_phase(option.rect.x(),option.rect.y()+celloffset, option.rect.width(),option.rect.height()-celloffset-1);
                rectPhase.addRoundedRect(rect_phase,4,4);
                QRect rect_phase_text(option.rect.x()+textMargin,option.rect.y()+celloffset, option.rect.width()-textMargin,option.rect.height()-celloffset-1);

                rect_color.setAlpha(225);
                rectGradient.setColorAt(0,gradColor);
                rectGradient.setColorAt(1,rect_color);

                painter->setPen(rectPen);
                painter->setFont(phase_font);
                painter->setBrush(rectGradient);
                painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
                painter->drawPath(rectPhase);
                painter->drawText(rect_phase_text,Qt::AlignVCenter,phase);
             }
        }
        painter->restore();
    }

};

#endif // CALENDAR_DELEGATE

