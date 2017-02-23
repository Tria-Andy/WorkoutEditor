#ifndef DEL_DAYPOP_H
#define DEL_DAYPOP_H
#include <QtGui>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QComboBox>
#include <QTimeEdit>
#include <QDebug>
#include "calculation.h"

class del_daypop: public QStyledItemDelegate, public calculation
{
    Q_OBJECT

public:
    explicit del_daypop(QObject *parent = 0) : QStyledItemDelegate(parent) {}

    bool addWork;
    int selCol;

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
         painter->save();
         const QAbstractItemModel *model = index.model();
         int textMargin = 2;
         QPainterPath rectRow;
         QString sport = model->data(model->index(1,index.column())).toString();
         QString workEntry = index.data(Qt::DisplayRole).toString();
         QRect rectEntry(option.rect.x(),option.rect.y(), option.rect.width(),option.rect.height());
         rectRow.addRoundedRect(rectEntry,2,2);
         QRect rectText(option.rect.x()+textMargin,option.rect.y(), option.rect.width()-textMargin,option.rect.height());

         QLinearGradient rectGradient;
         rectGradient.setCoordinateMode(QGradient::ObjectBoundingMode);
         rectGradient.setSpread(QGradient::RepeatSpread);
         QColor gradColor;
         gradColor.setHsv(0,0,180,200);
         QColor rectColor;

         if(option.state & QStyle::State_Selected)
         {
             rectColor.setHsv(240,255,150,200);
             rectColor.setAlpha(200);
             rectGradient.setColorAt(0,rectColor);
             rectGradient.setColorAt(1,gradColor);
             painter->setPen(Qt::white);
         }
         else
         {
             if(sport.isEmpty())
             {
                rectColor.setHsv(0,0,220);
             }
             else
             {
                rectColor = settings::get_itemColor(sport).toHsv();
             }
             rectColor.setAlpha(200);
             rectGradient.setColorAt(0,rectColor);
             rectGradient.setColorAt(1,gradColor);
             painter->setPen(Qt::black);
         }

         painter->setBrush(rectGradient);
         painter->setRenderHints(QPainter::TextAntialiasing | QPainter::Antialiasing);
         painter->drawPath(rectRow);
         painter->drawText(rectText,Qt::AlignLeft | Qt::AlignVCenter, workEntry);
         painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)

        if(index.row() == 0 && index.column() == selCol)
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("hh:mm");
            editor->setFrame(true);
            return editor;
        }
        if(index.row() == 1 && addWork)
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setFrame(false);
            return editor;
        }
        if(index.row() == 2 && index.column() == selCol)
        {
            QComboBox *editor = new QComboBox(parent);
            editor->setFrame(false);
            return editor;
        }
        if(index.row() == 3 && index.column() == selCol)
        {
            QLineEdit *editor = new QLineEdit(parent);
            editor->setFrame(true);
            return editor;
        }
        if(index.row() ==  4 && index.column() == selCol)
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("hh:mm:ss");
            editor->setFrame(true);     
            return editor;
        }
        if(index.row() == 5 && index.column() == selCol)
        {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(true);
            editor->setDecimals(3);
            editor->setMinimum(0.0);
            editor->setMaximum(500.0);
            editor->setSingleStep(0.100);
            return editor;
        }
        if(index.row() == 6 && index.column() == selCol)
        {
            QSpinBox *editor = new QSpinBox(parent);
            editor->setFrame(true);
            editor->setMinimum(0);
            editor->setMaximum(500);
            return editor;
        }
        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        if(index.row() == 0)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"hh:mm"));
            timeEdit->
        }
        if(index.row() == 1 && addWork)
        {
            QString value = index.data(Qt::DisplayRole).toString();
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            comboBox->addItems(settings::get_listValues("Sport"));
            comboBox->setCurrentText(value);
        }
        if(index.row() == 2)
        {
            QString value = index.data(Qt::DisplayRole).toString();
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            comboBox->addItems(settings::get_listValues("WorkoutCode"));
            comboBox->setCurrentText(value);
        }
        if(index.row() == 3)
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            lineEdit->setText(index.data().toString());
        }
        if(index.row() == 4)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"hh:mm:ss"));
        }
        if(index.row() == 5)
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->setValue(index.data(Qt::DisplayRole).toDouble());
        }
        if(index.row() == 6)
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            spinBox->setValue(index.data(Qt::DisplayRole).toInt());
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        int col = index.column();

        if(index.row() == 0) //Phase and Level
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            QTime value = timeEdit->time();
            timeEdit->interpretText();
            model->setData(index,value.toString("hh:mm"), Qt::EditRole);
        }
        if(index.row() == 1) //Sport
        {
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            QString value = comboBox->currentText();
            model->setData(index,value, Qt::EditRole);
        }
        if(index.row() == 2) //WorkoutCode
        {
            QComboBox *comboBox = static_cast<QComboBox*>(editor);
            QString value = comboBox->currentText();
            model->setData(index,value, Qt::EditRole);
        }
        if(index.row() == 3) //Title
        {
            QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
            QString value = lineEdit->text();
            model->setData(index,value);
        }
        if(index.row() == 4) //Duration
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            QTime value = timeEdit->time();
            timeEdit->interpretText();
            model->setData(index,value.toString("hh:mm:ss"), Qt::EditRole);
            model->setData(model->index(7,col),get_workout_pace(model->data(model->index(5,col)).toDouble(),value,model->data(model->index(1,col)).toString(),true));
        }
        if(index.row() == 5) //Distance
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->interpretText();
            double value = spinBox->value();
            model->setData(index, value, Qt::EditRole);
            model->setData(model->index(7,col),get_workout_pace(value,QTime::fromString(model->data(model->index(4,col)).toString(),"hh:mm:ss"),model->data(model->index(1,col)).toString(),true));
        }
        if(index.row() == 6) //Stress
        {
            QSpinBox *spinBox = static_cast<QSpinBox*>(editor);
            int value = spinBox->value();
            spinBox->interpretText();
            model->setData(index,value, Qt::EditRole);
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }
};
#endif // DEL_DAYPOP_H
