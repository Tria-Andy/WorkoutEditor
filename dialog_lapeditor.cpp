#include "dialog_lapeditor.h"
#include "ui_dialog_lapeditor.h"

Dialog_lapeditor::Dialog_lapeditor(QWidget *parent,Activity *p_act,int p_index) :
    QDialog(parent),
    ui(new Ui::Dialog_lapeditor)
{
    ui->setupUi(this);
    curr_act = p_act;
    selIndex = p_index;

    if(curr_act->get_sport() == settings::isSwim)
    {
        ui->comboBox_edit->addItem("Swim Laps");
        ui->comboBox_edit->addItem("Intervalls");
        ui->comboBox_edit->setVisible(true);
        ui->label_edit->setVisible(true);
        this->set_visible(true,false);
        this->setFixedHeight(300);
    }
    else
    {
        editModel = curr_act->edit_int_model;
        ui->comboBox_edit->setVisible(false);
        ui->label_edit->setVisible(false);
        this->set_visible(false,true);
        this->setFixedHeight(280);
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
    ui->comboBox_lap->setCurrentIndex(selIndex);
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

    ui->doubleSpinBox_distance->setVisible(intMode);
    ui->label_distance->setVisible(intMode);
    ui->label_km->setVisible(intMode);
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

int Dialog_lapeditor::calc_strokes(int duration)
{
    int strokes;
    double val = ((curr_act->get_swim_track() / 2) / (curr_act->get_swim_track() / duration));
    strokes = ceil(val + (val*0.125));

    return strokes;
}


void Dialog_lapeditor::edit_laps(int editMode,int index)
{
    double lapSpeed = 0;
    int duration = 0, stroke = 0;

    duration = ui->spinBox_endtime->value()-ui->spinBox_starttime->value();

    if(curr_act->get_sport() == settings::isSwim && ui->lineEdit_newName->text().contains(settings::get_breakName()))
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
        lapSpeed = settings::get_speed(QTime::fromString(settings::set_time(duration),"mm:ss"),curr_act->get_swim_track(),settings::isSwim,false).toDouble();
    }

    if(editMode == 0)
    {
        if(curr_act->get_sport() == settings::isSwim)
        {
            editModel->insertRow(index,QModelIndex());
            editModel->setData(editModel->index(index,0,QModelIndex()),ui->lineEdit_newName->text());
            editModel->setData(editModel->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
            editModel->setData(editModel->index(index+1,1,QModelIndex()),ui->spinBox_endtime->value());
            editModel->setData(editModel->index(index,2,QModelIndex()),duration);
            editModel->setData(editModel->index(index,3,QModelIndex()),stroke);
            editModel->setData(editModel->index(index,4,QModelIndex()),lapSpeed);
            if(ui->comboBox_edit->currentIndex() == 1)
            {
                curr_act->edit_dist_model->insertRow(index,QModelIndex());
                curr_act->edit_dist_model->setData(curr_act->edit_dist_model->index(index,0,QModelIndex()),ui->lineEdit_newName->text());
            }
            this->recalulateData(index);
        }
        else
        {
            curr_act->edit_dist_model->insertRow(index,QModelIndex());
            curr_act->curr_act_model->insertRow(index,QModelIndex());
            editModel->insertRow(index,QModelIndex());
            editModel->setData(editModel->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
            editModel->setData(editModel->index(index,2,QModelIndex()),ui->spinBox_endtime->value());
            editModel->setData(editModel->index(index+1,1,QModelIndex()),ui->spinBox_endtime->value());
        }
        this->set_lapinfo();
    }
    if(editMode == 1)
    {
        if(curr_act->get_sport() == settings::isSwim)
        {
            editModel->setData(editModel->index(index,0,QModelIndex()),ui->lineEdit_newName->text());
            editModel->setData(editModel->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
            editModel->setData(editModel->index(index,2,QModelIndex()),duration);
            editModel->setData(editModel->index(index,3,QModelIndex()),ui->spinBox_strokes->value());
            editModel->setData(editModel->index(index,4,QModelIndex()),lapSpeed);
            editModel->setData(editModel->index(index+1,1,QModelIndex()),ui->spinBox_starttime->value()+duration);
            this->recalulateData(index);
        }
        else
        {
            editModel->setData(editModel->index(index,1,QModelIndex()),ui->spinBox_starttime->value());
            editModel->setData(editModel->index(index,2,QModelIndex()),ui->spinBox_endtime->value());
            editModel->setData(editModel->index(index+1,1,QModelIndex()),ui->spinBox_endtime->value());
            curr_act->edit_dist_model->setData(curr_act->edit_dist_model->index(index,1,QModelIndex()),ui->doubleSpinBox_distance->value());
        }
    }
    if(editMode == 2)
    {
        if(curr_act->get_sport() == settings::isSwim)
        {
            editModel->removeRow(index,QModelIndex());
            this->recalulateData(index);
        }
        else
        {
            editModel->removeRow(index,QModelIndex());
            curr_act->edit_dist_model->removeRow(index,QModelIndex());      
            curr_act->curr_act_model->removeRow(index,QModelIndex());
        }
        this->set_lapinfo();
    }
}

void Dialog_lapeditor::recalulateData(int index)
{
    double swimLap = curr_act->get_swim_track()/1000;
    double lapDist = 0;
    bool isBreak = false;
    qDebug() << index;
    for(int i = 0; i < curr_act->swim_xdata->rowCount(); ++i)
    {
        QString lap = curr_act->swim_xdata->data(curr_act->swim_xdata->index(i,0,QModelIndex())).toString();
        if(lap == settings::get_breakName())
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
        curr_act->swim_xdata->setData(curr_act->swim_xdata->index(i,5,QModelIndex()),lapDist);
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
    int editMode;
    if(ui->radioButton_add->isChecked()) editMode = 0;
    if(ui->radioButton_update->isChecked()) editMode = 1;
    if(ui->radioButton_del->isChecked()) editMode = 2;
    this->edit_laps(editMode,ui->comboBox_lap->currentIndex());
}

void Dialog_lapeditor::on_comboBox_lap_currentIndexChanged(int vLap)
{
    int duration,stroke;

    if(curr_act->get_sport() == settings::isSwim)
    {
        ui->spinBox_starttime->setValue(editModel->data(editModel->index(vLap,1,QModelIndex())).toInt());

        if(ui->comboBox_edit->currentIndex() == 0)
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
        else
        {
            ui->spinBox_endtime->setValue(editModel->data(editModel->index(vLap,2,QModelIndex())).toInt());
            ui->doubleSpinBox_distance->setValue(curr_act->edit_dist_model->data(curr_act->edit_dist_model->index(vLap,1,QModelIndex())).toDouble());
            this->set_duration();
        }
    }
    else
    {
        ui->spinBox_starttime->setValue(editModel->data(editModel->index(vLap,1,QModelIndex())).toInt());
        ui->spinBox_endtime->setValue(editModel->data(editModel->index(vLap,2,QModelIndex())).toInt());
        ui->doubleSpinBox_distance->setValue(curr_act->edit_dist_model->data(curr_act->edit_dist_model->index(vLap,1,QModelIndex())).toDouble());
        this->set_duration();
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
        this->set_visible(true,false);
    }
    else
    {
        editModel = curr_act->edit_int_model;
        this->set_visible(false,true);
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
