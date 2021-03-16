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

#include "dialog_week_copy.h"
#include "ui_dialog_week_copy.h"

Dialog_week_copy::Dialog_week_copy(QWidget *parent,QString selected_week,schedule *p_sched,bool hasWeek) :
    QDialog(parent),
    ui(new Ui::Dialog_week_copy)
{
    ui->setupUi(this);
    workSched = p_sched;
    isSaveWeek = false;
    if(hasWeek)
    {
        sourceWeek = selected_week.split("#").first();
        ui->lineEdit_currweek->setText(sourceWeek);
        ui->lineEdit_currweek->setVisible(true);
        ui->comboBox_select->setVisible(false);
    }
    else
    {
        ui->comboBox_select->addItems(workSched->get_remainWeeks());
        ui->comboBox_select->setVisible(true);
        ui->lineEdit_currweek->setVisible(false);
    }

    this->setFixedHeight(400);
    ui->comboBox_copyto->addItems(workSched->get_remainWeeks());
    ui->frame_copy->setVisible(true);
    ui->comboBox_phase->addItems(settings::get_listValues("Phase"));
    ui->comboBox_cycle->addItems(settings::get_listValues("Cycle"));
    ui->comboBox_program->addItems(settings::get_listValues("Program"));
    ui->comboBox_cardio->addItems(settings::get_listValues("Cardio"));
    ui->comboBox_goal->addItems(settings::get_listValues("Mode"));
    weekMeta = workSched->get_weekMeta(selected_week);
    weekMeta = workSched->get_weekScheduleMeta(selected_week);

    ui->comboBox_phase->setCurrentText(weekMeta.at(1).split("_").first());
    ui->comboBox_cycle->setCurrentText(weekMeta.at(1).split("_").last());
    ui->comboBox_program->setCurrentText(weekMeta.at(2).split(" - ").first());
    ui->comboBox_cardio->setCurrentText(weekMeta.at(2).split(" - ").last());
    ui->comboBox_goal->setCurrentText(weekMeta.at(3).split(" ").first());
    ui->doubleSpinBox_weight->setValue(weekMeta.at(3).split(" ").last().split("kg").first().toDouble());
    ui->comboBox_copyto->setEnabled(false);

    editMode = 0;

}

enum {EDIT,COPY,CLEAR};

Dialog_week_copy::~Dialog_week_copy()
{
    delete ui;
}

void Dialog_week_copy::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_week_copy::on_pushButton_ok_clicked()
{    
    if(editMode == EDIT)
    {
        weekMeta[1] = ui->comboBox_phase->currentText() +"_"+ ui->comboBox_cycle->currentText();
        weekMeta[2] = ui->comboBox_program->currentText()+" - "+ui->comboBox_cardio->currentText();
        weekMeta[3] = ui->comboBox_goal->currentText()+ " "+QString::number(ui->doubleSpinBox_weight->value())+"kg";

        workSched->set_weekScheduleMeta(weekMeta);
        accept();
    }
    else if(editMode == COPY)
    {
        QString targetWeek = ui->comboBox_copyto->currentText();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Copy Week",
                                      "Copy Week "+sourceWeek+" to Week "+targetWeek+"?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            ui->progressBar_copy->setValue(50);
            workSched->copyWeek(sourceWeek,targetWeek);
            ui->progressBar_copy->setValue(100);
            accept();
        }
    }
    else if(editMode == CLEAR)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Clear Week",
                                      "Clear all Workouts of Week "+sourceWeek+"?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            workSched->clearWeek(sourceWeek);
            accept();
        }
    }
    else
    {
        accept();
    }
}

void Dialog_week_copy::on_radioButton_edit_clicked()
{
    editMode = EDIT;
    ui->comboBox_copyto->setEnabled(false);
}

void Dialog_week_copy::on_radioButton_copy_clicked()
{
    editMode = COPY;
    ui->comboBox_copyto->setEnabled(true);
}

void Dialog_week_copy::on_radioButton_clear_clicked()
{
    editMode = CLEAR;
    ui->comboBox_copyto->setEnabled(false);
}

void Dialog_week_copy::on_comboBox_select_currentIndexChanged(const QString &value)
{
    sourceWeek = value;
}
