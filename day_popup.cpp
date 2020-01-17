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

day_popup::day_popup(QWidget *parent, const QDate w_date, schedule *p_sched,standardWorkouts *p_stdWorkout) :
    QDialog(parent),
    ui(new Ui::day_popup)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    popupDate = w_date;
    stdWorkouts = p_stdWorkout;
    workSchedule = p_sched;
    moveWorkout = false;
    editIcon = QIcon(":/images/icons/Modify.png");
    addIcon = QIcon(":/images/icons/Create.png");
    maxWorkouts = settings::get_intValue("maxworkouts");
    ui->toolButton_editMove->setIcon(editIcon);
    ui->toolButton_editMove->setToolTip("Edit/Move");
    ui->lineEdit_selected->setReadOnly(true);
    ui->lineEdit_workoutInfo->setReadOnly(true);
    ui->dateEdit_workDate->setEnabled(false);

    ui->tableWidget_day->setItemDelegate(&daypop_del);
    ui->tableWidget_day->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_day->setColumnCount(1);
    ui->tableWidget_day->horizontalHeader()->hide();
    ui->tableWidget_day->verticalHeader()->hide();
    ui->tableWidget_day->setMinimumSize(this->width()/2,this->height()-ui->frame_dayInfo->height());
    ui->tableWidget_day->verticalHeader()->setDefaultSectionSize(ui->tableWidget_day->height() / maxWorkouts);

    ui->comboBox_workCode->addItems(settings::get_listValues("WorkoutCode"));
    ui->comboBox_workSport->addItems(settings::get_listValues("Sportuse"));
    this->reset_controls();
    this->init_dayWorkouts(popupDate);
    ui->lineEdit_workoutInfo->setText(workSchedule->get_weekPhase(w_date)+" - Week: "+ QString::number(w_date.weekNumber()));
}

enum {EDIT,MOVE,COPY,DEL};
enum {SAISON,SCHEDULE};

day_popup::~day_popup()
{
    delete ui;
}

void day_popup::init_dayWorkouts(QDate workDate)
{
    ui->tableWidget_day->clear();
    QString viewBackground = "background-color: #e6e6e6";
    QString workoutDate = workDate.toString(workSchedule->dateFormat);

    workoutMap = workSchedule->get_workouts(true,workoutDate);
    int workCount = workoutMap.count();
    ui->tableWidget_day->setRowCount(workCount);

    ui->label_weekinfo->setText(workoutDate + " - Phase: " + workSchedule->get_weekPhase(workDate));

    QString conString = " - ";
    QString itemValue = QString();
    QString delimiter = "#";

    for(QMap<int,QStringList>::const_iterator it = workoutMap.cbegin(); it != workoutMap.cend(); ++it)
    {
        this->set_currentSport(it.value().at(1));

        QTableWidgetItem *item = new QTableWidgetItem();
        itemValue = set_sectoTime(it.value().at(0).toInt()).toString()+conString+it.value().at(1)+conString+it.value().at(2)+delimiter+
                    it.value().at(3)+delimiter+
                    it.value().at(4)+delimiter+
                    set_time(it.value().at(5).toInt())+delimiter+
                    it.value().at(6)+" Km"+conString+it.value().at(7)+" TSS"+conString+it.value().at(8)+" KJ";
        item->setData(Qt::DisplayRole,itemValue);
        item->setData(Qt::AccessibleTextRole,it.value().at(1));
        ui->tableWidget_day->setItem(it.key(),0,item);
    }

    if(workCount == 0)
    {
        ui->toolButton_dayEdit->setEnabled(false);
    }
    else
    {
        ui->toolButton_dayEdit->setEnabled(true);
    }
    ui->dateEdit_workDate->setDate(workDate);
}

