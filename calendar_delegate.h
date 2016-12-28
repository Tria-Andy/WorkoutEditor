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
        QFont phase_font,date_font, work_font;
        QString temp_value,dayDate;
        QStringList phaseList = settings::get_phaseList();
        QStringList sportList = settings::get_sportList();
        QStringList calendar_values;
        QString delimiter = "#";
        QColor rect_color;
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
        QRect rect_head(option.rect.x(),option.rect.y(), option.rect.width(),20);
        QRect rect_head_text(option.rect.x()+ textMargin,option.rect.y(), option.rect.width(),20);
        rect_color.setRgb(128,128,128);
        painter->fillRect(rect_head,QBrush(rect_color));
        painter->fillRect(rect_head_text,QBrush(rect_color));
        QTextOption dateOption(Qt::AlignLeft);

        if(QDate::fromString(calendar_values.at(0),"dd MMM yy").addYears(100) ==(QDate::currentDate()))
        {
            painter->fillRect(rect_head,QBrush(QColor(200,0,0)));
        }
        else
        {
            painter->fillRect(rect_head,QBrush(rect_color));
        }

        painter->setPen(Qt::white);
        painter->setFont(date_font);
        painter->drawText(rect_head_text,dayDate,dateOption);

        if(index.column() != 0)
        {
            temp_value = index.data(Qt::DisplayRole).toString();
            calendar_values = temp_value.split(delimiter,QString::SkipEmptyParts);
            calendar_values.removeFirst();

            if(!calendar_values.isEmpty())
            {
                int height = (option.rect.height()- celloffset) / calendar_values.count();
                int y = option.rect.y()+ celloffset;

                for(int i = 0; i < calendar_values.count(); ++i)
                {
                    QString workout = calendar_values.at(i);

                    QRect rect_work(option.rect.x(),y,option.rect.width(),height);
                    QRect rect_work_text(option.rect.x()+ textMargin,y,option.rect.width(),height);

                    y += height+1;

                    for(int pos = 0; pos < sportList.count();++pos)
                    {
                        if(workout.contains(sportList.at(pos)))
                        {
                            rect_color = settings::get_itemColor(sportList.at(pos));
                            break;
                        }
                    }

                    painter->fillRect(rect_work,QBrush(rect_color));
                    painter->fillRect(rect_work_text,QBrush(rect_color));

                    QTextOption workoption(Qt::AlignLeft);
                    painter->setPen(Qt::black);
                    painter->setFont(work_font);
                    painter->drawText(rect_work_text,workout,workoption);
                }

            }
        }
        else
        {

            QString phase = index.data(Qt::DisplayRole).toString();
            phase = phase.remove(0,phase.indexOf(delimiter)+1);
            phase = phase.remove(0,phase.indexOf(delimiter)+1);
            if(!phase.isEmpty())
            {
                for(int pos = 0; pos < phaseList.count();++pos)
                {
                    if(phase.contains(phaseList.at(pos)))
                    {
                        rect_color = settings::get_itemColor(phaseList.at(pos));
                        break;
                    }
                    else
                    {
                        rect_color = settings::get_itemColor("emptycolor");
                    }
                }

                QRect rect_phase(option.rect.x(),option.rect.y()+celloffset, option.rect.width(),option.rect.height()-celloffset);
                QRect rect_phase_text(option.rect.x()+ textMargin,option.rect.y()+celloffset, option.rect.width(),option.rect.height()-celloffset);
                painter->fillRect(rect_phase,QBrush(rect_color));
                painter->fillRect(rect_phase_text,QBrush(rect_color));

                QTextOption phaseOption(Qt::AlignVCenter);
                painter->setPen(Qt::black);
                painter->setFont(phase_font);
                painter->drawText(rect_phase_text,phase,phaseOption);
             }
        }
        painter->restore();
    }

};

#endif // CALENDAR_DELEGATE

