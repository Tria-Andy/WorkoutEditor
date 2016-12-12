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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTableWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    settings::loadSettings();
    userSetup = true;
    saisonWeeks = settings::get_saisonInfo("weeks").toInt();
    workSchedule = new schedule();
    work_list << "Phase:" << "Week:" << "Date:" << "Time:" << "Sport:" << "Code:" << "Title:" << "Duration:" << "Distance:" << "Stress:";
    sum_name << "Workouts:" << "Duration:" << "Distance:" << "StressScore:";
    schedMode << "Week" << "Year";
    sum_header << "Summery:";
    year_header << "Week"
                << settings::isSwim
                << settings::isBike
                << settings::isRun
                << settings::isStrength
                << settings::isAlt
                << "Summery";
    selectedDate = QDate::currentDate();
    firstdayofweek = selectedDate.addDays(1 - selectedDate.dayOfWeek());
    weeknumber = QString::number(selectedDate.weekNumber()) +"_"+QString::number(selectedDate.year());
    weekRange = settings::get_weekRange();
    weekpos = 0;
    weekDays = 7;
    weekCounter = 0;
    work_sum.resize(7);
    dur_sum.resize(7);
    dist_sum.resize(7);
    stress_sum.resize(7);
    isWeekMode = true;
    safeFlag = false;
    sel_count = 0;
    ui->label_month->setText("Week " + weeknumber + " - " + QString::number(selectedDate.addDays(weekRange*7).weekNumber()-1));
    ui->pushButton_current_week->setEnabled(false);
    ui->pushButton_week_minus->setEnabled(false);
    ui->pushButton_ClearWorkContent->setEnabled(false);
    ui->pushButton_sync->setEnabled(false);
    ui->pushButton_putInt->setEnabled(false);
    fontSize = 10;
    cal_header << "Week";
    for(int d = 1; d < 8; ++d)
    {
        cal_header << QDate::longDayName(d);
    }
    stdWorkout = new standardWorkouts();
    calendar_model = new QStandardItemModel();
    sum_model = new QStandardItemModel();
    connect(ui->actionExit_and_Save, SIGNAL(triggered()), this, SLOT(close()));

    ui->actionSave_Workout_Schedule->setEnabled(false);
    ui->actionEditor->setEnabled(true);
    ui->actionPlaner->setIcon(QIcon(":/images/icons/Yes.png"));
    ui->actionPlaner->setEnabled(false);
    ui->stackedWidget->setGeometry(5,5,0,0);
    ui->frame_avgValue->setVisible(false);
    ui->frame_polish->setVisible(false);
    this->summery_view();
    ui->textBrowser_Info->setStyleSheet("background-color: lightgrey");
    ui->tableView_int_times->setStyleSheet("background-color: lightgrey");
    ui->tableView_int->setStyleSheet("background-color: lightgrey");
    ui->comboBox_schedMode->addItems(schedMode);
    ui->comboBox_phasefilter->addItem("All");
    ui->comboBox_phasefilter->addItems(settings::get_phaseList());
    ui->comboBox_phasefilter->setEnabled(false);
    ui->horizontalSlider_polish->setEnabled(false);
    ui->horizontalSlider_factor->setEnabled(false);
    ui->horizontalSlider_factor->setVisible(false);
    this->set_menuItems(false,true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::freeMem()
{
    if(userSetup)
    {
        calendar_model->clear();
        delete stdWorkout;
        delete workSchedule;
        delete sum_model;
        delete calendar_model;
    }
}

void MainWindow::openPreferences()
{
    Dialog_settings dia_settings(this);
    dia_settings.setModal(true);
    dia_settings.exec();
}

void MainWindow::set_menuItems(bool mEditor,bool mPlaner)
{
    if(mEditor)
    {
        ui->actionPlaner->setEnabled(mEditor);
        ui->actionPlaner->setIcon(QIcon(""));
        ui->actionEditor->setIcon(QIcon(":/images/icons/Yes.png"));
        ui->actionEditor->setEnabled(mPlaner);
    }
    if(mPlaner)
    {
        ui->actionEditor->setEnabled(mPlaner);
        ui->actionEditor->setIcon(QIcon(""));
        ui->actionPlaner->setIcon(QIcon(":/images/icons/Yes.png"));
        ui->actionPlaner->setEnabled(mEditor);
    }

    //Editor
    ui->menuAction->setEnabled(mEditor);
    ui->actionSave_Workout_File->setVisible(mEditor);
    ui->actionSave_to_GoldenCheetah->setVisible(mEditor);
    ui->actionReset->setVisible(mEditor);
    ui->actionSelect_File->setVisible(mEditor);
    ui->actionUnselect_all_rows->setVisible(mEditor);
    ui->actionEdit_Distance->setVisible(mEditor);
    ui->actionEdit_Undo->setVisible(mEditor);
    ui->actionLapEditor->setVisible(mEditor);

    ui->actionReset->setEnabled(settings::get_act_isload());
    ui->actionSave_to_GoldenCheetah->setEnabled(settings::get_act_isload());
    ui->actionUnselect_all_rows->setEnabled(settings::get_act_isload());
    ui->actionEdit_Distance->setEnabled(settings::get_act_isload());
    ui->actionEdit_Undo->setEnabled(settings::get_act_isload());
    ui->actionLapEditor->setEnabled(settings::get_act_isload());

    //Schedule
    ui->menuWorkout->setEnabled(mPlaner);
    ui->actionSave_Workout_Schedule->setVisible(mPlaner);
    ui->actionExport_to_Golden_Cheetah->setVisible(mPlaner);
    ui->actionNew->setVisible(mPlaner);
}

QString MainWindow::set_summeryString(int pos,bool week)
{
    QString sumString;
    QString sum_name = "Summery";
    double percent = 0.0;

    if(week)
    {
        if(pos == 0)
        {
            if(dur_sum[pos] != 0) percent = (static_cast<double>(dur_sum[pos]) / static_cast<double>(dur_sum[0]))*100;
            sumString = sum_name +"-"+ QString::number(work_sum[pos]) +"-"+ settings::set_time(dur_sum[pos]) +"-"+ QString::number(settings::set_doubleValue(percent,false)) +"-"+ QString::number(dist_sum[pos]) +"-"+ QString::number(stress_sum[pos]);
        }
        else
        {
            if(dur_sum[pos] != 0) percent = static_cast<double>(dur_sum[pos]) / (static_cast<double>(dur_sum[0]))*100;
            sumString = settings::get_sportList().at(pos-1) +"-"+ QString::number(work_sum[pos]) +"-"+ settings::set_time(dur_sum[pos]) +"-"+ QString::number(settings::set_doubleValue(percent,false)) +"-"+ QString::number(dist_sum[pos]) +"-"+QString::number(stress_sum[pos]);
        }
    }
    else
    {
        if(pos == 0)
        {
            if(dur_sum[5] != 0) percent = (static_cast<double>(dur_sum[5]) / static_cast<double>(dur_sum[5]))*100;
            sumString = sum_name +"-"+ QString::number(work_sum[5]) +"-"+ settings::set_time(dur_sum[5]) +"-"+ QString::number(settings::set_doubleValue(percent,false)) +"-"+ QString::number(dist_sum[5]) +"-"+ QString::number(stress_sum[5]);
        }
        else
        {
            if(dur_sum[pos-1] != 0) percent = (static_cast<double>(dur_sum[pos-1]) / static_cast<double>(dur_sum[5]))*100;
            sumString = settings::get_sportList().at(pos-1) +"-"+ QString::number(work_sum[pos-1]) +"-"+ settings::set_time(dur_sum[pos-1]) +"-"+ QString::number(settings::set_doubleValue(percent,false)) +"-"+ QString::number(dist_sum[pos-1]) +"-"+QString::number(stress_sum[pos-1]);
        }
    }
    return sumString;
}

void MainWindow::summery_calc(int pos, QModelIndex index,bool week)
{
    if(week)
    {
        work_sum[pos] = work_sum[pos] + 1;
        dur_sum[pos] = dur_sum[pos] + settings::get_timesec(workSchedule->workout_schedule->item(index.row(),6)->text());
        dist_sum[pos] = dist_sum[pos] + workSchedule->workout_schedule->item(index.row(),7)->text().toDouble();
        stress_sum[pos] = stress_sum[pos] + workSchedule->workout_schedule->item(index.row(),8)->text().toInt();
    }
    else
    {
        QStringList sumValues;
        QString work,dura,dist,stress;
        sumValues = workSchedule->week_content->data(index).toString().split("-");

        work = sumValues.at(0);
        dist = sumValues.at(1);
        dura = sumValues.at(2);
        stress = sumValues.at(3);

        work_sum[pos] = work_sum[pos] + work.toInt();
        dur_sum[pos] = dur_sum[pos] + (settings::get_timesec(dura)*60);
        dist_sum[pos] = dist_sum[pos] + dist.toDouble();
        stress_sum[pos] = stress_sum[pos] + stress.toInt();
    }
}

void MainWindow::set_summerInfo()
{
    if(isWeekMode)
    {
        QStringList weekInfo = weeknumber.split("_");
        QString year = weekInfo.at(1);
        QString week = weekInfo.at(0);
        QDate firstday,calcDay;
        calcDay.setDate(year.toInt(),1,1);
        firstday = calcDay.addDays(week.toInt()*7).addDays(1 - calcDay.dayOfWeek());
        ui->label_selWeek->setText("Week: "+weeknumber+" - Phase: " +workSchedule->get_weekPhase(firstday));
    }
    else
    {
        if(ui->comboBox_phasefilter->currentIndex() == 0)
        {
            ui->label_selWeek->setText("All Phases " +settings::get_saisonInfo("saison"));
        }
        else
        {
            ui->label_selWeek->setText("Phase: "+ ui->comboBox_phasefilter->currentText());
        }
    }
}

void MainWindow::summery_view()
{
    sum_model->clear();
    sum_model->setHorizontalHeaderLabels(sum_header);
    ui->tableView_summery->setModel(sum_model);
    ui->tableView_summery->verticalHeader()->resetDefaultSectionSize();
    ui->tableView_summery->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_summery->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView_summery->verticalHeader()->setDefaultSectionSize(ui->tableView_summery->verticalHeader()->defaultSectionSize()*3.5);
    ui->tableView_summery->horizontalHeader()->setVisible(false);
    ui->tableView_summery->verticalHeader()->setVisible(false);
    ui->tableView_summery->setItemDelegate(&sum_del);
    ui->tableView_summery->setEditTriggers(QAbstractItemView::NoEditTriggers);
    QList<QStandardItem*> list;

    QModelIndex index;
    QString sport,weekID;
    QStringList sumValues;
    int rowcount;

    work_sum.fill(0);
    dur_sum.fill(0);
    dist_sum.fill(0.0);
    stress_sum.fill(0);

    if(isWeekMode)
    {
        list = workSchedule->workout_schedule->findItems(weeknumber,Qt::MatchExactly,0);

        for(int i = 0; i < list.count(); ++i)
        {
            index = workSchedule->workout_schedule->indexFromItem(list.at(i));
            sport = workSchedule->workout_schedule->item(index.row(),3)->text();

            if(sport != settings::isOther)
            {
                this->summery_calc(0,index,isWeekMode);
            }
            if(sport == settings::isSwim)
            {
                this->summery_calc(1,index,isWeekMode);
            }
            if(sport == settings::isBike)
            {
                this->summery_calc(2,index,isWeekMode);
            }
            if(sport == settings::isRun)
            {
                this->summery_calc(3,index,isWeekMode);
            }
            if(sport == settings::isStrength)
            {
                this->summery_calc(4,index,isWeekMode);
            }
            if(sport == settings::isAlt)
            {
                this->summery_calc(5,index,isWeekMode);
            }
            if(sport == settings::isTria)
            {
                this->summery_calc(6,index,isWeekMode);
            }
        }

        for(int i = 0; i < 7; ++i)
        {
            if(work_sum[i] > 0)
            {
                sumValues << this->set_summeryString(i,isWeekMode);
            }
        }
    }
    else
    {
        if(ui->comboBox_phasefilter->currentIndex() == 0)
        {
            for(int row = 0; row < workSchedule->week_content->rowCount(); ++row)
            {
                for(int col = 2; col < workSchedule->week_content->columnCount(); ++col)
                {
                    index = workSchedule->week_content->index(row,col,QModelIndex());
                    this->summery_calc(col-2,index,isWeekMode);
                }
            }
            sumValues << this->set_summeryString(0,isWeekMode);
        }
        else
        {
            list = workSchedule->week_meta->findItems(ui->comboBox_phasefilter->currentText(),Qt::MatchContains,2);

            for(int i = 0; i < list.count(); ++i)
            {
                index = workSchedule->week_meta->indexFromItem(list.at(i));
                weekID = workSchedule->week_meta->item(index.row(),1)->text();

                for(int x = 0; x < workSchedule->week_content->rowCount(); ++x)
                {
                    if(weekID == workSchedule->week_content->data(workSchedule->week_content->index(x,1,QModelIndex())).toString())
                    {
                        for(int col = 2; col < workSchedule->week_content->columnCount(); ++col)
                        {
                            index = workSchedule->week_content->index(x,col,QModelIndex());
                            this->summery_calc(col-2,index,isWeekMode);
                        }
                    }
                }
            }
            sumValues << this->set_summeryString(0,isWeekMode);
        }

        for(int i = 1; i < 6; ++i)
        {
            sumValues << this->set_summeryString(i,isWeekMode);
        }
    }

    for(int i = 0; i < sumValues.count(); ++i)
    {
        rowcount = sum_model->rowCount();
        sum_model->insertRow(rowcount,QModelIndex());
        sum_model->setData(sum_model->index(i,0,QModelIndex()),sumValues.at(i));
    }
    this->set_summerInfo();
}

void MainWindow::workout_calendar()
{
    QModelIndex index,cal_index;
    QDate currentdate = QDate::currentDate();
    QDate workout_date = currentdate;
    QString delimiter = "#";
    QString w_connect = " - ";
    QString weekValue,cal_value,phase_value;
    int dayofweek = currentdate.dayOfWeek();
    int rowcount;
    QList<QStandardItem*> worklist,meta;

    delete calendar_model;
    workSchedule->workout_schedule->sort(2);

    if(isWeekMode)
    {
        calendar_model = new QStandardItemModel();
        calendar_model->setHorizontalHeaderLabels(cal_header);
        ui->tableView_cal->setModel(calendar_model);
        ui->tableView_cal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView_cal->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView_cal->verticalHeader()->hide();
        ui->tableView_cal->setItemDelegate(&calender_del);

        int offset = (1 - dayofweek) + (weekDays*weekCounter);

        for(int week = 0; week < weekRange; ++week)
        {
            rowcount = calendar_model->rowCount();
            calendar_model->insertRow(rowcount,QModelIndex());
            weekValue = QString::number(currentdate.addDays(offset).weekNumber()) +"_"+ QString::number(currentdate.addDays(offset).year());
            meta = workSchedule->week_meta->findItems(weekValue,Qt::MatchExactly,1);

            for(int day = 0; day < 8 ; ++day)
            {
                workout_date = currentdate.addDays(offset);

                worklist = workSchedule->workout_schedule->findItems(workout_date.toString("dd.MM.yyyy"),Qt::MatchExactly,1);

                for(int wa = 0; wa < worklist.count(); ++wa)
                {
                    if(!worklist.isEmpty())
                    {
                        index = workSchedule->workout_schedule->indexFromItem(worklist.at(wa));
                        cal_value = cal_value + (workSchedule->workout_schedule->item(index.row(),3)->text() + w_connect);
                        cal_value = cal_value + (workSchedule->workout_schedule->item(index.row(),4)->text() + "\n");
                        cal_value = cal_value + (workSchedule->workout_schedule->item(index.row(),6)->text().left(5) + w_connect);
                        cal_value = cal_value + (workSchedule->workout_schedule->item(index.row(),7)->text() + " km" + delimiter);
                    }
                    else
                    {
                        cal_value = QString();
                    }
                }
                cal_index = calendar_model->index(week,day,QModelIndex());

                if(day == 0)
                {
                    if(!meta.isEmpty())
                    {
                        phase_value = workSchedule->get_weekPhase(workout_date);
                    }
                    else
                    {
                        phase_value = settings::get_emptyPhase();
                    }
                    calendar_model->setData(cal_index,weekValue + delimiter + phase_value);
                }
                else
                {
                    calendar_model->setData(cal_index,workout_date.toString("dd MMM yy") + delimiter + cal_value);
                    ++offset;
                }
                cal_value = phase_value = QString();
            }
        }
    }
    else
    {
          calendar_model = new QStandardItemModel();
          calendar_model->setHorizontalHeaderLabels(year_header);
          ui->tableView_cal->setModel(calendar_model);
          ui->tableView_cal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
          ui->tableView_cal->verticalHeader()->hide();
          ui->tableView_cal->setItemDelegate(&week_del);
          QString weekInfo,weekID;
          QString empty = "0-0-00:00:00-0";
          int weekoffset;
          bool showAll;
          workSchedule->week_meta->sort(0);

          if(ui->comboBox_phasefilter->currentIndex() == 0)
          {
            ui->tableView_cal->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            showAll = true;
            weekoffset = settings::get_weekOffSet();
          }
          else
          {
            ui->tableView_cal->verticalHeader()->resetDefaultSectionSize();
            ui->tableView_cal->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
            ui->tableView_cal->verticalHeader()->setDefaultSectionSize(ui->tableView_cal->verticalHeader()->defaultSectionSize()*2.5);
            showAll = false;
            worklist.clear();
            meta.clear();
            meta = workSchedule->week_meta->findItems(ui->comboBox_phasefilter->currentText(),Qt::MatchContains,2);
            weekoffset = meta.count();
          }

          for(int w = weekpos,i=0; w < weekpos+weekoffset;++w,++i)
          {
              rowcount = calendar_model->rowCount();
              calendar_model->insertRow(rowcount,QModelIndex());
              weekID = workSchedule->week_meta->data(workSchedule->week_meta->index(w,1,QModelIndex())).toString();

              if(showAll)
              {
                worklist = workSchedule->week_content->findItems(weekID,Qt::MatchExactly,1);
              }
              else
              {
                  index = workSchedule->week_meta->indexFromItem(meta.at(i));
                  weekID = workSchedule->week_meta->item(index.row(),1)->text();
                  worklist << workSchedule->week_content->findItems(weekID,Qt::MatchExactly,1);
              }

              for(int col = 0; col < 7;++col)
              {
                  cal_index = calendar_model->index(i,col,QModelIndex());
                  if(!worklist.isEmpty())
                  {
                      if(showAll)
                      {
                        index = workSchedule->week_content->indexFromItem(worklist.at(0));
                        if(col == 0)
                        {
                          weekInfo = workSchedule->week_meta->data(workSchedule->week_meta->index(w,0,QModelIndex())).toString() + "-" +
                                     workSchedule->week_meta->data(workSchedule->week_meta->index(w,1,QModelIndex())).toString() + "-" +
                                     workSchedule->week_meta->data(workSchedule->week_meta->index(w,3,QModelIndex())).toString() + "-" +
                                     workSchedule->week_meta->data(workSchedule->week_meta->index(w,2,QModelIndex())).toString();
                        }
                      }
                      else
                      {
                        index = workSchedule->week_content->indexFromItem(worklist.at(i));
                        if(col == 0)
                        {
                          weekInfo = workSchedule->week_meta->item(index.row(),0)->text() + "-" +
                                     workSchedule->week_meta->item(index.row(),1)->text() + "-" +
                                     workSchedule->week_meta->item(index.row(),3)->text() + "-" +
                                     workSchedule->week_meta->item(index.row(),2)->text();
                        }
                      }

                      if(col > 0)
                      {
                        weekInfo = workSchedule->week_content->item(index.row(),col+1)->text();
                      }
                  }
                  else
                  {
                    weekInfo = empty;
                  }
                  calendar_model->setData(cal_index,weekInfo);
              }
          }
     }
}

void MainWindow::refresh_model()
{
    this->summery_view();
    this->workout_calendar();
}

void MainWindow::on_actionNew_triggered()
{
    int dialog_code;

    if(isWeekMode)
    {
        Dialog_add new_workout(this,workSchedule,stdWorkout);
        new_workout.setModal(true);
        dialog_code = new_workout.exec();

        if(dialog_code == QDialog::Accepted)
        {
            workSchedule->add_workout();
            safeFlag = true;
            ui->actionSave_Workout_Schedule->setEnabled(true);
            this->refresh_model();
        }
    }
}

void MainWindow::on_actionStress_Calculator_triggered()
{
    Dialog_stresscalc stressCalc(this);
    stressCalc.setModal(true);
    stressCalc.exec();
}

void MainWindow::on_actionSave_Workout_Schedule_triggered()
{
    if(isWeekMode)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      tr("Save Workouts"),
                                      "Save Week Workout Schedule?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            workSchedule->save_workout_file();
            ui->actionSave_Workout_Schedule->setEnabled(false);
            safeFlag = false;
        }
    }
    else
    {
        QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          tr("Save Schedule"),
                                          "Save Current Year Schedule?",
                                          QMessageBox::Yes|QMessageBox::No
                                          );
        if (reply == QMessageBox::Yes)
        {
            workSchedule->save_week_files();
            ui->actionSave_Workout_Schedule->setEnabled(false);
            safeFlag = false;
        }
    }
}