QMap<int,QStringList> day_popup::reorder_workouts(QMap<int,QStringList>*orderMap)
{
    QMap<int,QStringList> dayWorkouts,orderWorkouts;
    int workCounter = 0;

    for(QMap<int,QStringList>::const_iterator it = orderMap->cbegin(); it != orderMap->cend(); ++it)
    {
        if(orderWorkouts.contains(it.value().at(0).toInt()))
        {
            orderWorkouts.insert(it.value().at(0).toInt()+900,it.value());
        }
        else
        {
            orderWorkouts.insert(it.value().at(0).toInt(),it.value());
        }
    }

    for(QMap<int,QStringList>::const_iterator it = orderWorkouts.cbegin(); it != orderWorkouts.cend(); ++it)
    {
        dayWorkouts.insert(workCounter++,it.value());
    }
    return dayWorkouts;
}

void day_popup::on_comboBox_workSport_currentIndexChanged(const QString &sport)
{
    if(ui->comboBox_workSport->currentIndex() >= 0)
    {
        this->set_currentSport(sport);
        ui->lineEdit_selected->setText("Workout: "+QString::number(ui->spinBox_selWorkout->value()+1)+" "+sport);
    }
    else
    {
        this->set_currentSport(QString());
    }
    this->set_comboWorkouts(QString());
}

void day_popup::set_comboWorkouts(QString stdworkID)
{
    ui->comboBox_stdworkout->blockSignals(true);
    ui->comboBox_stdworkout->clear();
    QString workoutString;

    QHash<QString,QVector<QString>> workoutMap = stdWorkouts->get_workoutMap()->value(currentSport);
    for(QHash<QString,QVector<QString>>::const_iterator it = workoutMap.cbegin(), end = workoutMap.cend(); it != end; ++it)
    {
        workoutString = it.value().at(0)+ " - " + it.value().at(1) +" ("+ set_time(it.value().at(2).toInt()) + " - "+it.value().at(3)+" - "+it.value().at(4)+" - "+it.value().at(5)+")";
        ui->comboBox_stdworkout->addItem(workoutString,it.key());
    }
    ui->comboBox_stdworkout->model()->sort(0);

    if(!workoutMap.value(stdworkID).isEmpty())
    {
        ui->toolButton_map->setProperty("Image",workoutMap.value(stdworkID).at(7));
        ui->comboBox_stdworkout->setCurrentIndex(ui->comboBox_stdworkout->findData(stdworkID,Qt::UserRole));
    }
    else
    {
        ui->toolButton_map->setProperty("Image","none.png");
    }

    ui->comboBox_stdworkout->blockSignals(false);
}

void day_popup::set_controls(bool active)
{
    ui->dateEdit_workDate->setEnabled(active);
    ui->timeEdit_workTime->setEnabled(active);
    ui->comboBox_workSport->setEnabled(active);
    ui->comboBox_workCode->setEnabled(active);
    ui->lineEdit_workTitle->setEnabled(active);
    ui->lineEdit_workComment->setEnabled(active);
    ui->timeEdit_workDuration->setEnabled(active);
    ui->doubleSpinBox_workDistance->setEnabled(active);
    ui->spinBox_workStress->setEnabled(active);
    ui->spinBox_workKJoule->setEnabled(active);

    ui->toolButton_addWorkout->setEnabled(!active);
    ui->toolButton_addWorkout->setVisible(!active);
    ui->toolButton_clearMask->setVisible(active);

    ui->toolButton_copy->setEnabled(active);
    ui->toolButton_delete->setEnabled(active);
    ui->toolButton_editMove->setEnabled(active);
    ui->toolButton_upload->setEnabled(active);
    ui->comboBox_stdworkout->setEnabled(active);

    ui->spinBox_selWorkout->setVisible(false);
    ui->lineEdit_stdWorkID->setVisible(false);
}

