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
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    workSched = p_sched;
    metaProxy = new QSortFilterProxyModel(this);
    metaProxy->setSourceModel(p_sched->week_meta);
    metaProxy->setFilterFixedString(workSched->get_selSaison());
    metaProxy->setFilterKeyColumn(0);
    metaProxyFilter = new QSortFilterProxyModel(this);
    metaProxyFilter->setSourceModel(metaProxy);
    contentProxy = new QSortFilterProxyModel(this);
    contentProxy->setSourceModel(p_sched->week_content);
    delimiter = "#";
    ui->comboBox_phase->addItems(settings::get_listValues("Phase"));
    ui->comboBox_cycle->addItems(settings::get_listValues("Cycle"));
    timeFormat = "hh:mm:ss";
    empty = "0-0-00:00-0";
    weekHeader << "Sport" << "Workouts" << "Duration" << "%" << "Distance" << "Pace" << "Stress";
    sumString = generalValues->value("sum");
    sportuseList = settings::get_listValues("Sportuse");
    sportlistCount = sportuseList.count();
    weekModel = new QStandardItemModel(this);
    ui->toolButton_paste->setEnabled(false);
    this->setFixedHeight(100+(35*(sportuseList.count()+1)));
    this->setFixedWidth(650);
    this->fill_values(sel_week);
}

Dialog_addweek::~Dialog_addweek()
{
    delete ui;
}
void Dialog_addweek::on_toolButton_close_clicked()
{
    reject();
}

void Dialog_addweek::fill_values(QString selWeek)
{
    ui->dateEdit_selectDate->blockSignals(true);
    QStringList weekInfo = selWeek.split(delimiter);
    QString selWeekID = weekInfo.at(1);
    metaProxyFilter->invalidate();
    metaProxyFilter->setFilterRegExp("\\b"+selWeekID+"\\b");
    metaProxyFilter->setFilterKeyColumn(2);
    contentProxy->setFilterRegExp("\\b"+selWeekID+"\\b");
    contentProxy->setFilterKeyColumn(1);

    ui->label_header->clear();
    ui->label_header->setText("Saison: "+metaProxy->data(metaProxy->index(0,0)).toString()+" - Week: "+ weekInfo.at(0)+" ("+selWeekID +") - Phase: "+weekInfo.at(3));

    QTime duration;
    QString value,work,dura,dist,stress;
    QStringList values;

    weekModel->clear();
    weekModel->setRowCount(sportuseList.count()+1);
    weekModel->setColumnCount(weekHeader.count());
    weekModel->setHorizontalHeaderLabels(weekHeader);
    ui->tableView_sportValues->setModel(weekModel);
    ui->tableView_sportValues->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_sportValues->horizontalHeader()->setSectionsClickable(false);
    ui->tableView_sportValues->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_sportValues->verticalHeader()->hide();
    ui->tableView_sportValues->setItemDelegate(&week_del);

    QAbstractItemModel *ab_model = ui->tableView_sportValues->model();

    if(metaProxyFilter->rowCount() > 0)
    {
        editWeekID = selWeekID;
        ui->dateEdit_selectDate->setDate(QDate::fromString(metaProxyFilter->data(metaProxyFilter->index(0,4)).toString(),"dd.MM.yyyy"));
        ui->lineEdit_week->setText(QString::number(ui->dateEdit_selectDate->date().weekNumber()));
        value = weekInfo.at(3);
        ui->comboBox_phase->setCurrentText(value.split("_").first());
        ui->comboBox_cycle->setCurrentText(value.split("_").last());
        ui->lineEdit_weekContent->setText(metaProxyFilter->data(metaProxyFilter->index(0,5)).toString());
        ui->lineEdit_weekGoals->setText(metaProxyFilter->data(metaProxyFilter->index(0,6)).toString());
        update = true;
    }
    else
    {
        update = false;
    }

    if(contentProxy->rowCount() > 0)
    {
        for(int i = 2,row = 0; i < sportlistCount+2; ++i,++row)
        {            
            value = contentProxy->data(contentProxy->index(0,i)).toString();
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
            weekModel->setData(weekModel->index(row,4,QModelIndex()),this->set_doubleValue(dist.toDouble(),false));
            weekModel->setData(weekModel->index(row,5,QModelIndex()),this->get_workout_pace(dist.toDouble(),duration,sportuseList.at(row),false));
            weekModel->setData(weekModel->index(row,6,QModelIndex()),stress.toInt());        
        }

        this->fill_weekSumRow(ab_model);
    }
    else
    {
        for(int row = 0; row < sportlistCount; ++row)
        {
            weekModel->setData(weekModel->index(row,0,QModelIndex()),sportuseList.at(row));
            weekModel->setData(weekModel->index(row,1,QModelIndex()),0);
            weekModel->setData(weekModel->index(row,2,QModelIndex()),QTime::fromString("00:00:00"));
            weekModel->setData(weekModel->index(row,3,QModelIndex()),0.0);
            weekModel->setData(weekModel->index(row,4,QModelIndex()),0.0);
            weekModel->setData(weekModel->index(row,5,QModelIndex()),"--");
            weekModel->setData(weekModel->index(row,6,QModelIndex()),0);
        }
        weekModel->setData(weekModel->index(sportlistCount,0,QModelIndex()),sumString);
    }

    week_del.calc_percent(&sportuseList,ab_model);
    metaProxyFilter->invalidate();
    contentProxy->invalidate();
    ui->dateEdit_selectDate->blockSignals(false);
}

