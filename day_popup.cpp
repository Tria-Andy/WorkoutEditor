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
    this->setFixedHeight(380);
    editMode = false;
    addWorkout = false;
    popupDate = newDate = w_date;
    workSched = p_sched;
    scheduleProxy = new QSortFilterProxyModel();
    scheduleProxy->setSourceModel(workSched->workout_schedule);
    stdProxy = new QSortFilterProxyModel();
    stdProxy->setSourceModel(workouts_meta);
    stdlistModel = new QStandardItemModel();
    editIcon = QIcon(":/images/icons/Modify.png");
    addIcon = QIcon(":/images/icons/Create.png");
    ui->toolButton_editMove->setIcon(editIcon);
    ui->toolButton_editMove->setToolTip("Edit/Move");
    ui->lineEdit_selected->setReadOnly(true);
    ui->lineEdit_workoutInfo->setReadOnly(true);

    workListHeader << "Time" << "Sport" << "Code" << "Title" << "Duration" << "Distance" << "Stress" << "Work(kj)" << "StdId" << "Pace";
    dayModel = new QStandardItemModel();
    workSched->itemList.clear();

    this->init_dayWorkouts(popupDate);
    ui->lineEdit_workoutInfo->setText(workSched->get_weekPhase(w_date,false)+" - Week: "+ QString::number(w_date.weekNumber()));

    connect(ui->tableView_day->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(load_workoutData(int)));
    connect(ui->dateEdit_workDate,SIGNAL(dateChanged(QDate)),this,SLOT(edit_workoutDate(QDate)));
    connect(ui->tableView_day->itemDelegate(),SIGNAL(closeEditor(QWidget*,QAbstractItemDelegate::EndEditHint)),this,SLOT(setNextEditRow()));
}

enum {ADD,EDIT,COPY,DEL};

