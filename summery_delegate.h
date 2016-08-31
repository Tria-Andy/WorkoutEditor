#ifndef SUMMERY_DELEGATE
#define SUMMERY_DELEGATE

#include <QtGui>
#include <QItemDelegate>
#include <QTableView>
#include "settings.h"

class summery_delegate : public QItemDelegate
{
    Q_OBJECT

public:
    summery_delegate(QTableView *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont phase_font,date_font, work_font;
        QString temp_value;
        QStringList sum_values;
        QString delimiter = "-";
        QColor rect_color;
        QString cRed,cGreen,cBlue;
        int textMargin = 2;
        phase_font.setBold(true);
        phase_font.setPixelSize(16);
        date_font.setBold(true);
        date_font.setPixelSize(14);
        work_font.setBold(false);
        work_font.setPixelSize(12);

        temp_value = index.data(Qt::DisplayRole).toString();
        sum_values = temp_value.split(delimiter);
        /*
        if(sum_values.at(0) == "Summery") rect_color.setRgb(0,255,255);
        if(sum_values.at(0) == "Swim") rect_color.setRgb(0,154,231);
        if(sum_values.at(0) == "Bike") rect_color.setRgb(255,170,0);
        if(sum_values.at(0) == "Run") rect_color.setRgb(0,170,0);
        if(sum_values.at(0) == "Strength") rect_color.setRgb(192,192,192);
        if(sum_values.at(0) == "Tria") rect_color.setRgb(255,0,0);
        if(sum_values.at(0) == "Alternativ") rect_color.setRgb(255,255,0);
        if(sum_values.at(0) == "Other") rect_color.setRgb(170,255,255);
        */
        for(int i = 0; i < settings().get_sportList().count(); ++i)
        {
            if(sum_values.at(0) == settings().get_sportList().at(i))
            {
                QString sColor = settings().get_sportColor().at(i);
                cRed = sColor.split("-").at(0);
                cGreen = sColor.split("-").at(1);
                cBlue = sColor.split("-").at(2);
                rect_color.setRgb(cRed.toInt(),cGreen.toInt(),cBlue.toInt());
                break;
            }
            else
            {
                //Summery
                rect_color.setRgb(0,255,255);
            }
        }

        QRect rect_head(option.rect.x(),option.rect.y(),option.rect.width(),20);
        QRect rect_head_text(option.rect.x()+textMargin,option.rect.y(),option.rect.width(),20);
        painter->setBrush(QBrush(rect_color));
        painter->fillRect(rect_head,QBrush(rect_color));
        painter->fillRect(rect_head_text,QBrush(rect_color));
        painter->drawRect(rect_head);
        QTextOption headoption(Qt::AlignLeft);
        painter->setPen(Qt::black);
        painter->setFont(date_font);
        painter->drawText(rect_head_text,sum_values.at(0),headoption);

        QString labels;
        labels = "Workouts:\n";
        labels = labels + "Duration(Hours):\n";
        labels = labels + "Amount(%):\n";
        labels = labels + "Distance(Km):\n";
        labels = labels + "Stress(TSS):";

        QRect rect_label(option.rect.x(),option.rect.y()+21,option.rect.width()/2,option.rect.height()-21);
        QRect rect_label_text(option.rect.x()+textMargin,option.rect.y()+21,option.rect.width()/2,option.rect.height()-21);
        painter->setBrush(QBrush(rect_color));
        painter->fillRect(rect_label,QBrush(rect_color));
        painter->fillRect(rect_label_text,QBrush(rect_color));
        QTextOption labeloption(Qt::AlignLeft);
        painter->setPen(Qt::black);
        painter->setFont(work_font);
        painter->drawText(rect_label_text,labels,labeloption);

        if(!sum_values.isEmpty())
        {

            QString partValue;
            partValue = sum_values.at(1) + "\n";
            partValue = partValue + sum_values.at(2) + "\n";
            partValue = partValue + sum_values.at(3) + "\n";
            partValue = partValue + sum_values.at(4) + "\n";
            partValue = partValue + sum_values.at(5) + "\n";

            QRect rect_work(option.rect.x()+(option.rect.width()/2) ,option.rect.y()+21,(option.rect.width()/2)+1,option.rect.height()-21);
            painter->setBrush(QBrush(rect_color));
            painter->fillRect(rect_work,QBrush(rect_color));
            QTextOption workoption(Qt::AlignLeft);
            painter->setPen(Qt::black);
            painter->setFont(work_font);
            painter->drawText(rect_work,partValue,workoption);
        }
        painter->restore();
    }
};

#endif // SUMMERY_DELEGATE