void MainWindow::on_actionSave_to_GoldenCheetah_triggered()
{
    QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      tr("Save File"),
                                      "Save Changes to Golden Cheetah?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
    if (reply == QMessageBox::Yes)
    {
        jsonhandler->write_json();
    }
}

void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
   weeknumber = QString::number(date.weekNumber())+"_"+QString::number(date.year());
   this->summery_view();
}

void MainWindow::on_tableView_cal_clicked(const QModelIndex &index)
{
    int dialog_code;
    if(isWeekMode)
    {
        if(index.column() != 0)
        {
            QString getdate = calendar_model->data(index,Qt::DisplayRole).toString().left(9);
            QDate selectDate = QDate::fromString(getdate,"dd MMM yy").addYears(100);
            day_popup day_pop(this,selectDate,workSchedule);
            day_pop.setModal(true);
            dialog_code = day_pop.exec();

            if(dialog_code == QDialog::Accepted)
            {

              Dialog_edit edit_workout(this,selectDate,workSchedule,stdWorkout);
              edit_workout.setModal(true);
              dialog_code = edit_workout.exec();
              if(dialog_code == QDialog::Accepted)
              {
                  if(edit_workout.get_result() == 1) workSchedule->edit_workout(edit_workout.get_edit_index());
                  if(edit_workout.get_result() == 2) workSchedule->add_workout();
                  if(edit_workout.get_result() == 3) workSchedule->delete_workout(edit_workout.get_edit_index());
                  safeFlag = true;
                  ui->actionSave_Workout_Schedule->setEnabled(true);
                  this->refresh_model();
              }
            }
        }
        else
        {
            QString selected_week =  calendar_model->data(index,Qt::DisplayRole).toString();
            weeknumber = selected_week.split("#").at(0);

            this->summery_view();

            week_popup week_pop(this,selected_week,workSchedule);
            week_pop.setModal(true);
            dialog_code = week_pop.exec();

            if(dialog_code == QDialog::Accepted)
            {
                Dialog_week_copy week_copy(this,selected_week,workSchedule);
                week_copy.setModal(true);
                dialog_code = week_copy.exec();

                if(dialog_code == QDialog::Accepted)
                {
                    this->workout_calendar();
                    //weekCounter = 0;
                    this->set_calender();
                    safeFlag = true;
                    ui->actionSave_Workout_Schedule->setEnabled(true);
                }
            }

            if(dialog_code == QDialog::Rejected)
            {
                weekCounter = 0;
                this->set_calender();
            }
        }
    }
    else
    {
        if(index.column() == 0)
        {
            QString selected_week = calendar_model->data(index,Qt::DisplayRole).toString();

            Dialog_addweek new_week(this,selected_week,workSchedule);
            new_week.setModal(true);
            int dialog_code = new_week.exec();

            if(dialog_code == QDialog::Accepted)
            {
                this->workout_calendar();
                this->summery_view();
                safeFlag = true;
                ui->actionSave_Workout_Schedule->setEnabled(true);
            }
        }
    }
}

