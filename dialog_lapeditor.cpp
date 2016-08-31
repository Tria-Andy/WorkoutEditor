#include "dialog_lapeditor.h"
#include "ui_dialog_lapeditor.h"

Dialog_lapeditor::Dialog_lapeditor(QWidget *parent,Activity *p_act) :
    QDialog(parent),
    ui(new Ui::Dialog_lapeditor)
{
    ui->setupUi(this);
    curr_act = p_act;

    for(int i = 0; i < curr_act->edit_int_model->rowCount(); ++i)
    {
        ui->comboBox_lap->addItem(curr_act->edit_int_model->data(curr_act->edit_int_model->index(i,0,QModelIndex())).toString());
    }
}

Dialog_lapeditor::~Dialog_lapeditor()
{
    delete ui;
}

void Dialog_lapeditor::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_lapeditor::set_components(bool bSet)
{
    ui->doubleSpinBox_distance->setEnabled(bSet);
    ui->spinBox_starttime->setEnabled(bSet);
    ui->spinBox_endtime->setEnabled(bSet);
}

void Dialog_lapeditor::edit_laps(bool isAdd,int index)
{
    if(isAdd)
    {
        curr_act->edit_dist_model->insertRow(index,QModelIndex());
        curr_act->edit_int_model->insertRow(index,QModelIndex());
        curr_act->curr_act_model->insertRow(index,QModelIndex());
        curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
        curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index,2,QModelIndex()),ui->spinBox_endtime->value());
        curr_act->edit_int_model->setData(curr_act->edit_int_model->index(index+1,1,QModelIndex()),ui->spinBox_endtime->value());
        if(curr_act->get_sport() == curr_act->isSwim)
        {

        }

    }
    else
    {
        curr_act->edit_dist_model->removeRow(index,QModelIndex());
        curr_act->edit_int_model->removeRow(index,QModelIndex());
        curr_act->curr_act_model->removeRow(index,QModelIndex());
    }
    accept();
}


void Dialog_lapeditor::on_radioButton_add_clicked()
{
    this->set_components(true);
}

void Dialog_lapeditor::on_radioButton_del_clicked()
{
    this->set_components(false);
}

void Dialog_lapeditor::on_pushButton_ok_clicked()
{
    bool isAdd;
    if(ui->radioButton_add->isChecked()) isAdd = true;
    if(ui->radioButton_del->isChecked()) isAdd = false;
    this->edit_laps(isAdd,ui->comboBox_lap->currentIndex());
}

void Dialog_lapeditor::on_comboBox_lap_currentIndexChanged(int vLap)
{
    ui->spinBox_starttime->setValue(curr_act->edit_int_model->data(curr_act->edit_int_model->index(vLap,1,QModelIndex())).toInt());
    ui->spinBox_endtime->setValue(curr_act->edit_int_model->data(curr_act->edit_int_model->index(vLap,2,QModelIndex())).toInt());
}
