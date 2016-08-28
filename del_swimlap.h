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
        QColor lapColor,breakColor;
        lapColor.setRgb(192,192,192);
        breakColor.setRgb(128,128,128);
        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width(),option.rect.height());
        if(index.column() == 0)
        {

            if(index.data(Qt::DisplayRole) == "Break")
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
        }
        else
        {
            if(index.data(Qt::DisplayRole).toInt() > 0)
            {
                painter->setPen(Qt::black);
                painter->fillRect(option.rect,QBrush(lapColor));
                painter->fillRect(rect_text,QBrush(lapColor));
            }
            else
            {
                painter->setPen(Qt::white);
                painter->fillRect(option.rect,QBrush(breakColor));
                painter->fillRect(rect_text,QBrush(breakColor));
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
        QModelIndex new_index,curr_index;
        QModelIndex speed_index = model->index(index.row(),4,QModelIndex());
        QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
        spinBox->interpretText();
        QString isBreak = "Break";
        QString currLap;
        int lapTime,startTime;


        int value = spinBox->value();
        double lapSpeed = settings().get_speed(QTime::fromString(settings().set_time(value),"mm:ss"),50,settings().isSwim,false).toDouble();

        model->setData(index, value, Qt::EditRole);
        model->setData(speed_index,lapSpeed);

        int laprow = index.row();

        do
        {
            curr_index = model->index(laprow,1,QModelIndex());
            startTime = model->data(model->index(laprow,1,QModelIndex())).toInt();
            lapTime = model->data(model->index(laprow,2,QModelIndex())).toInt();

            new_index = model->index(laprow+1,1,QModelIndex());
            model->setData(new_index,startTime+lapTime,Qt::EditRole);
            ++laprow;
            currLap = model->data(model->index(laprow,0,QModelIndex())).toString();

        } while (currLap != isBreak);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }
};

#endif // DEL_SWIMLAP_H
