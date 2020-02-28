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

#include "dialog_addweek.h"
#include "ui_dialog_addweek.h"

Dialog_addweek::Dialog_addweek(QWidget *parent,QString sel_week, schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::Dialog_addweek)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setFixedSize(settings::screenSize.first*0.35,settings::screenSize.second*0.45);

    generalValues = settings::getStringMapPointer(settings::stingMap::General);
    workSched = p_sched;
    ui->comboBox_phase->addItems(settings::get_listValues("Phase"));
    ui->comboBox_cycle->addItems(settings::get_listValues("Cycle"));

    weekSummery.resize(4);
    weekSummery.fill(0);
    sportuseList = settings::get_listValues("Sportuse");
    sportuseList.removeLast();
    sportlistCount = sportuseList.count();
    ui->toolButton_paste->setEnabled(false);
    ui->tableWidget_sportValues->setColumnCount(1);
    ui->tableWidget_sportValues->setRowCount(sportlistCount);

    this->fill_weekValues(sel_week);
}

Dialog_addweek::~Dialog_addweek()
{
    delete ui;
}
void Dialog_addweek::on_toolButton_close_clicked()
{
    accept();
}

void Dialog_addweek::fill_weekValues(QString selWeekID)
{
    ui->dateEdit_selectDate->blockSignals(true);

    weekMeta = workSched->get_weekMeta(selWeekID);

    compValues = workSched->get_compWeekValues()->value(selWeekID);

    ui->label_header->clear();
    ui->label_header->setText("Saison: "+ generalValues->value("saison")  +" - Week: "+ weekMeta.at(1)+" ("+selWeekID +") - Phase: "+weekMeta.at(2));

    ui->tableWidget_sportValues->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_sportValues->horizontalHeader()->hide();
    ui->tableWidget_sportValues->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_sportValues->verticalHeader()->hide();
    ui->tableWidget_sportValues->setItemDelegate(&week_del);

    if(compValues.count() > 0)
    {
        ui->dateEdit_selectDate->setDate(QDate::fromString(weekMeta.at(3),workSched->dateFormat));
        ui->lineEdit_week->setText(QString::number(ui->dateEdit_selectDate->date().weekNumber()));

        ui->comboBox_phase->setCurrentText(weekMeta.at(2).split("_").first());
        ui->comboBox_cycle->setCurrentText(weekMeta.at(2).split("_").last());
        ui->lineEdit_weekContent->setText(weekMeta.at(4));
        ui->lineEdit_weekGoals->setText(weekMeta.at(5));
        update = true;

        for(int sport = 0; sport < sportlistCount; ++sport)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            this->set_sportData(sportuseList.at(sport),compValues.value(sportuseList.at(sport)),item);

            ui->tableWidget_sportValues->setItem(sport,0,item);
        }
     }
    ui->dateEdit_selectDate->blockSignals(false);
    this->calc_weekSummery();
}

void Dialog_addweek::calc_weekSummery()
{
    weekSummery.fill(0);
    for(QMap<QString, QVector<double>>::const_iterator it = compValues.cbegin(); it != compValues.cend(); ++it)
    {
        weekSummery[0] += it.value().at(0);
        weekSummery[1] += it.value().at(1);
        weekSummery[2] += it.value().at(3);
        weekSummery[3] += it.value().at(4);
    }
    weekDuration = weekSummery.at(1);
    ui->spinBox_workSum->setValue(weekSummery.at(0));
    ui->timeEdit_duraSum->setTime(set_sectoTime(weekSummery.at(1)));
    ui->doubleSpinBox_distSum->setValue(weekSummery.at(2));
    ui->spinBox_stressSum->setValue(weekSummery.at(3));
}

void Dialog_addweek::on_tableWidget_sportValues_itemClicked(QTableWidgetItem *item)
{
    this->set_currentSport(item->data(Qt::AccessibleTextRole).toString());
    sportDuration = item->data(Qt::UserRole+1).toInt();

    ui->lineEdit_sportName->setText(item->data(Qt::AccessibleTextRole).toString());
    ui->spinBox_workouts->setValue(item->data(Qt::UserRole).toInt());
    ui->timeEdit_duration->setTime(set_sectoTime(item->data(Qt::UserRole+1).toInt()));
    ui->doubleSpinBox_percent->setValue(item->data(Qt::UserRole+2).toDouble());
    ui->doubleSpinBox_distance->setValue(item->data(Qt::UserRole+3).toDouble());
    ui->spinBox_stressScore->setValue(item->data(Qt::UserRole+4).toInt());
}

