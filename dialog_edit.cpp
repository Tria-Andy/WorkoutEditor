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

#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include "dialog_edit.h"
#include "ui_dialog_edit.h"

Dialog_edit::Dialog_edit(QWidget *parent, const QDate w_date, schedule *p_sched,standardWorkouts *p_stdworkout) :
    QDialog(parent),
    ui(new Ui::Dialog_edit)
{
    ui->setupUi(this);
    workSched = p_sched;
    std_workouts = p_stdworkout;
    ui->comboBox_wcode->addItems(settings::get_listValues("WorkoutCode"));
    this->set_workout_info(w_date);

    connect(ui->dateEdit_edit_date, SIGNAL(dateChanged(QDate)),this, SLOT(set_edit_calweek()));
}

enum {EDIT,COPY,DEL};

Dialog_edit::~Dialog_edit()
{
    delete ui;
}

void Dialog_edit::set_workout_info(const QDate &date)
{
    QModelIndex index;
    ui->dateEdit_workoutdate->setDate(date);

    list = workSched->workout_schedule->findItems(date.toString("dd.MM.yyyy"),Qt::MatchExactly,1);

    for(int i = 0; i < list.count();++i)
    {
        index = workSched->workout_schedule->indexFromItem(list.at(i));
        ui->comboBox_time->addItem(workSched->workout_schedule->item(index.row(),2)->text());
    }
}

void Dialog_edit::show_workout(int c_index)
{
    curr_index = workSched->workout_schedule->indexFromItem(list.at(c_index));

    ui->dateEdit_edit_date->setDate(QDate::fromString(workSched->workout_schedule->item(curr_index.row(),1)->text(),"dd.MM.yyyy"));
    ui->timeEdit_edit_time->setTime(QTime::fromString(workSched->workout_schedule->item(curr_index.row(),2)->text()));
    ui->lineEdit_cweek->setText(QString::number(QDate::fromString(workSched->workout_schedule->item(curr_index.row(),1)->text(),"dd.MM.yyyy").weekNumber()));
    ui->lineEdit_phase->setText(workSched->get_weekPhase(QDate::fromString(workSched->workout_schedule->item(curr_index.row(),1)->text(),"dd.MM.yyyy")));
    ui->lineEdit_sport->setText(workSched->workout_schedule->item(curr_index.row(),3)->text());
    ui->comboBox_wcode->setCurrentText(workSched->workout_schedule->item(curr_index.row(),4)->text());
    ui->lineEdit_wtitle->setText(workSched->workout_schedule->item(curr_index.row(),5)->text());
    ui->timeEdit_duration->setTime(QTime::fromString(workSched->workout_schedule->item(curr_index.row(),6)->text()));
    ui->doubleSpinBox_distance->setValue(workSched->workout_schedule->item(curr_index.row(),7)->text().toDouble());
    ui->label_speed->setText(this->get_workout_pace(ui->doubleSpinBox_distance->value(),ui->timeEdit_duration->time(),ui->lineEdit_sport->text(),false));
    ui->spinBox_stress->setValue(workSched->workout_schedule->item(curr_index.row(),8)->text().toInt());
}

void Dialog_edit::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_edit::on_comboBox_time_currentIndexChanged(int index)
{
    this->show_workout(index);
}

void Dialog_edit::set_edit_calweek()
{
    ui->lineEdit_cweek->setText(QString::number(ui->dateEdit_edit_date->date().weekNumber()));
    ui->lineEdit_phase->setText(workSched->get_weekPhase(ui->dateEdit_edit_date->date()));
}

void Dialog_edit::set_workout_data()
{
    QDate newDate = ui->dateEdit_edit_date->date();
    workSched->set_workout_date(ui->dateEdit_edit_date->date().toString("dd.MM.yyyy"));
    workSched->set_workout_time(ui->timeEdit_edit_time->time().toString("hh:mm"));
    workSched->set_workout_calweek(ui->lineEdit_cweek->text()+"_"+QString::number(newDate.addDays(1 - ui->dateEdit_edit_date->date().dayOfWeek()).year()));
    workSched->set_workout_sport(ui->lineEdit_sport->text());
    workSched->set_workout_code(ui->comboBox_wcode->currentText());
    workSched->set_workout_title(ui->lineEdit_wtitle->text());
    workSched->set_workout_duration(ui->timeEdit_duration->time().toString("hh:mm:ss"));
    workSched->set_workout_distance(ui->doubleSpinBox_distance->value());
    workSched->set_workout_stress(ui->spinBox_stress->value());
}


void Dialog_edit::set_result(QString result_text,int result_code)
{
    QMessageBox::StandardButton reply = QMessageBox::No;

    if(result_code == EDIT)reply = QMessageBox::warning(this,result_text + " Workout",result_text + " Workout?",QMessageBox::Yes|QMessageBox::No);
    if(result_code == COPY)reply = QMessageBox::question(this,result_text + " Workout",result_text + " Workout?",QMessageBox::Yes|QMessageBox::No);
    if(result_code == DEL) reply = QMessageBox::critical(this,result_text + " Workout",result_text + " Workout?",QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        edit_result = result_code;
        accept();
    }
}

void Dialog_edit::on_pushButton_edit_clicked()
{
    this->set_workout_data();
    this->set_result("Edit",EDIT);
}

void Dialog_edit::on_pushButton_copy_clicked()
{
    this->set_workout_data();
    this->set_result("Copy",COPY);
}

void Dialog_edit::on_pushButton_delete_clicked()
{
    this->set_result("Delete",DEL);
}

void Dialog_edit::on_doubleSpinBox_distance_valueChanged(double dist)
{
    ui->label_speed->setText(this->get_workout_pace(dist,ui->timeEdit_duration->time(),ui->lineEdit_sport->text(),false));
}

void Dialog_edit::on_timeEdit_duration_timeChanged(const QTime &time)
{
    ui->label_speed->setText(this->get_workout_pace(ui->doubleSpinBox_distance->value(),time,ui->lineEdit_sport->text(),false));
}

void Dialog_edit::on_dateEdit_edit_date_dateChanged(const QDate &date)
{
    bool sameDate;
    QList<QStandardItem*> workCount = workSched->workout_schedule->findItems(date.toString("dd.MM.yyyy"),Qt::MatchExactly,1);

    if(ui->dateEdit_workoutdate->date() == ui->dateEdit_edit_date->date())
    {
        sameDate = true;
    }
    else
    {
        sameDate = false;
    }

    if(workCount.count() == 3 && !sameDate)
    {
        ui->pushButton_copy->setEnabled(false);
        ui->pushButton_edit->setEnabled(false);
    }
    else
    {
        ui->pushButton_copy->setEnabled(true);
        ui->pushButton_edit->setEnabled(true);
    }
}

void Dialog_edit::on_toolButton_workouts_clicked()
{
    Dialog_workouts stdWorkouts(this,ui->lineEdit_sport->text(),std_workouts);
    stdWorkouts.setModal(true);
    int returnCode = stdWorkouts.exec();
    if(returnCode == QDialog::Accepted)
    {
        ui->comboBox_wcode->setCurrentText(stdWorkouts.get_workout_code());
        ui->lineEdit_wtitle->setText(stdWorkouts.get_workout_title());
        ui->timeEdit_duration->setTime(stdWorkouts.get_workout_duration());
        ui->doubleSpinBox_distance->setValue(stdWorkouts.get_workout_distance());
        ui->spinBox_stress->setValue(stdWorkouts.get_workout_stress());
    }
}
