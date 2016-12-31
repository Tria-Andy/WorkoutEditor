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

#include "dialog_lapeditor.h"
#include "ui_dialog_lapeditor.h"
#include <QMouseEvent>

Dialog_lapeditor::Dialog_lapeditor(QWidget *parent,Activity *p_act,QModelIndex p_index) :
    QDialog(parent),
    ui(new Ui::Dialog_lapeditor)
{
    ui->setupUi(this);
    curr_act = p_act;
    selIndex = p_index;
    selRow = selIndex.row();
    swimLapLen = curr_act->get_swim_track();
    breakName = settings::get_generalValue("breakname");

    if(curr_act->get_sport() == settings::isSwim)
    {
        typeList << "Break" << "Freestyle" << "Backstroke" << "Breaststroke" << "Butterfly" << "Drill" << "Mixed" << "IM";
        ui->comboBox_edit->addItem("Swim Laps");
        ui->comboBox_edit->addItem("Intervals");
        ui->comboBox_type->addItems(typeList);
        ui->comboBox_edit->setVisible(true);
        ui->label_edit->setVisible(true);
        this->set_visible(true,false);
        this->setFixedHeight(320);
    }
    else
    {
        editModel = curr_act->edit_int_model;
        ui->comboBox_edit->setVisible(false);
        ui->label_edit->setVisible(false);
        this->set_visible(false,true);
        this->setFixedHeight(300);
    }

    this->set_lapinfo();

}

enum {UPDATE,ADD,DELETE};
enum {SWIMLAPS,INTERVALS};

Dialog_lapeditor::~Dialog_lapeditor()
{
    delete ui;
}

void Dialog_lapeditor::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_lapeditor::set_lapinfo()
{
    ui->comboBox_lap->clear();

    for(int i = 0; i < editModel->rowCount(); ++i)
    {
        ui->comboBox_lap->addItem(editModel->data(editModel->index(i,0,QModelIndex())).toString());
    }
    ui->comboBox_lap->setCurrentIndex(selRow);
}

void Dialog_lapeditor::set_components(bool bSet)
{
    ui->doubleSpinBox_distance->setEnabled(bSet);
    ui->spinBox_starttime->setEnabled(bSet);
    ui->spinBox_endtime->setEnabled(bSet);
}

void Dialog_lapeditor::set_visible(bool swimLap,bool intMode)
{
    ui->spinBox_strokes->setVisible(swimLap);
    ui->label_strokes->setVisible(swimLap);
    ui->comboBox_type->setVisible(swimLap);
    ui->label_type->setVisible(swimLap);

    ui->doubleSpinBox_distance->setVisible(intMode);
    ui->label_distance->setVisible(intMode);
    ui->label_km->setVisible(intMode);
    ui->progressBar_updateInt->setVisible(intMode);
}

void Dialog_lapeditor::set_duration()
{
    int duration = ui->spinBox_endtime->value() - ui->spinBox_starttime->value();
    if(duration <= 3600)
    {
        ui->timeEdit_duration->setDisplayFormat("mm:ss");
        ui->timeEdit_duration->setTime(QTime::fromString(settings::set_time(duration),"mm:ss"));
    }
    else
    {
        ui->timeEdit_duration->setDisplayFormat("hh:mm:ss");
        ui->timeEdit_duration->setTime(QTime::fromString(settings::set_time(duration),"hh:mm:ss"));
    }
}
void Dialog_lapeditor::set_lapSpeed(double dist)
{
    double lapSpeed = 0,pace = 0;
    int duration = ui->spinBox_endtime->value()-ui->spinBox_starttime->value();
    if(dist > 0)
    {
        pace = duration / (dist*10.0);
        lapSpeed = 360.0 / pace;
        ui->label_speed->setText(QString::number(settings::set_doubleValue(lapSpeed,false))+" km/h");
    }
    else
    {
        ui->label_speed->setText("0 km/h");
    }
}

int Dialog_lapeditor::calc_strokes(int duration)
{
    int strokes;
    double val = ((swimLapLen / 2) / (swimLapLen / duration));
    strokes = ceil(val + (val*0.125));

    return strokes;
}

