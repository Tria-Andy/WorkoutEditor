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

Dialog_addweek::Dialog_addweek(QWidget *parent, QString sel_week, schedule *p_sched) :
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
    timeFormat = "hh:mm:ss";
    empty = "0-0-00:00-0";
    weekHeader = settings::getHeaderMap("addweek");
    sportuseList = settings::get_listValues("Sportuse");
    sportuseList.removeLast();
    week_del.sportUse = &sportuseList;
    sportlistCount = sportuseList.count();
    ui->toolButton_paste->setEnabled(false);
    ui->tableWidget_sportValues->setColumnCount(1);
    ui->tableWidget_sportValues->setRowCount(sportlistCount);
    this->fill_values(sel_week);
}

Dialog_addweek::~Dialog_addweek()
{
    delete ui;
}
void Dialog_addweek::on_toolButton_close_clicked()
{
    reject();
}

void Dialog_addweek::fill_values(QString selWeekID)
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
        editWeekID = selWeekID;
        ui->dateEdit_selectDate->setDate(QDate::fromString(weekMeta.at(3),workSched->dateFormat));
        ui->lineEdit_week->setText(QString::number(ui->dateEdit_selectDate->date().weekNumber()));

        ui->comboBox_phase->setCurrentText(weekMeta.at(2).split("_").first());
        ui->comboBox_cycle->setCurrentText(weekMeta.at(2).split("_").last());
        ui->lineEdit_weekContent->setText(weekMeta.at(4));
        ui->lineEdit_weekGoals->setText(weekMeta.at(5));
        update = true;

        QVector<double> weekComp(5,0);
        QString delimiter = "#";
        QString itemString;

        for(int sport = 0; sport < sportlistCount; ++sport)
        {
            weekComp = compValues.value(sportuseList.at(sport));
            this->set_currentSport(sportuseList.at(sport));

            QTableWidgetItem *item = new QTableWidgetItem();
            itemString = sportuseList.at(sport)+delimiter+
                         "Workouts: " +QString::number(weekComp.at(0))+delimiter+
                         set_sectoTime(weekComp.at(1)).toString()+delimiter+
                         QString::number(weekComp.at(2))+delimiter+
                         QString::number(this->set_doubleValue(weekComp.at(3),false))+delimiter+
                         QString::number(weekComp.at(4));
            item->setData(Qt::DisplayRole,itemString);
            item->setData(Qt::AccessibleTextRole,sportuseList.at(sport));
            ui->tableWidget_sportValues->setItem(sport,0,item);

        }
        //this->fill_weekSumRow(ui->tableWidget_sportValues->model());

     }
     ui->dateEdit_selectDate->blockSignals(false);
}

void Dialog_addweek::update_values()
{
    ui->dateEdit_selectDate->setFocus();

    QVector<double> weekData(5,0);
    QString sport;
    for(int sport = 0; sport < sportlistCount; ++sport)
    {
        /*
        weekData[0] = weekModel->data(weekModel->index(sport,1)).toDouble();
        weekData[1] = get_timesec(weekModel->data(weekModel->index(sport,2)).toString());
        weekData[2] = weekModel->data(weekModel->index(sport,3)).toDouble();
        weekData[3] = weekModel->data(weekModel->index(sport,4)).toDouble();
        weekData[4] = weekModel->data(weekModel->index(sport,6)).toDouble();
        */
        compValues.insert(sportuseList.at(sport),weekData);
    }

    weekMeta[0] = calc_weekID(ui->dateEdit_selectDate->date());
    weekMeta[2] = ui->comboBox_phase->currentText() +"_"+ ui->comboBox_cycle->currentText();
    weekMeta[3] = ui->dateEdit_selectDate->date().toString(workSched->dateFormat);
    weekMeta[4] = ui->lineEdit_weekContent->text();
    weekMeta[5] = ui->lineEdit_weekGoals->text();

    workSched->set_weekCompValues(weekMeta,compValues);

    ui->toolButton_paste->setEnabled(true);
}

void Dialog_addweek::on_dateEdit_selectDate_dateChanged(const QDate &date)
{
    this->fill_values(calc_weekID(date));
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
