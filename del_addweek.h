#ifndef DEL_ADDWEEK_H
#define DEL_ADDWEEK_H
#include <QtGui>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTimeEdit>
#include <QDebug>
#include "settings.h"

class del_addweek : public QStyledItemDelegate
{
    Q_OBJECT

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)

        if(index.column() == 1 || index.column() == 6)
        {
            QSpinBox *editor = new QSpinBox(parent);
            editor->setFrame(false);
            editor->setMinimum(0);
            editor->setMaximum(1000);

            return editor;
        }
        if(index.column() == 4)
        {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(false);
            editor->setDecimals(2);
            editor->setMinimum(0.0);
            editor->setMaximum(500.0);

            return editor;
        }
        if(index.column() == 2)
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("HH:mm");
            editor->setFrame(false);

            return editor;
        }

        return 0;
    }

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont cFont;
        QString sportname,indexData;
        QStringList sportuse = settings::get_sportUseList();
        const QAbstractItemModel *model = index.model();
        cFont.setPixelSize(12);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());
        sportname = model->data(model->index(index.row(),0,QModelIndex())).toString().trimmed();
        indexData = index.data().toString();
        painter->setPen(Qt::black);

        for(int i = 0; i < sportuse.count(); ++i)
        {
            if(sportname == sportuse.at(i))
            {
                painter->fillRect(option.rect,QBrush(settings::get_itemColor(sportname)));
            }
        }

        painter->setFont(cFont);
        painter->drawText(rect_text,indexData,QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }




};
#endif // DEL_ADDWEEK_H