void Dialog_lapeditor::updateSwimModel(int index,int duration, double lapSpeed,int stroke)
{
    editModel->setData(editModel->index(index,0,QModelIndex()),ui->lineEdit_newName->text());
    editModel->setData(editModel->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
    editModel->setData(editModel->index(index,2,QModelIndex()),duration);
    editModel->setData(editModel->index(index,3,QModelIndex()),stroke);
    editModel->setData(editModel->index(index,4,QModelIndex()),lapSpeed);
    editModel->setData(editModel->index(index,6,QModelIndex()),ui->comboBox_type->currentIndex());
    editModel->setData(editModel->index(index+1,1,QModelIndex()),ui->spinBox_starttime->value()+duration);
}

void Dialog_lapeditor::updateIntModel(int index)
{
    editModel->setData(editModel->index(index,0,QModelIndex()),ui->lineEdit_newName->text());
    editModel->setData(editModel->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
    editModel->setData(editModel->index(index,2,QModelIndex()),ui->spinBox_endtime->value());
    editModel->setData(editModel->index(index,3,QModelIndex()),ui->doubleSpinBox_distance->value());
    editModel->setData(editModel->index(index+1,1,QModelIndex()),ui->spinBox_endtime->value()+1);
    ui->progressBar_updateInt->setValue(1);
}

void Dialog_lapeditor::edit_laps(int editMode,int index)
{
    double lapSpeed = 0;
    int duration = 0, stroke = 0;

    duration = ui->spinBox_endtime->value()-ui->spinBox_starttime->value();

    if(curr_act->get_sport() == settings::isSwim && ui->lineEdit_newName->text().contains(breakName))
    {
        lapSpeed = 0;
        stroke = 0;
    }
    else
    {
        stroke = editModel->data(editModel->index(index,3,QModelIndex())).toInt();
        if(stroke == 0)
        {
            stroke = this->calc_strokes(duration);
        }
        else
        {
            stroke = ui->spinBox_strokes->value();
        }
        lapSpeed = settings::get_speed(QTime::fromString(settings::set_time(duration),"mm:ss"),swimLapLen,settings::isSwim,false).toDouble();
    }

    if(editMode == UPDATE)
    {
        if(curr_act->get_sport() == settings::isSwim)
        {
            if(ui->comboBox_edit->currentIndex() == SWIMLAPS)
            {
                this->updateSwimModel(index,duration,lapSpeed,stroke);
                this->recalulateData(index);
            }
            if(ui->comboBox_edit->currentIndex() == INTERVALS)
            {
                this->updateIntModel(index);
            }
        }
        else
        {
            this->updateIntModel(index);
        }
        this->set_lapinfo();
    }
    if(editMode == ADD)
    {
        if(curr_act->get_sport() == settings::isSwim)
        {
            ++index;
            editModel->insertRow(index,QModelIndex());
            this->updateSwimModel(index,duration,lapSpeed,stroke);
            if(ui->comboBox_edit->currentIndex() == INTERVALS)
            {
                curr_act->edit_int_model->insertRow(index,QModelIndex());
                curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index,0,QModelIndex()),ui->lineEdit_newName->text());
            }
            this->recalulateData(index);
        }
        else
        {
            editModel->insertRow(index,QModelIndex());
            this->updateIntModel(index);
            curr_act->curr_act_model->insertRow(index,QModelIndex());
            curr_act->curr_act_model->setData(curr_act->curr_act_model->index(index,0,QModelIndex()),editModel->data(editModel->index(index,0,QModelIndex())).toString());
        }
        this->set_lapinfo();
    }
    if(editMode == DELETE)
    {
        if(curr_act->get_sport() == settings::isSwim)
        {
            if(ui->comboBox_edit->currentIndex() == SWIMLAPS)
            {
                editModel->removeRow(index,QModelIndex());
            }
            else
            {
                editModel->removeRow(index,QModelIndex());
                curr_act->curr_act_model->removeRow(index,QModelIndex());
            }
            this->recalulateData(index-1);
        }
        else
        {
            editModel->removeRow(index,QModelIndex());
            curr_act->curr_act_model->removeRow(index,QModelIndex());
        }
        this->set_lapinfo();
    }
}

void Dialog_lapeditor::recalulateData(int row)
{
    QString currLap;
    QModelIndex new_index,curr_index;
    QStandardItemModel *model = curr_act->swim_xdata;
    double swimLap = swimLapLen/1000;
    double lapDist = 0;
    bool isBreak = false;
    int lapCount,lapTime,startTime;
    lapCount = model->rowCount()-1;

    for(int i = 0; i < curr_act->swim_xdata->rowCount(); ++i)
    {
        QString lap = model->data(model->index(i,0,QModelIndex())).toString();
        if(lap == breakName)
        {
            lapDist = lapDist + swimLap;
            isBreak = true;
        }
        else
        {
            if(!isBreak)
            {
                if(i != 0)lapDist = lapDist + swimLap;
            }
            else
            {
                isBreak = false;
            }
        }
        model->setData(model->index(i,5,QModelIndex()),lapDist);
    }

    if(row != lapCount)
    {
        do
        {
            curr_index = model->index(row,1,QModelIndex());
            startTime = model->data(model->index(row,1,QModelIndex())).toInt();
            lapTime = model->data(model->index(row,2,QModelIndex())).toInt();

            new_index = model->index(row+1,1,QModelIndex());
            model->setData(new_index,startTime+lapTime,Qt::EditRole);
            ++row;
            currLap = model->data(model->index(row,0,QModelIndex())).toString();
        } while (currLap != breakName && row < lapCount-1);

        startTime = model->data(new_index).toInt();
        if(row == lapCount)
        {
            lapTime = model->data(model->index(row,2,QModelIndex())).toInt();
            model->setData(model->index(row,2,QModelIndex()),lapTime);
        }
        else
        {
            lapTime = model->data(model->index(row+1,1,QModelIndex())).toInt() - startTime;
            model->setData(model->index(row,2,QModelIndex()),lapTime);
        }
    }
    else
    {
        lapTime = model->data(model->index(row,2,QModelIndex())).toInt();
        model->setData(model->index(row,2,QModelIndex()),lapTime);
    }
}

