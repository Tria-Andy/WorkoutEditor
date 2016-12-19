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

#include "dialog_addweek.h"
#include "ui_dialog_addweek.h"

Dialog_addweek::Dialog_addweek(QWidget *parent, QString sel_week, schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::Dialog_addweek)
{
    ui->setupUi(this);
    workSched = p_sched;
    ui->comboBox_phase->addItems(settings::get_phaseList());
    ui->comboBox_cycle->addItems(settings::get_cycleList());
    ui->dateEdit_selectDate->setDate(QDate().currentDate());
    timeFormat = "hh:mm:ss";
    empty = "0-0-00:00-0";
    weekHeader << "Sport" << "Workouts" << "Duration" << "%" << "Distance" << "Pace" << "Stress";
    sportuseList = settings::get_sportUseList();
    this->setFixedHeight(100+(35*(sportuseList.count()+1)));
    this->setFixedWidth(650);
    this->fill_values(sel_week);
}

enum{SUMMERY,SWIM,BIKE,RUN};

Dialog_addweek::~Dialog_addweek()
{
    delete ui;
    delete weekModel;
}

void Dialog_addweek::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_addweek::fill_values(QString selWeek)
{
    QStringList weekInfo = selWeek.split("-");
    QList<QStandardItem*> content = workSched->week_content->findItems(weekInfo.at(1),Qt::MatchExactly,1);
    QList<QStandardItem*> meta = workSched->week_meta->findItems(weekInfo.at(1),Qt::MatchExactly,1);
    QModelIndex index;
    QTime duration;
    QString value,work,dura,dist,stress;
    QStringList values;
    int timeSum,timePart;
    int listCount = sportuseList.count();

    weekModel = new QStandardItemModel(sportuseList.count()+1,7);
    weekModel->setHorizontalHeaderLabels(weekHeader);
    ui->tableView_sportValues->setModel(weekModel);
    ui->tableView_sportValues->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_sportValues->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_sportValues->verticalHeader()->hide();
    ui->tableView_sportValues->setItemDelegate(&week_del);

    if(!meta.isEmpty())
    {
        openID = weekInfo.at(1);
        index = workSched->week_meta->indexFromItem(meta.at(0));
        ui->dateEdit_selectDate->setDate(QDate::fromString(workSched->week_meta->item(index.row(),3)->text(),"dd.MM.yyyy"));
        ui->lineEdit_week->setText(QString::number(ui->dateEdit_selectDate->date().weekNumber()));
        value = weekInfo.at(3);
        ui->comboBox_phase->setCurrentText(value.split("_").first());
        ui->comboBox_cycle->setCurrentText(value.split("_").last());
        update = true;
    }
    else
    {
        update = false;
    }

    if(!content.isEmpty())
    {
        index = workSched->week_content->indexFromItem(content.at(0));
        for(int i = 2,row = 0; i < listCount+2; ++i,++row)
        {            
            value = workSched->week_content->item(index.row(),i)->text();
            values = value.split("-");
            work = values.at(0);
            dist = values.at(1);
            dura = values.at(2);
            stress = values.at(3);
            duration = QTime::fromString(dura,"hh:mm");

            weekModel->setData(weekModel->index(row,0,QModelIndex()),sportuseList.at(row));
            weekModel->setData(weekModel->index(row,1,QModelIndex()),work.toInt());
            weekModel->setData(weekModel->index(row,2,QModelIndex()),duration);
            weekModel->setData(weekModel->index(row,3,QModelIndex()),0.0);
            weekModel->setData(weekModel->index(row,4,QModelIndex()),settings::set_doubleValue(dist.toDouble(),false));
            weekModel->setData(weekModel->index(row,5,QModelIndex()),settings::get_workout_pace(dist.toDouble(),duration,sportuseList.at(row),false));
            weekModel->setData(weekModel->index(row,6,QModelIndex()),stress.toInt());        
        }
        weekModel->setData(weekModel->index(listCount,0,QModelIndex()),"Summery");
        weekModel->setData(weekModel->index(listCount,1,QModelIndex()),this->sum_int(weekModel,&sportuseList,1));
        weekModel->setData(weekModel->index(listCount,2,QModelIndex()),this->sum_time(weekModel,&sportuseList,2));
        weekModel->setData(weekModel->index(listCount,3,QModelIndex()),100);
        weekModel->setData(weekModel->index(listCount,4,QModelIndex()),settings::set_doubleValue(this->sum_double(weekModel,&sportuseList,4),false));
        weekModel->setData(weekModel->index(listCount,5,QModelIndex()),"--");
        weekModel->setData(weekModel->index(listCount,6,QModelIndex()),this->sum_int(weekModel,&sportuseList,6));
    }
    else
    {
        for(int row = 0; row < listCount; ++row)
        {
            weekModel->setData(weekModel->index(row,0,QModelIndex()),sportuseList.at(row));
            weekModel->setData(weekModel->index(row,1,QModelIndex()),0);
            weekModel->setData(weekModel->index(row,2,QModelIndex()),QTime::fromString("00:00:00"));
            weekModel->setData(weekModel->index(row,3,QModelIndex()),0.0);
            weekModel->setData(weekModel->index(row,4,QModelIndex()),0.0);
            weekModel->setData(weekModel->index(row,5,QModelIndex()),"--");
            weekModel->setData(weekModel->index(row,6,QModelIndex()),0);
        }
        weekModel->setData(weekModel->index(listCount,0,QModelIndex()),"Summery");
    }

    timeSum = settings::get_timesec(weekModel->data(weekModel->index(listCount,2,QModelIndex())).toTime().toString(timeFormat));
    timePart = 0;
    for(int i = 0; i < listCount; ++i)
    {
        timePart = settings::get_timesec(weekModel->data(weekModel->index(i,2,QModelIndex())).toTime().toString(timeFormat));
        weekModel->setData(weekModel->index(i,3,QModelIndex()),week_del.calc_percent(timeSum,timePart));
    }
}