void MainWindow::on_actionExport_to_Golden_Cheetah_triggered()
{
    Dialog_export export_workout(this,workSchedule);
    export_workout.setModal(true);
    export_workout.exec();
}

QString MainWindow::get_weekRange()
{
    QString display_weeks;
    QModelIndex index;
    int phaseStart;
    if(isWeekMode)
    {
        if(weekCounter != 0)
        {
            display_weeks = QString::number(firstdayofweek.addDays(weekDays*weekCounter).weekNumber()) + " - " +
                            QString::number(firstdayofweek.addDays(((weekRange-1)*weekDays)+(weekDays*weekCounter)).weekNumber());
        }
        else
        {
            display_weeks = QString::number(firstdayofweek.addDays(weekDays*weekCounter).weekNumber()) + " - " +
                            QString::number(firstdayofweek.addDays((weekRange-1)*weekDays).weekNumber());
        }

    }
    else
    {
        if(ui->comboBox_phasefilter->currentIndex() == 0)
        {
            display_weeks = QString::number(weekpos+1) + " - " + QString::number(weekpos + settings::get_weekOffSet());
        }
        else
        {
            QList<QStandardItem*> list = workSchedule->week_meta->findItems(ui->comboBox_phasefilter->currentText(),Qt::MatchContains,2);
            index = workSchedule->week_meta->indexFromItem(list.at(0));
            phaseStart = workSchedule->week_meta->item(index.row(),0)->text().toInt();
            display_weeks = QString::number(phaseStart) + " - " + QString::number(phaseStart + (list.count()-1));
        }
    }
    return display_weeks;
}

