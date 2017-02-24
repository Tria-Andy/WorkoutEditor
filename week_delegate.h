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
#include <QGraphicsView>
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
        QFont phase_font,date_font,work_font;
        QString temp_value,headInfo;
        QStringList calendar_values;
        QStringList phaseList = settings::get_listValues("Phase");
        QStringList sportList = settings::get_listValues("Sportuse");
        QString delimiter = "-";
        QColor rectColor,gradColor;
        gradColor.setHsv(0,0,180,200);
        int textMargin = 4;
        int celloffset = 21;
        QString phase;
        phase_font.setBold(true);
        phase_font.setPixelSize(settings::get_fontValue("fontBig"));
        date_font.setBold(true);
        date_font.setPixelSize(settings::get_fontValue("fontMedium"));

        work_font.setStyleStrategy(QFont::PreferAntialias);
        work_font.setBold(false);
        work_font.setPixelSize(settings::get_fontValue("fontSmall"));
        work_font.setStyleHint(QFont::SansSerif);

        temp_value = index.data(Qt::DisplayRole).toString();
        calendar_values = temp_value.split(delimiter);

        QLinearGradient rectGradient;
        rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        rectGradient.setSpread(QGradient::RepeatSpread);

        if(index.column() == 0)
        {
            headInfo = calendar_values.at(0) + " - " + calendar_values.at(1) + " - " + calendar_values.at(2);

            QPainterPath rectHead;
            QRect rect_head(option.rect.x(),option.rect.y(), option.rect.width(),20);
            rectHead.addRoundedRect(rect_head,4,4);
            QRect rect_head_text(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,20);

            gradColor.setHsv(0,0,180,200);
            rectColor.setHsv(360,0,80,200);
            rectGradient.setColorAt(0,rectColor);
            rectGradient.setColorAt(1,gradColor);

            painter->setPen(Qt::black);
            painter->setBrush(rectGradient);
            painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
            painter->drawPath(rectHead);
            painter->setPen(Qt::white);
            painter->setFont(date_font);
            painter->drawText(rect_head_text,Qt::AlignLeft | Qt::AlignVCenter,headInfo);

            phase = calendar_values.at(3);

            for(int pos = 0; pos < phaseList.count();++pos)
            {
                if(phase.contains(phaseList.at(pos)))
                {
                    rectColor = settings::get_itemColor(phaseList.at(pos)).toHsv();
                    break;
                }
                else
                {
                    rectColor = settings::get_itemColor(settings::get_generalValue("empty")).toHsv();
                }
            }

            QPainterPath rectPhase;
            QRect rect_phase(option.rect.x(),option.rect.y()+celloffset, option.rect.width(),option.rect.height()-celloffset-1);
            rectPhase.addRoundedRect(rect_phase,4,4);
            QRect rect_phase_text(option.rect.x()+textMargin,option.rect.y()+celloffset, option.rect.width(),option.rect.height()-celloffset-1);

            rectColor.setAlpha(225);
            rectGradient.setColorAt(0,rectColor);
            rectGradient.setColorAt(1,gradColor);

            painter->setBrush(rectGradient);
            painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
            painter->setPen(Qt::black);
            painter->drawPath(rectPhase);
            painter->setFont(phase_font);
            painter->drawText(rect_phase_text,Qt::AlignVCenter,phase);
        }
        else
        {
            for(int pos = 0; pos < sportList.count()+1;++pos)
            {
                if(index.column() == pos+1)
                {
                    if(index.column() != sportList.count()+1)
                    {
                        rectColor = settings::get_itemColor(sportList.at(pos)).toHsv();
                    }
                    else
                    {
                        rectColor = settings::get_itemColor(settings::get_generalValue("sum")).toHsv();
                    }
                    break;
                }
            }

            QString labels;
            labels = "Workouts:\n";
            labels = labels + "Distance:\n";
            labels = labels + "Duration:\n";
            labels = labels + "Stress:";

            QPainterPath rectWork;
            QRect rectWeek(option.rect.x(),option.rect.y(),option.rect.width(),option.rect.height()-1);
            QRect rectLabel(option.rect.x()+textMargin,option.rect.y()+textMargin,(option.rect.width()/2)-textMargin,option.rect.height()-textMargin-1);
            rectWork.addRoundedRect(rectWeek,5,5);

            rectColor.setAlpha(225);
            rectGradient.setColorAt(0,rectColor);
            rectGradient.setColorAt(1,gradColor);

            if(!calendar_values.isEmpty())
            {
                QString phaseValue;
                phaseValue = calendar_values.at(0) + "\n";
                phaseValue = phaseValue + calendar_values.at(1) + "\n";
                phaseValue = phaseValue + calendar_values.at(2) + "\n";
                phaseValue = phaseValue + calendar_values.at(3);

                QRect rectValues(option.rect.x()+(option.rect.width()/2),option.rect.y()+textMargin,(option.rect.width()/2)-textMargin-1,option.rect.height()-textMargin-2);

                painter->setBrush(rectGradient);
                painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
                painter->setPen(Qt::black);
                painter->drawPath(rectWork);

                painter->drawText(rectLabel,Qt::AlignLeft,labels);
                painter->drawText(rectValues,Qt::AlignLeft,phaseValue);
            }
        }
        painter->restore();
    }
};
#endif // WEEK_DELEGATE

