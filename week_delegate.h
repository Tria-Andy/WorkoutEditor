#ifndef WEEK_DELEGATE
#define WEEK_DELEGATE
#include <QtGui>
#include <QItemDelegate>
#include <QTableView>
#include <QDebug>
#include <QDate>

class week_delegate : public QItemDelegate
{
    Q_OBJECT

public:
    week_delegate(QTableView *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont phase_font,date_font, work_font;
        QString temp_value;
        QString headInfo;
        QStringList calendar_values;
        QString delimiter = "-";
        QColor rect_color;
        QString phase;
        phase_font.setBold(true);
        phase_font.setPixelSize(16);
        date_font.setBold(true);
        date_font.setPixelSize(14);
        work_font.setBold(false);
        work_font.setPixelSize(12);

        temp_value = index.data(Qt::DisplayRole).toString();
        calendar_values = temp_value.split(delimiter);

        if(index.column() == 0)
        {
            headInfo = calendar_values.at(0) + " - " + calendar_values.at(1) + " - " + calendar_values.at(2);
            QRect rect_head(option.rect.x(),option.rect.y(), option.rect.width(),20);
            painter->fillRect(rect_head,QBrush(QColor(128,128,128)));
            QTextOption dateOption(Qt::AlignLeft);
            painter->setPen(Qt::white);
            painter->setFont(date_font);
            painter->drawText(rect_head,headInfo,dateOption);
            phase = calendar_values.at(3);
            if(phase.contains("OFF")) rect_color.setRgb(0,170,255);
            if(phase.contains("PREP")) rect_color.setRgb(0,170,0);
            if(phase.contains("BASE")) rect_color.setRgb(240,240,0);
            if(phase.contains("BUILD")) rect_color.setRgb(255,150,0);
            if(phase.contains("PEAK")) rect_color.setRgb(255,85,0);
            if(phase.contains("WK")) rect_color.setRgb(255,50,0);

            QRect rect_phase(option.rect.x(),option.rect.y()+21, option.rect.width(),option.rect.height()-21);
            painter->fillRect(rect_phase,QBrush(rect_color));
            QTextOption phaseOption(Qt::AlignVCenter);
            painter->setPen(Qt::black);
            painter->setFont(phase_font);
            painter->drawText(rect_phase,phase,phaseOption);
        }
        else
        {
            if(index.column() == 1) rect_color.setRgb(0,154,231);
            if(index.column() == 2) rect_color.setRgb(255,170,0);
            if(index.column() == 3) rect_color.setRgb(0,170,0);
            if(index.column() == 4) rect_color.setRgb(192,192,192);
            if(index.column() == 5) rect_color.setRgb(255,255,0);
            if(index.column() == 6) rect_color.setRgb(0,255,255);

            QString labels;
            labels = "Workouts:\n";
            labels = labels + "Distance:\n";
            labels = labels + "Duration:\n";
            labels = labels + "Stress:";

            QRect rect_label(option.rect.x(),option.rect.y(),option.rect.width()/2,option.rect.height());
            painter->fillRect(rect_label,QBrush(rect_color));
            QTextOption labeloption(Qt::AlignLeft);
            painter->setPen(Qt::black);
            painter->setFont(work_font);
            painter->drawText(rect_label,labels,labeloption);

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