void MainWindow::set_buttons(bool set_value)
{
    ui->pushButton_week_minus->setEnabled(set_value);
    ui->pushButton_current_week->setEnabled(set_value);
}

void MainWindow::set_calender()
{

    if(weekCounter == 0)
    {
        ui->calendarWidget->setSelectedDate(QDate::currentDate());
    }
    else
    {
        ui->calendarWidget->setSelectedDate(selectedDate.addDays((1-selectedDate.currentDate().dayOfWeek())+weekDays*weekCounter));
    }

    weeknumber = QString::number(ui->calendarWidget->selectedDate().weekNumber())+"_"+QString::number(ui->calendarWidget->selectedDate().year());
    this->summery_view();
}

void MainWindow::set_comboIntervall()
{
    if(settings::get_act_isrecalc())
    {
        int rowCount = curr_activity->edit_int_model->rowCount();

        for(int i = 0; i < rowCount; ++i)
        {
            ui->comboBox_intervals->setItemText(i,curr_activity->edit_int_model->data(curr_activity->edit_int_model->index(i,0,QModelIndex())).toString());
        }
        if(ui->comboBox_intervals->count() > rowCount)
        {
            while(ui->comboBox_intervals->count() != rowCount)
            {
                ui->comboBox_intervals->removeItem(ui->comboBox_intervals->count()-1);
            }
        }
        else
        {
            for(int i = ui->comboBox_intervals->count(); i < rowCount; ++i)
            {
                ui->comboBox_intervals->addItem(curr_activity->edit_int_model->data(curr_activity->edit_int_model->index(i,0,QModelIndex())).toString());
            }
        }
    }
    else
    {
        for(int i = 0; i < curr_activity->edit_int_model->rowCount();++i)
        {
            ui->comboBox_intervals->addItem(curr_activity->edit_int_model->data(curr_activity->edit_int_model->index(i,0,QModelIndex())).toString());
        }
    }
}

