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

#include "dialog_stresscalc.h"
#include "ui_dialog_stresscalc.h"
#include <QDebug>

Dialog_stresscalc::Dialog_stresscalc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_stresscalc)
{
    ui->setupUi(this);
    combo_sport << "--" << settings::isSwim << settings::isBike << settings::isRun;
    ui->comboBox_sport->addItems(combo_sport);
    lab_power = "Threshold ----:";
    lab_workout = "Workout ----:";
    ui->lineEdit_power->setText("0");
    ui->label_threshold->setText(lab_power);
    ui->label_workout_power->setText(lab_workout);
}

Dialog_stresscalc::~Dialog_stresscalc()
{
    delete ui;
}

void Dialog_stresscalc::estimateStress()
{
    double stressScore,current;

    if(sport == settings::isSwim)
    {
        current = this->get_timesec(ui->lineEdit_goal_power->text());
        current = thresPace / current;
        current = pow(current,3.0);
        ui->lineEdit_intensity->setText(QString::number(current));
    }
    if(sport == settings::isBike)
    {   
        current = ui->lineEdit_goal_power->text().toDouble();
        ui->lineEdit_intensity->setText(QString::number(current / thresPower));
    }
    if(sport == settings::isRun)
    {
        current = this->get_timesec(ui->lineEdit_goal_power->text());
        current = thresPace / current;
        ui->lineEdit_intensity->setText(QString::number(current));
    }
    stressScore = this->estimate_stress(sport,ui->lineEdit_goal_power->text(),this->get_timesec(ui->timeEdit_duration->time().toString("hh:mm:ss")));
    ui->lineEdit_stressScore->setText(QString::number(stressScore));

}

void Dialog_stresscalc::set_sport_threshold()
{
    if(sport == settings::isSwim)
    {
        thresPower = settings::get_thresValue("swimpower");
        thresPace = settings::get_thresValue("swimpace");
        ui->label_threshold->setText("Threshold Pace:");
        ui->label_workout_power->setText("Workout Pace:");
        ui->lineEdit_power->setText(this->set_time(thresPace));
        ui->lineEdit_goal_power->setInputMask("00:00");
    }
    if(sport == settings::isBike)
    {
        thresPower = settings::get_thresValue("bikepower");
        ui->label_threshold->setText("Threshold Power:");
        ui->label_workout_power->setText("Workout Power:");
        ui->lineEdit_power->setText(QString::number(thresPower));
        ui->lineEdit_goal_power->setInputMask("000");
    }
    if(sport == settings::isRun)
    {
        thresPower = settings::get_thresValue("runpower");
        thresPace = settings::get_thresValue("runpace");
        ui->label_threshold->setText("Threshold Pace:");
        ui->label_workout_power->setText("Workout Pace:");
        ui->lineEdit_power->setText(this->set_time(thresPace));
        ui->lineEdit_goal_power->setInputMask("00:00");
    }
}

void Dialog_stresscalc::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_stresscalc::reset_calc()
{
    ui->label_threshold->setText(lab_power);
    ui->label_workout_power->setText(lab_workout);
    ui->timeEdit_duration->setTime(QTime::fromString("00:00:00"));
    ui->lineEdit_power->clear();
    ui->lineEdit_stressScore->clear();
    ui->lineEdit_goal_power->clear();
    ui->lineEdit_intensity->clear();
}

void Dialog_stresscalc::on_pushButton_calc_clicked()
{
    this->estimateStress();
}

void Dialog_stresscalc::on_comboBox_sport_currentIndexChanged(int index)
{
    this->reset_calc();
    sport_index = index;
    sport = ui->comboBox_sport->currentText();
    this->set_sport_threshold();
}


void Dialog_stresscalc::on_pushButton_clear_clicked()
{
    ui->comboBox_sport->setCurrentIndex(0);
    this->reset_calc();

}
