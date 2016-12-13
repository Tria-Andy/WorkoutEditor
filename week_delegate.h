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

#ifndef WEEK_DELEGATE
#define WEEK_DELEGATE
#include <QtGui>
#include <QItemDelegate>
#include <QTableView>
#include <QDebug>
#include <QDate>
#include "settings.h"

class week_delegate : public QItemDelegate
{
    Q_OBJECT

public:
    week_delegate(QTableView *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont phase_font,date_font, work_font;
        QString temp_value,headInfo;
        QString emptyPhase = settings::get_emptyPhase();
        QStringList calendar_values;
        QStringList phaseList = settings::get_phaseList();
        QStringList sportList = settings::get_sportList();
        QString delimiter = "-";
        QColor rect_color;
        int textMargin = 2;
        QString phase;
        phase_font.setBold(true);
        phase_font.setPixelSize(settings::get_generalValue("fontBig"));
        date_font.setBold(true);
        date_font.setPixelSize(settings::get_generalValue("fontMedium"));
        work_font.setBold(false);
        work_font.setPixelSize(settings::get_generalValue("fontSmall"));

        temp_value = index.data(Qt::DisplayRole).toString();
        calendar_values = temp_value.split(delimiter);

        if(index.column() == 0)
        {
            headInfo = calendar_values.at(0) + " - " + calendar_values.at(1) + " - " + calendar_values.at(2);
            QRect rect_head(option.rect.x(),option.rect.y(), option.rect.width(),20);
            QRect rect_head_text(option.rect.x()+textMargin,option.rect.y(), option.rect.width(),20);
            painter->fillRect(rect_head,QBrush(QColor(128,128,128)));
            painter->fillRect(rect_head_text,QBrush(QColor(128,128,128)));
            QTextOption dateOption(Qt::AlignLeft);
            painter->setPen(Qt::white);
            painter->setFont(date_font);
            painter->drawText(rect_head_text,headInfo,dateOption);
            phase = calendar_values.at(3);

            for(int pos = 0; pos < phaseList.count();++pos)
            {
                if(phase.contains(phaseList.at(pos)))
                {
                    rect_color = settings::get_itemColor(phaseList.at(pos));
                    break;
                }
                else
                {
                    rect_color = settings::get_itemColor(emptyPhase);
                }
            }

            QRect rect_phase(option.rect.x(),option.rect.y()+21, option.rect.width(),option.rect.height()-21);
            QRect rect_phase_text(option.rect.x()+textMargin,option.rect.y()+21, option.rect.width(),option.rect.height()-21);
            painter->fillRect(rect_phase,QBrush(rect_color));
            painter->fillRect(rect_phase_text,QBrush(rect_color));
            QTextOption phaseOption(Qt::AlignVCenter);
            painter->setPen(Qt::black);
            painter->setFont(phase_font);
            painter->drawText(rect_phase_text,phase,phaseOption);
        }
        else
        {
            for(int pos = 0; pos < sportList.count();++pos)
            {
                if(index.column() == pos+1)
                {
                    if(index.column() != 6)
                    {
                        rect_color = settings::get_itemColor(sportList.at(pos));
                    }
                    else
                    {
                        rect_color.setRgb(0,255,255);
                    }
                    break;
                }
            }

            QString labels;
            labels = "Workouts:\n";
            labels = labels + "Distance:\n";
            labels = labels + "Duration:\n";
            labels = labels + "Stress:";

            QRect rect_label(option.rect.x(),option.rect.y(),option.rect.width()/2,option.rect.height());
            QRect rect_label_text(option.rect.x()+textMargin,option.rect.y(),option.rect.width()/2,option.rect.height());
            painter->fillRect(rect_label,QBrush(rect_color));
            painter->fillRect(rect_label_text,QBrush(rect_color));
            QTextOption labeloption(Qt::AlignLeft);
            painter->setPen(Qt::black);
            painter->setFont(work_font);
            painter->drawText(rect_label_text,labels,labeloption);

            if(!calendar_values.isEmpty())
            {
                QString phaseValue;
                phaseValue = calendar_values.at(0) + "\n";
                phaseValue = phaseValue + calendar_values.at(1) + "\n";
                phaseValue = phaseValue + calendar_values.at(2) + "\n";
                phaseValue = phaseValue + calendar_values.at(3);

                QRect rect_work(option.rect.x()+(option.rect.width()/2) ,option.rect.y(),option.rect.width()/2,option.rect.height());
                painter->fillRect(rect_work,QBrush(rect_color));
                QTextOption workoption(Qt::AlignLeft);
                painter->setPen(Qt::black);
                painter->setFont(work_font);
                painter->drawText(rect_work,phaseValue,workoption);
            }
        }
        painter->restore();
    }

};
#endif // WEEK_DELEGATE