void day_popup::set_exportContent()
{
    QStringList selectedWorkout = workoutMap.value(ui->spinBox_selWorkout->value());
    int sampCount = selectedWorkout.at(5).toInt();
    QVector<double> sampleValues(2,0);

    intervallMap.insert(0,qMakePair(0,sampCount));
    intNameMap.insert(0,"Workout");

    sampleUseKeys.insert(0,"SECS");
    for(int i = 0; i < sampCount; ++i)
    {
        sampleMap.insert(i,sampleValues);
    }

    QTime workoutTime;
    QDateTime workoutDateTime;
    QString tempDate,tempTime,sport,stressType,commonRI;
    QString totalWork = selectedWorkout.at(8);
    QString jsonFile;
    tempDate = popupDate.toString(workSchedule->dateFormat);
    tempTime = selectedWorkout.at(0);

    workoutTime = QTime::fromString(tempTime,"hh:mm");
    jsonFile = popupDate.toString("yyyy_MM_dd_") + workoutTime.toString("hh_mm_ss") +".json";
    workoutDateTime = QDateTime::fromString(tempDate+"T"+tempTime+":00","dd.MM.yyyyThh:mm:ss").toUTC();

    sport = selectedWorkout.at(1);

    if(sport == settings::SwimLabel) stressType = "swimscore";
    if(sport == settings::BikeLabel) stressType = "skiba_bike_score";
    if(sport == settings::RunLabel) stressType = "govss";
    if(sport == settings::AltLabel || sport == settings::StrengthLabel)
    {
        commonRI = QString::number(set_doubleValue(totalWork.toDouble() / this->calc_totalWork(10.0,sampCount,0)*10.0,false));
        if(sport == settings::StrengthLabel) commonRI = "4.0";
        stressType = "triscore";
    }

    this->rideData.insert("STARTTIME",workoutDateTime.toString("yyyy/MM/dd hh:mm:ss UTC"));
    this->rideData.insert("DEVICETYPE","Manual Import");
    this->rideData.insert("IDENTIFIER","");
    this->rideData.insert("OVERRIDES","");

    this->tagData.insert("Sport",sport);
    this->tagData.insert("Athlete",jsonHandler::gcValues->value("athlete"));
    this->tagData.insert("Filename",jsonFile);
    this->tagData.insert("CommonRI",commonRI);
    this->tagData.insert("Device","Manual Import");
    this->tagData.insert("Workout Code",selectedWorkout.at(2));
    this->tagData.insert("Workout Title",selectedWorkout.at(3));
    this->tagData.insert("Comment",selectedWorkout.at(4));
    this->tagData.insert("Workout Content",selectedWorkout.at(3));
    this->tagData.insert("Year",QString::number(popupDate.year()));
    this->tagData.insert("Month",QLocale().monthName(popupDate.month()));
    this->tagData.insert("Weekday",QLocale().dayName(popupDate.dayOfWeek(),QLocale::ShortFormat));

    this->hasOverride = true;
    overrideData.insert("time_riding",QString::number(sampCount));
    overrideData.insert("workout_time",QString::number(sampCount));
    overrideData.insert("total_kcalories",totalWork);
    overrideData.insert("total_distance",selectedWorkout.at(6));
    overrideData.insert("total_work",totalWork);
    overrideData.insert(stressType,selectedWorkout.at(7));

    this->prepareWrite_JsonFile();

    QMessageBox::information(this,"Export Workout","Workout Informations Exported!",QMessageBox::Ok);
}


void day_popup::on_tableWidget_day_itemClicked(QTableWidgetItem *item)
{
    QStringList workValues = workoutMap.value(item->row());
    this->set_currentSport(workValues.at(1));

    ui->lineEdit_selected->setText("Workout: "+QString::number(item->row()+1)+" "+workValues.at(1));
    ui->spinBox_selWorkout->setValue(item->row());

    ui->timeEdit_workTime->setTime(set_sectoTime(workValues.at(0).toInt()));
    ui->comboBox_workSport->setCurrentText(workValues.at(1));
    ui->comboBox_workCode->setCurrentText(workValues.at(2));
    ui->lineEdit_workTitle->setText(workValues.at(3));
    ui->lineEdit_workComment->setText(workValues.at(4));
    ui->timeEdit_workDuration->setTime(set_sectoTime(workValues.at(5).toInt()));
    ui->doubleSpinBox_workDistance->setValue(workValues.at(6).toDouble());
    ui->spinBox_workStress->setValue(workValues.at(7).toInt());
    ui->spinBox_workKJoule->setValue(workValues.at(8).toInt());
    ui->lineEdit_workPace->setText(workValues.at(9));
    ui->lineEdit_stdWorkID->setText(workValues.at(10));

    this->set_comboWorkouts(workValues.at(10));
    this->set_controls(true);
}

