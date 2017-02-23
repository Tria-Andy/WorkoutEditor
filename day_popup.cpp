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
    this->setFixedHeight(260);
    ui->frame_edit->hide();
    popupMode = 0;
    daypop_del.addWork = false;
    popupDate = newDate = w_date;
    workSched = p_sched;
    scheduleProxy = new QSortFilterProxyModel();
    scheduleProxy->setSourceModel(workSched->workout_schedule);

    workListHeader << "Time" << "Sport" << "Code" << "Title" << "Duration" << "Distance" << "Stress" << "Pace";
    dayModel = new QStandardItemModel();
    this->init_dayWorkouts(popupDate);
    ui->toolButton_dayEdit->hide();
    ui->label_workoutInfo->setText(workSched->get_weekPhase(w_date)+" - Week: "+ QString::number(w_date.weekNumber()));

    connect(ui->tableView_day->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(load_workValues(int)));
    connect(ui->dateEdit_workDate,SIGNAL(dateChanged(QDate)),this,SLOT(set_editValues(QDate)));
}

enum {SHOW,EDIT};
enum {ADD,EDITMOVE,COPY,DEL};

day_popup::~day_popup()
{
    delete dayModel;
    delete scheduleProxy;
    delete ui;
}

void day_popup::init_dayWorkouts(QDate workDate)
{
    scheduleProxy->setFilterRegExp("");
    dayModel->clear();

    QString viewBackground = "background-color: #e6e6e6";
    QString workoutDate = workDate.toString("dd.MM.yyyy");

    scheduleProxy->setFilterRegExp("\\b"+workoutDate+"\\b");
    scheduleProxy->setFilterKeyColumn(1);
    scheduleProxy->sort(2);
    int workCount = scheduleProxy->rowCount();
    QStringList workoutHeader;

    if(workCount == 0)
    {
        workoutHeader << "Add";
        this->setFixedWidth(475);
    }
    else
    {
        for(int i = 1; i <= workCount; ++i)
        {
            workoutHeader << "Workout " + QString::number(i);
        }

        if(workCount < 3)
        {
            this->setFixedWidth(250*(workCount+1));
            workoutHeader << "Add";
        }
        else
        {
            this->setFixedWidth(250*(workCount));
        }
    }

    ui->label_weekinfo->setText(workoutDate + " - Phase: " + workSched->get_weekPhase(workDate));

    int worklistCount = workListHeader.count();
    dayModel->setColumnCount(workoutHeader.count());
    dayModel->setRowCount(worklistCount);
    dayModel->setVerticalHeaderLabels(workListHeader);
    dayModel->setHorizontalHeaderLabels(workoutHeader);

    ui->tableView_day->setModel(dayModel);
    ui->tableView_day->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_day->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_day->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_day->horizontalHeader()->setSectionsClickable(false);
    ui->tableView_day->verticalHeader()->setSectionsClickable(false);
    ui->tableView_day->verticalHeader()->setFixedWidth(100);
    ui->tableView_day->setItemDelegate(&daypop_del);
    ui->tableView_day->setStyleSheet(viewBackground);

    for(int col = 0; col < workCount; ++col)
    {
        for(int row = 0; row < workListHeader.count(); ++row)
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

    connect(dayModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(update_workValues()));
    connect(ui->tableView_day->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),this,SLOT(setNextEditRow()));
    ui->dateEdit_workDate->setDate(workDate);
    this->set_controlButtons(false);
    ui->frame_edit->hide();
}

void day_popup::set_controlButtons(bool active)
{
    ui->toolButton_copy->setEnabled(active);
    ui->toolButton_delete->setEnabled(active);
    ui->toolButton_editMove->setEnabled(active);
    ui->toolButton_stdwork->setEnabled(active);
}

void day_popup::set_dayData(bool completeDay)
{
    QString weekNumber = QString::number(newDate.weekNumber());
    weekNumber = weekNumber+"_"+QString::number(newDate.addDays(1 - ui->dateEdit_workDate->date().dayOfWeek()).year());
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
}