void Dialog_lapeditor::on_radioButton_add_clicked()
{
    this->set_components(true);
    ui->lineEdit_newName->setEnabled(true);
}

void Dialog_lapeditor::on_radioButton_del_clicked()
{
    this->set_components(false);
    ui->lineEdit_newName->setEnabled(false);
}

void Dialog_lapeditor::on_radioButton_update_clicked()
{
    this->set_components(true);
    ui->lineEdit_newName->setEnabled(true);
    ui->lineEdit_newName->setText(ui->comboBox_lap->currentText());
}

void Dialog_lapeditor::on_pushButton_ok_clicked()
{
    int editMode,index;
    index = ui->comboBox_lap->currentIndex();
    selRow = index;
    if(ui->radioButton_update->isChecked()) editMode = UPDATE;
    if(ui->radioButton_add->isChecked()) editMode = ADD;
    if(ui->radioButton_del->isChecked()) editMode = DELETE;
    this->edit_laps(editMode,index);
}

void Dialog_lapeditor::on_comboBox_lap_currentIndexChanged(int vLap)
{
    int duration,stroke;

    if(curr_act->get_sport() == settings::isSwim)
    {
        ui->spinBox_starttime->setValue(editModel->data(editModel->index(vLap,1,QModelIndex())).toInt());
        ui->comboBox_type->setCurrentIndex(editModel->data(editModel->index(vLap,6,QModelIndex())).toInt());

        if(ui->comboBox_edit->currentIndex() == SWIMLAPS)
        {
            duration = editModel->data(editModel->index(vLap,2,QModelIndex())).toInt();
            ui->spinBox_endtime->setValue(ui->spinBox_starttime->value()+duration);
            stroke = editModel->data(editModel->index(vLap,3,QModelIndex())).toInt();
            this->set_duration();

            if(stroke == 0)
            {
                stroke = this->calc_strokes(duration);
            }

            ui->spinBox_strokes->setValue(stroke);
        }
        if(ui->comboBox_edit->currentIndex() == INTERVALS)
        {
            ui->spinBox_endtime->setValue(editModel->data(editModel->index(vLap,2,QModelIndex())).toInt());
            ui->doubleSpinBox_distance->setValue(editModel->data(editModel->index(vLap,3,QModelIndex())).toDouble());
            this->set_duration();
        }
    }
    else
    {
        ui->spinBox_starttime->setValue(editModel->data(editModel->index(vLap,1,QModelIndex())).toInt());
        ui->spinBox_endtime->setValue(editModel->data(editModel->index(vLap,2,QModelIndex())).toInt());
        ui->doubleSpinBox_distance->setValue(editModel->data(editModel->index(vLap,3,QModelIndex())).toDouble());       
        this->set_duration();
    }

    if(ui->radioButton_update->isChecked())
    {
        ui->lineEdit_newName->setText(ui->comboBox_lap->currentText());
    }

    ui->label_start->setText(settings::set_time(ui->spinBox_starttime->value()));
    ui->label_end->setText(settings::set_time(ui->spinBox_endtime->value()));
    this->set_lapSpeed(ui->doubleSpinBox_distance->value());
}

void Dialog_lapeditor::on_comboBox_edit_currentIndexChanged(int index)
{
    if(index == SWIMLAPS)
    {
        editModel = curr_act->swim_xdata;
        this->set_visible(true,false);
    }
    if(index == INTERVALS)
    {
        editModel = curr_act->edit_int_model;
        this->set_visible(false,true);
        ui->progressBar_updateInt->setValue(0);
    }
    this->set_lapinfo();
}

void Dialog_lapeditor::on_spinBox_starttime_valueChanged(int value)
{
    ui->label_start->setText(settings::set_time(value));
    this->set_duration();
}

void Dialog_lapeditor::on_spinBox_endtime_valueChanged(int value)
{
    ui->label_end->setText(settings::set_time(value));
    this->set_duration();
}

void Dialog_lapeditor::on_comboBox_lap_activated(int index)
{
    Q_UNUSED(index)
    ui->progressBar_updateInt->setValue(0);
}

void Dialog_lapeditor::on_doubleSpinBox_distance_valueChanged(double value)
{
    this->set_lapSpeed(value);
}
