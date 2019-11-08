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


    QDate currDate = QDate().currentDate();
    exportProxy->setFilterRegExp("\\b"+currDate.addDays(1 - currDate.dayOfWeek()).toString(dateFormat)+"\\b");
    exportProxy->setFilterKeyColumn(3);
    exportProxy->sort(0);
    int weekID = exportProxy->data(exportProxy->index(0,0)).toInt()-1;

    exportProxy->invalidate();
    exportMode = 0;

    ui->dateEdit_export->setDate(currDate);
    ui->comboBox_week_export->setEnabled(false);
    ui->comboBox_week_export->setEnabled(false);
    this->set_comboBox_time();
}

enum{ALL,TIME,WEEK};

Dialog_export::~Dialog_export()
{
    delete ui;
}

void Dialog_export::set_comboBox_time()
{
    ui->comboBox_time_export->clear();
    ui->comboBox_time_export->addItem("all");

    for(int i = 0; i < exportProxy->rowCount();++i)
    {
        ui->comboBox_time_export->addItem(exportProxy->data(exportProxy->index(i,2)).toString());
    }
}

void Dialog_export::set_filecontent(int row)
{
    QDate workoutDate;
    QTime workoutTime;
    QDateTime workoutDateTime;
    QString tempDate,tempTime,sport,stressType;

    tempDate = exportProxy->data(exportProxy->index(row,1)).toString();
    workoutDate = QDate::fromString(tempDate,dateFormat);

    tempTime = exportProxy->data(exportProxy->index(row,2)).toString();
    workoutTime = QTime::fromString(tempTime,"hh:mm");

    fileName = workoutDate.toString("yyyy_MM_dd_") + workoutTime.toString("hh_mm_ss") +".json";
    workoutDateTime = QDateTime::fromString(tempDate+"T"+tempTime+":00","dd.MM.yyyyThh:mm:ss").toUTC();

    sport = exportProxy->data(exportProxy->index(row,3)).toString();
    tempTime = exportProxy->data(exportProxy->index(row,6)).toString();

    if(sport == settings::isSwim) stressType = "swimscore";
    if(sport == settings::isBike) stressType = "skiba_bike_score";
    if(sport == settings::isRun) stressType = "govss";
    if(sport == settings::isAlt || sport == settings::isStrength) stressType = "triscore";


    this->rideData.insert("STARTTIME",workoutDateTime.toString("yyyy/MM/dd hh:mm:ss UTC"));
    this->rideData.insert("RECINTSECS","");
    this->rideData.insert("DEVICETYPE","Manual");
    this->rideData.insert("IDENTIFIER","");
    this->rideData.insert("OVERRIDES","");

    this->tagData.insert("Sport",sport);
    this->tagData.insert("Workout Code",exportProxy->data(exportProxy->index(row,4)).toString());
    this->tagData.insert("Workout Title",exportProxy->data(exportProxy->index(row,5)).toString());

    this->hasOverride = true;
    overrideData.insert("time_riding",QString::number(this->get_timesec(tempTime)));
    overrideData.insert("workout_time",QString::number(this->get_timesec(tempTime)));
    overrideData.insert(stressType,exportProxy->data(exportProxy->index(row,8)).toString());

    this->init_jsonFile();
    this->write_jsonFile();
}

void Dialog_export::workout_export()
{
    fileName = QString();
    fileContent = QString();

    for(int i = 0; i < exportProxy->rowCount(); ++i)
    {
        if(exportProxy->data(exportProxy->index(i,3,QModelIndex())).toString() != settings::isOther)
        {
            if(exportMode == ALL || exportMode == WEEK)
            {
                this->set_filecontent(i);
            }
            if(exportMode == TIME)
            {
                if(ui->comboBox_time_export->currentText() == exportProxy->data(exportProxy->index(i,2)).toString())
                {
                   this->set_filecontent(i);
                }
            }
            ui->progressBar->setValue((100/exportProxy->rowCount()*i));
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

void Dialog_export::get_exportinfo(QString filterValue,int filterCol)
{
    int workcount = 0;
    int workouts = 0;

    exportProxy->setFilterRegExp("\\b"+filterValue+"\\b");
    exportProxy->setFilterKeyColumn(filterCol);
    workouts = exportProxy->rowCount();

    if(workouts == 0)
    {
        ui->pushButton_export->setEnabled(false);
    }
    else
    {
        ui->pushButton_export->setEnabled(true);
    }

    if(exportMode == ALL || exportMode == WEEK)
    {
        for(int i = 0; i < workouts; i++)
        {
            if(exportProxy->data(exportProxy->index(i,3,QModelIndex())).toString() != settings::isOther)
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
    exportMode = ALL;
    ui->progressBar->setValue(0);
    this->set_exportselection(true,false);
    this->get_exportinfo(ui->dateEdit_export->date().toString(dateFormat),1);
}

void Dialog_export::on_radioButton_week_clicked()
{
    exportMode = WEEK;
    ui->progressBar->setValue(0);
    this->set_exportselection(false,true);
    this->get_exportinfo(ui->comboBox_week_export->currentText(),0);
}

void Dialog_export::set_infolabel(int value)
{
    ui->label_exportinfo->setText(QString::number(value) + " Workout(s) selected for Export");
}

void Dialog_export::on_comboBox_week_export_currentIndexChanged(const QString &weekvalue)
{
    ui->progressBar->setValue(0);
    this->get_exportinfo(weekvalue,0);
}

void Dialog_export::on_dateEdit_export_dateChanged(const QDate &date)
{
    QString selDate = date.toString(dateFormat);
    exportProxy->setFilterRegExp("\\b"+selDate+"\\b");
    exportProxy->setFilterKeyColumn(1);
    ui->progressBar->setValue(0);
    this->set_comboBox_time();
    this->get_exportinfo(selDate,1);
}

void Dialog_export::on_comboBox_time_export_currentIndexChanged(const QString &value)
{
    ui->progressBar->setValue(0);
    if(value == "all")
    {
        exportMode = ALL;
    }
    else
    {
        exportMode = TIME;
    }
    this->get_exportinfo(ui->dateEdit_export->date().toString(dateFormat),1);
}
