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
        QStringList calendar_values;
        QString delimiter = "#";
        QColor rect_color;
        int textMargin = 2;
        int celloffset = 21;
        phase_font.setBold(true);
        phase_font.setPixelSize(16);
        date_font.setBold(true);
        date_font.setPixelSize(14);
        work_font.setBold(false);
        work_font.setPixelSize(11);

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

                    if(workout.contains(settings().isSwim,Qt::CaseSensitive)) rect_color.setRgb(0,154,231);
                    if(workout.contains(settings().isBike,Qt::CaseSensitive)) rect_color.setRgb(255,170,0);
                    if(workout.contains(settings().isRun,Qt::CaseSensitive)) rect_color.setRgb(0,170,0);
                    if(workout.contains(settings().isStrength,Qt::CaseSensitive)) rect_color.setRgb(192,192,192);
                    if(workout.contains(settings().isAlt,Qt::CaseSensitive)) rect_color.setRgb(255,255,0);
                    if(workout.contains(settings().isTria,Qt::CaseSensitive)) rect_color.setRgb(255,0,0);
                    if(workout.contains(settings().isOther,Qt::CaseSensitive)) rect_color.setRgb(170,255,255);

                    /*
                    for(int pos = 0; pos < settings().get_sportList().count();++pos)
                    {
                        if(workout.contains(settings().get_sportList().at(pos)))
                        {
                            qDebug() << workout << settings().get_sportList().at(pos);
                        }
                    }
                    */
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
                if(phase.contains(settings().get_phaseList().at(0))) rect_color.setRgb(0,185,255);
                if(phase.contains(settings().get_phaseList().at(1))) rect_color.setRgb(60,235,60);
                if(phase.contains(settings().get_phaseList().at(2))) rect_color.setRgb(230,255,0);
                if(phase.contains(settings().get_phaseList().at(3))) rect_color.setRgb(255,150,100);
                if(phase.contains(settings().get_phaseList().at(4))) rect_color.setRgb(255,85,0);
                if(phase.contains(settings().get_phaseList().at(5))) rect_color.setRgb(255,50,0);
                if(phase.contains("Phase")) rect_color.setRgb(125,125,200);

                QRect rect_phase(option.rect.x(),option.rect.y()+celloffset, option.rect.width(),option.rect.height()-celloffset);
                QRect rect_phase_text(option.rect.x()+ textMargin,option.rect.y()+celloffset, option.rect.width(),option.rect.height()-celloffset);
                painter->fillRect(rect_phase,QBrush(rect_color));
                painter->fillRect(rect_phase_text,QBrush(rect_color));

                QTextOption phaseOption(Qt::AlignVCenter);
                painter->setPen(Qt::white);
                painter->setFont(phase_font);
                painter->drawText(rect_phase_text,phase,phaseOption);
             }
        }

        painter->restore();
    }

};

#endif // CALENDAR_DELEGATE

