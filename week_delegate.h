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
        QVector<int> fontsize = settings::get_fontSize();
        QFont phase_font,date_font, work_font;
        QString temp_value;
        QString headInfo;
        QStringList calendar_values;
        QString delimiter = "-";
        QColor rect_color;
        QString cRed,cGreen,cBlue;
        int textMargin = 2;
        QString phase;
        phase_font.setBold(true);
        phase_font.setPixelSize(fontsize[0]);
        date_font.setBold(true);
        date_font.setPixelSize(fontsize[1]);
        work_font.setBold(false);
        work_font.setPixelSize(fontsize[2]);

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

            for(int pos = 0; pos < settings::get_phaseList().count();++pos)
            {
                if(phase.contains(settings::get_phaseList().at(pos)))
                {
                    rect_color = settings::get_color(settings::get_phaseColor().at(pos));
                    break;
                }
                else
                {
                    rect_color = settings::get_color(settings::get_emptyPhaseColor());
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
            for(int pos = 0; pos < settings::get_sportList().count();++pos)
            {
                if(index.column() == pos+1)
                {
                    if(index.column() != 6)
                    {
                        rect_color = settings::get_color(settings::get_sportColor().at(pos));
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

