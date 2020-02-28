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


class schedule_delegate : public QStyledItemDelegate
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
        int textMargin = option.rect.width()/50;
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

        QPainterPath headPath;
        QRect rectHead(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height()*0.175);
        headPath.addRoundedRect(rectHead,3,3);
        QRect rectHeadText(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,rectHead.height());


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
        painter->drawPath(headPath);

        painter->setPen(Qt::white);
        painter->drawText(rectHeadText,Qt::AlignLeft | Qt::AlignVCenter, dayDate);

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
                int height = static_cast<int>(floor((option.rect.height()*0.825) / workoutValues.count()));
                int yPos = rectHead.bottom();

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
                QRect rectEmpty(option.rect.x(),rectHead.bottom(),option.rect.width(),option.rect.height()*0.825);
                rectDay.addRoundedRect(rectEmpty,4,4);

                painter->setPen(rectPen);
                painter->setFont(date_font);
                painter->setBrush(QBrush(gradColor));
                painter->setRenderHint(QPainter::Antialiasing);
                painter->drawPath(rectDay);
                painter->drawText(rectEmpty,Qt::AlignCenter | Qt::AlignVCenter,settings::getStringMapPointer(settings::stingMap::General)->value("breakname"));
            }
        }
        else
        {
            QPainterPath phasePath;
            temp_value = index.data(Qt::DisplayRole).toString();
            workoutValues = index.data(Qt::DisplayRole).toString().split(delimiter,QString::SkipEmptyParts);
            int valueCount = workoutValues.count();

            if(!temp_value.isEmpty())
            {
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

                QRect rectPhase(option.rect.x(),rectHead.bottom(),option.rect.width(),option.rect.height()*0.825);
                phasePath.addRoundedRect(rectPhase,4,4);
                QRect phaseName(option.rect.x()+textMargin,rectHead.bottom(), option.rect.width()-textMargin,rectPhase.height()*0.4);
                QRect phaseContent(option.rect.x()+textMargin,phaseName.bottom(), option.rect.width()-textMargin,rectPhase.height()*0.3);
                QRect phaseGoal(option.rect.x()+textMargin,phaseContent.bottom(), option.rect.width()-textMargin,rectPhase.height()*0.3);

                rectColor.setAlpha(225);
                rectGradient.setColorAt(0,rectColor);
                rectGradient.setColorAt(1,gradColor);

                painter->setPen(rectPen);
                painter->setFont(phase_font);
                painter->setBrush(rectGradient);
                painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
                painter->drawPath(phasePath);
                painter->drawText(phaseName,Qt::AlignVCenter,workoutValues.at(1));

                if(valueCount == 2)
                {
                    painter->setFont(content_font);
                    painter->drawText(phaseContent,Qt::AlignVCenter,"");
                    painter->drawText(phaseGoal,Qt::AlignVCenter,"");
                }
                else if(valueCount == 3)
                {
                    painter->setFont(content_font);
                    painter->drawText(phaseContent,Qt::AlignVCenter,workoutValues.at(2));
                    painter->drawText(phaseGoal,Qt::AlignVCenter,"");
                }
                else if(valueCount == 4)
                {
                    painter->setFont(content_font);
                    painter->drawText(phaseContent,Qt::AlignVCenter,workoutValues.at(2));
                    painter->drawText(phaseGoal,Qt::AlignVCenter | Qt::TextWordWrap,workoutValues.at(3));
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
class del_treeview : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit del_treeview(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QString lapName;
        QString breakName = "Break";
        QStringList levels = settings::get_listValues("Level");
        QString levelName = breakName;

        lapName = index.siblingAtColumn(0).data(Qt::DisplayRole).toString();

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
class del_avgweek : public QStyledItemDelegate, public calculation
{
    Q_OBJECT
public:
    explicit del_avgweek(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    QHash<QString,QString>* generalValues = settings::getStringMapPointer(settings::stingMap::General);

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont headFont,bodyFont;
        int textMargin = option.rect.width()/50;
        int iconMargin = 2;
        QStringList sportuse = settings::get_listValues("Sportuse");

        QString sport = index.data(Qt::AccessibleTextRole).toString();
        QStringList avgValues = index.data(Qt::DisplayRole).toString().split("-");

        headFont.setBold(true);
        headFont.setPixelSize(settings::get_fontValue("fontBig"));
        bodyFont.setBold(false);
        bodyFont.setPixelSize(settings::get_fontValue("fontMedium"));

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
        QRect sportHead(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height()*0.45);
        sportPath.addRoundedRect(sportHead,5,5);

        QRect sportIcon(option.rect.x(),option.rect.y(), sportHead.height(),sportHead.height());
        QRect sportTitle(sportIcon.right()+textMargin,option.rect.y(),(option.rect.width()-sportIcon.width()-textMargin)*0.75,sportHead.height());

        painter->setBrush(rectGradient);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->setPen(Qt::black);
        painter->setFont(headFont);
        painter->drawPath(sportPath);
        painter->drawPixmap(sportIcon,settings::sportIcon.value(sport));
        painter->drawText(sportTitle,Qt::AlignLeft | Qt::AlignVCenter,sport);


        QPainterPath avgPath;
        QRect avgInfo(option.rect.x(),sportHead.bottom(),option.rect.width(),option.rect.height()*0.55);
        avgPath.addRoundedRect(avgInfo,5,5);

        rectGradient.setColorAt(0,rectColor);
        rectGradient.setColorAt(1,gradColor);

        if(index.row() == 0)
        {
            rectColor = settings::get_itemColor(generalValues->value("sum")).toHsv();
            headFont.setBold(true);
        }
        else
        {
            rectColor = settings::get_itemColor(sport).toHsv();
        }

        if(!avgValues.isEmpty())
        {

         int rectHeight = avgInfo.height();
         int rectWidth = (option.rect.width()-(rectHeight*avgValues.count()))/avgValues.count();

         QRect workIcon(option.rect.x(),sportHead.bottom()+iconMargin,rectHeight-iconMargin,rectHeight-iconMargin);
         QRect workValue(workIcon.right()+textMargin,sportHead.bottom(),rectWidth,rectHeight);

         QRect duraIcon(workValue.right(),sportHead.bottom()+iconMargin,rectHeight-iconMargin,rectHeight-iconMargin);
         QRect duraValue(duraIcon.right()+textMargin,sportHead.bottom(),rectWidth,rectHeight);

         QRect distIcon(duraValue.right(),sportHead.bottom()+iconMargin,rectHeight-iconMargin,rectHeight-iconMargin);
         QRect distValue(distIcon.right()+textMargin,sportHead.bottom(),rectWidth,rectHeight);

         QRect stressIcon(distValue.right(),sportHead.bottom()+iconMargin,rectHeight-iconMargin,rectHeight-iconMargin);
         QRect stressValue(stressIcon.right()+textMargin,sportHead.bottom(),rectWidth,rectHeight);

         painter->setBrush(rectGradient);
         painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
         painter->setPen(Qt::black);
         painter->setFont(bodyFont);
         painter->drawPath(avgPath);

         painter->drawPixmap(workIcon,settings::sportIcon.value("Workouts"));
         painter->drawText(workValue,Qt::AlignLeft | Qt::AlignVCenter,avgValues.at(0));
         painter->drawPixmap(duraIcon,settings::sportIcon.value("Duration"));
         painter->drawText(duraValue,Qt::AlignLeft | Qt::AlignVCenter,avgValues.at(1));
         painter->drawPixmap(distIcon,settings::sportIcon.value("Distance"));
         painter->drawText(distValue,Qt::AlignLeft | Qt::AlignVCenter,avgValues.at(2));
         painter->drawPixmap(stressIcon,settings::sportIcon.value("TSS"));
         painter->drawText(stressValue,Qt::AlignLeft | Qt::AlignVCenter,avgValues.at(3));
        }
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
            QMap<int,QPair<int,QColor>> colorMap;
            colorMap.insert(0,get_color(index.data().toString().split(" - ").first().toInt(),dayCal,index.data(Qt::UserRole).toString()));
            colorMap.insert(1,get_color(index.data().toString().split(" - ").last().toInt(),dayCal,index.data(Qt::UserRole).toString()));

            if(colorMap.value(0).first == colorMap.value(1).first)
            {
                itemColor = colorMap.value(0).second;
            }
            else
            {
                itemColor = colorMap.value(1).second;
                gradColor = colorMap.value(0).second;
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

    QPair<int,QColor> get_color(int dataValue, int dayCal,QString mode) const
    {
        QVector<double> percent = settings::doubleVector.value(mode);
        QPair<int,QColor> colorMap;

        if(dataValue >= (dayCal*(percent.at(0)/100.0)))
        {
            colorMap.first = 0;
            colorMap.second = settings::get_itemColor("max").toHsv();
        }
        if(dataValue < (dayCal*(percent.at(0)/100.0)) && dataValue > (dayCal*(percent.at(1)/100.0)))
        {
            colorMap.first = 1;
            colorMap.second = settings::get_itemColor("high").toHsv();
        }
        if(dataValue <= (dayCal*(percent.at(1)/100.0)) && dataValue > (dayCal*(percent.at(2)/100.0)))
        {
            colorMap.first = 2;
            colorMap.second = settings::get_itemColor("low").toHsv();
        }
        if(dataValue <= (dayCal*(percent.at(2)/100.0)))
        {
            colorMap.first = 3;
            colorMap.second = settings::get_itemColor("min").toHsv();
        }

        return colorMap;
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
    Activity *currActivity;
    foodplanner *foodPlan;
    schedule_delegate schedule_del;
    saison_delegate saison_del;
    summery_delegate sum_del;
    del_filelist fileList_del;
    del_treeview tree_del;
    del_avgselect avgSelect_del;
    del_level level_del;
    del_avgweek avgweek_del;
    del_foodplan foodPlan_del;
    del_foodSummery foodSumDay_del;
    del_foodWeekSum foodSumWeek_del;
    del_foodDaySum foodSumSelect_del;
    del_mousehover mousehover_del;
    QStringList modus_list,cal_header,menuCopy;
    QStringList *schedMode, *avgHeader;
    QLabel *planerMode;
    QToolButton *planMode;
    QMap<QString,QIcon> iconMap;
    QWidget *menuSpacer;
    QButtonGroup *phaseGroup;
    QComboBox *modules;
    QDate firstdayofweek;
    QString dateFormat,longTime,shortTime;
    QHash<QString,QString> *gcValues,*generalValues;
    QLabel *statusLabel;
    QProgressBar *statusProgress;

    //Intervall Chart
    void set_polishPlot(int);
    void init_polishgraph();
    void set_speedPlot(double,double,int);
    void set_polishMinMax(double);
    void resetPlot();

    QMap<QString,QStringList> *saisonValues;

    QDate selectedDate;
    QString weeknumber,buttonStyle,dateformat,timeshort,timelong;
    int userSetup,saisonWeek,saisonWeeks,weekDays,weekCounter,weekRange,sportUse,selectedInt;
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
    void set_progress(int);

    //Editor
    bool clearActivtiy();
    void save_activity();
    QTreeWidgetItem* set_activityLaps(QPair<int,QString>,QVector<double>,int);
    void updated_changedInterval(QTreeWidgetItem*);
    void set_activityTree();
    void set_selecteditem(QTreeWidgetItem*,int);
    void refresh_activityTree();
    void reset_avgSelection();
    void activityList(int);
    void load_activity(const QString &filename,bool);
    void recalc_selectedInt(QTime,double);
    void init_controlStyleSheets();
    void set_activityInfo();
    void fill_WorkoutContent();
    void set_menuItems(int);
    void freeMem();

    //Food
    QMap<int,QStringList> selectedLine;
    void fill_foodPlanTable(QDate);
    void fill_foodSumTable(QDate);
    void fill_foodPlanList(bool,int);
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
    void on_actionExit_and_Save_triggered();
    void on_actionSelect_File_triggered();
    void on_actionReset_triggered();
    void on_actionStress_Calculator_triggered();
    void on_actionIntervall_Editor_triggered();
    void on_actionPreferences_triggered();
    void on_actionPace_Calculator_triggered();
    void on_actionVersion_triggered();
    void set_module(int);

    //Planner
    void on_calendarWidget_clicked(const QDate &date);
    void on_actionEdit_Week_triggered();
    void on_toolButton_weekCurrent_clicked();
    void on_toolButton_weekMinus_clicked();
    void on_toolButton_weekPlus_clicked();
    void on_toolButton_weekFour_clicked();
    void toolButton_planMode(bool);
    void set_phaseFilter(int);
    void refresh_schedule();
    void refresh_saison();
    void selectionChanged_stressPlot();

    //Editor
    void on_horizontalSlider_factor_valueChanged(int value);
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
    void on_treeWidget_activityfiles_itemClicked(QTreeWidgetItem *item, int column);
    void on_treeWidget_activity_itemClicked(QTreeWidgetItem *item, int column);
    void on_timeEdit_intDuration_userTimeChanged(const QTime &time);
    void on_doubleSpinBox_intDistance_valueChanged(double arg1);
    void on_toolButton_split_clicked();
    void on_toolButton_merge_clicked();
    void on_actionExpand_Activity_Tree_triggered();
    void on_pushButton_stressPlot_clicked(bool checked);
    void on_pushButton_currentWeek_clicked(bool checked);
    void on_spinBox_extWeeks_valueChanged(int arg1);
    void on_toolButton_extReset_clicked();
    void on_comboBox_saisonSport_currentIndexChanged(const QString &arg1);
};

#endif // MAINWINDOW_H