void day_popup::copy_workoutValue(QString value)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(value);
}

void day_popup::set_result(int result)
{
    QMap<int,QStringList> valueList;

    if(ui->toolButton_dayEdit->isChecked())
    {
        if(result == MOVE || result == COPY)
        {
            workSchedule->workoutUpdates.insert(ui->dateEdit_workDate->date(),workoutMap);
            if(result == MOVE)
            {
                workSchedule->workoutUpdates.insert(popupDate,valueList);
            }
        }
        if(result == DEL)
        {
            workSchedule->workoutUpdates.insert(popupDate,valueList);
        }
    }
    else
    {
        if(result == MOVE || result == COPY)
        {
            valueList = workSchedule->get_workouts(SCHEDULE,ui->dateEdit_workDate->date().toString(workSchedule->dateFormat));

            if(result == MOVE)
            {
                valueList.insert(valueList.count(), workoutMap.value(ui->spinBox_selWorkout->value()));
                workoutMap.remove(ui->spinBox_selWorkout->value());
            }
            else
            {
                valueList.insert(valueList.count(),workoutMap.value(ui->spinBox_selWorkout->value()));
            }

            workSchedule->workoutUpdates.insert(ui->dateEdit_workDate->date(),this->reorder_workouts(&valueList));
        }
        if(result == EDIT)
        {
            QStringList updateValues;
            updateValues.append(QString::number(get_secFromTime(ui->timeEdit_workTime->time())));
            updateValues.append(ui->comboBox_workSport->currentText());
            updateValues.append(ui->comboBox_workCode->currentText());
            updateValues.append(ui->lineEdit_workTitle->text());
            updateValues.append(ui->lineEdit_workComment->text());
            updateValues.append(QString::number(get_secFromTime(ui->timeEdit_workDuration->time())));
            updateValues.append(QString::number(ui->doubleSpinBox_workDistance->value()));
            updateValues.append(QString::number(ui->spinBox_workStress->value()));
            updateValues.append(QString::number(ui->spinBox_workKJoule->value()));
            updateValues.append(ui->lineEdit_workPace->text());
            updateValues.append(ui->lineEdit_stdWorkID->text());

            workoutMap.insert(ui->spinBox_selWorkout->value(),updateValues);
            workSchedule->workoutUpdates.insert(popupDate,this->reorder_workouts(&workoutMap));
        }
        if(result == DEL)
        {
            workSchedule->update_linkedWorkouts(popupDate,ui->lineEdit_stdWorkID->text(),ui->spinBox_selWorkout->value(),false);
            workoutMap.remove(ui->spinBox_selWorkout->value());
            workSchedule->workoutUpdates.insert(popupDate,this->reorder_workouts(&workoutMap));
        }
    }

    workSchedule->set_workoutData();

    ui->dateEdit_workDate->setEnabled(false);
    this->set_controls(false);
    ui->lineEdit_selected->setText("-");
    this->init_dayWorkouts(popupDate);
    ui->toolButton_dayEdit->setChecked(false);

}

void day_popup::reset_controls()
{
    ui->lineEdit_selected->setText("-");
    ui->timeEdit_workTime->setTime(QTime(0,0,0));
    ui->comboBox_workSport->setCurrentIndex(-1);
    ui->comboBox_workCode->setCurrentIndex(-1);
    ui->lineEdit_workTitle->setText("-");
    ui->lineEdit_workComment->setText("-");
    ui->timeEdit_workDuration->setTime(QTime(0,0,0));
    ui->doubleSpinBox_workDistance->setValue(0);
    ui->spinBox_workStress->setValue(0);
    ui->spinBox_workKJoule->setValue(0);
    ui->lineEdit_workPace->setText("-");
    ui->lineEdit_stdWorkID->setText("-");
    ui->spinBox_selWorkout->setValue(-1);

    this->set_comboWorkouts(QString());
    this->set_controls(false);
}

void day_popup::on_toolButton_close_clicked()
{
    reject();
}