day_popup::~day_popup()
{
    delete dayModel;
    delete stdlistModel;
    delete scheduleProxy;
    delete stdProxy;
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

    ui->label_weekinfo->setText(workoutDate + " - Phase: " + workSched->get_weekPhase(workDate,false));

    int worklistCount = workListHeader.count();
    dayModel->setColumnCount(workoutHeader.count());
    dayModel->setRowCount(worklistCount);
    dayModel->setVerticalHeaderLabels(workListHeader);
    dayModel->setHorizontalHeaderLabels(workoutHeader);

    ui->tableView_day->setModel(dayModel);
    ui->tableView_day->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_day->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_day->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_day->hideRow(worklistCount-2);
    ui->tableView_day->verticalHeader()->setSectionsClickable(false);
    ui->tableView_day->verticalHeader()->setFixedWidth(100);
    ui->tableView_day->setItemDelegate(&daypop_del);
    ui->tableView_day->setStyleSheet(viewBackground);

    for(int col = 0; col < workCount; ++col)
    {
        for(int row = 0; row <= workListHeader.count(); ++row)
        {
            dayModel->setData(dayModel->index(row,col,QModelIndex()),scheduleProxy->data(scheduleProxy->index(col,row+2)).toString());
            if(row == 9)
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
        ui->toolButton_dayEdit->setEnabled(false);
    }
    else
    {
        ui->toolButton_dayEdit->setEnabled(true);
    }

    connect(dayModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(update_workValues()));
    ui->dateEdit_workDate->setDate(workDate);
}

void day_popup::set_comboWorkouts(QString workoutSport, QString stdid)
{
    ui->comboBox_stdworkout->blockSignals(true);
    ui->comboBox_stdworkout->clear();
    int comboIndex = -1;

    if(!workoutSport.isEmpty())
    {
        QString workID,workTitle,listString;
        stdProxy->invalidate();
        stdProxy->setFilterFixedString(workoutSport);
        stdProxy->setFilterKeyColumn(0);
        stdlistModel->clear();

        stdlistModel->setRowCount(stdProxy->rowCount());
        stdlistModel->setColumnCount(2);

        for(int i = 0; i < stdProxy->rowCount(); ++i)
        {
            workID = stdProxy->data(stdProxy->index(i,1)).toString();
            workTitle = stdProxy->data(stdProxy->index(i,3)).toString();
            listString = stdProxy->data(stdProxy->index(i,2)).toString() + " - " + workTitle
                            + " (" +stdProxy->data(stdProxy->index(i,4)).toString()
                            + " - " +stdProxy->data(stdProxy->index(i,5)).toString()
                            + " - " +stdProxy->data(stdProxy->index(i,6)).toString()
                            + " - " +stdProxy->data(stdProxy->index(i,7)).toString()+")";
            stdlistModel->setData(stdlistModel->index(i,0,QModelIndex()),listString);
            stdlistModel->setData(stdlistModel->index(i,1,QModelIndex()),workID);
        }
        stdlistModel->sort(0);

        for(int i = 0; i < stdlistModel->rowCount(); ++i)
        {
            if(stdlistModel->data(stdlistModel->index(i,1)).toString() == stdid) comboIndex = i;
        }

        ui->comboBox_stdworkout->setModel(stdlistModel);
        ui->comboBox_stdworkout->setModelColumn(0);
        ui->comboBox_stdworkout->setCurrentIndex(comboIndex);
    }
    ui->comboBox_stdworkout->blockSignals(false);
}

void day_popup::set_controlButtons(bool active)
{
    ui->toolButton_copy->setEnabled(active);
    ui->toolButton_delete->setEnabled(active);
    ui->toolButton_editMove->setEnabled(active);
    ui->toolButton_upload->setEnabled(active);
    ui->comboBox_stdworkout->setEnabled(active);
}

void day_popup::set_exportContent()
{
    QStringList intLabels,sampLabels;
    QMap<int,QString> intMap = settings::get_intList();
    int sampCount = get_timesec(workSched->itemList.value(selIndex).value(6));
    intExport = new QStandardItemModel(1,3);
    sampExport = new QStandardItemModel(sampCount,1);

    intExport->setData(intExport->index(0,0),"Workout");
    intExport->setData(intExport->index(0,1),0);
    intExport->setData(intExport->index(0,2),sampCount);

    for(int i = 0; i < intMap.count(); ++i)
    {
        intLabels << intMap.value(i);
    }

    sampLabels << "SECS";
    for(int i = 0; i < sampCount; ++i)
    {
       sampExport->setData(sampExport->index(i,0),i);
    }

    QTime workoutTime;
    QDateTime workoutDateTime;
    QString tempDate,tempTime,sport,stressType,commonRI;
    QString totalWork = workSched->itemList.value(selIndex).value(9);

    tempDate = popupDate.toString("dd.MM.yyyy");
    tempTime = workSched->itemList.value(selIndex).value(2);

    workoutTime = QTime::fromString(tempTime,"hh:mm");
    fileName = popupDate.toString("yyyy_MM_dd_") + workoutTime.toString("hh_mm_ss") +".json";
    workoutDateTime = QDateTime::fromString(tempDate+"T"+tempTime+":00","dd.MM.yyyyThh:mm:ss").toUTC();

    sport = workSched->itemList.value(selIndex).value(3);

    if(sport == settings::isSwim) stressType = "swimscore";
    if(sport == settings::isBike) stressType = "skiba_bike_score";
    if(sport == settings::isRun) stressType = "govss";
    if(sport == settings::isAlt || sport == settings::isStrength)
    {
        commonRI = QString::number(set_doubleValue(totalWork.toDouble() / this->calc_totalWork(sport,10.0,sampCount,0)*10.0,false));
        if(sport == settings::isStrength) commonRI = "4.0";
        stressType = "triscore";
    }

    this->rideData.insert("STARTTIME",workoutDateTime.toString("yyyy/MM/dd hh:mm:ss UTC"));
    this->rideData.insert("DEVICETYPE","Manual Import");
    this->rideData.insert("IDENTIFIER","");
    this->rideData.insert("OVERRIDES","");

    this->tagData.insert("Sport",sport);
    this->tagData.insert("Athlete",settings::get_gcInfo("athlete"));
    this->tagData.insert("Filename",fileName);
    this->tagData.insert("CommonRI",commonRI);
    this->tagData.insert("Device","Manual Import");
    this->tagData.insert("Workout Code",workSched->itemList.value(selIndex).value(4));
    this->tagData.insert("Workout Content",workSched->itemList.value(selIndex).value(5));
    this->tagData.insert("Workout Title",workSched->itemList.value(selIndex).value(5));
    this->tagData.insert("Year",QString::number(popupDate.year()));
    this->tagData.insert("Month",QDate::longMonthName(popupDate.month()));
    this->tagData.insert("Weekday",QDate::shortDayName(popupDate.dayOfWeek()));

    this->hasOverride = true;
    overrideData.insert("time_riding",QString::number(sampCount));
    overrideData.insert("workout_time",QString::number(sampCount));
    overrideData.insert("total_kcalories",totalWork);
    overrideData.insert("total_work",totalWork);
    overrideData.insert(stressType,workSched->itemList.value(selIndex).value(8));

    this->init_jsonFile();
    this->write_actModel("INTERVALS",intExport,&intLabels);
    this->write_actModel("SAMPLES",sampExport,&sampLabels);
    this->write_jsonFile();

    delete intExport;
    delete sampExport;
    QMessageBox::information(this,"Export Workout","Workout Informations Exported!",QMessageBox::Ok);
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
        dayModel->setData(dayModel->index(7,workout),"0");
        dayModel->setData(dayModel->index(8,workout),"-");
        ui->toolButton_editMove->setIcon(addIcon);
        ui->toolButton_editMove->setToolTip("Add Workout");
        ui->toolButton_copy->setEnabled(false);
        ui->toolButton_delete->setEnabled(false);
    }
    else
    {
        addWorkout = false;
        selIndex = scheduleProxy->mapToSource(scheduleProxy->index(workout,2));
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
        QString sport = dayModel->data(dayModel->index(1,selWorkout)).toString();
        QString stdid = dayModel->data(dayModel->index(8,selWorkout)).toString();
        workSched->itemList.insert(selIndex,valueList);
        ui->lineEdit_selected->setText(ui->tableView_day->model()->headerData(selWorkout,Qt::Horizontal).toString()+" - "+sport);
        this->set_comboWorkouts(sport,stdid);
    }
}

