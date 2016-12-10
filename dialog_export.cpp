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

#include "dialog_export.h"
#include "ui_dialog_export.h"
#include <QDebug>
#include <math.h>
#include <QDir>

Dialog_export::Dialog_export(QWidget *parent, schedule *p_schedule) :
    QDialog(parent),
    ui(new Ui::Dialog_export)
{
    ui->setupUi(this);
    QModelIndex index;
    jsonhandler = new jsonHandler(false,"",nullptr);
    workoutschedule = p_schedule;
    modelMeta = workoutschedule->week_meta;
    modelSchedule = workoutschedule->workout_schedule;
    QDate currDate = QDate().currentDate();
    workoutWeek = modelMeta->findItems(currDate.addDays(1 - currDate.dayOfWeek()).toString("dd.MM.yyyy"),Qt::MatchExactly,3);
    workoutschedule->workout_schedule->sort(0);

    QModelIndex modelindex = modelMeta->indexFromItem(workoutWeek.at(0));
    int weekID = modelMeta->item(modelindex.row(),0)->text().toInt()-1;

    for(int i = weekID; i < modelMeta->rowCount();++i)
    {
        index = modelMeta->index(i,1,QModelIndex());
        ui->comboBox_week_export->addItem(modelMeta->data(index).toString());
    }
    export_mode = 0;

    ui->dateEdit_export->setDate(currDate);
    ui->comboBox_week_export->setEnabled(false);
    ui->comboBox_week_export->setEnabled(false);
    this->set_comboBox_time();
}

enum{ALL,TIME,WEEK};

Dialog_export::~Dialog_export()
{
    delete jsonhandler;
    delete ui;
}

void Dialog_export::set_comboBox_time()
{
    ui->comboBox_time_export->clear();
    ui->comboBox_time_export->addItem("all");

    for(int i = 0; i < workoutCount.count();++i)
    {
        ui->comboBox_time_export->addItem(modelSchedule->item(modelSchedule->indexFromItem(workoutCount.at(i)).row(),2)->text());
    }
}

void Dialog_export::set_filecontent(QModelIndex index)
{
    QDate workoutDate;
    QTime workoutTime;
    QDateTime workoutDateTime;
    QString tempDate,tempTime,sport,stressType;

    tempDate = modelSchedule->item(index.row(),1)->text();
    workoutDate = QDate::fromString(tempDate,"dd.MM.yyyy");

    tempTime = modelSchedule->item(index.row(),2)->text();
    workoutTime = QTime::fromString(tempTime,"hh:mm");

    fileName = workoutDate.toString("yyyy_MM_dd_") + workoutTime.toString("hh_mm_ss") +".json";
    workoutDateTime = QDateTime::fromString(tempDate+"T"+tempTime+":00","dd.MM.yyyyThh:mm:ss").toUTC();

    sport = modelSchedule->item(index.row(),3)->text();
    if(sport == settings::isSwim) stressType = "swimscore";
    if(sport == settings::isBike) stressType = "skiba_bike_score";
    if(sport == settings::isRun) stressType = "govss";
    if(sport == settings::isAlt || sport == settings::isStrength) stressType = "triscore";

    jsonhandler->reset_maps();
    jsonhandler->set_filename(fileName);
    jsonhandler->set_rideData("STARTTIME",workoutDateTime.toString("yyyy/MM/dd hh:mm:ss UTC"));
    jsonhandler->set_rideData("RECINTSECS","");
    jsonhandler->set_rideData("DEVICETYPE","Manual");
    jsonhandler->set_rideData("IDENTIFIER","");
    jsonhandler->set_rideData("OVERRIDES","");

    jsonhandler->set_tagData("Sport",sport);
    jsonhandler->set_tagData("Workout Code",modelSchedule->item(index.row(),4)->text());
    jsonhandler->set_tagData("Workout Title",modelSchedule->item(index.row(),5)->text());

    jsonhandler->set_overrideFlag(true);
    jsonhandler->set_overrideData("time_riding",QString::number(settings::get_timesec(modelSchedule->item(index.row(),6)->text())));
    jsonhandler->set_overrideData("workout_time",QString::number(settings::get_timesec(modelSchedule->item(index.row(),6)->text())));
    jsonhandler->set_overrideData(stressType,modelSchedule->item(index.row(),8)->text());

    jsonhandler->write_json();
}

