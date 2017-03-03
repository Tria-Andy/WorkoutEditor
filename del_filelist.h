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

        QLinearGradient rowGradient(option.rect.topLeft(),option.rect.bottomLeft());
        rowGradient.setSpread(QGradient::RepeatSpread);

        QColor rowColor,gradColor;
        gradColor.setHsv(0,0,200,150);

        if(option.state & (QStyle::State_Selected | QStyle::State_MouseOver))
        {
            rowColor.setHsv(240,255,255,180);
            painter->setPen(Qt::white);
        }
        else
        {
            rowColor = settings::get_itemColor(sportName).toHsv();
            rowColor.setAlpha(125);
            painter->setPen(Qt::black);
        }

        rowGradient.setColorAt(0,gradColor);
        rowGradient.setColorAt(1,rowColor);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,rowGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }
};
#endif // DEL_FILELIST_H
