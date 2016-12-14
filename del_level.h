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

#ifndef DEL_LEVEL_H
#define DEL_LEVEL_H
#include <QtGui>
#include <QItemDelegate>
#include <QLabel>
#include <QSpinBox>
#include <QDebug>
#include "settings.h"

class del_level : public QItemDelegate
{
    Q_OBJECT

public:
    explicit del_level(QObject *parent = 0) : QItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont cFont;
        QString levelName,indexData;
        QStringList levels = settings::get_levelList();
        const QAbstractItemModel *model = index.model();
        cFont.setPixelSize(12);

        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());
        levelName = model->data(model->index(index.row(),0,QModelIndex())).toString().trimmed();
        indexData = index.data().toString();
        painter->setPen(Qt::black);

        for(int i = 0; i < levels.count(); ++i)
        {
            if(levelName == levels.at(i))
            {
                painter->fillRect(option.rect,QBrush(settings::get_itemColor(levelName)));
            }
        }

        painter->setFont(cFont);
        painter->drawText(rect_text,indexData,QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)

        if(index.column() == 1)
        {
            QFont eFont;
            eFont.setPixelSize(12);

            QSpinBox *editor = new QSpinBox(parent);
            editor->setFont(eFont);
            editor->setFrame(false);
            editor->setMinimum(0);
            editor->setMaximum(200);

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
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        int value = spinBox->value();

        model->setData(index, value, Qt::EditRole);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

};
#endif // DEL_LEVEL_H
