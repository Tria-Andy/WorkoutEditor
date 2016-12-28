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
public:
    explicit del_addweek(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        QStringList sportList = settings::get_sportUseList();
        int listCount = sportList.count();

        if((index.column() == 1 || index.column() == 6) && index.row() != listCount)
        {
            QSpinBox *editor = new QSpinBox(parent);
            editor->setFrame(false);
            editor->setMinimum(0);
            editor->setMaximum(1000);
            return editor;
        }
        if(index.column() == 2 && index.row() != listCount)
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("HH:mm");
            editor->setFrame(false);
            return editor;
        }
        if(index.column() == 4 && index.row() != listCount)
        {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(false);
            editor->setDecimals(2);
            editor->setMinimum(0.0);
            editor->setMaximum(500.0);
            return editor;
        }

        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QStringList sportList = settings::get_sportUseList();
        int listCount = sportList.count();

        if((index.column() == 1 || index.column() == 6) && index.row() != listCount)
        {
            int value = index.model()->data(index, Qt::EditRole).toInt();
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            spinBox->setValue(value);
        }
        if(index.column() == 2 && index.row() != listCount)
        {
            QTime dura = index.model()->data(index,Qt::EditRole).toTime();
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            timeEdit->setTime(dura);
        }
        if(index.column() == 4 && index.row() != listCount)
        {
            double dist = index.model()->data(index, Qt::EditRole).toDouble();
            QDoubleSpinBox *doublespinBox = static_cast<QDoubleSpinBox*>(editor);
            doublespinBox->setValue(dist);
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        QStringList sportList = settings::get_sportUseList();

        int listCount = sportList.count();
        double factor = 1.0;
        QString sport = model->data(model->index(index.row(),0,QModelIndex())).toString();
        QModelIndex sum_index = model->index(listCount,index.column(),QModelIndex());

        if(sport == settings::isSwim) factor = settings::get_thresValue("swimfactor");
        if(sport == settings::isBike) factor = settings::get_thresValue("bikefactor");
        if(sport == settings::isRun) factor = settings::get_thresValue("runfactor");

        if(index.column() == 1 && index.row() != listCount)
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            int works = spinBox->value();
            spinBox->interpretText();
            model->setData(index,works);
            model->setData(sum_index,sum_int(model,&sportList,1));
        }
        if(index.column() == 2 && index.row() != listCount)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            QTime dura = timeEdit->time();
            timeEdit->interpretText();
            model->setData(index,dura);
            model->setData(sum_index,sum_time(model,&sportList,2));
            model->setData(model->index(index.row(),6,QModelIndex()),round(get_timeMin(dura)*factor));
            model->setData(model->index(listCount,6,QModelIndex()),sum_int(model,&sportList,6));
            calc_percent(&sportList,model);
            setPace(model,index.row());
        }
        if(index.column() == 4 && index.row() != listCount)
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->interpretText();
            double dist = spinBox->value();
            model->setData(index, dist, Qt::EditRole);
            model->setData(sum_index,sum_double(model,&sportList,4));
            setPace(model,index.row());
        }
        if(index.column() == 6 && index.row() != listCount)
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            int stress = spinBox->value();
            spinBox->interpretText();
            model->setData(index,stress);
            model->setData(sum_index,sum_int(model,&sportList,6));
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
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

        if(index.row() == sportuse.count())
        {
            painter->fillRect(option.rect,QBrush(settings::get_itemColor("sumcolor")));
            cFont.setBold(true);
        }
        else
        {
            painter->fillRect(option.rect,QBrush(settings::get_itemColor(sportname)));
            cFont.setBold(false);
        }

        painter->setFont(cFont);
        painter->drawText(rect_text,indexData,QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

    int get_timeMin(QTime time) const
    {
        int minutes;
        minutes = time.hour() * 60;
        minutes = minutes + time.minute();
        return minutes;
    }

    void setPace(QAbstractItemModel *model,int row) const
    {
        QString sport = model->data(model->index(row,0,QModelIndex())).toString();
        double dist = model->data(model->index(row,4,QModelIndex())).toDouble();
        QTime dura = model->data(model->index(row,2,QModelIndex())).toTime();
        model->setData(model->index(row,5,QModelIndex()),settings::get_workout_pace(dist,dura,sport,false));
    }

    void calc_percent(QStringList *list,QAbstractItemModel *model) const
    {
        int sum = 0,part = 0;
        sum = get_timeMin(model->data(model->index(list->count(),2,QModelIndex())).toTime());
        if(sum > 0)
        {
            for(int i = 0; i < list->count(); ++i)
            {
                part = get_timeMin(model->data(model->index(i,2,QModelIndex())).toTime());
                model->setData(model->index(i,3,QModelIndex()),settings::set_doubleValue(static_cast<double>(part) / static_cast<double>(sum)*100.0,false));
            }
        }
    }

    int sum_int(QAbstractItemModel *model,QStringList *list, int col) const
    {
        int sum = 0;
        for(int i = 0; i < list->count(); ++i)
        {
           sum = sum + model->data(model->index(i,col,QModelIndex())).toInt();
        }
        return sum;
    }

    double sum_double(QAbstractItemModel *model,QStringList *list, int col) const
    {
        double sum = 0;
        for(int i = 0; i < list->count(); ++i)
        {
           sum = sum + model->data(model->index(i,col,QModelIndex())).toDouble();
        }
        return sum;
    }

    QTime sum_time(QAbstractItemModel *model,QStringList *list, int col) const
    {
        QTime sum(0,0,0);
        QString sportTime;
        for(int i = 0; i < list->count(); ++i)
        {
           sportTime =  model->data(model->index(i,col,QModelIndex())).toTime().toString("hh:mm:ss");
           sum = sum.addSecs(settings::get_timesec(sportTime));
        }
        return sum;
    }
};
#endif // DEL_ADDWEEK_H
