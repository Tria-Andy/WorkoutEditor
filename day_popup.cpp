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
    this->setFixedHeight(400);
    editMode = false;
    popupDate = w_date;
    workSched = p_sched;
    moveWorkout = false;
    stdlistModel = new QStandardItemModel(this);
    editIcon = QIcon(":/images/icons/Modify.png");
    addIcon = QIcon(":/images/icons/Create.png");
    ui->toolButton_editMove->setIcon(editIcon);
    ui->toolButton_editMove->setToolTip("Edit/Move");
    ui->lineEdit_selected->setReadOnly(true);
    ui->lineEdit_workoutInfo->setReadOnly(true);
    ui->dateEdit_workDate->setEnabled(false);

    workListHeader = settings::getHeaderMap("workoutlist");
    dayModel = new QStandardItemModel(this);

    this->init_dayWorkouts(popupDate);
    ui->lineEdit_workoutInfo->setText(workSched->get_weekPhase(w_date)+" - Week: "+ QString::number(w_date.weekNumber()));
    connect(ui->tableView_day->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(load_workoutData(int)));
    connect(ui->tableView_day->itemDelegate(),SIGNAL(closeEditor(QWidget *,QAbstractItemDelegate::EndEditHint)),this,SLOT(setNextEditRow()));
}

enum {EDIT,MOVE,COPY,DEL};

day_popup::~day_popup()
{
    delete workouts_meta;
    delete workouts_steps;
    delete ui;
}

