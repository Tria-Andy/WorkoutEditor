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

#include "dialog_workouts.h"
#include "ui_dialog_workouts.h"
#include <QDebug>

Dialog_workouts::Dialog_workouts(QWidget *parent, QString v_sport, standardWorkouts *p_workouts) :
    QDialog(parent),
    ui(new Ui::Dialog_workouts)
{
    ui->setupUi(this);
    w_sport = v_sport;
    stdWorkouts = p_workouts;
    this->create_workout_list();
}

Dialog_workouts::~Dialog_workouts()
{
    delete workout_model;
    delete ui;
}

void Dialog_workouts::create_workout_list()
{
    QStandardItemModel *std_model;
    std_model = stdWorkouts->workouts_meta;

    QModelIndex model_index,index;
    QList<QStandardItem*> list = std_model->findItems(w_sport,Qt::MatchExactly,0);
    workout_model = new QStandardItemModel(list.count(),6,this);

    for(int i = 0; i < list.count(); ++i)
    {
        QString listString;
        model_index = std_model->indexFromItem(list.at(i));
        index = workout_model->index(i,0,QModelIndex());
        listString = std_model->item(model_index.row(),2)->text() + " - " + std_model->item(model_index.row(),3)->text();
        workout_model->setData(index,listString);

        workout_model->setData(workout_model->index(i,1,QModelIndex()),std_model->item(model_index.row(),0)->text());
        workout_model->setData(workout_model->index(i,2,QModelIndex()),std_model->item(model_index.row(),1)->text());
        workout_model->setData(workout_model->index(i,3,QModelIndex()),std_model->item(model_index.row(),4)->text());
        workout_model->setData(workout_model->index(i,4,QModelIndex()),std_model->item(model_index.row(),5)->text());
        workout_model->setData(workout_model->index(i,5,QModelIndex()),std_model->item(model_index.row(),6)->text());
    }
    workout_model->sort(0);
    ui->label_sport->setText(w_sport + " Workouts");
    ui->listView_workouts->setModel(workout_model);
}

void Dialog_workouts::on_pushButton_ok_clicked()
{
    accept();
}

void Dialog_workouts::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_workouts::on_listView_workouts_clicked(const QModelIndex &index)
{
    QStandardItemModel *std_model;
    std_model = stdWorkouts->workouts_meta;
    QModelIndex model_index;

    ui->pushButton_ok->setEnabled(true);

    w_id = workout_model->item(index.row(),2)->text();

    selected_workout = std_model->findItems(w_id,Qt::MatchExactly,1);

    model_index = std_model->indexFromItem(selected_workout.at(0));
    w_sport = std_model->item(model_index.row(),1)->text();
    w_code = std_model->item(model_index.row(),2)->text();
    w_title = std_model->item(model_index.row(),3)->text();
    w_duration = std_model->item(model_index.row(),4)->text();
    w_distance = std_model->item(model_index.row(),5)->text().toDouble();
    w_stress = std_model->item(model_index.row(),6)->text().toInt();

    ui->label_selected->setText(workout_model->item(index.row(),0)->text() + " - " + w_duration + " - " + QString::number(w_distance) + " - " + QString::number(w_stress));
}