void Dialog_addweek::get_currentValues()
{
    ui->dateEdit_selectDate->setFocus();
    int currID = metaProxyFilter->rowCount()+1;
    weekMeta.clear();
    weekContent.clear();

    if(update)
    {
        weekMeta << editWeekID
                 << ui->comboBox_phase->currentText()+"_"+ui->comboBox_cycle->currentText()
                 << ui->dateEdit_selectDate->date().toString("dd.MM.yyyy")
                 << ui->lineEdit_weekContent->text()
                 << ui->lineEdit_weekGoals->text();

        weekContent << editWeekID
                    << this->create_values();
    }
    else
    {
        weekMeta << QString::number(currID)
                 << editWeekID
                 << ui->comboBox_phase->currentText()+"_"+ui->comboBox_cycle->currentText()
                 << ui->dateEdit_selectDate->date().toString("dd.MM.yyyy")
                 << ui->lineEdit_weekContent->text()
                 << ui->lineEdit_weekGoals->text();

        weekContent << QString::number(currID)
                    << editWeekID
                    << this->create_values();
    }
    ui->toolButton_paste->setEnabled(true);
}

void Dialog_addweek::fill_currentValues()
{
    if(update)
    {
        metaProxyFilter->invalidate();
        metaProxyFilter->setFilterRegExp("\\b"+editWeekID+"\\b");
        metaProxyFilter->setFilterKeyColumn(2);
        contentProxy->invalidate();
        contentProxy->setFilterRegExp("\\b"+editWeekID+"\\b");
        contentProxy->setFilterKeyColumn(1);

        if(metaProxyFilter->rowCount() > 0)
        {
            for(int col = 2,pos = 0; col < workSched->week_meta->columnCount(); ++col,++pos)
            {
                metaProxyFilter->setData(metaProxyFilter->index(0,col),weekMeta.at(pos));
            }
        }

        if(contentProxy->rowCount() > 0)
        {
            for(int i = 1; i <= weekContent.count(); ++i)
            {
                contentProxy->setData(contentProxy->index(0,i),weekContent.at(i-1));
            }
        }
        metaProxyFilter->invalidate();
        contentProxy->invalidate();
    }
    else
    {
        int rowcount;
        rowcount = metaProxy->rowCount();
        metaProxyFilter->insertRow(rowcount,QModelIndex());

        for(int i = 0; i < weekMeta.count(); ++i)
        {
            metaProxyFilter->setData(metaProxyFilter->index(rowcount,i,QModelIndex()),weekMeta.at(i));
        }

        rowcount = contentProxy->rowCount();
        contentProxy->insertRow(rowcount,QModelIndex());

        for(int i = 0; i < weekContent.count(); ++i)
        {
            contentProxy->setData(contentProxy->index(rowcount,i,QModelIndex()),weekContent.at(i));
        }
    }
}