void day_popup::set_result(int resultCode)
{
    editMode = false;

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
    workSched->itemList.clear();
    this->set_controlButtons(false);
    ui->lineEdit_selected->setText("-");
    this->init_dayWorkouts(popupDate);
    ui->toolButton_dayEdit->setChecked(false);
    this->set_comboWorkouts(QString(),QString());
}

void day_popup::edit_workoutDate(QDate workDate)
{
    newDate = workDate;
    ui->lineEdit_workoutInfo->setText(workSched->get_weekPhase(workDate,false)+" - Week: "+ QString::number(workDate.weekNumber()));

    workSched->itemList.clear();

    if(ui->toolButton_dayEdit->isChecked())
    {
        QString weekNumber = QString::number(newDate.weekNumber());
        weekNumber = weekNumber+"_"+QString::number(newDate.addDays(1 - ui->dateEdit_workDate->date().dayOfWeek()).year());
        QHash<int,QString> valueList;
        QModelIndex modIndex;

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
    reject();
}

void day_popup::on_toolButton_editMove_clicked()
{
    ui->dateEdit_workDate->setFocus();

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
       this->set_controlButtons(checked);
    }
    ui->lineEdit_selected->setPalette(selectBox);
    ui->tableView_day->clearSelection();
    ui->lineEdit_selected->setFocus();
}

void day_popup::on_toolButton_upload_clicked()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::question(this,"Export Workout","Export Workout Informations to GC?",QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        this->set_exportContent();
    }
}

void day_popup::on_comboBox_stdworkout_currentIndexChanged(int stdindex)
{
    ui->tableView_day->setCurrentIndex(dayModel->index(0,selWorkout));
    QString workoutID = ui->comboBox_stdworkout->model()->data(ui->comboBox_stdworkout->model()->index(stdindex,1)).toString();
    stdworkData.clear();
    stdProxy->setFilterRegExp("\\b"+workoutID+"\\b");
    stdProxy->setFilterKeyColumn(1);

    for(int i = 1; i < stdProxy->columnCount()-1; ++i)
    {
        stdworkData.insert(i,stdProxy->data(stdProxy->index(0,i)).toString());
    }

    for(QHash<int,QString>::const_iterator it = stdworkData.cbegin(), end = stdworkData.cend(); it != end; ++it)
    {
        if(it.key() == 1)
        {
            dayModel->setData(dayModel->index(8,selWorkout),it.value());
        }
        else
        {
            dayModel->setData(dayModel->index(it.key(),selWorkout),it.value());
        }
    }

    dayModel->setData(dayModel->index(9,selWorkout),this->get_workout_pace(dayModel->data(dayModel->index(5,selWorkout)).toDouble(),QTime::fromString(dayModel->data(dayModel->index(4,selWorkout)).toString(),"hh:mm:ss"),dayModel->data(dayModel->index(1,selWorkout)).toString(),true));
    ui->tableView_day->setCurrentIndex(dayModel->index(0,selWorkout));
}
