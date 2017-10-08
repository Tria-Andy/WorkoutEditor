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
#include <QClipboard>
#include "dialog_pacecalc.h"
#include "ui_dialog_pacecalc.h"

Dialog_paceCalc::Dialog_paceCalc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_paceCalc)
{
    ui->setupUi(this);
    sportList << settings::isSwim << settings::isBike << settings::isRun;
    model_header << "Distance" << "Duration";
    race_header << "Lap" << "Distance" << "Pace" << "Speed" << "Duration";
    dist <<25<<50<<100<<200<<300<<400<<500<<600<<800<<1000;
    distFactor = 1;
    paceModel = new QStandardItemModel(this);
    raceModel = new QStandardItemModel(this);
    triathlonMap = settings::get_triaMap();
    triaDist = settings::get_triaDistance();
    this->init_paceView();
    runRaces << "5K" << "10K" << "HM" << "M";
    raceDist << 5.0 << 10.0 << 21.1 << 42.2;
    ui->comboBox_sport->addItems(sportList);
    ui->comboBox_race->addItem("---");
    for(int i = 0; i < runRaces.count(); ++i)
    {
        ui->comboBox_race->addItem(settings::isRun +" "+runRaces.at(i));
    }
    for(int i = 0; i < triaDist.count(); ++i)
    {
        ui->comboBox_race->addItem(settings::isTria +" "+triaDist.at(i)+ ": "+triathlonMap.value(triaDist.at(i)));
    }
    ui->tableView_raceCalc->setItemDelegate(&race_del);
}

Dialog_paceCalc::~Dialog_paceCalc()
{
    delete ui;
}

void Dialog_paceCalc::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_paceCalc::init_paceView()
{
    paceModel->setHorizontalHeaderLabels(model_header);
    ui->tableView_pace->setModel(paceModel);
    ui->tableView_pace->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_pace->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_pace->verticalHeader()->hide();


    for(int i = 0; i < 10; ++i)
    {
        paceModel->insertRows(i,1,QModelIndex());
        paceModel->setData(paceModel->index(i,0,QModelIndex()),dist[i]);
    }
}

void Dialog_paceCalc::set_pace()
{
    int pace = this->get_timesec(ui->timeEdit_pace->time().toString("mm:ss"));

    for(int i = 0; i < 10; ++i)
    {
        if(ui->comboBox_sport->currentText() == settings::isSwim)
        {
            paceModel->setData(paceModel->index(i,0,QModelIndex()),dist[i]*distFactor);
            paceModel->setData(paceModel->index(i,1,QModelIndex()),this->set_time(static_cast<int>(round(pace * (dist[i]*distFactor)/100))));
        }
        else
        {
            paceModel->setData(paceModel->index(i,0,QModelIndex()),dist[i]*distFactor);
            paceModel->setData(paceModel->index(i,1,QModelIndex()),this->set_time(static_cast<int>(round(pace * (dist[i]*distFactor)/1000))));
        }
    }
}

void Dialog_paceCalc::set_freeField(int dist)
{
    int pace = this->get_timesec(ui->timeEdit_pace->time().toString("mm:ss"));

    if(ui->comboBox_sport->currentText() == settings::isSwim)
    {
        ui->lineEdit_dura->setText(this->set_time(pace * dist/100));
    }
    else
    {
        ui->lineEdit_dura->setText(this->set_time(pace * dist/1000));
    }
}

void Dialog_paceCalc::on_comboBox_sport_currentTextChanged(const QString &sport)
{
    if(sport == settings::isSwim)
    {
        ui->label_pace->setText("/100m");
    }
    else
    {
        ui->label_pace->setText("/km");
    }

    this->set_pace();
    ui->lineEdit_speed->setText(QString::number(this->get_speed(ui->timeEdit_pace->time(),0,sport,true)));
    this->set_freeField(ui->lineEdit_dist->text().toInt());
}


void Dialog_paceCalc::on_timeEdit_pace_timeChanged(const QTime &time)
{
    ui->lineEdit_speed->setText(QString::number(this->get_speed(time,0,ui->comboBox_sport->currentText(),true)));
    this->set_freeField(ui->lineEdit_dist->text().toInt());
    this->set_pace();
}

void Dialog_paceCalc::on_lineEdit_dist_textChanged(const QString &dist)
{
    this->set_freeField(dist.toInt());
    ui->lineEdit_distkm->setText(QString::number(dist.toDouble()/1000)+" Km");
}

void Dialog_paceCalc::on_spinBox_factor_valueChanged(int value)
{
    distFactor = value;
    this->set_pace();
}

void Dialog_paceCalc::on_timeEdit_intTime_timeChanged(const QTime &time)
{
    ui->lineEdit_IntSpeed->setText(QString::number(this->get_speed(time,ui->spinBox_IntDist->value(),ui->comboBox_sport->currentText(),false)));
}

