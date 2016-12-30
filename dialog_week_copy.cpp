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
    for(int i = 0; i < workSched->week_meta->rowCount(); ++i)
    {
        weekList << workSched->week_meta->data(workSched->week_meta->index(i,1,QModelIndex())).toString();
    }
    fixWeek = hasWeek;

    if(fixWeek)
    {
        ui->lineEdit_currweek->setText(selected_week.split("#").first());
        ui->lineEdit_currweek->setVisible(true);
        ui->comboBox_select->setVisible(false);
    }
    else
    {
        ui->comboBox_select->addItems(weekList);
        ui->comboBox_select->setVisible(true);
        ui->lineEdit_currweek->setVisible(false);
    }

    this->setFixedHeight(200);
    ui->comboBox_copyto->addItems(weekList);
    ui->frame_copy->setVisible(true);
    ui->frame_save->setVisible(false);
    ui->frame_delete->setVisible(false);

    editMode = 0;
}

enum {COPY,SAVE,CLEAR};

Dialog_week_copy::~Dialog_week_copy()
{
    delete ui;
}

void Dialog_week_copy::processWeek()
{
    QString sourceWeek ;

    if(fixWeek)
    {
        sourceWeek = ui->lineEdit_currweek->text();
    }
    else
    {
        sourceWeek = ui->comboBox_select->currentText();
    }

    if(editMode == COPY)
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
            workSched->copyWeek(sourceWeek,targetWeek);
            accept();
        }
    }
    if(editMode == SAVE)
    {

    }
    if(editMode == CLEAR)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Clear Week",
                                      "Clear all Workouts of Week "+sourceWeek+"?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            workSched->deleteWeek(sourceWeek);
            accept();
        }
    }
}


void Dialog_week_copy::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_week_copy::on_pushButton_ok_clicked()
{
    this->processWeek();
}

void Dialog_week_copy::on_radioButton_copy_clicked()
{
    editMode = COPY;
    ui->frame_copy->setVisible(true);
    ui->frame_save->setVisible(false);
    ui->frame_delete->setVisible(false);
}

void Dialog_week_copy::on_radioButton_save_clicked()
{
    editMode = SAVE;
    ui->frame_copy->setVisible(false);
    ui->frame_save->setVisible(true);
    ui->frame_delete->setVisible(false);
}

void Dialog_week_copy::on_radioButton_clear_clicked()
{
    editMode = CLEAR;
    ui->frame_copy->setVisible(false);
    ui->frame_save->setVisible(false);
    ui->frame_delete->setVisible(true);
}
