/*
 * Copyright (c) 2016 Andreas Hunner (andy-atech@gmx.net)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef DEL_INTSELECT_H
#define DEL_INTSELECT_H
#include <QtGui>
#include <QItemDelegate>
#include <QLabel>
#include <QDebug>
#include "settings.h"


class del_intselect : public QItemDelegate
{
    Q_OBJECT

private:
    QColor get_backcolor(bool isSel,QString name) const
    {
        if(isSel)
        {
            return QColor(Qt::green);
        }
        else
        {
            if(name == settings::get_gcInfo("breakName"))
            {
                return QColor(Qt::darkGray);
            }
            else
            {
                return QColor(Qt::lightGray);
            }
        }
    }

    bool get_isSelected(const QModelIndex &index) const
    {
        return index.model()->data(index.model()->index(index.row(),0,QModelIndex()),(Qt::UserRole+1)).toBool();
    }

    QString get_lapName(const QModelIndex &index) const
    {
        return index.model()->data(index.model()->index(index.row(),0,QModelIndex())).toString().trimmed();
    }

public:
    explicit del_intselect(QObject *parent = 0) : QItemDelegate(parent) {}

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        drawBackground(painter, option, index);
        QItemDelegate::paint(painter, option, index);
    }

    protected:
    virtual void drawBackground(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        painter->save();
        bool isSelected = del_intselect::get_isSelected(index);
        QString lapName = del_intselect::get_lapName(index);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->fillRect(option.rect,del_intselect::get_backcolor(isSelected,lapName));
        painter->restore();
    }
};

#endif // DEL_INTSELECT_H