void MainWindow::select_activity_file()
{
    QMessageBox::StandardButton reply;
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Select File"),
                "C://",
                "JSON Files (*.json)"
                );

    if(filename != "")
    {
        reply = QMessageBox::question(this,
                                  tr("Open Selected File!"),
                                  filename,
                                  QMessageBox::Yes|QMessageBox::No
                                  );
        if (reply == QMessageBox::Yes)
        {
            this->loadfile(filename);
        }
    }
}

void MainWindow::on_pushButton_current_week_clicked()
{
    if(isWeekMode)
    {
        weekCounter = 0;
        this->set_calender();
        this->set_buttons(false);
        this->refresh_model();
    }
    else
    {
        weekpos = 0;
        ui->pushButton_fourplus->setEnabled(true);
        ui->pushButton_week_plus->setEnabled(true);
        this->set_buttons(false);
        this->workout_calendar();
    }
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::loadfile(const QString &filename)
{
    QFile file(filename);
    QFileInfo fileinfo(filename);
    QString filecontent;

    if(fileinfo.suffix() == "json")
    {
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox::warning(this, tr("Application"),
                                     tr("Cannot read file %1:\n%2.")
                                     .arg(filename)
                                     .arg(file.errorString()));
           return;
        }
        curr_activity = new Activity();
        filecontent = file.readAll();
        jsonhandler = new jsonHandler(true,filecontent,curr_activity);
        curr_activity->set_jsonhandler(jsonhandler);
        file.close();

        settings::set_act_isload(true);
        ui->actionSelect_File->setEnabled(false);
        this->set_menuItems(true,false);
        this->set_activty_infos();

        intChart = new QChart();
        intChartview = new QChartView(intChart);
        intChartview->setRenderHint(QPainter::Antialiasing);
        ui->verticalLayout_interpol->addWidget(intChartview);

        avgLine = workSchedule->get_qLineSeries(false);
        avgLine->setColor(QColor(Qt::yellow));
        avgLine->setName("Avg Speed");
        speedLine = workSchedule->get_qLineSeries(false);
        speedLine->setColor(QColor(Qt::green));
        speedLine->setName("Speed");


        axisX = new QCategoryAxis();
        axisX->setVisible(false);

        intChart->addSeries(avgLine);
        intChart->addSeries(speedLine);

        ySpeed = workSchedule->get_qValueAxis("Speed",true,20,5);
        intChart->addAxis(ySpeed,Qt::AlignLeft);
        avgLine->attachAxis(ySpeed);
        speedLine->attachAxis(ySpeed);

        intChart->setAxisX(axisX,avgLine);
        intChart->setAxisX(axisX,speedLine);


        if(curr_activity->get_sport() == settings::isRun || curr_activity->get_sport() == settings::isBike)
        {
            polishLine = workSchedule->get_qLineSeries(false);
            polishLine->setColor(QColor(Qt::red));
            polishLine->setName("Polished Speed");
            intChart->addSeries(polishLine);
            polishLine->attachAxis(ySpeed);
            intChart->setAxisX(axisX,polishLine);
            ui->horizontalSlider_polish->setEnabled(true);
            ui->horizontalSlider_factor->setEnabled(true);
            ui->horizontalSlider_factor->setVisible(true);
        }
        else
        {
            ui->horizontalSlider_factor->setVisible(false);
        }

        this->set_comboIntervall();
        this->set_activty_intervalls();
     }
}

void MainWindow::set_activty_infos()
{
    ui->textBrowser_Info->clear();

    QTextCursor cursor = ui->textBrowser_Info->textCursor();
    cursor.beginEditBlock();

    QTextTableFormat tableFormat;
    tableFormat.setCellSpacing(2);
    tableFormat.setCellPadding(2);
    tableFormat.setBorder(0);
    tableFormat.setBackground(QBrush(QColor(220,220,220)));
    QVector<QTextLength> constraints;
        constraints << QTextLength(QTextLength::PercentageLength, 40)
                    << QTextLength(QTextLength::PercentageLength, 60);
    tableFormat.setColumnWidthConstraints(constraints);

    QTextTable *table = cursor.insertTable(curr_activity->ride_info.count(),2,tableFormat);

        QTextFrame *frame = cursor.currentFrame();
        QTextFrameFormat frameFormat = frame->frameFormat();
        frameFormat.setBorder(0);
        frame->setFrameFormat(frameFormat);

        QTextCharFormat format = cursor.charFormat();
        format.setFontPointSize(8);

        QTextCharFormat infoFormat = format;
        infoFormat.setFontWeight(QFont::Bold);

        QTextCharFormat valueFormat = format;

    int i = 0;
    for(QMap<QString,QString>::const_iterator it =  curr_activity->ride_info.cbegin(), end = curr_activity->ride_info.cend(); it != end; ++it,++i)
    {
        QTextTableCell cell = table->cellAt(i,0);
        QTextCursor cellCurser = cell.firstCursorPosition();
        cellCurser.insertText(it.key(),infoFormat);
        cell = table->cellAt(i,1);
        cellCurser = cell.firstCursorPosition();
        cellCurser.insertText(it.value(),valueFormat);
    }
    //table->insertRows(table->rows(),1);

    cursor.endEditBlock();
    cursor.setPosition(0);
    ui->textBrowser_Info->setTextCursor(cursor);
}

void MainWindow::set_activty_intervalls()
{
    ui->tableView_int->setModel(curr_activity->curr_act_model);
    ui->tableView_int->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_int->setItemDelegate(&intervall_del);
    ui->tableView_int->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_int->verticalHeader()->setVisible(false);

    if(curr_activity->get_sport() == settings::isSwim)
    {
        ui->tableView_int_times->setModel(curr_activity->swim_xdata);
        ui->tableView_int_times->setItemDelegate(&swimlap_del);
        ui->tableView_int_times->hideColumn(5);
        ui->tableView_int_times->hideColumn(6);
        ui->tableView_int_times->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView_int_times->verticalHeader()->setVisible(false);


        ui->tableView_swimzone->setModel(curr_activity->swim_pace_model);
        ui->tableView_swimzone->setItemDelegate(&level_del);
        ui->tableView_swimzone->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        ui->tableView_hfzone->setModel(curr_activity->swim_hf_model);
        ui->tableView_hfzone->setItemDelegate(&level_del);
        ui->tableView_hfzone->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        ui->lineEdit_swimcv->setText(settings::set_time(settings::get_thresValue("swimpace")));
        ui->lineEdit_hf_threshold->setText(QString::number(settings::get_thresValue("hfthres")));

        ui->lineEdit_laplen->setText(QString::number(curr_activity->get_swim_track()));
        ui->lineEdit_swimtime->setText(QDateTime::fromTime_t(curr_activity->get_move_time()).toUTC().toString("hh:mm:ss"));
        ui->lineEdit_swimpace->setText(settings::set_time(curr_activity->get_swim_pace()));
        this->write_hf_infos();
    }
    else
    {
        ui->tableView_int_times->setModel(curr_activity->edit_int_model);
        ui->tableView_int_times->setEditTriggers(QAbstractItemView::NoEditTriggers);
        //ui->tableView_int_times->setItemDelegate(&time_del);
        ui->tableView_int_times->hideColumn(4);
        ui->tableView_int_times->hideColumn(5);
        ui->tableView_int_times->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView_int_times->verticalHeader()->setVisible(false);
    }
    if(curr_activity->get_sport() != settings::isSwim)
    {
        ui->frame_polish->setVisible(true);
    }
}

