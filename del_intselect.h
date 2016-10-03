#ifndef DEL_INTSELECT_H
#define DEL_INTSELECT_H
#include <QtGui>
#include <QItemDelegate>
#include <QLabel>
#include <QDebug>
#include "settings.h"


class del_intselect : public QItemDelegate
{
    Q_OBJECT

public:
    explicit del_intselect(QObject *parent = 0) : QItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        drawBackground(painter, option, index);
        QItemDelegate::paint(painter, option, index);
    }

    protected:
    virtual void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        int checkValue;
        QColor selectColor;

        checkValue = index.model()->data(index.model()->index(index.row(),0,QModelIndex())).toInt();
        qDebug() << checkValue;
        if(checkValue == 0)
        {
            qDebug() << checkValue << index.column() << "Green";
            selectColor.setNamedColor(QColor(Qt::green).name());

        }
        else
        {
            qDebug() << checkValue << index.column() << "White";
            selectColor.setNamedColor(QColor(Qt::white).name());
        }
        painter->fillRect(option.rect,selectColor);
    }


};

#endif // DEL_INTSELECT_H
