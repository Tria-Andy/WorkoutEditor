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

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        qDebug() << index.row();
        QLabel *editor = new QLabel(parent);
        editor->setText(index.data(Qt::DisplayRole).toString());

        return editor;
    }

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont cFont;
        QString lapName;
        const QAbstractItemModel *model = index.model();
        cFont.setPixelSize(12);
        QColor lapColor,breakColor;
        lapColor.setRgb(192,192,192);
        breakColor.setRgb(128,128,128);
        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());

        lapName = model->data(model->index(index.row(),0,QModelIndex())).toString();
        if(lapName.contains(settings::get_breakName()))
        {
            painter->setPen(Qt::white);
            painter->fillRect(option.rect,QBrush(breakColor));
            painter->fillRect(rect_text,QBrush(breakColor));
        }
        else
        {
            painter->setPen(Qt::black);
            painter->fillRect(option.rect,QBrush(lapColor));
            painter->fillRect(rect_text,QBrush(lapColor));
        }

        painter->setFont(cFont);
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

};


#endif // DEL_INTVIEW_H
