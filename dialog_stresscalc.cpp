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

    combo_sport << "--" << settings::SwimLabel << settings::BikeLabel << settings::RunLabel;
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
    double stressScore,speed,totalWork,current;
    double pValue = 0.0;
    int timeSec = 0;
    QString xPace;
    int thresBase;

    if(isSwim)
    {
        current = this->get_timesec(ui->lineEdit_goal_power->text());
        pValue = current;
        current = pow(thresPace / current,3.0);
        ui->lineEdit_intensity->setText(QString::number(current));
        thresBase = 0;
    }
    if(isBike)
    {   
        current = ui->lineEdit_goal_power->text().toDouble();
        pValue = current;
        ui->lineEdit_intensity->setText(QString::number(current / thresPower));
        thresBase = 1;
    }
    if(isRun)
    {
        xPace = ui->lineEdit_goal_power->text();
        speed = this->get_speed(QTime::fromString(xPace,"mm:ss"),0,true);
        pValue = speed;
        current = this->calc_lnp(speed/3.6,athleteValues->value("height"),athleteValues->value("weight")) / thresPower;
        ui->lineEdit_intensity->setText(QString::number(current));
        thresBase = 0;
    }
    timeSec = get_secFromTime(ui->timeEdit_duration->time());
    stressScore = this->calc_stressScore(thresBase,timeSec);
    totalWork = round(this->calc_totalWork(pValue,timeSec,6));
    ui->lineEdit_stressScore->setText(QString::number(stressScore));
    ui->lineEdit_work->setText(QString::number(totalWork));
}

void Dialog_stresscalc::set_sport_threshold()
{
    if(isSwim)
    {
        thresPower = thresValues->value("swimpower");
        thresPace = thresValues->value("swimpace");
        ui->label_threshold->setText("Threshold Pace:");
        ui->label_workout_power->setText("Workout Pace:");
        ui->lineEdit_power->setText(this->set_time(thresPace));
        ui->lineEdit_goal_power->setInputMask("00:00");
    }
    if(isBike)
    {
        thresPower = thresValues->value("bikepower");
        ui->label_threshold->setText("Threshold Power:");
        ui->label_workout_power->setText("Workout Power:");
        ui->lineEdit_power->setText(QString::number(thresPower));
        ui->lineEdit_goal_power->setInputMask("000");
    }
    if(isRun)
    {
        thresPower = thresValues->value("runpower");
        thresPace = thresValues->value("runpace");
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
    ui->lineEdit_work->clear();
}

void Dialog_stresscalc::on_pushButton_calc_clicked()
{
    this->estimateStress();
}

void Dialog_stresscalc::on_pushButton_clear_clicked()
{
    ui->comboBox_sport->setCurrentIndex(0);
    this->reset_calc();
}

void Dialog_stresscalc::on_comboBox_sport_currentIndexChanged(const QString &sport)
{
    this->reset_calc();
    this->set_currentSport(sport);
    this->set_sport_threshold();
}
