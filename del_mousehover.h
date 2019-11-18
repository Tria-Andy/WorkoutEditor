#ifndef DEL_MOUSEHOVER_H
#define DEL_MOUSEHOVER_H

#include <QtGui>
#include <QStyledItemDelegate>
#include <QDebug>

class del_mousehover : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit del_mousehover(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();

        QLinearGradient itemGradient(option.rect.topLeft(),option.rect.bottomLeft());
        itemGradient.setSpread(QGradient::ReflectSpread);
        QColor itemColor,gradColor;

        gradColor.setHsv(0,0,200,150);

        if(option.state & QStyle::State_Selected)
        {
            itemColor.setHsv(240,255,255,180);
            painter->setPen(Qt::white);
        }
        else if(option.state & QStyle::State_MouseOver)
        {
            itemColor.setHsv(240,180,255,180);
            painter->setPen(Qt::white);
        }
        else
        {
            itemColor.setHsv(0,0,255,200);
            itemColor.setAlpha(180);
            painter->setPen(Qt::black);
        }

        itemGradient.setColorAt(0,gradColor);
        itemGradient.setColorAt(1,itemColor);
        painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
        painter->fillRect(option.rect,itemGradient);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));

        painter->restore();
    }
};

#endif // DEL_MOUSEHOVER_H
