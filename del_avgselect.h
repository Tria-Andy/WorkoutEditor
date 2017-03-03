#ifndef DEL_AVGSELECT_H
#define DEL_AVGSELECT_H

#include <QtGui>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QDebug>
#include "settings.h"


class del_avgselect : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit del_avgselect(QObject *parent = 0) : QStyledItemDelegate(parent) {}
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

        rectColor = settings::get_itemColor(sport).toHsv();
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

#endif // DEL_AVGSELECT_H
