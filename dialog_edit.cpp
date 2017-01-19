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
    scheduleProxy = new QSortFilterProxyModel();
    scheduleProxy->setSourceModel(p_sched->workout_schedule);
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
    ui->dateEdit_workoutdate->setDate(date);
    scheduleProxy->setFilterRegExp("\\b"+date.toString("dd.MM.yyyy")+"\\b");
    scheduleProxy->setFilterKeyColumn(1);

    for(int i = 0; i < scheduleProxy->rowCount();++i)
    {
        ui->comboBox_time->addItem(scheduleProxy->data(scheduleProxy->index(i,2)).toString());
    }
    this->controlButtons(date);
}

void Dialog_edit::show_workout(int row)
{
    curr_index = scheduleProxy->mapToSource(scheduleProxy->index(row,2));
    QDate workDate = QDate::fromString(scheduleProxy->data(scheduleProxy->index(row,1)).toString(),"dd.MM.yyyy");

    ui->dateEdit_edit_date->setDate(workDate);
    ui->timeEdit_edit_time->setTime(QTime::fromString(scheduleProxy->data(scheduleProxy->index(row,2)).toString()));
    ui->lineEdit_cweek->setText(QString::number(workDate.weekNumber()));
    ui->lineEdit_phase->setText(workSched->get_weekPhase(workDate));
    ui->lineEdit_sport->setText(scheduleProxy->data(scheduleProxy->index(row,3)).toString());
    ui->comboBox_wcode->setCurrentText(scheduleProxy->data(scheduleProxy->index(row,4)).toString());
    ui->lineEdit_wtitle->setText(scheduleProxy->data(scheduleProxy->index(row,5)).toString());
    ui->timeEdit_duration->setTime(QTime::fromString(scheduleProxy->data(scheduleProxy->index(row,6)).toString(),"hh:mm:ss"));
    ui->doubleSpinBox_distance->setValue(scheduleProxy->data(scheduleProxy->index(row,7)).toString().toDouble());
    ui->label_speed->setText(this->get_workout_pace(ui->doubleSpinBox_distance->value(),ui->timeEdit_duration->time(),ui->lineEdit_sport->text(),false));
    ui->spinBox_stress->setValue(scheduleProxy->data(scheduleProxy->index(row,8)).toString().toInt());
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

void Dialog_edit::set_workout_data(bool completeDay)
{    
    QDate newDate = ui->dateEdit_edit_date->date();
    QString weekNumber = QString::number(newDate.weekNumber());
    weekNumber = weekNumber+"_"+QString::number(newDate.addDays(1 - ui->dateEdit_edit_date->date().dayOfWeek()).year());
    QStringList addValues;
    QModelIndex modIndex;

    if(completeDay)
    {
        for(int row = 0; row < scheduleProxy->rowCount(); ++row)
        {
            addValues.clear();
            addValues << weekNumber << newDate.toString("dd.MM.yyyy");
            modIndex = scheduleProxy->mapToSource(scheduleProxy->index(row,0));
            for(int col = 2; col < scheduleProxy->columnCount(); ++col)
            {
                addValues << scheduleProxy->data(scheduleProxy->index(row,col)).toString();
            }
            workSched->add_itemList(modIndex,addValues);
        }
    }
    else
    {
        addValues << weekNumber
                  << newDate.toString("dd.MM.yyyy")
                  << ui->timeEdit_edit_time->time().toString("hh:mm")
                  << ui->lineEdit_sport->text()
                  << ui->comboBox_wcode->currentText()
                  << ui->lineEdit_wtitle->text()
                  << ui->timeEdit_duration->time().toString("hh:mm:ss")
                  << QString::number(ui->doubleSpinBox_distance->value())
                  << QString::number(ui->spinBox_stress->value());

        workSched->add_itemList(curr_index,addValues);
    }
}

void Dialog_edit::controlButtons(QDate date)
{
    int count = workSched->check_workouts(date);

    if(count == 3)
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

void Dialog_edit::set_result(QString result_text,int result_code)
{
    QString stMessage;
    bool completeDay = ui->checkBox_complete->isChecked();
    QMessageBox::StandardButton reply = QMessageBox::No;

    if(completeDay)
    {
        stMessage = " Complete Day";
    }
    else
    {
        stMessage = " Workout";
    }

    if(result_code == EDIT)reply = QMessageBox::warning(this,result_text + stMessage,result_text + stMessage+"?",QMessageBox::Yes|QMessageBox::No);
    if(result_code == COPY)reply = QMessageBox::question(this,result_text + stMessage,result_text + stMessage+"?",QMessageBox::Yes|QMessageBox::No);
    if(result_code == DEL) reply = QMessageBox::critical(this,result_text + stMessage,result_text + stMessage+"?",QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        if(result_code == EDIT)
        {
            this->set_workout_data(completeDay);
            workSched->set_workoutData(EDIT);
        }
        if(result_code == COPY)
        {
            this->set_workout_data(completeDay);
            workSched->set_workoutData(COPY);
        }
        if(result_code == DEL)
        {
            if(completeDay)
            {
                QString deleteDate = scheduleProxy->data(scheduleProxy->index(0,1)).toString();
                QList<QStandardItem*> deleteList = workSched->workout_schedule->findItems(deleteDate,Qt::MatchExactly,1);
                for(int i = 0; i < deleteList.count();++i)
                {
                    workSched->delete_workout(workSched->workout_schedule->indexFromItem(deleteList.at(i)));
                }
            }
            else
            {
                workSched->delete_workout(curr_index);
            }
        }
        accept();
    }
}

void Dialog_edit::on_pushButton_edit_clicked()
{
    this->set_result("Edit",EDIT);
}

void Dialog_edit::on_pushButton_copy_clicked()
{
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
    this->controlButtons(date);
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

void Dialog_edit::on_checkBox_complete_toggled(bool checked)
{
    ui->comboBox_wcode->setEnabled(!checked);
    ui->comboBox_wcode->setCurrentIndex(0);
    ui->lineEdit_sport->setText("-");
    ui->lineEdit_sport->setEnabled(!checked);
    ui->lineEdit_wtitle->setText("-");
    ui->lineEdit_wtitle->setEnabled(!checked);
    ui->timeEdit_duration->setTime(QTime::fromString("00:00:00","hh:mm:ss"));
    ui->timeEdit_duration->setEnabled(!checked);
    ui->doubleSpinBox_distance->setValue(0.0);
    ui->doubleSpinBox_distance->setEnabled(!checked);
    ui->label_speed->setText("-");
    ui->label_speed->setEnabled(!checked);
    ui->spinBox_stress->setValue(0);
    ui->spinBox_stress->setEnabled(!checked);
    ui->comboBox_time->setEnabled(!checked);
    ui->timeEdit_edit_time->setEnabled(!checked);
    if(!checked) this->show_workout(ui->comboBox_time->currentIndex());
}
