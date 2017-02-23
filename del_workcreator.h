#ifndef DEL_WORKCREATOR_H
#define DEL_WORKCREATOR_H

#include <QtGui>
#include <QStyledItemDelegate>
#include <QDebug>
#include "settings.h"

class del_workcreator : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit del_workcreator(QObject *parent = 0) : QStyledItemDelegate(parent) {}
    QStringList groupList;


    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        const QAbstractItemModel *model = index.model();

        QString levelName;
        if(index.parent().isValid())
        {
            levelName = index.parent().child(index.row(),1).data().toString();
        }
        else
        {
            levelName = model->data(model->index(index.row(),1,QModelIndex())).toString();;
        }

        QLinearGradient itemGradient(option.rect.topLeft(),option.rect.bottomLeft());
        itemGradient.setSpread(QGradient::ReflectSpread);
        QColor itemColor,gradColor;

        gradColor.setHsv(0,0,200,150);

        if(option.state & (QStyle::State_Selected | QStyle::State_MouseOver))
        {
            itemColor.setHsv(240,255,255,180);
            painter->setPen(Qt::white);
        }
        else
        {
            itemColor = settings::get_itemColor(levelName).toHsv();
            itemColor.setAlpha(180);
            if(levelName.isEmpty()) itemColor.setHsv(0,0,255,200);
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

#endif // DEL_WORKCREATOR_H
