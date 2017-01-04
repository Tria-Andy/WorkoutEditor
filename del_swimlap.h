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

#ifndef DEL_SWIMLAP_H
#define DEL_SWIMLAP_H
#include <QtGui>
#include <QItemDelegate>
#include <QSpinBox>
#include <QDebug>
#include "settings.h"
#include "calculation.h"

class del_swimlap : public QItemDelegate, public calculation
{
    Q_OBJECT

public:
    explicit del_swimlap(QObject *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont cFont;
        QString lapName;
        const QAbstractItemModel *model = index.model();
        cFont.setPixelSize(settings::get_fontValue("fontSmall"));
        QColor lapColor(Qt::lightGray);
        QColor breakColor = settings::get_itemColor(settings::get_generalValue("breakname"));

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());

        lapName = model->data(model->index(index.row(),0,QModelIndex())).toString();

        if(lapName == settings::get_generalValue("breakname"))
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

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        Q_UNUSED(index)

        const QAbstractItemModel *model = index.model();
        QString lapName = model->data(model->index(index.row(),0,QModelIndex())).toString();

        if(index.column() == 2 && lapName != settings::get_generalValue("breakname"))
        {
            QFont eFont;
            eFont.setPixelSize(12);

            QSpinBox *editor = new QSpinBox(parent);
            editor->setFont(eFont);
            editor->setFrame(false);
            editor->setMinimum(0);
            editor->setMaximum(10800);

            return editor;
        }
        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        int value = index.model()->data(index, Qt::EditRole).toInt();

        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->setValue(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QModelIndex new_index,curr_index;
        QModelIndex speed_index = model->index(index.row(),4,QModelIndex());
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        QString currLap;
        int lapCount,lapTime,startTime;
        lapCount = model->rowCount()-1;
        int value = spinBox->value();
        double lapSpeed = get_speed(QTime::fromString(set_time(value),"mm:ss"),settings::get_swimLaplen(),settings::isSwim,false).toDouble();

        model->setData(index, value, Qt::EditRole);
        model->setData(speed_index,lapSpeed);

        int laprow = index.row();

        if(laprow != lapCount)
        {
            do
            {
                curr_index = model->index(laprow,1,QModelIndex());
                startTime = model->data(model->index(laprow,1,QModelIndex())).toInt();
                lapTime = model->data(model->index(laprow,2,QModelIndex())).toInt();

                new_index = model->index(laprow+1,1,QModelIndex());
                model->setData(new_index,startTime+lapTime,Qt::EditRole);
                ++laprow;
                currLap = model->data(model->index(laprow,0,QModelIndex())).toString();
            } while ((currLap != settings::get_generalValue("breakname")) && (laprow < lapCount-1));

            startTime = model->data(new_index).toInt();
            if(laprow == lapCount)
            {
                lapTime = model->data(model->index(laprow,2,QModelIndex())).toInt();
                model->setData(model->index(laprow,2,QModelIndex()),lapTime);

            }
            else
            {
                lapTime = model->data(model->index(laprow+1,1,QModelIndex())).toInt() - startTime;
                model->setData(model->index(laprow,2,QModelIndex()),lapTime);
            }
        }
        else
        {
            lapTime = model->data(model->index(laprow,2,QModelIndex())).toInt();
            model->setData(model->index(laprow,2,QModelIndex()),lapTime);
        }
    }


    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }
};

#endif // DEL_SWIMLAP_H
