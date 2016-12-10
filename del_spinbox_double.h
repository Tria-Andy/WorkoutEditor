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

#ifndef DEL_SPINBOX_DOUBLE
#define DEL_SPINBOX_DOUBLE
#include <QtGui>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QDebug>

class del_spinbox_double : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit del_spinbox_double(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        if(index.column() == 1)
        {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(false);
            editor->setDecimals(3);
            editor->setMinimum(0.0);
            editor->setMaximum(500.0);

            return editor;
        }

        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        double value = index.model()->data(index, Qt::EditRole).toInt();

        QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
        spinBox->setValue(value);
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        if(index.column() == 1)
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->interpretText();
            double value = spinBox->value();
            model->setData(index, value, Qt::EditRole);
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

};
#endif // DEL_SPINBOX_DOUBLE