void Dialog_addweek::on_toolButton_editSport_clicked()
{
    QVector<double> weekComp(5,0);

    weekComp[0] = ui->spinBox_workouts->value();
    weekComp[1] = get_secFromTime(ui->timeEdit_duration->time());
    weekComp[2] = ui->doubleSpinBox_percent->value();
    weekComp[3] = ui->doubleSpinBox_distance->value();
    weekComp[4] = ui->spinBox_stressScore->value();

    this->set_sportData(ui->lineEdit_sportName->text(),weekComp,ui->tableWidget_sportValues->currentItem());
    compValues.insert(ui->lineEdit_sportName->text(),weekComp);
    this->calc_weekSummery();
}

void Dialog_addweek::set_sportData(QString sport,QVector<double> weekComp, QTableWidgetItem *item)
{
    QString delimiter = "#";

    QString itemString = sport+delimiter+
                 "Workouts: " +QString::number(weekComp.at(0))+delimiter+
                 set_sectoTime(weekComp.at(1)).toString()+delimiter+
                 QString::number(weekComp.at(2))+delimiter+
                 QString::number(this->set_doubleValue(weekComp.at(3),false))+delimiter+
                 QString::number(weekComp.at(4));

    item->setData(Qt::DisplayRole,itemString);
    item->setData(Qt::AccessibleTextRole,sport);
    item->setData(Qt::UserRole,weekComp.at(0));
    item->setData(Qt::UserRole+1,weekComp.at(1));
    item->setData(Qt::UserRole+2,weekComp.at(2));
    item->setData(Qt::UserRole+3,weekComp.at(3));
    item->setData(Qt::UserRole+4,weekComp.at(4));
}


void Dialog_addweek::update_values()
{
    weekMeta[0] = calc_weekID(ui->dateEdit_selectDate->date());
    weekMeta[2] = ui->comboBox_phase->currentText() +"_"+ ui->comboBox_cycle->currentText();
    weekMeta[3] = ui->dateEdit_selectDate->date().toString(workSched->dateFormat);
    weekMeta[4] = ui->lineEdit_weekContent->text();
    weekMeta[5] = ui->lineEdit_weekGoals->text();

    workSched->set_weekCompValues(weekMeta,compValues);
}

void Dialog_addweek::on_dateEdit_selectDate_dateChanged(const QDate &date)
{
    this->fill_weekValues(calc_weekID(date));
}

void Dialog_addweek::on_toolButton_update_clicked()
{
    this->update_values();
}

void Dialog_addweek::on_toolButton_copy_clicked()
{
    this->update_values();
}

void Dialog_addweek::on_toolButton_paste_clicked()
{
    QString vPhase = weekMeta.at(1),contentSport,dist;
    QStringList content;

    ui->comboBox_phase->setCurrentText(vPhase.split("_").first());
    ui->comboBox_cycle->setCurrentText(vPhase.split("_").last());
    ui->lineEdit_weekContent->setText(weekMeta.at(3));
    ui->lineEdit_weekGoals->setText(weekMeta.at(4));

    ui->dateEdit_selectDate->setFocus();
}

void Dialog_addweek::on_timeEdit_duration_userTimeChanged(const QTime &time)
{
    double sportDura = get_secFromTime(time);
    double weekDura = (weekDuration - sportDuration) + sportDura;

    ui->timeEdit_duraSum->setTime(set_sectoTime(weekDura));
    ui->doubleSpinBox_percent->setValue(sportDura/weekDura*100.0);
    ui->spinBox_stressScore->setValue(estimate_stressScore(sportDura,ui->doubleSpinBox_distance->value()));
    ui->lineEdit_pace->setText(get_workout_pace(ui->doubleSpinBox_distance->value(),sportDura,false));
}

void Dialog_addweek::on_doubleSpinBox_distance_valueChanged(double distance)
{
    ui->lineEdit_pace->setText(get_workout_pace(distance,get_secFromTime(ui->timeEdit_duration->time()),false));
}