void Dialog_addweek::store_values()
{
    weekMeta = QStringList();
    weekID = ui->lineEdit_week->text()+"_"+selYear;
    int currID = workSched->week_meta->rowCount()+1;
    if(update)
    {
        weekMeta << weekID
                 << ui->comboBox_phase->currentText()+"_"+ui->comboBox_cycle->currentText()
                 << ui->dateEdit_selectDate->date().toString("dd.MM.yyyy");

        weekContent << weekID
                    << this->create_values();
    }
    else
    {
        weekMeta << QString::number(currID)
                 << weekID
                 << ui->comboBox_phase->currentText()+"_"+ui->comboBox_cycle->currentText()
                 << ui->dateEdit_selectDate->date().toString("dd.MM.yyyy");

        weekContent << QString::number(currID)
                    << weekID
                    << this->create_values();
    }

}

QStringList Dialog_addweek::create_values()
{
    QString splitter = "-",vString;
    QStringList list;

    for(int i = 0; i < sportuseList.count()+1; ++i)
    {
        vString = weekModel->data(weekModel->index(i,1,QModelIndex())).toString()+splitter;
        vString = vString+weekModel->data(weekModel->index(i,4,QModelIndex())).toString()+splitter;
        vString = vString+weekModel->data(weekModel->index(i,2,QModelIndex())).toTime().toString("hh:mm")+splitter;
        vString = vString+weekModel->data(weekModel->index(i,6,QModelIndex())).toString();
        list << vString;
    }
    return list;
}

void Dialog_addweek::on_dateEdit_selectDate_dateChanged(const QDate &date)
{
    ui->lineEdit_week->setText(QString::number(date.weekNumber()));
    selYear = QString::number(date.year());
}

void Dialog_addweek::on_pushButton_ok_clicked()
{
    this->store_values();

    if(update)
    {
        QList<QStandardItem*> openItem = workSched->week_meta->findItems(openID,Qt::MatchExactly,1);

        if(!openItem.isEmpty())
        {
            QModelIndex index = workSched->week_meta->indexFromItem(openItem.at(0));
            workSched->week_meta->setData(workSched->week_meta->index(index.row(),1,QModelIndex()),weekMeta.at(0));
            workSched->week_meta->setData(workSched->week_meta->index(index.row(),2,QModelIndex()),weekMeta.at(1));
            workSched->week_meta->setData(workSched->week_meta->index(index.row(),3,QModelIndex()),weekMeta.at(2));
        }
        openItem.clear();

        openItem = workSched->week_content->findItems(openID,Qt::MatchExactly,1);

        if(!openItem.isEmpty())
        {
            QModelIndex index = workSched->week_content->indexFromItem(openItem.at(0));
            for(int i = 1; i <= weekContent.count(); ++i)
            {
                workSched->week_content->setData(workSched->week_content->index(index.row(),i,QModelIndex()),weekContent.at(i-1));
            }
        }
    }
    else
    {
        int rowcount;
        rowcount = workSched->week_meta->rowCount();
        workSched->week_meta->insertRow(rowcount,QModelIndex());

        for(int i = 0; i < weekMeta.count(); ++i)
        {
            workSched->week_meta->setData(workSched->week_meta->index(rowcount,i,QModelIndex()),weekMeta.at(i));
        }

        rowcount = workSched->week_content->rowCount();
        workSched->week_content->insertRow(rowcount,QModelIndex());

        for(int i = 0; i < weekContent.count(); ++i)
        {
            workSched->week_content->setData(workSched->week_content->index(rowcount,i,QModelIndex()),weekContent.at(i));
        }
    }
    accept();
}
int Dialog_addweek::sum_int(QStandardItemModel *model,QStringList *list, int col)
{
    int sum = 0;
    for(int i = 0; i < list->count(); ++i)
    {
       sum = sum + model->data(model->index(i,col,QModelIndex())).toInt();
    }
    return sum;
}

double Dialog_addweek::sum_double(QStandardItemModel *model,QStringList *list, int col)
{
    double sum = 0;
    for(int i = 0; i < list->count(); ++i)
    {
       sum = sum + model->data(model->index(i,col,QModelIndex())).toDouble();
    }
    return sum;
}

QTime Dialog_addweek::sum_time(QStandardItemModel *model,QStringList *list, int col)
{
    QTime sum(0,0,0);
    QString sportTime;
    for(int i = 0; i < list->count(); ++i)
    {
       sportTime =  model->data(model->index(i,col,QModelIndex())).toTime().toString(timeFormat);
       sum = sum.addSecs(settings::get_timesec(sportTime));
    }
    return sum;
}
