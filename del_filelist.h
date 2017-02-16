#ifndef DEL_FILELIST_H
#define DEL_FILELIST_H


#include <QtGui>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QDebug>
#include "settings.h"


class del_filelist : public QStyledItemDelegate
{
    Q_OBJECT

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        const QAbstractItemModel *model = index.model();
        QString sportName = model->data(model->index(index.row(),2)).toString();

        QColor setColor;

        QLinearGradient setGradient(option.rect.topLeft(),option.rect.bottomLeft());
        setGradient.setColorAt(0,QColor(255,255,255,100));
        setGradient.setSpread(QGradient::RepeatSpread);

        if(option.state & QStyle::State_Selected)
        {
            setColor.setRgb(0,0,255,100);
            setGradient.setColorAt(1,setColor);
            painter->fillRect(option.rect,setGradient);
            painter->setPen(Qt::white);
        }
        else
        {
            setColor = settings::get_itemColor(sportName);
            setColor.setAlpha(100);
        }

        painter->fillRect(option.rect,QBrush(setColor));

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }
};
#endif // DEL_FILELIST_H
