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
    ui->comboBox_phase->addItems(settings::get_listValues("Phase"));
    ui->comboBox_cycle->addItems(settings::get_listValues("Cycle"));
    timeFormat = "hh:mm:ss";
    empty = "0-0-00:00-0";
    weekHeader = settings::getHeaderMap("addweek");
    sportuseList = settings::get_listValues("Sportuse");
    sportuseList.removeLast();
    week_del.sportUse = &sportuseList;
    sportlistCount = sportuseList.count();
    weekModel = new QStandardItemModel(this);
    ui->toolButton_paste->setEnabled(false);
    this->setFixedHeight(100+(35*(sportuseList.count()+1)));
    this->setFixedWidth(650);
    this->fill_values(sel_week.split("#").first().split(" - ").first());
}

Dialog_addweek::~Dialog_addweek()
{
    delete ui;
}
void Dialog_addweek::on_toolButton_close_clicked()
{
    reject();
}

void Dialog_addweek::fill_values(QString selWeekID)
{
    ui->dateEdit_selectDate->blockSignals(true);

    weekMeta = workSched->get_weekMeta(selWeekID);
    compValues = workSched->get_compWeekValues()->value(selWeekID);
    ui->label_header->clear();
    ui->label_header->setText("Saison: "+ generalValues->value("saison")  +" - Week: "+ weekMeta.at(1)+" ("+selWeekID +") - Phase: "+weekMeta.at(2));

    weekModel->clear();
    weekModel->setRowCount(sportuseList.count()+1);
    weekModel->setColumnCount(weekHeader->count());
    ui->tableView_sportValues->setModel(weekModel);
    ui->tableView_sportValues->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_sportValues->horizontalHeader()->setSectionsClickable(false);
    ui->tableView_sportValues->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_sportValues->verticalHeader()->hide();
    ui->tableView_sportValues->setItemDelegate(&week_del);

    for(int vheader = 0; vheader < weekHeader->count(); ++vheader)
    {
        weekModel->setHorizontalHeaderItem(vheader,new QStandardItem(weekHeader->at(vheader)));
    }

    if(compValues.count() > 0)
    {
        editWeekID = selWeekID;
        ui->dateEdit_selectDate->setDate(QDate::fromString(weekMeta.at(3),dateFormat));
        ui->lineEdit_week->setText(QString::number(ui->dateEdit_selectDate->date().weekNumber()));

        ui->comboBox_phase->setCurrentText(weekMeta.at(2).split("_").first());
        ui->comboBox_cycle->setCurrentText(weekMeta.at(2).split("_").last());
        ui->lineEdit_weekContent->setText(weekMeta.at(4));
        ui->lineEdit_weekGoals->setText(weekMeta.at(5));
        update = true;

        QVector<double> weekComp(4,0);
        QString dura;

        for(int sport = 0; sport < sportlistCount; ++sport)
        {
            weekComp = compValues.value(sportuseList.at(sport));
            dura = set_time(static_cast<int>((weekComp.at(2))));

            weekModel->setData(weekModel->index(sport,0,QModelIndex()),sportuseList.at(sport));
            weekModel->setData(weekModel->index(sport,1,QModelIndex()),weekComp.at(0));
            weekModel->setData(weekModel->index(sport,2,QModelIndex()),dura);
            weekModel->setData(weekModel->index(sport,3,QModelIndex()),0.0);
            weekModel->setData(weekModel->index(sport,4,QModelIndex()),this->set_doubleValue(weekComp.at(1),false));
            weekModel->setData(weekModel->index(sport,5,QModelIndex()),this->get_workout_pace(weekComp.at(1),QTime::fromString(dura,timeFormat),sportuseList.at(sport),false));
            weekModel->setData(weekModel->index(sport,6,QModelIndex()),weekComp.at(3));
        }
        this->fill_weekSumRow(ui->tableView_sportValues->model());

     }
     ui->dateEdit_selectDate->blockSignals(false);
     week_del.calc_percent(&sportuseList,ui->tableView_sportValues->model());

}

void Dialog_addweek::update_values()
{
    ui->dateEdit_selectDate->setFocus();

    QVector<double> weekData(4,0);
    QString sport;
    for(int sport = 0; sport < sportlistCount; ++sport)
    {
        weekData[0] = weekModel->data(weekModel->index(sport,1)).toDouble();
        weekData[1] = weekModel->data(weekModel->index(sport,4)).toDouble();
        weekData[2] = get_timesec(weekModel->data(weekModel->index(sport,2)).toString());
        weekData[3] = weekModel->data(weekModel->index(sport,6)).toDouble();
        compValues.insert(sportuseList.at(sport),weekData);
    }

    weekMeta[0] = calc_weekID(ui->dateEdit_selectDate->date());
    weekMeta[2] = ui->comboBox_phase->currentText() +"_"+ ui->comboBox_cycle->currentText();
    weekMeta[3] = ui->dateEdit_selectDate->date().toString(dateFormat);
    weekMeta[4] = ui->lineEdit_weekContent->text();
    weekMeta[5] = ui->lineEdit_weekGoals->text();

    workSched->set_weekCompValues(weekMeta,compValues);

    ui->toolButton_paste->setEnabled(true);
}

void Dialog_addweek::fill_weekSumRow(QAbstractItemModel *ab_model)
{
    weekModel->setData(weekModel->index(sportlistCount,0,QModelIndex()),generalValues->value("sum"));
    weekModel->setData(weekModel->index(sportlistCount,1,QModelIndex()),week_del.sum_int(ab_model,&sportuseList,1));
    weekModel->setData(weekModel->index(sportlistCount,2,QModelIndex()),week_del.sum_time(ab_model,&sportuseList,2).toString(timeFormat));
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
    this->fill_values(calc_weekID(date));
}

void Dialog_addweek::on_toolButton_update_clicked()
{
    this->update_values();
}

void Dialog_addweek::on_toolButton_copy_clicked()
{
    this->update_values();
}

void Dialog_addweek::on_toolButton_paste_clicked()
{
    QString vPhase = weekMeta.at(1),contentSport,dist;
    QStringList content;

    QAbstractItemModel *ab_model = ui->tableView_sportValues->model();

    ui->comboBox_phase->setCurrentText(vPhase.split("_").first());
    ui->comboBox_cycle->setCurrentText(vPhase.split("_").last());
    ui->lineEdit_weekContent->setText(weekMeta.at(3));
    ui->lineEdit_weekGoals->setText(weekMeta.at(4));


    week_del.calc_percent(&sportuseList,ab_model);

    this->fill_weekSumRow(ab_model);

    ui->dateEdit_selectDate->setFocus();
}
