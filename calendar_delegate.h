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
        QString cRed,cGreen,cBlue;
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

                    for(int pos = 0; pos < settings::get_sportList().count();++pos)
                    {
                        if(workout.contains(settings::get_sportList().at(pos)))
                        {
                            QString sColor = settings::get_sportColor().at(pos);
                            cRed = sColor.split("-").at(0);
                            cGreen = sColor.split("-").at(1);
                            cBlue = sColor.split("-").at(2);
                            rect_color.setRgb(cRed.toInt(),cGreen.toInt(),cBlue.toInt());
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
                for(int pos = 0; pos < settings::get_phaseList().count();++pos)
                {
                    if(phase.contains(settings::get_phaseList().at(pos)))
                    {
                        QString sColor = settings::get_phaseColor().at(pos);
                        cRed = sColor.split("-").at(0);
                        cGreen = sColor.split("-").at(1);
                        cBlue = sColor.split("-").at(2);
                        rect_color.setRgb(cRed.toInt(),cGreen.toInt(),cBlue.toInt());
                        break;
                    }
                    else
                    {
                        QString sColor = settings::get_emptyPhaseColor();
                        cRed = sColor.split("-").at(0);
                        cGreen = sColor.split("-").at(1);
                        cBlue = sColor.split("-").at(2);
                        rect_color.setRgb(cRed.toInt(),cGreen.toInt(),cBlue.toInt());
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

