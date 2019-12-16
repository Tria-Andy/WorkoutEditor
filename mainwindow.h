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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include <QtXml>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QDebug>
#include "del_level.h"
#include "del_mousehover.h"
#include "schedule.h"
#include "foodplanner.h"
#include "dialog_addweek.h"
#include "dialog_version.h"
#include "day_popup.h"
#include "week_popup.h"
#include "year_popup.h"
#include "standardworkouts.h"
#include "stress_popup.h"
#include "foodmacro_popup.h"
#include "foodhistory_popup.h"
#include "dialog_export.h"
#include "dialog_stresscalc.h"
#include "dialog_workcreator.h"
#include "dialog_settings.h"
#include "dialog_pacecalc.h"
#include "dialog_week_copy.h"
#include "settings.h"
#include "jsonhandler.h"
#include "activity.h"
#include "calculation.h"
#include "logger.h"


class schedule_delegate : public QStyledItemDelegate, public calculation
{
    Q_OBJECT

public:
    schedule_delegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QHash<QString,QString>* generalValues = settings::getStringMapPointer(settings::stingMap::General);

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QPen rectPen;
        rectPen.setColor(Qt::black);
        rectPen.setWidth(1);
        QFont phase_font,content_font,date_font, work_font;
        QString temp_value,dayDate;
        QStringList phaseList = settings::get_listValues("Phase");
        QStringList sportList = settings::get_listValues("Sport");
        QStringList workoutValues;
        QString delimiter = "#";
        QColor rectColor,gradColor;
        gradColor.setHsv(0,0,180,200);
        int textMargin = 2;
        int celloffset = 21;
        phase_font.setBold(true);
        phase_font.setPixelSize(settings::get_fontValue("fontBig"));
        content_font.setBold(false);
        content_font.setPixelSize(settings::get_fontValue("fontSmall"));
        date_font.setBold(true);
        date_font.setPixelSize(settings::get_fontValue("fontMedium"));
        work_font.setBold(false);
        work_font.setPixelSize(settings::get_fontValue("fontSmall")-1);

        temp_value = index.data(Qt::DisplayRole).toString();
        workoutValues = temp_value.split(delimiter);
        dayDate = workoutValues.at(0);
        dayDate = dayDate.left(7);

        QLinearGradient rectGradient;
        rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        rectGradient.setSpread(QGradient::RepeatSpread);

        QPainterPath rectHead;

        QRect rect_head(option.rect.x(),option.rect.y(), option.rect.width(),20);
        rectHead.addRoundedRect(rect_head,3,3);
        QRect rect_head_text(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,20);


        if(option.state & QStyle::State_Selected)
        {
            rectColor.setHsv(240,255,255,180);
            painter->setPen(Qt::white);
        }
        else
        {
            if(QDate::fromString(workoutValues.at(0),"dd MMM yy").addYears(100) ==(QDate::currentDate()))
            {
                rectColor.setHsv(0,255,255,225);
            }
            else
            {
                rectColor.setHsv(360,0,85,200);
            }
        }
        rectGradient.setColorAt(0,rectColor);
        rectGradient.setColorAt(1,gradColor);

        painter->setPen(rectPen);
        painter->setFont(date_font);
        painter->setBrush(rectGradient);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->drawPath(rectHead);

        painter->setPen(Qt::white);
        painter->drawText(rect_head_text,Qt::AlignLeft | Qt::AlignVCenter, dayDate);

        temp_value = index.data(Qt::DisplayRole).toString();
        workoutValues = temp_value.split(delimiter,QString::SkipEmptyParts);
        workoutValues.removeFirst();

