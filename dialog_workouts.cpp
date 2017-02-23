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

Dialog_workouts::Dialog_workouts(QWidget *parent, QString v_sport) :
    QDialog(parent),
    ui(new Ui::Dialog_workouts)
{
    ui->setupUi(this);
    workoutSport = v_sport;
    metaProxy = new QSortFilterProxyModel;
    metaProxy->setSourceModel(this->workouts_meta);
    listModel = new QStandardItemModel;
    this->create_workout_list();
}

Dialog_workouts::~Dialog_workouts()
{
    delete metaProxy;
    delete listModel;
    delete ui;
}

void Dialog_workouts::on_pushButton_ok_clicked()
{
    accept();
}

void Dialog_workouts::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_workouts::create_workout_list()
{
    QString workID,workTitle,listString;
    metaProxy->setFilterFixedString(workoutSport);
    metaProxy->setFilterKeyColumn(0);

    listModel->setRowCount(metaProxy->rowCount());
    listModel->setColumnCount(2);

    for(int i = 0; i < metaProxy->rowCount(); ++i)
    {
        workID = metaProxy->data(metaProxy->index(i,1)).toString();
        workTitle = metaProxy->data(metaProxy->index(i,3)).toString();
        listString = metaProxy->data(metaProxy->index(i,2)).toString() + " - " + workTitle;
        listModel->setData(listModel->index(i,0,QModelIndex()),listString);
        listModel->setData(listModel->index(i,1,QModelIndex()),workID);
    }
    listModel->sort(0);

    ui->label_sport->setText(workoutSport + " Workouts");
    ui->listView_workouts->setModel(listModel);
    ui->listView_workouts->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void Dialog_workouts::on_listView_workouts_clicked(const QModelIndex &index)
{
    QString workoutTitle = listModel->data(listModel->index(index.row(),0)).toString();
    QString workoutID = listModel->data(listModel->index(index.row(),1)).toString();
    metaProxy->setFilterRegExp("\\b"+workoutID+"\\b");
    metaProxy->setFilterKeyColumn(1);

    //"sport" << "id" << "code" << "title" << "duration" << "distance" << "stress";
    for(int i = 0; i < metaProxy->columnCount()-1; ++i)
    {
        workData.insert(i,metaProxy->data(metaProxy->index(0,i+1)).toString());
    }

    ui->label_selected->setText(workoutTitle + " - " + workData.value(3) + " - " + workData.value(4) + " - " + workData.value(5));
    ui->pushButton_ok->setEnabled(true);
}