void MainWindow::on_horizontalSlider_factor_valueChanged(int value)
{
    ui->label_factorValue->setText(QString::number(10-value) + "%");
    double factor = static_cast<double>(value)/100;
    this->set_polishValues(ui->comboBox_intervals->currentIndex(),factor);
    ui->lineEdit_polMax->setText(QString::number(curr_activity->polish_SpeedValues(50.0,curr_activity->get_int_speed(ui->comboBox_intervals->currentIndex(),settings::get_act_isrecalc()),0.1-factor,false)));
    ui->lineEdit_polMin->setText(QString::number(curr_activity->polish_SpeedValues(1.0,curr_activity->get_int_speed(ui->comboBox_intervals->currentIndex(),settings::get_act_isrecalc()),0.1-factor,false)));
}

void MainWindow::on_comboBox_intervals_currentIndexChanged(int index)
{
    ui->horizontalSlider_factor->setValue(0);
    if(settings::get_act_isload())
    {
        ui->lineEdit_lapTime->setText(settings::set_time(curr_activity->get_int_duration(index,settings::get_act_isrecalc())));
        ui->lineEdit_lapPace->setText(settings::set_time(curr_activity->get_int_pace(index,settings::get_act_isrecalc())));
        ui->lineEdit_lapSpeed->setText(QString::number(curr_activity->get_int_speed(index,settings::get_act_isrecalc())));
        double avg = curr_activity->get_int_speed(index,settings::get_act_isrecalc());
        this->set_intChartValues(index,avg);
        if(curr_activity->get_sport() != settings::isSwim) this->set_polishValues(index,0.0);
    }
}

void MainWindow::set_polishValues(int lap,double factor)
{
    double value;
    double avg = curr_activity->get_int_speed(lap,settings::get_act_isrecalc());
    if(polishLine->count() > 0)
    {
        polishLine->clear();
    }

    for(int i = 0; i < speedValues.count(); ++i)
    {
        if(lap == 0 && i < 5)
        {
            value = speedValues[i];
        }
        else
        {
            //Ignored NaN, Inf, or -Inf value. Check speedValues fill!
            value = curr_activity->polish_SpeedValues(speedValues[i],avg,0.10-factor,true);
        }
        polishLine->append(i,value);
    }
}

void MainWindow::set_intChartValues(int lapindex,double avgSpeed)
{
    QStandardItemModel *intmodel = curr_activity->edit_int_model;
    QStandardItemModel *sampmodel = curr_activity->samp_model;
    int start = intmodel->data(intmodel->index(lapindex,1,QModelIndex())).toInt();
    int stop = intmodel->data(intmodel->index(lapindex,2,QModelIndex())).toInt();
    double max = 0.0,min = 40.0,current;
    if(speedValues.count() > 0)
    {
        speedValues.clear();
        avgLine->clear();
    }
    speedValues.resize((stop-start)+1);

    if(speedLine->count() > 0)
    {
        speedLine->clear();
    }

    for(int i = start, pos=0; i <= stop; ++i,++pos)
    {
        current = sampmodel->data(sampmodel->index(i,2,QModelIndex())).toDouble();
        speedLine->append(pos,current);
        avgLine->append(pos,avgSpeed);
        speedValues[pos] = current;
        //axisValues << QString::number(pos);
        if(max < current) max = current;
        if(min > current) min = current;
    }

    axisX->setMin(0);
    axisX->setMax(stop-start);
    ySpeed->setMax(max);
    ySpeed->setMin(min);
    ySpeed->applyNiceNumbers();
}

void MainWindow::write_hf_infos()
{
    int hf_value;
    double totalWork;

    ui->lineEdit_hfavg->setText(QString::number(curr_activity->get_hf_avg()));
    totalWork = settings::calc_totalWork(jsonhandler->get_tagData("Weight").toDouble(),curr_activity->get_hf_avg(),curr_activity->get_move_time());
    totalWork = totalWork * curr_activity->get_swim_sri();
    ui->lineEdit_kal->setText(QString::number(ceil((totalWork/4)*4.184)));
    ui->lineEdit_kj->setText(QString::number(ceil(totalWork)));

    jsonhandler->set_overrideFlag(true);
    jsonhandler->set_overrideData("average_hr",ui->lineEdit_hfavg->text());
    jsonhandler->set_overrideData("total_work",ui->lineEdit_kj->text());
    jsonhandler->set_overrideData("total_kcalories",ui->lineEdit_kal->text());
    for(int i = 0; i < 7; i++)
    {
        hf_value = settings::get_timesec(curr_activity->swim_hf_model->data(curr_activity->swim_hf_model->index(i,3,QModelIndex())).toString());
        jsonhandler->set_overrideData("time_in_zone_H" + QString::number(i+1),QString::number(hf_value));
    }
}

void MainWindow::fill_WorkoutContent()
{
    QString content,newEntry,contentValue,label;
    double dist;
    int time;
    content = ui->lineEdit_workContent->text();

    if(ui->radioButton_time->isChecked())
    {
        if(ui->checkBox_exact->isChecked())
        {
            time = curr_activity->get_avg_laptime();
        }
        else
        {
            time = (ceil(curr_activity->get_avg_laptime()/10.0)*10);
        }

        if(time >= 60)
        {
            label = "Min";
        }
        else
        {
            label = "Sec";
        }

        contentValue = settings::set_time(time)+label;
    }

    if(ui->radioButton_distance->isChecked())
    {
        if(ui->checkBox_exact->isChecked())
        {
            dist = (round(curr_activity->get_avg_dist()*1000)/1000.0);
        }
        else
        {
            dist = (ceil(curr_activity->get_avg_dist()*10)/10.0);

        }

        if(dist < 1)
        {
            label = "M";
            dist = dist*1000.0;
        }
        else
        {
            label = "Km";
        }

        contentValue = QString::number(dist)+label;
    }

    if(curr_activity->get_sport() == settings::isSwim)
    {
        if(sel_count > 1)
        {
            newEntry = QString::number(sel_count)+"x"+QString::number(curr_activity->get_avg_dist()*curr_activity->get_dist_factor())+"/"+settings::set_time(curr_activity->get_avg_laptime());
        }
        else
        {
            newEntry = QString::number(curr_activity->get_avg_dist()*curr_activity->get_dist_factor())+"-"+settings::set_time(curr_activity->get_avg_laptime());
        }
    }

    if(curr_activity->get_sport() == settings::isBike)
    {
        if(sel_count > 1)
        {
            newEntry = QString::number(sel_count)+"x"+contentValue+"/" +QString::number(round(curr_activity->get_avg_watts()))+"W";
        }
        else
        {
            newEntry = contentValue+"-" +QString::number(round(curr_activity->get_avg_watts()))+"W";
        }
    }

    if(curr_activity->get_sport() == settings::isRun)
    {
        if(sel_count > 1)
        {
            newEntry = QString::number(sel_count)+"x"+contentValue+"-" +settings::set_time(curr_activity->get_avg_pace())+"/km";
        }
        else
        {
            newEntry = contentValue+"-" +settings::set_time(curr_activity->get_avg_pace())+"/km";
        }
    }

    if(ui->lineEdit_workContent->text() == "")
    {
        ui->lineEdit_workContent->setText(content+newEntry);
        ui->pushButton_ClearWorkContent->setEnabled(true);
    }
    else
    {
        ui->lineEdit_workContent->setText(content+" | "+newEntry);
    }

    jsonhandler->set_tagData("Workout Content",ui->lineEdit_workContent->text());
}

