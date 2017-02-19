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

#include "day_popup.h"
#include "ui_day_popup.h"

day_popup::day_popup(QWidget *parent, const QDate w_date, schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::day_popup)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->frame_edit->hide();
    popupMode = false;
    this->show_workouts(w_date,p_sched);
}

enum {SHOW,EDIT};

day_popup::~day_popup()
{
    delete dayModel;
    delete ui;
}

void day_popup::show_workouts(QDate w_date,schedule *schedP)
{
    QString workoutDate = w_date.toString("dd.MM.yyyy");
    QSortFilterProxyModel *scheduleProxy = new QSortFilterProxyModel();
    scheduleProxy->setSourceModel(schedP->workout_schedule);
    scheduleProxy->setFilterRegExp("\\b"+workoutDate+"\\b");
    scheduleProxy->setFilterKeyColumn(1);
    scheduleProxy->sort(2);
    int workCount = scheduleProxy->rowCount();

    if(workCount == 0)
    {
        ui->toolButton_edit->setEnabled(false);
        return;
    }

    if(workCount > 1)
    {
        this->setFixedWidth(280*workCount);
    }
    else
    {
        this->setFixedWidth(600);
    }

    this->setFixedHeight(260);
    ui->label_weekinfo->setText(workoutDate + " - Phase: " + schedP->get_weekPhase(w_date));
    QStringList workList,workoutHeader;
    workList << "Workout:" << "Sport:" << "Code:" << "Title:" << "Duration:" << "Distance:" << "Stress:" << "Pace:";

    for(int i = 1; i <= workCount; ++i)
    {
        workoutHeader << "Workout " + QString::number(i);
    }

    int worklistCount = workList.count();
    dayModel = new QStandardItemModel(worklistCount,workCount);
    dayModel->setVerticalHeaderLabels(workList);
    dayModel->setHorizontalHeaderLabels(workoutHeader);

    ui->tableView_day->setModel(dayModel);
    ui->tableView_day->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_day->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_day->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_day->horizontalHeader()->setSectionsClickable(false);
    ui->tableView_day->verticalHeader()->setSectionsClickable(false);
    ui->tableView_day->verticalHeader()->setFixedWidth(100);
    ui->tableView_day->setItemDelegate(&daypop_del);


    for(int col = 0; col < workCount; ++col)
    {
        for(int row = 0; row < workList.count(); ++row)
        {
            dayModel->setData(dayModel->index(row,col,QModelIndex()),scheduleProxy->data(scheduleProxy->index(col,row+2)).toString());
            if(row == 7)
            {
                dayModel->setData(dayModel->index(row,col,QModelIndex()),this->get_workout_pace(scheduleProxy->data(scheduleProxy->index(col,7)).toDouble(),
                                                                                                QTime::fromString(scheduleProxy->data(scheduleProxy->index(col,6)).toString(),"hh:mm:ss"),
                                                                                                scheduleProxy->data(scheduleProxy->index(col,3)).toString(),true));
            }
        }
    }
}

void day_popup::on_toolButton_close_clicked()
{
    reject();
}

void day_popup::on_toolButton_edit_clicked()
{
    if(popupMode)
    {
        popupMode = SHOW;
        this->setFixedHeight(260);
        ui->frame_edit->hide();
        ui->tableView_day->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }
    else
    {
        popupMode = EDIT;
        this->setFixedHeight(325);
        ui->frame_edit->show();
        ui->tableView_day->setEditTriggers(QAbstractItemView::CurrentChanged);
    }
}
