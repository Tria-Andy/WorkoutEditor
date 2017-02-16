#ifndef DEL_TREEVIEW_H
#define DEL_TREEVIEW_H

#include <QtGui>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QDebug>
#include "settings.h"

class del_treeview : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit del_treeview(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    QString sport;

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();

        const QAbstractItemModel *model = index.model();
        QString lapName;
        QString breakName = settings::get_generalValue("breakname");
        QStringList levels = settings::get_listValues("Level");
        QString levelName = breakName;


        if(index.parent().isValid())
        {
            lapName = index.parent().child(index.row(),0).data().toString();
        }
        else
        {
            lapName = model->data(model->index(index.row(),0,QModelIndex())).toString();;
        }

        for(int i = 0; i < levels.count(); ++i)
        {
            if(lapName.contains(levels.at(i))) levelName = levels.at(i);
        }

        QLinearGradient setGradient(option.rect.topLeft(),option.rect.bottomLeft());
        setGradient.setColorAt(0,QColor(255,255,255,100));
        setGradient.setSpread(QGradient::RepeatSpread);

        QColor setColor;

        if(option.state & QStyle::State_Selected)
        {
            setColor.setRgb(0,0,255,100);
            setGradient.setColorAt(1,setColor);
            painter->fillRect(option.rect,setGradient);
            painter->setPen(Qt::white);
        }
        else
        {
            setColor = settings::get_itemColor(levelName);
            setColor.setAlpha(75);

            if(lapName == breakName)
            {
                painter->setPen(Qt::white);
            }
            else
            {
                painter->setPen(Qt::black);
            }
        }

        painter->fillRect(option.rect,QBrush(setColor));


        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        //painter->setFont(cFont);
        painter->restore();
    }

};

#endif // DEL_TREEVIEW_H
