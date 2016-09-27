#include "dialog_lapeditor.h"
#include "ui_dialog_lapeditor.h"

Dialog_lapeditor::Dialog_lapeditor(QWidget *parent,Activity *p_act) :
    QDialog(parent),
    ui(new Ui::Dialog_lapeditor)
{
    ui->setupUi(this);
    curr_act = p_act;

    ui->lineEdit_newName->setEnabled(false);
    if(curr_act->get_sport() == curr_act->isSwim)
    {
        ui->comboBox_edit->addItem("Swim Laps");
        ui->comboBox_edit->addItem("Intervalls");
        this->setFixedHeight(265);
    }
    else
    {
        ui->comboBox_edit->setVisible(false);
        ui->label_edit->setVisible(false);
        this->setFixedHeight(245);
    }

    this->set_lapinfo();
}

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
}

void Dialog_lapeditor::set_components(bool bSet)
{
    ui->doubleSpinBox_distance->setEnabled(bSet);
    ui->spinBox_starttime->setEnabled(bSet);
    ui->spinBox_endtime->setEnabled(bSet);
}

void Dialog_lapeditor::edit_laps(int editMode,int index)
{
    double lapSpeed = 0;
    int duration = 0, stroke = 0;

    duration = ui->spinBox_endtime->value()-ui->spinBox_starttime->value();

    if(curr_act->get_sport() == curr_act->isSwim && ui->lineEdit_newName->text().contains("Break"))
    {
        lapSpeed = 0;
        stroke = 0;
    }
    else
    {
        stroke = static_cast<int>(floor(curr_act->get_swim_track() / 2));
        lapSpeed = settings::get_speed(QTime::fromString(settings::set_time(duration),"mm:ss"),curr_act->get_swim_track(),settings::isSwim,false).toDouble();
    }

    if(editMode == 0)
    {
        if(curr_act->get_sport() == curr_act->isSwim)
        {
            curr_act->swim_xdata->insertRow(index,QModelIndex());
            curr_act->swim_xdata->setData(curr_act->swim_xdata->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
            curr_act->swim_xdata->setData(curr_act->swim_xdata->index(index+1,1,QModelIndex()),ui->spinBox_endtime->value());
            if(ui->comboBox_edit->currentIndex() == 1)
            {
                curr_act->edit_dist_model->insertRow(index,QModelIndex());
                curr_act->edit_dist_model->setData(curr_act->edit_dist_model->index(index,0,QModelIndex()),ui->lineEdit_newName->text());
            }
        }
        else
        {
            curr_act->edit_dist_model->insertRow(index,QModelIndex());
            curr_act->curr_act_model->insertRow(index,QModelIndex());
            curr_act->edit_int_model->insertRow(index,QModelIndex());
            curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
            curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index,2,QModelIndex()),ui->spinBox_endtime->value());
            curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index+1,1,QModelIndex()),ui->spinBox_endtime->value());
        }

    }
    if(editMode == 1)
    {
        if(curr_act->get_sport() == curr_act->isSwim)
        {
            curr_act->swim_xdata->setData(curr_act->swim_xdata->index(index,0,QModelIndex()),ui->lineEdit_newName->text());
            curr_act->swim_xdata->setData(curr_act->swim_xdata->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
            curr_act->swim_xdata->setData(curr_act->swim_xdata->index(index,2,QModelIndex()),duration);
            curr_act->swim_xdata->setData(curr_act->swim_xdata->index(index,3,QModelIndex()),stroke);
            curr_act->swim_xdata->setData(curr_act->swim_xdata->index(index,4,QModelIndex()),lapSpeed);
            curr_act->swim_xdata->setData(curr_act->swim_xdata->index(index+1,1,QModelIndex()),ui->spinBox_starttime->value()+duration);
        }
        else
        {
            curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
            curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index,2,QModelIndex()),ui->spinBox_endtime->value());
            curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index+1,1,QModelIndex()),ui->spinBox_endtime->value());
            curr_act->edit_dist_model->setData(curr_act->edit_dist_model->index(index,1,QModelIndex()),ui->doubleSpinBox_distance->value());
        }
    }
    if(editMode == 2)
    {
        if(curr_act->get_sport() == curr_act->isSwim)
        {
            curr_act->swim_xdata->removeRow(index,QModelIndex());
        }
        else
        {
            curr_act->edit_int_model->removeRow(index,QModelIndex());
            curr_act->edit_dist_model->removeRow(index,QModelIndex());      
            curr_act->curr_act_model->removeRow(index,QModelIndex());
        }
    }
    this->set_lapinfo();
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
}

void Dialog_lapeditor::on_pushButton_ok_clicked()
{
    int editMode;
    if(ui->radioButton_add->isChecked()) editMode = 0;
    if(ui->radioButton_update->isChecked()) editMode = 1;
    if(ui->radioButton_del->isChecked()) editMode = 2;
    this->edit_laps(editMode,ui->comboBox_lap->currentIndex());
}

void Dialog_lapeditor::on_comboBox_lap_currentIndexChanged(int vLap)
{
    if(curr_act->get_sport() == curr_act->isSwim)
    {
        int duration;
        ui->spinBox_starttime->setValue(curr_act->swim_xdata->data(curr_act->swim_xdata->index(vLap,1,QModelIndex())).toInt());
        duration = curr_act->swim_xdata->data(curr_act->swim_xdata->index(vLap,2,QModelIndex())).toInt();
        ui->spinBox_endtime->setValue(ui->spinBox_starttime->value()+duration);
    }
    else
    {
        ui->spinBox_starttime->setValue(curr_act->edit_int_model->data(curr_act->edit_int_model->index(vLap,1,QModelIndex())).toInt());
        ui->spinBox_endtime->setValue(curr_act->edit_int_model->data(curr_act->edit_int_model->index(vLap,2,QModelIndex())).toInt());
    }
    if(ui->radioButton_update->isChecked())
    {
        ui->lineEdit_newName->setText(ui->comboBox_lap->currentText());
    }

    ui->label_start->setText(settings::set_time(ui->spinBox_starttime->value()));
    ui->label_end->setText(settings::set_time(ui->spinBox_endtime->value()));
}

void Dialog_lapeditor::on_comboBox_edit_currentIndexChanged(int index)
{
    if(index == 0)
    {
        editModel = curr_act->swim_xdata;
    }
    else
    {
        editModel = curr_act->edit_int_model;
    }
    this->set_lapinfo();
}

void Dialog_lapeditor::on_spinBox_starttime_valueChanged(int value)
{
    ui->label_start->setText(settings::set_time(value));
}

void Dialog_lapeditor::on_spinBox_endtime_valueChanged(int value)
{
    ui->label_end->setText(settings::set_time(value));
}
