#ifndef DEL_AVGSELECT_H
#define DEL_AVGSELECT_H

#include <QtGui>
#include <QStyledItemDelegate>
#include <QLabel>
#include <QDebug>
#include "settings.h"


class del_avgselect : public QStyledItemDelegate
{
    Q_OBJECT

private:
    QColor get_backcolor(bool isSel,QString name) const
    {
        QColor setColor;
        QString breakName = settings::get_generalValue("breakname");
        if(isSel)
        {
            setColor.setRgb(0,255,0,100);
        }
        else
        {
            if(name == breakName)
            {
                setColor = settings::get_itemColor(breakName);
                setColor.setAlpha(125);
            }
            else
            {
                setColor.setRgb(175,175,175,75);
            }
        }
        return setColor;
    }

    bool get_isSelected(const QModelIndex &index) const
    {
        return index.model()->data(index.model()->index(index.row(),index.model()->columnCount()-1,QModelIndex()),(Qt::UserRole+1)).toBool();
    }

    QString get_lapName(const QModelIndex &index) const
    {
        return index.model()->data(index.model()->index(index.row(),0,QModelIndex())).toString().trimmed();
    }

public:
    explicit del_avgselect(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        drawBackground(painter, option, index);
        QStyledItemDelegate::paint(painter, option, index);
    }

protected:
    virtual void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        painter->save();
        bool isSelected = del_avgselect::get_isSelected(index);
        QString lapName = del_avgselect::get_lapName(index);
        QLinearGradient setGradient(option.rect.topLeft(),option.rect.bottomRight());
        setGradient.setColorAt(0,Qt::white);
        setGradient.setColorAt(1,get_backcolor(isSelected,lapName));

        //QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());
        //painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->fillRect(option.rect,setGradient);
        painter->restore();
    }
};

#endif // DEL_AVGSELECT_H
