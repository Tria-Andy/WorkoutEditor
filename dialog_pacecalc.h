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

#ifndef DIALOG_PACECALC_H
#define DIALOG_PACECALC_H

#include <QDialog>
#include <QtCore>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTimeEdit>
#include "settings.h"
#include "calculation.h"

class del_racecalc : public QStyledItemDelegate, public calculation
{
    const QString trans = "T";

    void paint( QPainter *painter, const QStyleOptionViewItem& option, const QModelIndex& index ) const
    {
        painter->save();
        QFont cFont;
        QString sportname,indexData;
        const QAbstractItemModel *model = index.model();
        cFont.setPixelSize(12);

        QColor rectColor;
        QRect rect_text(option.rect.x()+2,option.rect.y(), option.rect.width()-2,option.rect.height());
        sportname = model->data(model->index(index.row(),0,QModelIndex())).toString().trimmed();
        indexData = index.data().toString();
        painter->setPen(Qt::black);

        if(index.row() == model->rowCount()-1 && model->rowCount() > 1)
        {
            rectColor = settings::get_itemColor(generalValues->value("sum")).toHsv();
            cFont.setBold(true);
        }
        else
        {
            if(!sportname.contains(trans))
            {
                rectColor = settings::get_itemColor(sportname).toHsv();
            }
            else
            {
                rectColor.setHsv(0,0,180,200);
            }
            cFont.setBold(false);
        }
        rectColor.setAlpha(175);
        painter->fillRect(option.rect,rectColor);
        painter->setFont(cFont);
        painter->drawText(rect_text,indexData,QTextOption(Qt::AlignLeft | Qt::AlignVCenter));
        painter->restore();
    }

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(option)
        QString lapName = index.model()->data(index.model()->index(index.row(),0)).toString();

        if((index.column() == 1 || index.column() == 3) && !lapName.contains(trans))
        {
            QDoubleSpinBox *editor = new QDoubleSpinBox(parent);
            editor->setFrame(false);
            editor->setDecimals(2);
            editor->setMinimum(0.0);
            editor->setMaximum(500.0);
            return editor;
        }
        else if(index.column() == 2 && !lapName.contains(trans))
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("mm:ss");
            editor->setFrame(false);
            return editor;
        }
        else if(index.column() == 4 && lapName.contains(trans))
        {
            QTimeEdit *editor = new QTimeEdit(parent);
            editor->setDisplayFormat("mm:ss");
            editor->setFrame(false);
            return editor;
        }


        return 0;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {

        if(index.column() == 1 || index.column() == 3)
        {
            double dist = index.model()->data(index, Qt::EditRole).toDouble();
            QDoubleSpinBox *doublespinBox = static_cast<QDoubleSpinBox*>(editor);
            doublespinBox->setValue(dist);
        }
        else if(index.column() == 2 || index.column() == 4)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            timeEdit->setTime(QTime::fromString(index.data(Qt::DisplayRole).toString(),"mm:ss"));
        }
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        if(index.column() == 1 || index.column() == 3)
        {
            QDoubleSpinBox *spinBox = static_cast<QDoubleSpinBox*>(editor);
            spinBox->interpretText();
            double dist = spinBox->value();
            model->setData(index, dist, Qt::EditRole);
            setLapData(index.column(),model,index);
        }
        else if(index.column() == 2 || index.column() == 4)
        {
            QTimeEdit *timeEdit = static_cast<QTimeEdit*>(editor);
            QTime value = timeEdit->time();
            timeEdit->interpretText();
            model->setData(index,value.toString("mm:ss"), Qt::EditRole);
            setLapData(index.column(),model,index);
        }
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        Q_UNUSED(index)
        editor->setGeometry(option.rect);
    }

    void setLapData(int col,QAbstractItemModel *model, const QModelIndex &index) const
    {
        QString sPace;
        QTime pace;
        QString sport = model->data(model->index(index.row(),0)).toString();
        double dist = model->data(model->index(index.row(),1)).toDouble();
        double factor = 1.0;

        if(sport == settings::isSwim) factor = 1000.0;

        if(col == 2 && col != model->columnCount()-1)
        {
            sPace = model->data(model->index(index.row(),2)).toString();
            pace =  QTime::fromString(sPace,"mm:ss");
            model->setData(model->index(index.row(),3),set_doubleValue(get_speed(pace,0,sport,true),false));
        }
        else
        {
            double speed = model->data(model->index(index.row(),3)).toDouble();
            int calcTime = dist / speed * 60*60;
            model->setData(model->index(index.row(),2),set_time(calc_lapPace(sport,calcTime,dist*factor)));
            sPace = model->data(model->index(index.row(),2)).toString();
        }

        if(col != 4)
        {
            model->setData(model->index(index.row(),4),calc_duration(sport,dist,sPace));
        }

        QTime timeSum;
        double distSum = 0;
        timeSum.setHMS(0,0,0);

        for(int i = 0; i < model->rowCount()-1; ++i)
        {
            timeSum = timeSum.addSecs(get_timesec(model->data(model->index(i,4,QModelIndex())).toString()));
            distSum += model->data(model->index(i,1)).toDouble();
        }
        model->setData(model->index(model->rowCount()-1,4),timeSum.toString("hh:mm:ss"));
        model->setData(model->index(model->rowCount()-1,1),distSum);
    }
};

namespace Ui {
class Dialog_paceCalc;
}

class Dialog_paceCalc : public QDialog, public calculation
{
    Q_OBJECT

public:
    explicit Dialog_paceCalc(QWidget *parent = nullptr);
    ~Dialog_paceCalc();

private slots:
    void on_pushButton_close_clicked();
    void on_comboBox_sport_currentTextChanged(const QString &arg1);
    void on_timeEdit_pace_timeChanged(const QTime &time);
    void on_lineEdit_dist_textChanged(const QString &arg1);
    void on_spinBox_factor_valueChanged(int arg1);
    void on_timeEdit_intTime_timeChanged(const QTime &time);
    void on_spinBox_IntDist_valueChanged(int arg1);
    void on_pushButton_clicked();
    void on_toolButton_copy_clicked();

    void on_comboBox_race_currentIndexChanged(int index);

private:
    Ui::Dialog_paceCalc *ui;
    QStringList sportList,model_header,race_header,triaDist,runRaces;
    QStandardItemModel *paceModel;
    QStandardItemModel *raceModel;
    QHash<QString,QString> *triaMap;
    QVector<double> dist,raceDist;
    del_racecalc race_del;
    int distFactor;
    void init_paceView();
    void set_pace();
    void set_freeField(int);
    void set_raceTable(QString);
};


#endif // DIALOG_PACECALC_H