void Dialog_export::workout_export()
{
    fileName = QString();
    fileContent = QString();
    QList<QStandardItem*> list;
    QModelIndex index;

    if(export_mode == WEEK)
    {
        QString weeknumber = ui->comboBox_week_export->currentText();
        list = modelSchedule->findItems(weeknumber,Qt::MatchExactly,0);

        for(int i = 0; i < list.count(); ++i)
        {
            index = modelSchedule->indexFromItem(list.at(i));
            if(modelSchedule->data(modelSchedule->index(index.row(),3,QModelIndex())).toString() != settings::isOther)
            {
                this->set_filecontent(modelSchedule->indexFromItem(list.at(i)));
                ui->progressBar->setValue((100/list.count())*i);
            }
        }
        ui->progressBar->setValue(100);

    }
    if(export_mode == ALL || export_mode == TIME)
    {
        QString workoutdate = ui->dateEdit_export->date().toString("dd.MM.yyyy");
        list = modelSchedule->findItems(workoutdate,Qt::MatchExactly,1);

        for(int i = 0; i < list.count(); ++i)
        {
            index = modelSchedule->indexFromItem(list.at(i));
            if(modelSchedule->data(modelSchedule->index(index.row(),3,QModelIndex())).toString() != settings::isOther)
            {
                if(export_mode == ALL)
                {
                    this->set_filecontent(modelSchedule->indexFromItem(list.at(i)));
                    ui->progressBar->setValue((100/list.count())*i);
                }
                if(export_mode == TIME)
                {
                    QString time_value = modelSchedule->data(modelSchedule->index(modelSchedule->indexFromItem(list.at(i)).row(),2,QModelIndex()),Qt::DisplayRole).toString();
                    if(time_value == ui->comboBox_time_export->currentText())
                    {
                        this->set_filecontent(modelSchedule->indexFromItem(list.at(i)));
                        ui->progressBar->setValue((100/list.count())*i);
                    }
                }
            }
        }
        ui->progressBar->setValue(100);
    }
}

void Dialog_export::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_export::on_pushButton_export_clicked()
{
    this->workout_export();
}

void Dialog_export::set_exportselection(bool b_day,bool b_week)
{
    ui->comboBox_week_export->setEnabled(b_week);
    ui->comboBox_time_export->setEnabled(b_day);
    ui->dateEdit_export->setEnabled(b_day);
}

void Dialog_export::get_exportinfo(QString v_week,QString v_date,bool week)
{
    QList<QStandardItem*> list;
    QModelIndex index;
    int workcount = 0;
    if(week)
    {
        list = modelSchedule->findItems(v_week,Qt::MatchExactly,0);
    }
    else
    {
        list = modelSchedule->findItems(v_date,Qt::MatchExactly,1);
    }

    if(list.count() == 0)
    {
        ui->pushButton_export->setEnabled(false);
    }
    else
    {
        ui->pushButton_export->setEnabled(true);
    }

    if(export_mode == ALL || export_mode == WEEK)
    {
        for(int i = 0; i < list.count(); i++)
        {
            index = modelSchedule->indexFromItem(list.at(i));
            if(modelSchedule->data(modelSchedule->index(index.row(),3,QModelIndex())).toString() != settings::isOther)
            {
                ++workcount;
            }
        }
        this->set_infolabel(workcount);
    }
    else
    {
        this->set_infolabel(1);
    }
}

void Dialog_export::on_radioButton_day_clicked()
{
    export_mode = ALL;
    ui->progressBar->setValue(0);
    this->set_exportselection(true,false);
    this->get_exportinfo("NULL",ui->dateEdit_export->date().toString("dd.MM.yyyy"),false);
}

void Dialog_export::on_radioButton_week_clicked()
{
    export_mode = WEEK;
    ui->progressBar->setValue(0);
    this->set_exportselection(false,true);
    this->get_exportinfo(ui->comboBox_week_export->currentText(),"NULL",true);
}

void Dialog_export::set_infolabel(int value)
{
    ui->label_exportinfo->setText(QString::number(value) + " Workout(s) selected for Export");
}

void Dialog_export::on_comboBox_week_export_currentIndexChanged(const QString &weekvalue)
{
    ui->progressBar->setValue(0);
    this->get_exportinfo(weekvalue,"NULL",true);
}

void Dialog_export::on_dateEdit_export_dateChanged(const QDate &date)
{
    ui->progressBar->setValue(0);
    workoutCount = modelSchedule->findItems(date.toString("dd.MM.yyyy"),Qt::MatchExactly,1);
    this->set_comboBox_time();
    this->get_exportinfo("NULL",date.toString("dd.MM.yyyy"),false);
}

void Dialog_export::on_comboBox_time_export_currentIndexChanged(const QString &value)
{
    ui->progressBar->setValue(0);
    if(value == "all")
    {
        export_mode = ALL;
    }
    else
    {
        export_mode = TIME;
    }
    this->get_exportinfo("NULL",ui->dateEdit_export->date().toString("dd.MM.yyyy"),false);
}
