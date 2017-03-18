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

#include <QKeyEvent>
#include "day_popup.h"
#include "ui_day_popup.h"

day_popup::day_popup(QWidget *parent, const QDate w_date, schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::day_popup)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setFixedHeight(325);
    editMode = false;
    addWorkout = false;
    popupDate = newDate = w_date;
    workSched = p_sched;
    scheduleProxy = new QSortFilterProxyModel();
    scheduleProxy->setSourceModel(workSched->workout_schedule);
    editIcon = QIcon(":/images/icons/Modify.png");
    addIcon = QIcon(":/images/icons/Create.png");
    ui->toolButton_editMove->setIcon(editIcon);
    ui->toolButton_editMove->setToolTip("Edit/Move");
    ui->lineEdit_selected->setReadOnly(true);
    ui->lineEdit_workoutInfo->setReadOnly(true);

    workListHeader << "Time" << "Sport" << "Code" << "Title" << "Duration" << "Distance" << "Stress" << "Pace";
    dayModel = new QStandardItemModel();

    this->init_dayWorkouts(popupDate);
    ui->lineEdit_workoutInfo->setText(workSched->get_weekPhase(w_date)+" - Week: "+ QString::number(w_date.weekNumber()));

    connect(ui->tableView_day->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(load_workoutData(int)));
    connect(ui->dateEdit_workDate,SIGNAL(dateChanged(QDate)),this,SLOT(edit_workoutDate(QDate)));
    connect(ui->tableView_day->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),this,SLOT(setNextEditRow()));

}

enum {ADD,EDIT,COPY,DEL};

day_popup::~day_popup()
{
    delete dayModel;
    delete scheduleProxy;
    delete ui;
}

void day_popup::init_dayWorkouts(QDate workDate)
{
    this->set_controlButtons(false);
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
        this->setFixedWidth(450);
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

    if(workCount == 0)
    {
        this->set_controlButtons(true);
        this->load_workoutData(0);
        ui->toolButton_dayEdit->setEnabled(false);

    }
    else
    {
        ui->toolButton_dayEdit->setEnabled(true);
    }

    connect(dayModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(update_workValues()));
    ui->dateEdit_workDate->setDate(workDate);
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
    QHash<int,QString> valueList;
    QModelIndex modIndex;

    if(completeDay)
    {
        for(int row = 0; row < scheduleProxy->rowCount(); ++row)
        {
            valueList.insert(0,weekNumber);
            valueList.insert(1,newDate.toString("dd.MM.yyyy"));
            modIndex = scheduleProxy->mapToSource(scheduleProxy->index(row,0));
            for(int col = 2; col < scheduleProxy->columnCount(); ++col)
            {
                valueList.insert(col,scheduleProxy->data(scheduleProxy->index(row,col)).toString());
            }
            workSched->itemList.insert(modIndex,valueList);
        }
    }
}

void day_popup::load_workoutData(int workout)
{
    this->set_controlButtons(true);
    daypop_del.blockSignals(true);
    ui->tableView_day->clearSelection();
    ui->lineEdit_selected->setFocus();

    editMode = true;
    QModelIndex workIndex = dayModel->index(0,workout);

    if(workout == scheduleProxy->rowCount())
    {
        addWorkout = true;
        selIndex = QModelIndex();
        dayModel->setData(dayModel->index(0,workout),"00:00");
        dayModel->setData(dayModel->index(1,workout),"");
        dayModel->setData(dayModel->index(2,workout),"");
        dayModel->setData(dayModel->index(3,workout),"-");
        dayModel->setData(dayModel->index(4,workout),"00:00:00");
        dayModel->setData(dayModel->index(5,workout),"0.0");
        dayModel->setData(dayModel->index(6,workout),"0");
        dayModel->setData(dayModel->index(7,workout),"-");
        ui->toolButton_editMove->setIcon(addIcon);
        ui->toolButton_editMove->setToolTip("Add Workout");
        ui->toolButton_copy->setEnabled(false);
        ui->toolButton_delete->setEnabled(false);
    }
    else
    {
        addWorkout = false;
        selIndex = scheduleProxy->mapToSource(scheduleProxy->index(workout,2));
        ui->timeEdit_time->setTime(QTime::fromString(scheduleProxy->data(scheduleProxy->index(workout,2)).toString()));
        ui->toolButton_editMove->setIcon(editIcon);
        ui->toolButton_editMove->setToolTip("Edit/Move");
    }

    daypop_del.selCol = selWorkout = workout;
    ui->tableView_day->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui->tableView_day->horizontalHeader()->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_day->edit(workIndex);
    ui->tableView_day->setCurrentIndex(workIndex);

    daypop_del.blockSignals(false);
    this->update_workValues();
}