void MainWindow::unselect_intRow()
{
    QModelIndex index;

    for(int i = 0; i < curr_activity->int_model->rowCount(); ++i)
    {
        index = curr_activity->curr_act_model->index(i,0,QModelIndex());
        curr_activity->curr_act_model->setData(index,0,Qt::UserRole+1);
        curr_activity->reset_avg();
        sel_count = 0;
        this->set_avg_fields();
        ui->tableView_int->setCurrentIndex(curr_activity->curr_act_model->index(0,0,QModelIndex()));
    }
    ui->pushButton_putInt->setEnabled(false);
}

void MainWindow::on_pushButton_week_minus_clicked()
{
    if(isWeekMode)
    {
        --weekCounter;
        if(weekCounter == 0)
        {
           this->set_buttons(false);
        }
        this->set_calender();
        this->refresh_model();
    }
    else
    {
        --weekpos;
        if(weekpos == 0)
        {
            this->set_buttons(false);
        }
        if(weekpos < 52)
        {
            ui->pushButton_fourplus->setEnabled(true);
            ui->pushButton_week_plus->setEnabled(true);
        }
        this->workout_calendar();
    }
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::on_pushButton_week_plus_clicked()
{
    if(isWeekMode)
    {
        ++weekCounter;
        this->set_calender();
        this->set_buttons(true);
        this->refresh_model();
    }
    else
    {
        ++weekpos;
        if(weekpos + 12 == saisonWeeks)
        {
            ui->pushButton_fourplus->setEnabled(false);
            ui->pushButton_week_plus->setEnabled(false);
            this->workout_calendar();
        }
        else
        {
            this->set_buttons(true);
            this->workout_calendar();
        }
    }
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::on_pushButton_fourplus_clicked()
{
    if(isWeekMode)
    {
        weekCounter = weekCounter+4;
        this->set_calender();
        this->set_buttons(true);
        this->refresh_model();
    }
    else
    {
        weekpos = weekpos+4;
        if(weekpos + 12 >= saisonWeeks)
        {
            weekpos = saisonWeeks-12;
            ui->pushButton_fourplus->setEnabled(false);
            ui->pushButton_week_plus->setEnabled(false);
            this->workout_calendar();
        }
        else
        {
            this->set_buttons(true);
            this->workout_calendar();
        }
    }
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::on_actionEditor_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);
    this->set_menuItems(true,false);
}

void MainWindow::on_actionPlaner_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
    this->set_menuItems(false,true);
}

void MainWindow::on_actionExit_triggered()
{
    if(safeFlag)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      tr("Save and Exit"),
                                      "Save Changes on Workouts?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            workSchedule->save_workout_file();
            this->freeMem();
            close();
        }
        else
        {
            this->freeMem();
            close();
        }
    }
    else
    {
        this->freeMem();
        close();
    }
}

void MainWindow::on_actionExit_and_Save_triggered()
{
    workSchedule->save_workout_file();
    this->freeMem();
}

void MainWindow::on_actionSelect_File_triggered()
{
    this->select_activity_file();
}

void MainWindow::set_avg_fields()
{
    ui->lineEdit_numsel->setText(QString::number(sel_count));
    ui->lineEdit_lap->setText(settings::set_time(curr_activity->get_avg_laptime()));
    ui->lineEdit_pace->setText(settings::set_time(curr_activity->get_avg_pace()));
    ui->lineEdit_dist->setText(QString::number(curr_activity->get_avg_dist()*curr_activity->get_dist_factor()));
    ui->lineEdit_watt->setText(QString::number(round(curr_activity->get_avg_watts())));
    ui->lineEdit_cad->setText(QString::number(round(curr_activity->get_avg_cad())));

    if(sel_count > 0)
    {
        ui->frame_avgValue->setVisible(true);
    }
    else
    {
        ui->frame_avgValue->setVisible(false);
    }

    if(curr_activity->get_sport() == settings::isBike)
    {
        ui->lineEdit_watt->setVisible(true);
        ui->label_avgWatt->setVisible(true);
        ui->lineEdit_cad->setVisible(true);
        ui->label_cad->setVisible(true);
    }
    else
    {
        ui->lineEdit_watt->setVisible(false);
        ui->label_avgWatt->setVisible(false);
        ui->lineEdit_cad->setVisible(false);
        ui->label_cad->setVisible(false);
    }
}

void MainWindow::on_tableView_int_times_clicked(const QModelIndex &index)
{
    if(index.column() == 0)
    {
        Dialog_lapeditor lapEdit(this,curr_activity,index);
        lapEdit.setModal(true);
        lapEdit.exec();
    }
}

void MainWindow::on_tableView_int_clicked(const QModelIndex &index)
{
    if(index.column() == 0)
    {
        int check_value = curr_activity->curr_act_model->data(index,(Qt::UserRole+1)).toInt();
        if(check_value == 0)
        {
            sel_count++;
            curr_activity->curr_act_model->setData(index,1,Qt::UserRole+1);
            curr_activity->set_avg_values(sel_count,index.row(),true);
        }
        else
        {
            sel_count--;
            curr_activity->curr_act_model->setData(index,0,Qt::UserRole+1);
            curr_activity->set_avg_values(sel_count,index.row(),false);
        }
        ui->tableView_int->setItemDelegateForRow(index.row(),&intSelect_del);
    }
    if(sel_count > 0)
    {
        ui->pushButton_putInt->setEnabled(true);
    }
    else
    {
        ui->pushButton_putInt->setEnabled(false);
    }
    this->set_avg_fields();
}