        if(index.column() != 0)
        {
            QString workout;
            temp_value = index.data(Qt::DisplayRole).toString();
            workoutValues = temp_value.split(delimiter,QString::SkipEmptyParts);
            workoutValues.removeFirst();

            if(!workoutValues.isEmpty())
            {
                int height = static_cast<int>(floor((option.rect.height()- celloffset) / workoutValues.count()));
                int yPos = (option.rect.y() + celloffset);

                for(int i = 0; i < workoutValues.count(); ++i)
                {
                    workout = workoutValues.at(i);
                    QPainterPath rectWork;
                    QRect rect_work(option.rect.x(),yPos,option.rect.width(),height-1);
                    rectWork.addRoundedRect(rect_work,4,4);
                    QRect rect_work_text(option.rect.x()+textMargin,yPos,option.rect.width()-textMargin,height-1);
                    yPos += height;

                    for(int pos = 0; pos < sportList.count();++pos)
                    {
                        if(workout.contains(sportList.at(pos)))
                        {
                            rectColor = settings::get_itemColor(sportList.at(pos)).toHsv();
                            break;
                        }
                    }

                    rectColor.setAlpha(225);
                    rectGradient.setColorAt(0,rectColor);
                    rectGradient.setColorAt(1,gradColor);

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
            temp_value = index.data(Qt::DisplayRole).toString();
            workoutValues = index.data(Qt::DisplayRole).toString().split(delimiter,QString::SkipEmptyParts);
            int valueCount = workoutValues.count();

            if(!temp_value.isEmpty())
            {
                int height = static_cast<int>(floor((option.rect.height()- celloffset) / 3));
                for(int pos = 0; pos < phaseList.count();++pos)
                {
                    if(temp_value.contains(phaseList.at(pos)))
                    {
                        rectColor = settings::get_itemColor(phaseList.at(pos)).toHsv();
                        break;
                    }
                    else
                    {
                        rectColor = settings::get_itemColor(generalValues->value("empty")).toHsv();
                    }
                }

                QRect rect_phase(option.rect.x(),option.rect.y()+celloffset, option.rect.width(),option.rect.height()-celloffset-1);
                rectPhase.addRoundedRect(rect_phase,4,4);
                QRect rect_phase_name(option.rect.x()+textMargin,option.rect.y()+celloffset, option.rect.width()-textMargin,height-1);
                QRect rect_phase_content(option.rect.x()+textMargin,option.rect.y()+height+celloffset, option.rect.width()-textMargin,height-1);
                QRect rect_phase_goal(option.rect.x()+textMargin,option.rect.y()+(height*2)+celloffset, option.rect.width()-textMargin,height-1);

                rectColor.setAlpha(225);
                rectGradient.setColorAt(0,rectColor);
                rectGradient.setColorAt(1,gradColor);

                painter->setPen(rectPen);
                painter->setFont(phase_font);
                painter->setBrush(rectGradient);
                painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
                painter->drawPath(rectPhase);
                painter->drawText(rect_phase_name,Qt::AlignVCenter,workoutValues.at(1));

                if(valueCount == 2)
                {
                    painter->setFont(content_font);
                    painter->drawText(rect_phase_content,Qt::AlignVCenter,"");
                    painter->drawText(rect_phase_goal,Qt::AlignVCenter,"");
                }
                else if(valueCount == 3)
                {
                    painter->setFont(content_font);
                    painter->drawText(rect_phase_content,Qt::AlignVCenter,workoutValues.at(2));
                    painter->drawText(rect_phase_goal,Qt::AlignVCenter,"");
                }
                else if(valueCount == 4)
                {
                    painter->setFont(content_font);
                    painter->drawText(rect_phase_content,Qt::AlignVCenter,workoutValues.at(2));
                    painter->drawText(rect_phase_goal,Qt::AlignVCenter | Qt::TextWordWrap,workoutValues.at(3));
                }
             }
        }
        painter->restore();
    }
};

class saison_delegate : public QStyledItemDelegate, public calculation
{
    Q_OBJECT

public:
    saison_delegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QHash<QString,QString>* generalValues = settings::getStringMapPointer(settings::stingMap::General);

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont phase_font,date_font,work_font;
        QString delimiter = "#";
        QStringList saison_values;
        QStringList phaseList = settings::get_listValues("Phase");
        QStringList sportList = settings::get_listValues("Sportuse");
        QColor rectColor,gradColor;
        gradColor.setHsv(0,0,180,200);
        int textMargin = 4;
        int celloffset = 21;

        phase_font.setBold(true);
        phase_font.setPixelSize(settings::get_fontValue("fontBig"));
        date_font.setBold(true);
        date_font.setPixelSize(settings::get_fontValue("fontMedium"));

        work_font.setStyleStrategy(QFont::PreferAntialias);
        work_font.setBold(false);
        work_font.setPixelSize(settings::get_fontValue("fontSmall"));
        work_font.setStyleHint(QFont::SansSerif);

        QLinearGradient rectGradient;
        rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        rectGradient.setSpread(QGradient::PadSpread);

