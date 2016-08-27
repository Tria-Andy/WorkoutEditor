#ifndef DEL_SWIMLAP_H
#define DEL_SWIMLAP_H
#include <QtGui>
#include <QStyledItemDelegate>
#include <QSpinBox>
#include <QDebug>
#include "settings.h"

class del_swimlap : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit del_swimlap(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont cFont;
        cFont.setPixelSize(12);
        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());
        if(index.column() == 0)
        {

            if(index.data(Qt::DisplayRole) == "Break")
            {
                painter->setPen(Qt::white);
                painter->fillRect(option.rect,QBrush(QColor(128,128,128)));
                painter->fillRect(rect_text,QBrush(QColor(128,128,128)));
            }
            else
            {
                painter->setPen(Qt::black);
                painter->fillRect(option.rect,QBrush(QColor(192,192,192)));
                painter->fillRect(rect_text,QBrush(QColor(192,192,192)));
            }
        }
        else
        {
            if(index.data(Qt::DisplayRole).toInt() > 0)
            {
                painter->setPen(Qt::black);
                painter->fillRect(option.rect,QBrush(QColor(192,192,192)));
                painter->fillRect(rect_text,QBrush(QColor(192,192,192)));
            }
            else
            {
                painter->setPen(Qt::white);
                painter->fillRect(option.rect,QBrush(QColor(128,128,128)));
                painter->fillRect(rect_text,QBrush(QColor(128,128,128)));
            }
        }
        painter->setFont(cFont);
        painter->drawText(rect_text,index.data().toString(),QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if(index.column() == 2)
        {
            Q_UNUSED(option)
            Q_UNUSED(index)
            QSpinBox *editor = new QSpinBox(parent);
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
        QModelIndex new_index = model->index(index.row()+1,index.column()-1,QModelIndex());
        QModelIndex curr_index = model->index(index.row(),index.column()-1,QModelIndex());
        QModelIndex speed_index = model->index(index.row(),4,QModelIndex());
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();

        int value = spinBox->value();
        int lapstart = model->data(curr_index,Qt::DisplayRole).toInt();
        double lapSpeed = settings().get_speed(QTime::fromString(settings().set_time(value),"mm:ss"),50,settings().isSwim,false).toDouble();
        qDebug() << settings().get_sportList();
        model->setData(index, value, Qt::EditRole);
        model->setData(new_index,lapstart+value, Qt::EditRole);
        model->setData(speed_index,lapSpeed);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

};

#endif // DEL_SWIMLAP_H