void Dialog_paceCalc::on_spinBox_IntDist_valueChanged(int dist)
{
    ui->lineEdit_IntSpeed->setText(QString::number(this->get_speed(ui->timeEdit_intTime->time(),dist,ui->comboBox_sport->currentText(),false)));
}

void Dialog_paceCalc::on_pushButton_clicked()
{
    ui->comboBox_sport->setCurrentIndex(0);
    ui->spinBox_factor->setValue(1);
    ui->timeEdit_pace->setTime(QTime::fromString("00:00","mm:ss"));
    ui->timeEdit_intTime->setTime(QTime::fromString("00:00","mm:ss"));
    ui->spinBox_IntDist->setValue(1);
    ui->lineEdit_IntSpeed->setText("");
}

void Dialog_paceCalc::on_toolButton_copy_clicked()
{
    QClipboard *clipboard = QApplication::clipboard();
    QByteArray speedArray;
    QMimeData *mimeData = new QMimeData();
    double speed = ui->lineEdit_IntSpeed->text().toDouble();
    int sec = this->get_timesec(ui->timeEdit_intTime->time().toString("mm:ss"));

    for(int i = 0; i < sec; ++i)
    {
        speedArray.append(QString::number(speed));
        speedArray.append("\r\n");
    }
    mimeData->setData("text/plain",speedArray);
    clipboard->setMimeData(mimeData);
}

void Dialog_paceCalc::set_raceTable(QString label)
{
    if(label.contains(settings::isSwim))
    {

    }
    else if(label.contains(settings::isBike))
    {

    }
    else if(label.contains(settings::isRun))
    {
        raceModel->insertRow(0,QModelIndex());
        raceModel->setData(raceModel->index(0,0,QModelIndex()),settings::isRun);
        int raceIndex = runRaces.indexOf(label.split(" ").at(1));
        raceModel->setData(raceModel->index(0,1,QModelIndex()),raceDist[raceIndex]);
    }
    else if(label.contains(settings::isTria))
    {
        QString vtria = label.split(" ").at(1);
        QStringList triaDistance;
        triaDistance << triathlonMap.value(vtria.remove(":")).split("-");

        raceModel->insertRows(0,5,QModelIndex());
        raceModel->setData(raceModel->index(0,0,QModelIndex()),settings::isSwim);
        raceModel->setData(raceModel->index(0,1,QModelIndex()),triaDistance.at(0));
        raceModel->setData(raceModel->index(1,0,QModelIndex()),"T1");
        raceModel->setData(raceModel->index(1,1,QModelIndex()),0.0);
        raceModel->setData(raceModel->index(2,0,QModelIndex()),settings::isBike);
        raceModel->setData(raceModel->index(2,1,QModelIndex()),triaDistance.at(1));
        raceModel->setData(raceModel->index(3,0,QModelIndex()),"T2");
        raceModel->setData(raceModel->index(3,1,QModelIndex()),0.0);
        raceModel->setData(raceModel->index(4,0,QModelIndex()),settings::isRun);
        raceModel->setData(raceModel->index(4,1,QModelIndex()),triaDistance.at(2));

    }
}

void Dialog_paceCalc::on_comboBox_race_currentIndexChanged(int index)
{
    raceModel->clear();
    raceModel->setHorizontalHeaderLabels(race_header);
    ui->tableView_raceCalc->setModel(raceModel);
    ui->tableView_raceCalc->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_raceCalc->verticalHeader()->hide();

    int rowCount = 0;

    if(index != 0)
    {
        this->set_raceTable(ui->comboBox_race->currentText());

        rowCount = raceModel->rowCount();

        if(rowCount > 1)
        {
            double sum  = 0.0;
            raceModel->insertRow(rowCount,QModelIndex());
            for(int i = 0; i < rowCount;++i)
            {
                sum += raceModel->data(raceModel->index(i,1)).toDouble();
            }
            raceModel->setData(raceModel->index(rowCount,0,QModelIndex()),"Sum");
            raceModel->setData(raceModel->index(rowCount,1,QModelIndex()),sum);
            raceModel->setData(raceModel->index(rowCount,2,QModelIndex()),0);
            raceModel->setData(raceModel->index(rowCount,3,QModelIndex()),0);
            rowCount = raceModel->rowCount();
        }
        if(rowCount > 3)
        {
            ui->tableView_raceCalc->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        }
        else
        {
            ui->tableView_raceCalc->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        }

        for(int i = 0; i < rowCount; ++i)
        {
            for(int x = 2; x < raceModel->columnCount(); ++x)
            {
                raceModel->setData(raceModel->index(i,x,QModelIndex()),"00:00");
            }
            raceModel->setData(raceModel->index(i,3,QModelIndex()),0.0);
        }
    }
    else
    {
        raceModel->clear();
    }
}