void day_popup::init_dayWorkouts(QDate workDate)
{
    this->set_controlButtons(false);
    dayModel->clear();

    QString viewBackground = "background-color: #e6e6e6";
    QString workoutDate = workDate.toString(workSched->dateFormat);

    dayWorkouts = workSched->get_workouts(true,workoutDate);
    workoutMap.insert(workDate,dayWorkouts);

    int workCount = dayWorkouts.count();
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

        if(workCount < settings::get_intValue("maxworkouts"))
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

    int worklistCount = workListHeader->count();
    dayModel->setColumnCount(workoutHeader.count());
    dayModel->setRowCount(worklistCount);
    dayModel->setHorizontalHeaderLabels(workoutHeader);
    ui->tableView_day->setModel(dayModel);
    ui->tableView_day->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_day->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_day->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_day->hideRow(worklistCount-1);
    ui->tableView_day->verticalHeader()->setSectionsClickable(false);
    ui->tableView_day->verticalHeader()->setFixedWidth(100);
    ui->tableView_day->setItemDelegate(&daypop_del);
    ui->tableView_day->setStyleSheet(viewBackground);

    int col = 0;

    for(QMap<int,QStringList>::const_iterator it = dayWorkouts.cbegin(), end = dayWorkouts.cend(); it != end; ++it)
    {
        for(int row = 0; row < worklistCount; ++row)
        {            
            if(row == 5)
            {
                dayModel->setData(dayModel->index(row,col),set_time(it.value().at(row).toInt()));
            }
            else if(row == 9)
            {
                dayModel->setData(dayModel->index(row,col),this->get_workout_pace(it.value().at(6).toDouble(),it.value().at(5).toDouble(),it.value().at(1),true));
            }
            else
            {
                dayModel->setData(dayModel->index(row,col),it.value().at(row));
            }
        }
        ++col;
    }

    for(int vheader = 0; vheader < worklistCount; ++vheader)
    {
        dayModel->setVerticalHeaderItem(vheader,new QStandardItem(workListHeader->at(vheader)));
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
    connect(dayModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(read_workValues()));
    ui->dateEdit_workDate->setDate(workDate);
}

void day_popup::set_comboWorkouts(QString workoutSport, QString stdid)
{
    ui->comboBox_stdworkout->blockSignals(true);
    ui->comboBox_stdworkout->clear();

    /*
    int comboIndex = -1;

    if(!workoutSport.isEmpty())
    {
        this->filter_workout(workoutSport,0,true);
        QString workID,workTitle,listString;

        stdlistModel->clear();
        stdlistModel->setRowCount(metaProxy->rowCount());
        stdlistModel->setColumnCount(2);

        for(int i = 0; i < metaProxy->rowCount(); ++i)
        {
            workID = metaProxy->data(metaProxy->index(i,1)).toString();
            workTitle = metaProxy->data(metaProxy->index(i,3)).toString();
            listString = metaProxy->data(metaProxy->index(i,2)).toString() + " - " + workTitle
                            + " (" +metaProxy->data(metaProxy->index(i,4)).toString()
                            + " - " +metaProxy->data(metaProxy->index(i,5)).toString()
                            + " - " +metaProxy->data(metaProxy->index(i,6)).toString()
                            + " - " +metaProxy->data(metaProxy->index(i,7)).toString()+")";
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
    */
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
    QMap<int,QString> *intMap = settings::getListMapPointer(settings::lMap::Interval);
    int sampCount = dayWorkouts.value(selWorkout).at(5).toInt();
    intExport = new QStandardItemModel(1,3,this);
    sampExport = new QStandardItemModel(sampCount,1,this);

    intExport->setData(intExport->index(0,0),"Workout");
    intExport->setData(intExport->index(0,1),0);
    intExport->setData(intExport->index(0,2),sampCount);

    for(int i = 0; i < intMap->count(); ++i)
    {
        intLabels << intMap->value(i);
    }

    sampLabels << "SECS";
    for(int i = 0; i < sampCount; ++i)
    {
       sampExport->setData(sampExport->index(i,0),i);
    }

    QTime workoutTime;
    QDateTime workoutDateTime;
    QString tempDate,tempTime,sport,stressType,commonRI;
    QString totalWork = dayWorkouts.value(selWorkout).at(8);

    tempDate = popupDate.toString(workSched->dateFormat);
    tempTime = dayWorkouts.value(selWorkout).at(0);

    workoutTime = QTime::fromString(tempTime,"hh:mm");
    fileName = popupDate.toString("yyyy_MM_dd_") + workoutTime.toString("hh_mm_ss") +".json";
    workoutDateTime = QDateTime::fromString(tempDate+"T"+tempTime+":00","dd.MM.yyyyThh:mm:ss").toUTC();

    sport = dayWorkouts.value(selWorkout).at(1);

    if(sport == settings::SwimLabel) stressType = "swimscore";
    if(sport == settings::BikeLabel) stressType = "skiba_bike_score";
    if(sport == settings::RunLabel) stressType = "govss";
    if(sport == settings::AltLabel || sport == settings::StrengthLabel)
    {
        commonRI = QString::number(set_doubleValue(totalWork.toDouble() / this->calc_totalWork(sport,10.0,sampCount,0)*10.0,false));
        if(sport == settings::StrengthLabel) commonRI = "4.0";
        stressType = "triscore";
    }

    this->rideData.insert("STARTTIME",workoutDateTime.toString("yyyy/MM/dd hh:mm:ss UTC"));
    this->rideData.insert("DEVICETYPE","Manual Import");
    this->rideData.insert("IDENTIFIER","");
    this->rideData.insert("OVERRIDES","");

    this->tagData.insert("Sport",sport);
    this->tagData.insert("Athlete",gcValues->value("athlete"));
    this->tagData.insert("Filename",fileName);
    this->tagData.insert("CommonRI",commonRI);
    this->tagData.insert("Device","Manual Import");
    this->tagData.insert("Workout Code",dayWorkouts.value(selWorkout).at(2));
    this->tagData.insert("Workout Title",dayWorkouts.value(selWorkout).at(3));
    this->tagData.insert("Comment",dayWorkouts.value(selWorkout).at(4));
    this->tagData.insert("Workout Content",dayWorkouts.value(selWorkout).at(3));
    this->tagData.insert("Year",QString::number(popupDate.year()));
    this->tagData.insert("Month",QLocale().monthName(popupDate.month()));
    this->tagData.insert("Weekday",QLocale().dayName(popupDate.dayOfWeek(),QLocale::ShortFormat));

    this->hasOverride = true;
    overrideData.insert("time_riding",QString::number(sampCount));
    overrideData.insert("workout_time",QString::number(sampCount));
    overrideData.insert("total_kcalories",totalWork);
    overrideData.insert("total_distance",dayWorkouts.value(selWorkout).at(6));
    overrideData.insert("total_work",totalWork);
    overrideData.insert(stressType,dayWorkouts.value(selWorkout).at(7));

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

    if(workout == dayWorkouts.count())
    {
        dayModel->setData(dayModel->index(0,workout),"00:00");
        dayModel->setData(dayModel->index(1,workout),"");
        dayModel->setData(dayModel->index(2,workout),"");
        dayModel->setData(dayModel->index(3,workout),"-");
        dayModel->setData(dayModel->index(4,workout),"-");
        dayModel->setData(dayModel->index(5,workout),"00:00:00");
        dayModel->setData(dayModel->index(6,workout),"0.0");
        dayModel->setData(dayModel->index(7,workout),"0");
        dayModel->setData(dayModel->index(8,workout),"0");
        dayModel->setData(dayModel->index(9,workout),"-");
        ui->toolButton_editMove->setIcon(addIcon);
        ui->toolButton_editMove->setToolTip("Add Workout");
        ui->toolButton_copy->setEnabled(false);
        ui->toolButton_delete->setEnabled(false);
    }
    else
    {
        ui->toolButton_editMove->setIcon(editIcon);
        ui->toolButton_editMove->setToolTip("Edit/Move");
        ui->dateEdit_workDate->setEnabled(true);
    }

    daypop_del.selCol = selWorkout = workout;
    ui->tableView_day->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui->tableView_day->horizontalHeader()->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_day->edit(workIndex);
    ui->tableView_day->setCurrentIndex(workIndex);
    this->read_workValues();
    daypop_del.blockSignals(false);
}

void day_popup::read_workValues()
{
        QString sport = dayModel->data(dayModel->index(1,selWorkout)).toString();
        QString stdid = dayModel->data(dayModel->index(9,selWorkout)).toString();
        ui->lineEdit_selected->setText(ui->tableView_day->model()->headerData(selWorkout,Qt::Horizontal).toString()+" - "+sport);

        this->set_comboWorkouts(sport,stdid);
}

void day_popup::update_workouts()
{
    QTime workTime;
    QMap<QTime,QStringList> valueMap;
    QStringList valueList;
    int workCounter = 0;

    for(int work = 0; work < dayModel->columnCount(); ++work)
    {
        for(int value = 0; value < dayModel->rowCount(); ++value)
        {
            if(value == 0)
            {
                workTime = QTime::fromString(dayModel->data(dayModel->index(value,work)).toString(),"hh:mm");
            }
            else if (value == 5)
            {
                valueList << QString::number(get_timesec(dayModel->data(dayModel->index(value,work)).toString()));
            }
            else
            {
                valueList << dayModel->data(dayModel->index(value,work)).toString();
            }
        }
        if(workTime.isValid())
        {
            if(valueMap.contains(workTime)) workTime = workTime.addSecs(900);
            valueMap.insertMulti(workTime,valueList);
        }
        valueList.clear();
    }

    dayWorkouts.clear();

    for(QMap<QTime,QStringList>::const_iterator it = valueMap.cbegin(), end = valueMap.cend(); it != end; ++it)
    {
        valueList.append(it.key().toString("hh:mm"));
        for(int i = 0; i < it.value().count(); ++i)
        {
            valueList << it.value().at(i);
        }
        dayWorkouts.insert(workCounter++,valueList);
        valueList.clear();
    }
}


void day_popup::set_result(int result)
{
    editMode = false;

    QStringList moveWorkout;
    QString moveDate = ui->dateEdit_workDate->date().toString(workSched->dateFormat);
    QMap<int,QStringList> valueList;

    if(ui->toolButton_dayEdit->isChecked())
    {
        if(result == MOVE || result == COPY)
        {
            workoutMap.insert(ui->dateEdit_workDate->date(),dayWorkouts);
            if(result == MOVE)
            {
                workoutMap.insert(popupDate,valueList);
            }
        }
        if(result == DEL)
        {
            workoutMap.insert(popupDate,valueList);
        }
    }
    else
    {
        if(result == MOVE || result == COPY)
        {
            valueList = workSched->get_workouts(true,moveDate);
            if(result == MOVE)
            {
                valueList.insert(valueList.count(),dayWorkouts.take(selWorkout));
                dayModel->removeColumn(selWorkout);
                this->update_workouts();
            }
            else
            {
                valueList.insert(valueList.count(),dayWorkouts.value(selWorkout));
            }
            workoutMap.insert(ui->dateEdit_workDate->date(),valueList);
        }
        if(result == EDIT)
        {
            this->update_workouts();
        }
        if(result == DEL)
        {
            dayModel->removeColumn(selWorkout);
            this->update_workouts();
        }
        workoutMap.insert(popupDate,dayWorkouts);
    }

    workSched->set_workoutData(workoutMap);

    ui->dateEdit_workDate->setEnabled(false);
    this->set_controlButtons(false);
    ui->lineEdit_selected->setText("-");
    this->init_dayWorkouts(popupDate);
    ui->toolButton_dayEdit->setChecked(false);
    this->set_comboWorkouts(QString(),QString());
}

void day_popup::set_proxyFilter(QString filterTo, int col,bool fixString)
{
    metaProxy->invalidate();

    if(fixString)
    {
        metaProxy->setFilterFixedString(filterTo);
    }
    else
    {
        metaProxy->setFilterRegExp("\\b"+filterTo+"\\b");
    }

    metaProxy->setFilterKeyColumn(col);
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

    if(moveWorkout)
    {
        moveWorkout = false;
        this->set_result(MOVE);
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
    ui->dateEdit_workDate->setEnabled(checked);
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

void day_popup::on_comboBox_stdworkout_activated(int stdindex)
{
    ui->tableView_day->setCurrentIndex(dayModel->index(0,selWorkout));
    QString workoutID = ui->comboBox_stdworkout->model()->data(ui->comboBox_stdworkout->model()->index(stdindex,1)).toString();

    stdworkData.clear();
    this->set_proxyFilter(workoutID,1,false);

    for(int i = 1; i < metaProxy->columnCount()-2; ++i)
    {
        stdworkData.insert(i,metaProxy->data(metaProxy->index(0,i)).toString());
    }

    for(QHash<int,QString>::const_iterator it = stdworkData.cbegin(), end = stdworkData.cend(); it != end; ++it)
    {
        if(it.key() == 1)
        {
            dayModel->setData(dayModel->index(10,selWorkout),it.value());
        }
        else
        {
            dayModel->setData(dayModel->index(it.key(),selWorkout),it.value());
        }
    }
    dayModel->setData(dayModel->index(9,selWorkout),this->get_workout_pace(dayModel->data(dayModel->index(6,selWorkout)).toDouble(),get_timesec(dayModel->data(dayModel->index(5,selWorkout)).toString()),dayModel->data(dayModel->index(1,selWorkout)).toString(),true));
    ui->tableView_day->setCurrentIndex(dayModel->index(0,selWorkout));
}

void day_popup::on_toolButton_map_clicked()
{
    QString workoutID = ui->comboBox_stdworkout->model()->data(ui->comboBox_stdworkout->model()->index(ui->comboBox_stdworkout->currentIndex(),1)).toString();
    this->set_proxyFilter(workoutID,1,false);
    QString image = metaProxy->data(metaProxy->index(0,9)).toString();

    Dialog_map dialogMap(this,workoutID,image);
    dialogMap.setModal(true);
    dialogMap.exec();
}

void day_popup::on_dateEdit_workDate_dateChanged(const QDate &date)
{
    ui->lineEdit_workoutInfo->setText(workSched->get_weekPhase(date)+" - Week: "+ QString::number(date.weekNumber()));

    QString weekNumber = this->calc_weekID(date);

    if(date != popupDate)
    {
        moveWorkout = true;
        this->set_controlButtons(true);
        ui->toolButton_delete->setEnabled(false);
    }
    else
    {
       this->set_controlButtons(false);
       ui->toolButton_delete->setEnabled(true);
    }
}