void Dialog_addweek::fill_weekSumRow(QAbstractItemModel *ab_model)
{
    weekModel->setData(weekModel->index(sportlistCount,0,QModelIndex()),sumString);
    weekModel->setData(weekModel->index(sportlistCount,1,QModelIndex()),week_del.sum_int(ab_model,&sportuseList,1));
    weekModel->setData(weekModel->index(sportlistCount,2,QModelIndex()),week_del.sum_time(ab_model,&sportuseList,2));
    weekModel->setData(weekModel->index(sportlistCount,3,QModelIndex()),100);
    weekModel->setData(weekModel->index(sportlistCount,4,QModelIndex()),this->set_doubleValue(week_del.sum_double(ab_model,&sportuseList,4),false));
    weekModel->setData(weekModel->index(sportlistCount,5,QModelIndex()),"--");
    weekModel->setData(weekModel->index(sportlistCount,6,QModelIndex()),week_del.sum_int(ab_model,&sportuseList,6));
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
    QString firstDay = date.toString("dd.MM.yyyy");
    QString weekString;
    metaProxyFilter->invalidate();
    metaProxyFilter->setFilterFixedString(firstDay);
    metaProxyFilter->setFilterKeyColumn(4);

    weekString = metaProxyFilter->data(metaProxyFilter->index(0,1)).toString() +delimiter+
                metaProxyFilter->data(metaProxyFilter->index(0,2)).toString() +delimiter+
                metaProxyFilter->data(metaProxyFilter->index(0,4)).toString() +delimiter+
                metaProxyFilter->data(metaProxyFilter->index(0,3)).toString() +delimiter+
                metaProxyFilter->data(metaProxyFilter->index(0,5)).toString();
    this->fill_values(weekString);
    ui->lineEdit_week->setText(QString::number(date.weekNumber()));
}

void Dialog_addweek::on_toolButton_update_clicked()
{
    this->get_currentValues();
    this->fill_currentValues();
    workSched->set_isUpdated(true);
}

void Dialog_addweek::on_toolButton_copy_clicked()
{
    this->get_currentValues();
}

void Dialog_addweek::on_toolButton_paste_clicked()
{
    QString vPhase = weekMeta.at(1),contentSport,dist;
    QStringList content;
    QTime duration;

    QAbstractItemModel *ab_model = ui->tableView_sportValues->model();

    ui->comboBox_phase->setCurrentText(vPhase.split("_").first());
    ui->comboBox_cycle->setCurrentText(vPhase.split("_").last());
    ui->lineEdit_weekContent->setText(weekMeta.at(3));
    ui->lineEdit_weekGoals->setText(weekMeta.at(4));

    for(int row = 0,sport = 1; row < weekModel->rowCount()-1; ++row,++sport)
    {
        contentSport = weekContent.at(sport);
        content = contentSport.split("-");
        dist = content.at(1);
        duration = QTime::fromString(content.at(2),"hh:mm");;
        weekModel->setData(weekModel->index(row,1),content.at(0));
        weekModel->setData(weekModel->index(row,2),duration);
        weekModel->setData(weekModel->index(row,3),week_del.sum_int(ab_model,&sportuseList,1));
        weekModel->setData(weekModel->index(row,4),dist);
        weekModel->setData(weekModel->index(row,5),this->get_workout_pace(dist.toDouble(),duration,sportuseList.at(row),false));
        weekModel->setData(weekModel->index(row,6),content.at(3));
    }

    week_del.calc_percent(&sportuseList,ab_model);

    this->fill_weekSumRow(ab_model);

    ui->dateEdit_selectDate->setFocus();
}
