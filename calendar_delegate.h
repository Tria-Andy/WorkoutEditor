#ifndef CALENDAR_DELEGATE
#define CALENDAR_DELEGATE
#include <QtGui>
#include <QItemDelegate>
#include <QTableView>
#include <QDebug>
#include <QDate>

class calendar_delegate : public QItemDelegate
{
    Q_OBJECT

public:
    calendar_delegate(QTableView *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont phase_font,date_font, work_font;
        QString temp_value;
        QStringList calendar_values;
        QString delimiter = "#";
        QColor rect_color;
        phase_font.setBold(true);
        phase_font.setPixelSize(16);
        date_font.setBold(true);
        date_font.setPixelSize(14);
        work_font.setBold(false);
        work_font.setPixelSize(11);

        temp_value = index.data(Qt::DisplayRole).toString();
        calendar_values = temp_value.split(delimiter);

        QRect rect_head(option.rect.x(),option.rect.y(), option.rect.width(),20);
        painter->fillRect(rect_head,QBrush(QColor(128,128,128)));
        QTextOption dateOption(Qt::AlignLeft);

        if(QDate::fromString(calendar_values.at(0),"dd MMM yy").addYears(100) ==(QDate::currentDate()))
        {
            painter->fillRect(rect_head,QBrush(QColor(200,0,0)));
        }
        else
        {
            painter->fillRect(rect_head,QBrush(QColor(128,128,128)));
        }

        painter->setPen(Qt::white);
        painter->setFont(date_font);
        painter->drawText(rect_head,calendar_values.at(0),dateOption);

        if(index.column() != 0)
        {
            temp_value = index.data(Qt::DisplayRole).toString();
            calendar_values = temp_value.split(delimiter,QString::SkipEmptyParts);
            calendar_values.removeFirst();

            if(!calendar_values.isEmpty())
            {
                int height = (option.rect.height()- 21) / calendar_values.count();
                int y = option.rect.y()+ 21;

                for(int i = 0; i < calendar_values.count(); ++i)
                {
                    QString workout = calendar_values.at(i);

                    QRect rect_work(option.rect.x(),y,option.rect.width(),height);
                    y += height+1;

                    if(workout.contains("Swim",Qt::CaseSensitive)) painter->fillRect(rect_work,QBrush(QColor(0,154,231)));
                    if(workout.contains("Bike",Qt::CaseSensitive)) painter->fillRect(rect_work,QBrush(QColor(255,170,0)));
                    if(workout.contains("Run",Qt::CaseSensitive)) painter->fillRect(rect_work,QBrush(QColor(0,170,0)));
                    if(workout.contains("Strength",Qt::CaseSensitive)) painter->fillRect(rect_work,QBrush(QColor(192,192,192)));
                    if(workout.contains("Alternativ",Qt::CaseSensitive)) painter->fillRect(rect_work,QBrush(QColor(255,255,0)));
                    if(workout.contains("Tria",Qt::CaseSensitive)) painter->fillRect(rect_work,QBrush(QColor(255,0,0)));
                    if(workout.contains("Other",Qt::CaseSensitive)) painter->fillRect(rect_work,QBrush(QColor(170,255,255)));

                    QTextOption workoption(Qt::AlignLeft);
                    painter->setPen(Qt::black);
                    painter->setFont(work_font);
                    painter->drawText(rect_work,workout,workoption);
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
                if(phase.contains("OFF")) rect_color.setRgb(0,185,255);
                if(phase.contains("PREP")) rect_color.setRgb(60,235,60);
                if(phase.contains("BASE")) rect_color.setRgb(230,255,0);
                if(phase.contains("BUILD")) rect_color.setRgb(255,150,100);
                if(phase.contains("PEAK")) rect_color.setRgb(255,85,0);
                if(phase.contains("WK")) rect_color.setRgb(255,50,0);
                if(phase.contains("Phase")) rect_color.setRgb(125,125,200);

                QRect rect_phase(option.rect.x(),option.rect.y()+21, option.rect.width(),option.rect.height()-21);
                painter->fillRect(rect_phase,QBrush(rect_color));

                QTextOption phaseOption(Qt::AlignVCenter);
                painter->setPen(Qt::white);
                painter->setFont(phase_font);
                painter->drawText(rect_phase,phase,phaseOption);
             }
        }

        painter->restore();
    }

};

#endif // CALENDAR_DELEGATE