void day_popup::on_toolButton_editMove_clicked()
{
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

void day_popup::on_toolButton_dayEdit_clicked(bool checked)
{
    QPalette selectBox;

    if(checked)
    {
        ui->lineEdit_selected->setText("COMPLETE DAY");

        selectBox.setColor(QPalette::Base,Qt::red);
        selectBox.setColor(QPalette::Text,Qt::white);
        this->set_controls(false);
        ui->toolButton_delete->setEnabled(checked);  
    }
    else
    {
       ui->lineEdit_selected->setText("-");

       selectBox.setColor(QPalette::NoRole,Qt::NoBrush);
       selectBox.setColor(QPalette::Text,Qt::black);
       this->set_controls(checked);
    }
    ui->dateEdit_workDate->setEnabled(checked);
    ui->lineEdit_selected->setPalette(selectBox);

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

void day_popup::on_comboBox_stdworkout_currentIndexChanged(int index)
{
    Q_UNUSED(index)

    QString workoutID = ui->comboBox_stdworkout->currentData().toString();
    QVector<QString> stdWorkoutData = stdWorkouts->get_workoutMap()->value(currentSport).value(workoutID);

    if(!stdWorkoutData.isEmpty())
    {
        ui->comboBox_workCode->setCurrentText(stdWorkoutData.at(0));
        ui->lineEdit_workTitle->setText(stdWorkoutData.at(1));
        ui->timeEdit_workDuration->setTime(set_sectoTime(stdWorkoutData.at(2).toInt()));
        ui->doubleSpinBox_workDistance->setValue(stdWorkoutData.at(3).toDouble());
        ui->spinBox_workStress->setValue(stdWorkoutData.at(4).toInt());
        ui->spinBox_workKJoule->setValue(stdWorkoutData.at(5).toInt());
        ui->lineEdit_workPace->setText(this->get_workout_pace(stdWorkoutData.at(3).toDouble(),stdWorkoutData.at(2).toInt(),currentSport,true));
        ui->toolButton_map->setProperty("Image",stdWorkoutData.at(7));
        ui->lineEdit_stdWorkID->setText(workoutID);
    }
}

void day_popup::on_toolButton_map_clicked()
{
    Dialog_map dialogMap(this,stdWorkouts,ui->toolButton_map->property("Image").toString());
    dialogMap.setModal(true);
    dialogMap.exec();
}

void day_popup::on_dateEdit_workDate_dateChanged(const QDate &date)
{
    ui->lineEdit_workoutInfo->setText(workSchedule->get_weekPhase(date)+" - Week: "+ QString::number(date.weekNumber()));

    QString weekNumber = this->calc_weekID(date);

    if(date != popupDate)
    {
        moveWorkout = true;
        this->set_controls(true);
        ui->toolButton_delete->setEnabled(false);
    }
    else
    {
       this->set_controls(false);
       ui->toolButton_delete->setEnabled(true);
    }
}

void day_popup::on_toolButton_title_clicked()
{
    this->copy_workoutValue(ui->lineEdit_workTitle->text());
}

void day_popup::on_toolButton_comment_clicked()
{
    this->copy_workoutValue(ui->lineEdit_workComment->text());
}

void day_popup::on_toolButton_addWorkout_clicked()
{
    ui->timeEdit_workTime->setTime(QTime());
    ui->comboBox_workSport->setCurrentIndex(-1);
    ui->comboBox_workCode->setCurrentIndex(-1);
    ui->lineEdit_workTitle->setText("-");
    ui->lineEdit_workComment->setText("-");
    ui->timeEdit_workDuration->setTime(QTime());
    ui->doubleSpinBox_workDistance->setValue(0);
    ui->spinBox_workStress->setValue(0);
    ui->spinBox_workKJoule->setValue(0);
    ui->lineEdit_workPace->setText("-");
    ui->lineEdit_stdWorkID->setText("-");
    ui->spinBox_selWorkout->setValue(workoutMap.count());

    ui->lineEdit_selected->setText("Workout: "+QString::number(workoutMap.count()+1));
    this->set_controls(true);
}

void day_popup::on_toolButton_clearMask_clicked()
{
    this->reset_controls();
}