        saison_values = index.data(Qt::DisplayRole).toString().split(delimiter);
        if(index.column() == 0)
        {
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
            painter->drawText(rect_head_text,Qt::AlignLeft | Qt::AlignVCenter,saison_values.at(0));

            for(int pos = 0; pos < phaseList.count();++pos)
            {
                if(saison_values.at(1).contains(phaseList.at(pos)))
                {
                    rectColor = settings::get_itemColor(phaseList.at(pos)).toHsv();
                    break;
                }
                else
                {
                    rectColor = settings::get_itemColor(generalValues->value("empty")).toHsv();
                }
            }

            int height = static_cast<int>(floor((option.rect.height()- celloffset) / 2));

            QPainterPath rectPhase;
            QRect rect_phase(option.rect.x(),option.rect.y()+celloffset, option.rect.width(),option.rect.height()-celloffset-1);
            rectPhase.addRoundedRect(rect_phase,4,4);
            QRect rect_phase_name(option.rect.x()+textMargin,option.rect.y()+celloffset, option.rect.width(),height-1);
            QRect rect_phase_content(option.rect.x()+textMargin,option.rect.y()+height+celloffset, option.rect.width(),height-1);

            rectColor.setAlpha(225);
            rectGradient.setColorAt(0,rectColor);
            rectGradient.setColorAt(1,gradColor);

            painter->setBrush(rectGradient);
            painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
            painter->setPen(Qt::black);
            painter->drawPath(rectPhase);
            painter->setFont(phase_font);
            painter->drawText(rect_phase_name,Qt::AlignVCenter,saison_values.at(1));
            painter->setFont(work_font);
            painter->drawText(rect_phase_content,Qt::AlignVCenter,saison_values.at(2));
        }
        else
        {
            for(int pos = 0; pos < sportList.count();++pos)
            {
                if(index.column() == pos+1)
                {
                    if(index.column() != sportList.count())
                    {
                        rectColor = settings::get_itemColor(sportList.at(pos)).toHsv();
                    }
                    else
                    {
                        rectColor = settings::get_itemColor(generalValues->value("sum")).toHsv();
                    }
                    break;
                }
            }

            QString labels;
            labels = "Workouts:\n";
            labels = labels + "Duration:\n";
            labels = labels + "Amount:\n";
            labels = labels + "Distance:\n";      
            labels = labels + "Stress:";

            QPainterPath rectWork;
            QRect rectWeek(option.rect.x(),option.rect.y(),option.rect.width(),option.rect.height()-1);
            QRect rectLabel(option.rect.x()+textMargin,option.rect.y()+textMargin,(option.rect.width()/2)-textMargin,option.rect.height()-textMargin-1);
            rectWork.addRoundedRect(rectWeek,5,5);

            rectColor.setAlpha(225);
            rectGradient.setColorAt(0,rectColor);
            rectGradient.setColorAt(1,gradColor);


            if(!saison_values.isEmpty())
            {
                QString phaseValue;
                phaseValue = saison_values.at(0) + "\n";
                phaseValue = phaseValue + set_time(saison_values.at(1).toInt()) + "\n";
                phaseValue = phaseValue + saison_values.at(2) + "\n";
                phaseValue = phaseValue + saison_values.at(3) + "\n";
                phaseValue = phaseValue + saison_values.at(4);

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
class summery_delegate : public QStyledItemDelegate, public calculation
{
    Q_OBJECT

public:
    summery_delegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QHash<QString,QString>* generalValues = settings::getStringMapPointer(settings::stingMap::General);

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont phase_font,date_font, work_font;
        QStringList sum_values;
        QStringList sportList = settings::get_listValues("Sport");
        QString delimiter = "-";
        QColor rectColor,gradColor;
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

        sum_values = index.data(Qt::DisplayRole).toString().split(delimiter);

        for(int i = 0; i < sportList.count(); ++i)
        {
            if(sum_values.at(0) == sportList.at(i))
            {
                rectColor = settings::get_itemColor(sportList.at(i)).toHsv();
                break;
            }
            else
            {
                //Summery
                rectColor = settings::get_itemColor(generalValues->value("sum")).toHsv();
            }
        }

        rectColor.setAlpha(225);
        rectGradient.setColorAt(0,rectColor);
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
        painter->drawText(rect_head_text,Qt::AlignLeft | Qt::AlignVCenter,sum_values.at(0));

        QStringList *sumLabels = settings::getHeaderMap("summery");
        QString labels;
        for(int i = 0; i < sumLabels->count(); ++i)
        {
            labels = labels + sumLabels->at(i) +"\n";
        }
        labels.chop(1);

        QPainterPath rectSummery;
        QRect rectSportSum(option.rect.x(),option.rect.y()+celloffset,option.rect.width(),option.rect.height()-celloffset-1);
        rectSummery.addRoundedRect(rectSportSum,5,5);
        QRect rectLabel(option.rect.x()+textMargin,option.rect.y()+celloffset+textMargin,(option.rect.width()/2)-textMargin,option.rect.height()-celloffset-textMargin-1);

        rectGradient.setColorAt(0,rectColor);
        rectGradient.setColorAt(1,gradColor);

        if(!sum_values.isEmpty())
        {
            QString partValue;

            for(int i = 1; i < sum_values.count(); ++i)
            {
                if(i == 2)
                {
                    partValue = partValue + set_time(sum_values.at(i).toInt()) + "\n";
                }
                else
                {
                    partValue = partValue + sum_values.at(i) + "\n";
                }
            }
            partValue.chop(1);

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
class del_treeview : public QStyledItemDelegate, public calculation
{
    Q_OBJECT

public:
    explicit del_treeview(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QHash<QString,QString>* generalValues = settings::getStringMapPointer(settings::stingMap::General);

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        const QAbstractItemModel *model = index.model();
        QString lapName;
        QString breakName = generalValues->value("breakname");
        QStringList levels = settings::get_listValues("Level");
        QString levelName = breakName;

        if(index.parent().isValid())
        {
            //lapName = index.parent().child(index.row(),0).data().toString();
            lapName = model->index(index.row(),0,index.parent()).data().toString();
        }
        else
        {
            lapName = model->data(model->index(index.row(),0,QModelIndex())).toString();
        }

        for(int i = 0; i < levels.count(); ++i)
        {
            if(lapName.contains(levels.at(i))) levelName = levels.at(i);
        }

        QLinearGradient rowGradient(option.rect.topLeft(),option.rect.bottomLeft());
        rowGradient.setSpread(QGradient::RepeatSpread);

        QColor rowColor,gradColor;
        gradColor.setHsv(0,0,200,150);

        if(option.state & (QStyle::State_Selected | QStyle::State_MouseOver))
        {
            rowColor.setHsv(240,255,255,180);
            painter->setPen(Qt::white);
        }
        else
        {
            rowColor = settings::get_itemColor(levelName).toHsv();
            rowColor.setAlpha(100);

            if(lapName == breakName)
            {
                painter->setPen(Qt::white);
            }
            else
            {
                painter->setPen(Qt::black);
            }
        }
        rowGradient.setColorAt(0,gradColor);
        rowGradient.setColorAt(1,rowColor);

        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,rowGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

};
class del_filelist : public QStyledItemDelegate
{
    Q_OBJECT

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        const QAbstractItemModel *model = index.model();
        QString sportName = model->data(model->index(index.row(),2)).toString();

        QLinearGradient rowGradient(option.rect.topLeft(),option.rect.bottomLeft());
        rowGradient.setSpread(QGradient::RepeatSpread);

        QColor rowColor,gradColor;
        gradColor.setHsv(0,0,200,150);

        if(option.state & (QStyle::State_Selected | QStyle::State_MouseOver))
        {
            rowColor.setHsv(240,255,255,180);
            painter->setPen(Qt::white);
        }
        else
        {
            rowColor = settings::get_itemColor(sportName).toHsv();
            rowColor.setAlpha(125);
            painter->setPen(Qt::black);
        }

        rowGradient.setColorAt(0,gradColor);
        rowGradient.setColorAt(1,rowColor);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,rowGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }
};
class del_avgselect : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit del_avgselect(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    QString sport;

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        painter->save();
        QColor rectColor,gradColor;
        gradColor.setHsv(0,0,180,200);

        QLinearGradient rectGradient;
        rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        rectGradient.setSpread(QGradient::RepeatSpread);

        QPainterPath rectValue;
        QRect rectBack(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height());
        rectValue.addRoundedRect(rectBack,1,1);
        QRect rectText(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());

        if(sport.isEmpty())
        {
            rectColor.setHsv(0,0,220,200);
        }
        else
        {
            rectColor = settings::get_itemColor(sport).toHsv();
        }
        painter->setPen(Qt::black);

        rectGradient.setColorAt(0,rectColor);
        rectGradient.setColorAt(1,gradColor);

        painter->setBrush(rectGradient);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->drawPath(rectValue);
        painter->drawText(rectText,Qt::AlignLeft | Qt::AlignVCenter,index.data().toString());
        painter->restore();
    }
};
class del_intselect : public QStyledItemDelegate, public calculation
{
    Q_OBJECT

public:
    explicit del_intselect(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}
    enum{SwimLap,Interval};
    int intType;
    QString sport;

    QHash<QString,QString>* generalValues = settings::getStringMapPointer(settings::stingMap::General);

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        painter->save();
        const QAbstractItemModel *model = index.model();
        QColor rectColor,gradColor;
        gradColor.setHsv(0,0,180,200);
        bool setEdit;

        QLinearGradient rectGradient;
        rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
        rectGradient.setSpread(QGradient::RepeatSpread);

        QPainterPath rectValue;
        QRect rectBack(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height());
        rectValue.addRoundedRect(rectBack,1,1);
        QRect rectText(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        setEdit = model->data(model->index(index.row(),1,QModelIndex())).toBool();

        if(setEdit)
        {
            rectColor.setHsv(120,180,200,200);
            painter->setPen(Qt::black);
        }
        else
        {
            rectColor.setHsv(0,180,220,200);
            painter->setPen(Qt::darkRed);
        }
        rectGradient.setColorAt(0,rectColor);
        rectGradient.setColorAt(1,gradColor);

        painter->setBrush(rectGradient);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->drawPath(rectValue);
        painter->drawText(rectText,Qt::AlignLeft | Qt::AlignVCenter,index.data().toString());
        painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        const QAbstractItemModel *model = index.model();
        bool setEdit = model->data(model->index(index.row(),1,QModelIndex())).toBool();

        if(index.row() == 0 && setEdit)
        {
            QLineEdit *editor = new QLineEdit(parent);
            editor->setFrame(true);
            return editor;
        }

        if(intType == SwimLap)
        {
            if(index.row() == 1 && setEdit)
            {
                QComboBox *editor = new QComboBox(parent);
                editor->setFrame(true);
                return editor;
            }
            if(index.row() == 2 && setEdit)
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setMinimum(0);
                editor->setMaximum(50);
                editor->setFrame(true);
                return editor;
            }
            if((index.row() == 3 || index.row() == 6) && setEdit)
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setMinimum(0);
                editor->setMaximum(200);
                editor->setFrame(true);
                return editor;
            }
        }
        if(intType == Interval)
        {
            if(index.row() == 1 && setEdit)
            {
                QSpinBox *editor = new QSpinBox(parent);
                editor->setMinimum(0);
                editor->setMaximum(10000);
                editor->setFrame(true);
                return editor;
            }
            if(index.row() == 2 && setEdit)
            {
                QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
                editor->setMinimum(0.0);
                editor->setFrame(true);
                editor->setDecimals(3);
                return editor;
            }
            if(index.row() == 3 && setEdit)
            {
                QTimeEdit *editor = new QTimeEdit(parent);
                editor->setDisplayFormat("mm:ss");
                editor->setFrame(true);
                return editor;
            }
        }

        return nullptr;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        if(index.row() == 0)
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            lineEdit->setText(index.data().toString());
        }

        if(intType == SwimLap)
        {
            if(index.row() == 1)
            {
                QString value = index.data().toString();
                QComboBox *comboBox = static_cast<QComboBox*>(editor);
                comboBox->addItems(settings::get_listValues("SwimStyle"));
                comboBox->setCurrentText(value);
            }
            if(index.row() == 2)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data().toInt());
            }
            if(index.row() == 3 || index.row() == 6)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data().toInt());
            }
        }
        if(intType == Interval)
        {
            if(index.row() == 1)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                spinBox->setValue(index.data().toInt());
            }
            if(index.row() == 2)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->setValue(index.data().toDouble());
            }
            if(index.row() == 3)
            {
                QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                timeEdit->setDisplayFormat("mm:ss");
                timeEdit->setTime(QTime::fromString(index.data().toString(),"mm:ss"));
            }
        }
    }
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        if(index.row() == 0)
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            QString value = lineEdit->text();
            model->setData(index,value);
            if(value.contains(generalValues->value("breakname")))
            {
                model->setData(model->index(3,0),"00:20");
                model->setData(model->index(2,0),0);
            }
        }

        if(intType == SwimLap)
        {
            if(index.row() == 1)
            {
                QComboBox *comboBox = static_cast<QComboBox*>(editor);
                model->setData(index,comboBox->currentText());
            }
            if(index.row() == 2)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value);
            }
            if(index.row() == 3 || index.row() == 6)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value);
                if(index.row() == 3) setPace(model,value);
            }
        }
        if(intType == Interval)
        {
            if(index.row() == 1)
            {
                QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
                int value = spinBox->value();
                spinBox->interpretText();
                model->setData(index,value);
            }
            if(index.row() == 2)
            {
                QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
                spinBox->interpretText();
                double value = spinBox->value();
                model->setData(index, value);
                setPace(model,get_timesec(model->data(model->index(3,0)).toString()));
            }
            if(index.row() == 3)
            {
                QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
                timeEdit->setDisplayFormat("mm:ss");
                timeEdit->interpretText();
                QString value = timeEdit->time().toString();
                model->setData(index,value);
                setPace(model,get_timesec(value));
            }
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

    void setPace(QAbstractItemModel *model,int sec) const
    {
        double dist = model->data(model->index(2,0)).toDouble();
        model->setData(model->index(4,0),set_time(calc_lapPace(sec,dist)));
        setSpeed(model,sec);
    }

    void setSpeed(QAbstractItemModel *model,int sec) const
    {
        double factor = sport == settings::SwimLabel ? 1000.0 : 1.0;
        double dist = model->data(model->index(2,0)).toDouble();
        model->setData(model->index(5,0),set_doubleValue(calc_Speed(sec,dist,factor),true));
    }

    void set_duration(QAbstractItemModel *model) const
    {
        model->data(model->index(1,0)).toInt();
    }
};
class del_avgweek : public QStyledItemDelegate, public calculation
{
    Q_OBJECT
public:
    explicit del_avgweek(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QHash<QString,QString>* generalValues = settings::getStringMapPointer(settings::stingMap::General);

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont cFont;
        QString sportname,indexData;
        QStringList sportuse = settings::get_listValues("Sportuse");
        const QAbstractItemModel *model = index.model();
        cFont.setPixelSize(12);

        QColor rectColor;
        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        sportname = model->data(model->index(index.row(),0,QModelIndex())).toString().trimmed();
        indexData = index.data().toString();
        painter->setPen(Qt::black);

        if(index.row() == sportuse.count())
        {
            rectColor = settings::get_itemColor(generalValues->value("sum")).toHsv();
            cFont.setBold(true);
        }
        else
        {
            rectColor = settings::get_itemColor(sportname).toHsv();
            cFont.setBold(false);
        }
        rectColor.setAlpha(175);
        painter->fillRect(option.rect,rectColor);
        painter->setFont(cFont);
        painter->drawText(rect_text,indexData,QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }
};
class del_foodplan : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit del_foodplan(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QLinearGradient itemGradient(option.rect.topLeft(),option.rect.bottomLeft());
        itemGradient.setSpread(QGradient::ReflectSpread);
        QColor itemColor,gradColor;
        QFont foodFont;
        foodFont.setPixelSize(settings::get_fontValue("fontSmall"));
        painter->setFont(foodFont);

        gradColor.setHsv(0,0,200,150);

        if(option.state & (QStyle::State_Selected | QStyle::State_MouseOver))
        {
            itemColor.setHsv(240,255,255,180);
            painter->setPen(Qt::white);
        }
        else
        {
            itemColor.setHsv(255,255,255,0);
            painter->setPen(Qt::black);
        }

        itemGradient.setColorAt(0,gradColor);
        itemGradient.setColorAt(1,itemColor);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,itemGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }
};
class del_foodSummery : public QStyledItemDelegate, public calculation
{
    Q_OBJECT
public:
    explicit del_foodSummery(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QLinearGradient itemGradient(option.rect.topLeft(),option.rect.bottomLeft());
        itemGradient.setSpread(QGradient::ReflectSpread);
        QColor itemColor,gradColor;
        QFont foodFont;
        foodFont.setPixelSize(settings::get_fontValue("fontSmall"));
        painter->setFont(foodFont);

        QVector<double> percent = settings::doubleVector.value(index.data(Qt::UserRole).toString());

        int dayCal = index.model()->data(index.model()->index(3,index.column())).toInt();

        gradColor.setHsv(0,0,200,150);

        if(index.row() == 0)
        {
            itemColor.setHsv(200,150,255,255);
            painter->setPen(Qt::black);
        }
        else if(index.row() > 0 && index.row() < 4)
        {
            itemColor.setHsv(240,255,255,255);
            painter->setPen(Qt::white);
        }
        else if(index.row() == 4)
        {
            if(index.data().toInt() >= (dayCal*(percent.at(0)/100.0)))
            {
                itemColor = settings::get_itemColor("max").toHsv();
                painter->setPen(Qt::black);
            }
            if(index.data().toInt() < (dayCal*(percent.at(0)/100.0)) && index.data().toInt() > (dayCal*(percent.at(1)/100.0)))
            {
                itemColor = settings::get_itemColor("high").toHsv();
                painter->setPen(Qt::black);
            }
            if(index.data().toInt() <= (dayCal*(percent.at(1)/100.0)) && index.data().toInt() > (dayCal*(percent.at(2)/100.0)))
            {
                itemColor = settings::get_itemColor("low").toHsv();
                painter->setPen(Qt::black);
            }
            if(index.data().toInt() <= (dayCal*(percent.at(2)/100.0)))
            {
                itemColor = settings::get_itemColor("min").toHsv();
                painter->setPen(Qt::black);
            }
        }

        itemGradient.setColorAt(0,gradColor);
        itemGradient.setColorAt(1,itemColor);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,itemGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }
};
class del_foodWeekSum : public QStyledItemDelegate, public calculation
{
    Q_OBJECT
public:
    explicit del_foodWeekSum(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QLinearGradient itemGradient(option.rect.topLeft(),option.rect.bottomLeft());
        itemGradient.setSpread(QGradient::ReflectSpread);
        QColor itemColor,gradColor;
        QFont foodFont;
        foodFont.setPixelSize(settings::get_fontValue("fontSmall"));
        painter->setFont(foodFont);

        QVector<double> percent = settings::doubleVector.value(index.data(Qt::UserRole).toString());

        int weekCal = index.model()->data(index.model()->index(3,index.column())).toInt();

        gradColor.setHsv(0,0,200,150);

        if(index.row() == 0)
        {
            itemColor.setHsv(200,150,255,255);
            painter->setPen(Qt::black);
        }
        else if(index.row() > 0 && index.row() < 4)
        {
            itemColor.setHsv(240,255,255,255);
            painter->setPen(Qt::white);
        }
        else if(index.row() == 4)
        {
            if(index.data().toInt() >= (weekCal*(percent.at(0)/100.0)))
            {
                itemColor = settings::get_itemColor("max").toHsv();
                painter->setPen(Qt::black);
            }
            if(index.data().toInt() < (weekCal*(percent.at(0)/100.0)) && index.data().toInt() > (weekCal*(percent.at(1)/100.0)))
            {
                itemColor = settings::get_itemColor("high").toHsv();
                painter->setPen(Qt::black);
            }
            if(index.data().toInt() <= (weekCal*(percent.at(1)/100.0)) && index.data().toInt() > (weekCal*(percent.at(2)/100.0)))
            {
                itemColor = settings::get_itemColor("low").toHsv();
                painter->setPen(Qt::black);
            }
            if(index.data().toInt() <= (weekCal*(percent.at(2)/100.0)))
            {
                itemColor = settings::get_itemColor("min").toHsv();
                painter->setPen(Qt::black);
            }
        }

        itemGradient.setColorAt(0,gradColor);
        itemGradient.setColorAt(1,itemColor);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,itemGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }
};
class del_foodDaySum : public QStyledItemDelegate, public calculation
{
    Q_OBJECT
public:
    explicit del_foodDaySum(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QLinearGradient itemGradient(option.rect.topLeft(),option.rect.bottomLeft());
        itemGradient.setSpread(QGradient::ReflectSpread);
        QColor itemColor,gradColor;
        QFont foodFont;
        foodFont.setPixelSize(settings::get_fontValue("fontSmall"));
        painter->setFont(foodFont);

        double percent = index.data(Qt::UserRole).toDouble();
        double macroRange = settings::getdoubleMapPointer(settings::dMap::Double)->value("Macrorange")/100.0;
        gradColor.setHsv(0,0,200,150);

        if(index.column() == 0)
        {   
            itemColor.setHsv(240,255,255,255);
            painter->setPen(Qt::white);
        }
        else
        {
            if(percent == 0.0)
            {
                itemColor.setHsv(200,255,255,255);
                painter->setPen(Qt::black);
            }
            else
            {
                double perHigh = settings::doubleVector.value("Macros").at(index.column()-1);
                double perLow = perHigh - perHigh*macroRange;
                perHigh = perHigh + perHigh*macroRange;

                if(percent < perLow)
                {
                    itemColor.setHsv(60,255,255,255);
                    painter->setPen(Qt::black);
                }
                else if(percent <= perHigh && percent >= perLow)
                {
                    itemColor.setHsv(120,255,170,255);
                    painter->setPen(Qt::black);
                }
                else if(percent > perHigh)
                {
                    itemColor.setHsv(0,255,255,255);
                    painter->setPen(Qt::white);
                }
            }
        }

        itemGradient.setColorAt(0,gradColor);
        itemGradient.setColorAt(1,itemColor);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,itemGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }
};

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow, public calculation
{
    Q_OBJECT

private:
    Ui::MainWindow *ui;

    schedule *workSchedule;
    standardWorkouts *stdWorkouts;
    Activity *loadActivity;
    foodplanner *foodPlan;
    jsonHandler *jsonhandler;
    settings editorSettings;
    schedule_delegate schedule_del;
    saison_delegate saison_del;
    summery_delegate sum_del;
    del_filelist fileList_del;
    del_treeview tree_del;
    del_intselect intSelect_del;
    del_avgselect avgSelect_del;
    del_level level_del;
    del_avgweek avgweek_del;
    del_foodplan foodPlan_del;
    del_foodSummery foodSumDay_del;
    del_foodWeekSum foodSumWeek_del;
    del_foodDaySum foodSumSelect_del;
    del_mousehover mousehover_del;
    QStandardItemModel *sumModel,*fileModel,*infoModel,*avgModel;
    QItemSelectionModel *treeSelection;
    QStringList modus_list,cal_header,y2Label,menuCopy;
    QStringList *schedMode, *avgHeader;
    QLabel *planerMode;
    QToolButton *planMode;
    QIcon editorIcon,planerIcon,foodIcon;
    QWidget *menuSpacer;
    QButtonGroup *phaseGroup;
    QComboBox *modules;
    QDate firstdayofweek;
    QString dateFormat,longTime,shortTime;
    QHash<QString,QString> *gcValues,*generalValues;

    //Intervall Chart
    QVector<double> secTicker,speedValues,secondValues,polishValues,speedMinMax,secondMinMax,rangeMinMax;
    void set_speedValues(int);
    void set_speedgraph();
    void set_speedPlot(double,double,int);
    void set_polishValues(int,double,double,double,int);
    void resetPlot();

    QMap<QString,QStringList> *saisonValues;
    int avgCounter,sportUse;
    QDate selectedDate;
    QString weeknumber,buttonStyle,dateformat,timeshort,timelong;
    int userSetup,saisonWeek,saisonWeeks,foodcopyLine;
    int weekDays,weekCounter,weekRange;
    bool isWeekMode,graphLoaded,actLoaded,foodcopyMode,lineSelected,dayLineSelected;

    void set_tableWidgetItems(QTableWidget*,int,int,QAbstractItemDelegate*);
    void set_tableHeader(QTableWidget*,QStringList*,bool);

    void openPreferences();
    void summery_Set(QDate,QStandardItem*);
    void summery_calc(QMap<QString,QVector<double>>*,QMap<QString,QVector<double>>*,QStringList*,int);
    QString set_summeryString(int,QMap<QString,QVector<double>>*,QString);
    void set_saisonValues(QStringList*,QString,int);
    void workoutSchedule(QDate);
    void saisonSchedule(QString);
    bool check_Date(QDate);
    QString get_weekRange();
    void set_buttons(QDate,bool);
    void set_calender();
    void set_phaseButtons();
    void refresh_saisonInfo();
    void loadUISettings();

    //Editor
    void select_activityFile();
    void read_activityFiles();
    void clearActivtiy();
    void loadfile(const QString &filename);
    void selectAvgValues(QModelIndex,int);
    void setCurrentTreeIndex(bool);
    void init_editorViews();
    void init_controlStyleSheets();
    void update_infoModel();
    void fill_WorkoutContent();
    void unselect_intRow(bool);
    void set_menuItems(int);
    void reset_jsontext();
    void freeMem();

    //Food
    QMap<int,QStringList> selectedLine;
    void fill_foodPlanTable(QDate);
    void fill_foodSumTable(QDate);
    void fill_foodPlanList(bool);
    void set_selectedMeal(QPair<QString,QVector<int>>,double);
    void set_menuList(QDate,QString);
    void reset_menuEdit();
    void change_foodOrder(int);
    void set_foodOrder();

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    //Menu
    void on_actionEditor_triggered();
    void on_actionPlaner_triggered();
    void on_actionExit_triggered();
    void on_actionNew_triggered();
    void on_actionExport_to_Golden_Cheetah_triggered();
    void on_actionExit_and_Save_triggered();
    void on_actionSelect_File_triggered();
    void on_actionReset_triggered();
    void on_actionStress_Calculator_triggered();
    void on_actionIntervall_Editor_triggered();
    void on_actionPreferences_triggered();
    void on_actionPace_Calculator_triggered();
    void on_actionVersion_triggered();
    void on_actionPMC_triggered();
    void set_module(int);

    //Planner
    void on_calendarWidget_clicked(const QDate &date);
    void on_tableWidget_summery_clicked(const QModelIndex &index);
    void on_actionEdit_Week_triggered();
    void on_toolButton_weekCurrent_clicked();
    void on_toolButton_weekMinus_clicked();
    void on_toolButton_weekPlus_clicked();
    void on_toolButton_weekFour_clicked();
    void toolButton_planMode(bool);
    void set_phaseFilter(int);
    void refresh_schedule();
    void refresh_saison();

    //Editor
    void setSelectedIntRow(QModelIndex);
    void on_horizontalSlider_factor_valueChanged(int value);
    void on_treeView_intervall_clicked(const QModelIndex &index);
    void on_lineEdit_workContent_textChanged(const QString &arg1);
    void on_toolButton_sync_clicked();
    void on_toolButton_clearContent_clicked();  
    void on_toolButton_update_clicked();
    void on_toolButton_delete_clicked();
    void on_toolButton_add_clicked();
    void on_toolButton_upInt_clicked();
    void on_toolButton_downInt_clicked();
    void on_toolButton_addSelect_clicked();
    void on_toolButton_clearSelect_clicked();
    void on_treeView_files_clicked(const QModelIndex &index);
    void on_actionRefresh_Filelist_triggered();
    void on_comboBox_saisonName_currentIndexChanged(const QString &arg1);
    void on_actionSave_triggered();
    void on_actionDelete_triggered();

    //Food
    void on_toolButton_addMenu_clicked();
    void on_toolButton_menuEdit_clicked();
    void on_toolButton_saveMeals_clicked();
    void on_calendarWidget_Food_clicked(const QDate &date);
    void on_toolButton_deleteMenu_clicked();
    void on_doubleSpinBox_portion_valueChanged(double arg1);
    void on_toolButton_addMeal_clicked();
    void on_toolButton_foodUp_clicked();
    void on_toolButton_foodDown_clicked();
    void on_treeView_meals_clicked(const QModelIndex &index);
    void on_treeView_meals_collapsed(const QModelIndex &index);
    void on_listWidget_menuEdit_itemClicked(QListWidgetItem *item);
    void on_treeView_meals_expanded(const QModelIndex &index);
    void mealSave(QStandardItem*);
    void on_toolButton_mealreset_clicked();
    void on_toolButton_menuCopy_clicked();
    void on_toolButton_menuPaste_clicked();
    void on_comboBox_weightmode_currentIndexChanged(const QString &arg1);
    void selectFoodSection(int);
    void selectFoodDay(int);
    void on_actionFood_Macros_triggered();
    void on_toolButton_linePaste_clicked();
    void on_actionfood_History_triggered();
    void on_actionFood_triggered();
    void on_tableWidget_schedule_itemClicked(QTableWidgetItem *item);
    void on_tableWidget_saison_itemClicked(QTableWidgetItem *item);
    void on_tableWidget_foodPlan_itemChanged(QTableWidgetItem *item);
    void on_tableWidget_foodPlan_itemClicked(QTableWidgetItem *item);
    void on_listWidget_weekPlans_itemClicked(QListWidgetItem *item);
    void on_listWidget_menuEdit_itemDoubleClicked(QListWidgetItem *item);
    void on_toolButton_menuClear_clicked();
};

#endif // MAINWINDOW_H