void day_popup::load_workValues(int workout)
{
    this->set_controlButtons(true);

    if(workout == scheduleProxy->rowCount())
    {
        daypop_del.addWork = true;
        selIndex = QModelIndex();
        dayModel->setData(dayModel->index(0,workout),"00:00");
        dayModel->setData(dayModel->index(1,workout),settings::get_listValues("Sport").at(0));
        dayModel->setData(dayModel->index(2,workout),settings::get_listValues("WorkoutCode").at(0));
        dayModel->setData(dayModel->index(3,workout),"-");
        dayModel->setData(dayModel->index(4,workout),"00:00:00");
        dayModel->setData(dayModel->index(5,workout),"0.0");
        dayModel->setData(dayModel->index(6,workout),"0");
        dayModel->setData(dayModel->index(7,workout),"-");
        ui->toolButton_copy->setEnabled(false);
        ui->toolButton_delete->setEnabled(false);
    }
    else
    {
        daypop_del.addWork = false;
        selIndex = scheduleProxy->mapToSource(scheduleProxy->index(workout,2));
        ui->label_selected->setText(scheduleProxy->data(scheduleProxy->index(workout,3)).toString());
        ui->timeEdit_time->setTime(QTime::fromString(scheduleProxy->data(scheduleProxy->index(workout,2)).toString()));
    }

    daypop_del.selCol = selWorkout = workout;
    ui->tableView_day->setEditTriggers(QAbstractItemView::CurrentChanged);
    ui->tableView_day->horizontalHeader()->setSelectionMode(QAbstractItemView::SingleSelection);

    this->update_workValues();
}

void day_popup::update_workValues()
{
    QString weekNumber = QString::number(newDate.weekNumber());
    weekNumber = weekNumber+"_"+QString::number(newDate.addDays(1 - ui->dateEdit_workDate->date().dayOfWeek()).year());
    QStringList addValues;
    addValues << weekNumber << newDate.toString("dd.MM.yyyy");
    workSched->itemList.clear();

    for(int i = 0; i < dayModel->rowCount()-1; ++i)
    {
        addValues << dayModel->data(dayModel->index(i,selWorkout)).toString();
    }

    workSched->itemList.insert(selIndex,addValues);
    ui->label_selected->setText(dayModel->data(dayModel->index(1,selWorkout)).toString());
    ui->timeEdit_time->setTime(QTime::fromString(dayModel->data(dayModel->index(0,selWorkout)).toString(),"hh:mm"));
}

void day_popup::set_result(QString resultText,int resultCode)
{
    QString stMessage;
    bool completeDay = ui->toolButton_dayEdit->isChecked();
    QMessageBox::StandardButton reply = QMessageBox::No;

    if(completeDay)
    {
        stMessage = " Complete Day";
    }
    else
    {
        stMessage = " Workout";
    }

    if(resultCode == ADD)reply = QMessageBox::question(this,resultText + stMessage,resultText + stMessage+"?",QMessageBox::Yes|QMessageBox::No);
    if(resultCode == EDITMOVE)reply = QMessageBox::warning(this,resultText + stMessage,resultText + stMessage+"?",QMessageBox::Yes|QMessageBox::No);
    if(resultCode == COPY)reply = QMessageBox::question(this,resultText + stMessage,resultText + stMessage+"?",QMessageBox::Yes|QMessageBox::No);
    if(resultCode == DEL) reply = QMessageBox::critical(this,resultText + stMessage,resultText + stMessage+"?",QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        if(resultCode == ADD || resultCode == EDITMOVE || resultCode == COPY)
        {
            workSched->set_workoutData(resultCode);
        }
        if(resultCode == DEL)
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
                workSched->delete_workout(selIndex);
            }
        }
    }

    this->set_controlButtons(false);
    ui->dateEdit_workDate->setDate(popupDate);
    ui->label_selected->setText("-");
    ui->timeEdit_time->setTime(QTime::fromString("00:00","mm:ss"));
    this->init_dayWorkouts(popupDate);
    this->set_popupMode();
}