void day_popup::update_workValues()
{
    if(editMode)
    {
        QString weekNumber = QString::number(newDate.weekNumber());
        weekNumber = weekNumber+"_"+QString::number(newDate.addDays(1 - ui->dateEdit_workDate->date().dayOfWeek()).year());
        QHash<int,QString> valueList;
        valueList.insert(0,weekNumber);
        valueList.insert(1,newDate.toString("dd.MM.yyyy"));

        for(int i = 2; i < dayModel->rowCount()+1; ++i)
        {
            valueList.insert(i,dayModel->data(dayModel->index(i-2,selWorkout)).toString());
        }

        workSched->itemList.insert(selIndex,valueList);
        ui->lineEdit_selected->setText(ui->tableView_day->model()->headerData(selWorkout,Qt::Horizontal).toString()+" - "+dayModel->data(dayModel->index(1,selWorkout)).toString());
        ui->timeEdit_time->setTime(QTime::fromString(dayModel->data(dayModel->index(0,selWorkout)).toString(),"hh:mm"));
    }
}

void day_popup::set_result(int resultCode)
{
    if(resultCode == ADD || resultCode == EDIT || resultCode == COPY)
    {
        workSched->set_workoutData(resultCode);
    }
    if(resultCode == DEL)
    {
        if(ui->toolButton_dayEdit->isChecked())
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

    editMode = false;
    this->set_controlButtons(false);
    ui->lineEdit_selected->setText("-");
    ui->timeEdit_time->setTime(QTime::fromString("00:00","mm:ss"));
    this->init_dayWorkouts(popupDate);
}

void day_popup::edit_workoutDate(QDate workDate)
{
    newDate = workDate;
    ui->lineEdit_workoutInfo->setText(workSched->get_weekPhase(workDate)+" - Week: "+ QString::number(workDate.weekNumber()));

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
        ui->toolButton_stdwork->setEnabled(false);
    }
    else
    {
        if(editMode) this->update_workValues();
    }
}

void day_popup::setNextEditRow()
{
    if(ui->toolButton_dayEdit->isChecked())
    {
        ui->lineEdit_selected->setFocus();
    }
    else
    {
        QModelIndex rowIndex = dayModel->index(ui->tableView_day->currentIndex().row()+1,selWorkout);
        ui->tableView_day->setCurrentIndex(rowIndex);
    }
}

void day_popup::on_toolButton_close_clicked()
{
    workSched->itemList.clear();
    reject();
}

void day_popup::on_toolButton_editMove_clicked()
{
    ui->lineEdit_workoutInfo->setFocus();

    if(addWorkout)
    {
        this->set_result(ADD);
    }
    else
    {
        this->set_result(EDIT);
    }
}

void day_popup::on_toolButton_copy_clicked()
{
    this->set_result(COPY);
}

void day_popup::on_toolButton_delete_clicked()
{
    this->set_result(DEL);
}

void day_popup::on_toolButton_stdwork_clicked()
{
    Dialog_workouts stdWorkouts(this,ui->lineEdit_selected->text().split(" - ").last());
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
    if(!editMode)
    {
        QClipboard *clipboard = QApplication::clipboard();
        QString rowValue = dayModel->data(index).toString();
        clipboard->setText(rowValue);
    }
    else
    {
        if(!ui->toolButton_dayEdit->isChecked()) ui->tableView_day->edit(index);
    }
}

void day_popup::on_toolButton_dayEdit_clicked(bool checked)
{
    QPalette selectBox;

    if(checked)
    {
        ui->lineEdit_selected->setText("COMPLETE DAY");
        ui->tableView_day->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView_day->setSelectionMode(QAbstractItemView::NoSelection);
        ui->tableView_day->horizontalHeader()->setSectionsClickable(!checked);
        ui->timeEdit_time->setTime(QTime::fromString("00:00","mm:ss"));
        ui->timeEdit_time->setEnabled(false);
        selectBox.setColor(QPalette::Base,Qt::red);
        selectBox.setColor(QPalette::Text,Qt::white);
        this->set_controlButtons(false);
        ui->toolButton_delete->setEnabled(checked);
    }
    else
    {
       ui->lineEdit_selected->setText("-");
       ui->tableView_day->setEditTriggers(QAbstractItemView::AllEditTriggers);
       ui->tableView_day->horizontalHeader()->setSectionsClickable(!checked);
       ui->tableView_day->horizontalHeader()->setSelectionMode(QAbstractItemView::SingleSelection);
       selectBox.setColor(QPalette::NoRole,Qt::NoBrush);
       selectBox.setColor(QPalette::Text,Qt::black);
       ui->timeEdit_time->setEnabled(true);
       ui->toolButton_stdwork->setEnabled(true);
       this->set_controlButtons(checked);
    }
    ui->lineEdit_selected->setPalette(selectBox);
    ui->tableView_day->clearSelection();
    ui->lineEdit_selected->setFocus();
}

void day_popup::keyPressEvent(QKeyEvent *event)
{
    qDebug() << event->key();

    if(event->key() == Qt::Key_Tab)
    {
        qDebug() << "Tab";
    }
}