void MainWindow::on_actionReset_triggered()
{
    ui->textBrowser_Info->clear();
    curr_activity->reset_avg();
    curr_activity->curr_act_model->clear();
    curr_activity->int_model->clear();
    curr_activity->samp_model->clear();
    curr_activity->edit_int_model->clear();
    if(settings::get_act_isrecalc())
    {
        curr_activity->edit_samp_model->clear();
        delete curr_activity->edit_samp_model;
    }
    settings::set_act_recalc(false);
    if(curr_activity->get_sport() == settings::isSwim)
    {
        curr_activity->swim_pace_model->clear();
        delete curr_activity->swim_pace_model;
        curr_activity->swim_hf_model->clear();
        delete curr_activity->swim_hf_model;
        curr_activity->swim_xdata->clear();
        delete curr_activity->swim_xdata;
        curr_activity->act_reset();
        ui->lineEdit_swimcv->clear();
        ui->lineEdit_hf_threshold->clear();
        ui->lineEdit_laplen->clear();
        ui->lineEdit_swimtime->clear();
        ui->lineEdit_swimpace->clear();
        ui->lineEdit_hfavg->clear();
        ui->lineEdit_kj->clear();
        ui->lineEdit_kal->clear();
    }

    settings::set_act_isload(false);
    this->sel_count = 0;
    this->set_avg_fields();
    this->set_menuItems(true,false);
    ui->actionSelect_File->setEnabled(true);
    ui->frame_polish->setVisible(false);

    ui->verticalLayout_interpol->removeWidget(intChartview);
    ui->lineEdit_lapPace->setText("-");
    ui->lineEdit_lapTime->setText("-");
    ui->lineEdit_lapSpeed->setText("-");
    ui->comboBox_intervals->clear();
    ui->lineEdit_workContent->clear();
    ui->pushButton_ClearWorkContent->setEnabled(false);
    ui->pushButton_sync->setEnabled(false);
    ui->pushButton_putInt->setEnabled(false);
    ui->radioButton_time->setChecked(true);
    ui->checkBox_exact->setChecked(false);

    delete ySpeed;
    delete speedLine;
    if(curr_activity->get_sport() == settings::isRun) delete polishLine;
    delete avgLine;
    delete axisX;
    delete intChartview;

    delete curr_activity->int_model;
    delete curr_activity->edit_int_model;
    delete curr_activity->samp_model;
    delete curr_activity;
}

void MainWindow::on_actionUnselect_all_rows_triggered()
{
    this->unselect_intRow();
}

void MainWindow::on_actionEdit_Distance_triggered()
{
    settings::set_act_recalc(true);
    curr_activity->recalculate_intervalls(settings::get_act_isrecalc());
    curr_activity->set_additional_ride_info();
    this->set_activty_intervalls();
    this->set_activty_infos();
    this->set_comboIntervall();
}

void MainWindow::on_actionEdit_Undo_triggered()
{
    settings::set_act_recalc(false);
    curr_activity->recalculate_intervalls(settings::get_act_isrecalc());
    curr_activity->set_additional_ride_info();
    this->set_activty_intervalls();
    this->set_activty_infos();
    this->set_comboIntervall();
}

void MainWindow::on_actionIntervall_Editor_triggered()
{
    Dialog_inteditor intEditor(this,stdWorkout);
    intEditor.setModal(true);
    intEditor.exec();
}

void MainWindow::on_actionPreferences_triggered()
{
    this->openPreferences();
}

void MainWindow::on_actionPace_Calculator_triggered()
{
    Dialog_paceCalc dia_pace(this);
    dia_pace.setModal(true);
    dia_pace.exec();
}


void MainWindow::on_comboBox_schedMode_currentIndexChanged(int index)
{
   if(index == 0)
   {
       isWeekMode = true;
       weekCounter = 0;
       ui->actionNew->setEnabled(true);
       this->set_buttons(false);
       ui->comboBox_phasefilter->setCurrentIndex(0);
   }
   else
   {
       isWeekMode = false;
       ui->actionNew->setEnabled(false);
       if(weekpos == 0)
       {
            this->set_buttons(false);
       }
       else
       {
           this->set_buttons(true);
       }
   }
   ui->comboBox_phasefilter->setEnabled(!isWeekMode);
   ui->label_month->setText("Week " + this->get_weekRange());
   this->workout_calendar();
   this->summery_view();
}

void MainWindow::on_tableView_summery_clicked(const QModelIndex &index)
{
    int filterindex = ui->comboBox_phasefilter->currentIndex();

    if(!isWeekMode)
    {
        int dialog_code;
        year_popup year_pop(this,sum_model->data(index,Qt::DisplayRole).toString(),index.row(),workSchedule,phaseFilter,filterindex);
        year_pop.setModal(true);
        dialog_code = year_pop.exec();
        if(dialog_code == QDialog::Rejected)
        {
            this->set_calender();
        }
    }
}

void MainWindow::on_actionSwitch_Year_triggered()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  tr("Change Year Schedule"),
                                  "Change to new Year Schedule?",
                                  QMessageBox::Yes|QMessageBox::No
                                  );
    if (reply == QMessageBox::Yes)
    {
        if(QDate::currentDate() >= QDate::fromString(settings::get_saisonInfo("startDate"),"dd.MM.yyyy"))
        {
            workSchedule->changeYear();
            this->workout_calendar();
        }
        else
        {
            QMessageBox::warning(this,"Season not finished","Season Schedule can not be changed!",QMessageBox::Ok);
        }
    }
}

void MainWindow::on_comboBox_phasefilter_currentIndexChanged(int index)
{
    if(index == 0)
    {
        this->set_buttons(false);
        ui->pushButton_fourplus->setEnabled(true);
        ui->pushButton_week_plus->setEnabled(true);
    }
    else
    {
        this->set_buttons(false);
        ui->pushButton_fourplus->setEnabled(false);
        ui->pushButton_week_plus->setEnabled(false);
    }
    phaseFilter = ui->comboBox_phasefilter->currentText();
    this->workout_calendar();
    this->summery_view();
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::on_actionVersion_triggered()
{
    Dialog_version versionBox(this);
    versionBox.setModal(true);
    versionBox.exec();
}

void MainWindow::on_actionLapEditor_triggered()
{
    Dialog_lapeditor lapeditor(this,curr_activity);
    lapeditor.setModal(true);
    lapeditor.exec();
}

void MainWindow::on_horizontalSlider_polish_valueChanged(int value)
{
    ui->label_WorkFactor->setText(QString::number(10-value) + "%");
    curr_activity->set_polishFactor(0.1-(static_cast<double>(value)/100));
}


void MainWindow::on_pushButton_ClearWorkContent_clicked()
{
    ui->lineEdit_workContent->clear();
    ui->pushButton_ClearWorkContent->setEnabled(false);
    ui->pushButton_sync->setEnabled(false);
}

void MainWindow::on_pushButton_sync_clicked()
{
    jsonhandler->set_tagData("Workout Content",ui->lineEdit_workContent->text());
    ui->pushButton_sync->setEnabled(false);
}

void MainWindow::on_lineEdit_workContent_textChanged(const QString &value)
{
    Q_UNUSED(value)
    ui->pushButton_sync->setEnabled(true);
}

void MainWindow::on_pushButton_putInt_clicked()
{
    this->fill_WorkoutContent();
    this->unselect_intRow();
}
