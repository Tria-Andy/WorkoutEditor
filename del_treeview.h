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
            rowColor = settings::get_itemColor(levelName).toHsv();
            rowColor.setAlpha(100);

            if(lapName == breakName)
            {
                painter->setPen(Qt::white);
            }
            else
            {
                painter->setPen(Qt::black);
            }
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

#endif // DEL_TREEVIEW_H
