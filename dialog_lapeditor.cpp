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


void Dialog_lapeditor::on_radioButton_add_clicked()
{
    this->set_components(true);
}

void Dialog_lapeditor::on_radioButton_del_clicked()
{
    this->set_components(false);
}
