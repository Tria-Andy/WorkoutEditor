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

Dialog_week_copy::Dialog_week_copy(QWidget *parent,QString selected_week,schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::Dialog_week_copy)
{
    ui->setupUi(this);
    ui->lineEdit_currweek->setText(selected_week.split("#").first());
    workSched = p_sched;

    for(int i = 0; i < workSched->week_meta->rowCount(); ++i)
    {
        weekList << workSched->week_meta->data(workSched->week_meta->index(i,1,QModelIndex())).toString();
    }
    ui->comboBox_copyto->addItems(weekList);
    ui->lineEdit_saveas->setEnabled(false);
    editMode = 0;
}

enum {COPY,SAVE,CLEAR};

Dialog_week_copy::~Dialog_week_copy()
{
    delete ui;
}

void Dialog_week_copy::editWeek()
{
    if(editMode == COPY)
    {
        QString wFrom = ui->lineEdit_currweek->text();
        QString wTo = ui->comboBox_copyto->currentText();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Copy Week",
                                      "Copy Week "+wFrom+" to Week "+wTo+"?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            workSched->set_copyWeeks(wFrom,wTo);
            accept();
        }
    }
    if(editMode == SAVE)
    {

    }
    if(editMode == CLEAR)
    {

    }
}


void Dialog_week_copy::on_pushButton_cancel_clicked()
{
    reject();
}


void Dialog_week_copy::on_pushButton_ok_clicked()
{
    this->editWeek();
}



void Dialog_week_copy::on_radioButton_copy_clicked()
{
    editMode = COPY;
    ui->lineEdit_saveas->setEnabled(false);
    ui->comboBox_copyto->setEnabled(true);
}

void Dialog_week_copy::on_radioButton_save_clicked()
{
    editMode = SAVE;
    ui->lineEdit_saveas->setEnabled(true);
    ui->comboBox_copyto->setEnabled(false);
}

void Dialog_week_copy::on_radioButton_clear_clicked()
{
    editMode = CLEAR;
    ui->lineEdit_saveas->setEnabled(false);
    ui->comboBox_copyto->setEnabled(false);
}
