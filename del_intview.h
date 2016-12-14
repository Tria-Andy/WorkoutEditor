#ifndef DEL_INTVIEW_H
#define DEL_INTVIEW_H
#include <QtGui>
#include <QItemDelegate>
#include <QLabel>
#include <QDebug>
#include "settings.h"

class del_intview : public QItemDelegate
{
    Q_OBJECT

public:
    explicit del_intview(QObject *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont cFont;
        QString lapName,indexData;
        const QAbstractItemModel *model = index.model();
        cFont.setPixelSize(12);
        QColor lapColor(Qt::lightGray);
        QColor breakColor(Qt::darkGray);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());
        lapName = model->data(model->index(index.row(),0,QModelIndex())).toString().trimmed();
        indexData = index.data().toString();
        painter->setPen(Qt::black);

        if(lapName == settings::get_breakName())
        {
            painter->fillRect(option.rect,QBrush(breakColor));
        }
        else
        {

            painter->fillRect(option.rect,QBrush(lapColor));
        }

        painter->setFont(cFont);
        painter->drawText(rect_text,indexData,QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

};


#endif // DEL_INTVIEW_H