void day_popup::set_editValues(QDate workDate)
{
    newDate = workDate;
    ui->label_workoutInfo->setText(workSched->get_weekPhase(workDate)+" - Week: "+ QString::number(workDate.weekNumber()));

    if(ui->toolButton_dayEdit->isChecked())
    {
        this->set_dayData(true);
        if(newDate != popupDate)
        {
            this->set_controlButtons(true);
            ui->toolButton_delete->setEnabled(false);
        }
        else
        {
           this->set_controlButtons(false);
           ui->toolButton_delete->setEnabled(true);
        }
    }
    else
    {
        this->update_workValues();
    }
}

void day_popup::setNextEditRow()
{
    QModelIndex rowIndex = dayModel->index(ui->tableView_day->currentIndex().row()+1,selWorkout);
    ui->tableView_day->setCurrentIndex(rowIndex);
}

void day_popup::set_popupMode()
{
    if(popupMode == EDIT)
    {
        popupMode = SHOW;
        this->setFixedHeight(260);
        ui->frame_edit->hide();
        ui->tableView_day->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView_day->horizontalHeader()->setSectionsClickable(false);
        ui->toolButton_dayEdit->hide();
        ui->toolButton_close->setFocus();
    }
    else if(popupMode == SHOW)
    {
        popupMode = EDIT;
        this->setFixedHeight(325);
        ui->frame_edit->show();
        ui->toolButton_dayEdit->show();
        ui->tableView_day->horizontalHeader()->setSectionsClickable(true);
        if(scheduleProxy->rowCount() <= 1)
        {
            this->load_workValues(0);
            ui->toolButton_dayEdit->setEnabled(false);
        }
        else
        {
            ui->toolButton_dayEdit->setEnabled(true);
        }
    }
}

void day_popup::on_toolButton_close_clicked()
{
    reject();
}

void day_popup::on_toolButton_edit_clicked()
{
    this->set_popupMode();
}

void day_popup::on_toolButton_editMove_clicked()
{
    ui->tableView_day->clearSelection();

    if(daypop_del.addWork)
    {
        this->set_result("Add",ADD);
    }
    else
    {
        this->set_result("Edit",EDITMOVE);
    }
}

void day_popup::on_toolButton_copy_clicked()
{
    this->set_result("Copy",COPY);
}

void day_popup::on_toolButton_delete_clicked()
{
    this->set_result("Delete",DEL);
}

void day_popup::on_toolButton_stdwork_clicked()
{
    Dialog_workouts stdWorkouts(this,ui->label_selected->text());
    stdWorkouts.setModal(true);
    int returnCode = stdWorkouts.exec();
    if(returnCode == QDialog::Accepted)
    {
       for(QHash<int,QString>::const_iterator it = stdWorkouts.workData.cbegin(), end = stdWorkouts.workData.cend(); it != end; ++it)
       {
           if(it.key() != 0) dayModel->setData(dayModel->index(it.key()+1,selWorkout),it.value());
       }
    }

    dayModel->setData(dayModel->index(7,selWorkout),this->get_workout_pace(dayModel->data(dayModel->index(5,selWorkout)).toDouble(),QTime::fromString(dayModel->data(dayModel->index(4,selWorkout)).toString(),"hh:mm:ss"),dayModel->data(dayModel->index(1,selWorkout)).toString(),true));
}

void day_popup::on_tableView_day_clicked(const QModelIndex &index)
{
    if(popupMode == SHOW)
    {
        QClipboard *clipboard = QApplication::clipboard();
        QString rowValue = dayModel->data(index).toString();
        clipboard->setText(rowValue);
    }
}

void day_popup::on_toolButton_dayEdit_clicked(bool checked)
{
    if(checked)
    {
        ui->label_selected->setText("Day");
        ui->tableView_day->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView_day->horizontalHeader()->setSectionsClickable(!checked);
        ui->timeEdit_time->setTime(QTime::fromString("00:00","mm:ss"));
        ui->timeEdit_time->setEnabled(false);
        ui->toolButton_stdwork->setEnabled(false);
        ui->toolButton_delete->setEnabled(checked);
    }
    else
    {
       ui->label_selected->setText("-");
       ui->tableView_day->setEditTriggers(QAbstractItemView::CurrentChanged);
       ui->tableView_day->horizontalHeader()->setSectionsClickable(!checked);
       ui->timeEdit_time->setEnabled(true);
       ui->toolButton_stdwork->setEnabled(true);
       this->set_controlButtons(checked);
    }
}
