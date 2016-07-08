#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include "dialog_edit.h"
#include "ui_dialog_edit.h"

Dialog_edit::Dialog_edit(QWidget *parent, const QDate w_date, schedule *p_sched, workout *p_workout, settings *p_setting,standardWorkouts *p_stdworkout) :
    QDialog(parent),
    ui(new Ui::Dialog_edit)
{
    ui->setupUi(this);
    edit_workout = p_workout;
    edit_settings = p_setting;
    workSched = p_sched;
    std_workouts = p_stdworkout;
    ui->comboBox_wcode->addItems(edit_settings->get_codeList());
    this->set_workout_info(w_date);

    connect(ui->dateEdit_edit_date, SIGNAL(dateChanged(QDate)),this, SLOT(set_edit_calweek()));
}

Dialog_edit::~Dialog_edit()
{
    delete ui;
}

void Dialog_edit::set_workout_info(const QDate &d)
{
    QModelIndex index;
    ui->dateEdit_workoutdate->setDate(d);

    list = workSched->workout_schedule->findItems(d.toString("dd.MM.yyyy"),Qt::MatchExactly,1);

    for(int i = 0; i < list.count();++i)
    {
        index = workSched->workout_schedule->indexFromItem(list.at(i));
        ui->comboBox_time->addItem(workSched->workout_schedule->item(index.row(),2)->text());
    }

}

void Dialog_edit::show_workout(int c_index)
{
    curr_index = workSched->workout_schedule->indexFromItem(list.at(c_index));

    ui->dateEdit_edit_date->setDate(QDate::fromString(workSched->workout_schedule->item(curr_index.row(),1)->text(),"dd.MM.yyyy"));
    ui->timeEdit_edit_time->setTime(QTime::fromString(workSched->workout_schedule->item(curr_index.row(),2)->text()));
    ui->lineEdit_cweek->setText(QString::number(QDate::fromString(workSched->workout_schedule->item(curr_index.row(),1)->text(),"dd.MM.yyyy").weekNumber()));
    ui->lineEdit_phase->setText(workSched->get_weekPhase(QDate::fromString(workSched->workout_schedule->item(curr_index.row(),1)->text(),"dd.MM.yyyy")));
    ui->lineEdit_sport->setText(workSched->workout_schedule->item(curr_index.row(),3)->text());
    ui->comboBox_wcode->setCurrentText(workSched->workout_schedule->item(curr_index.row(),4)->text());
    ui->lineEdit_wtitle->setText(workSched->workout_schedule->item(curr_index.row(),5)->text());
    ui->timeEdit_duration->setTime(QTime::fromString(workSched->workout_schedule->item(curr_index.row(),6)->text()));
    ui->doubleSpinBox_distance->setValue(workSched->workout_schedule->item(curr_index.row(),7)->text().toDouble());
    ui->label_speed->setText(edit_settings->get_workout_pace(ui->doubleSpinBox_distance->value(),ui->timeEdit_duration->time(),ui->lineEdit_sport->text(),false));
    ui->spinBox_stress->setValue(workSched->workout_schedule->item(curr_index.row(),8)->text().toInt());
}

void Dialog_edit::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_edit::on_comboBox_time_currentIndexChanged(int index)
{
    this->show_workout(index);
}

void Dialog_edit::set_edit_calweek()
{
    ui->lineEdit_cweek->setText(QString::number(ui->dateEdit_edit_date->date().weekNumber()));
    ui->lineEdit_phase->setText(workSched->get_weekPhase(ui->dateEdit_edit_date->date()));
}

void Dialog_edit::set_workout_data()
{
    edit_workout->set_workout_date(ui->dateEdit_edit_date->date().toString("dd.MM.yyyy"));
    edit_workout->set_workout_time(ui->timeEdit_edit_time->time().toString("hh:mm"));
    edit_workout->set_workout_calweek(ui->lineEdit_cweek->text()+"_"+QString::number(ui->dateEdit_edit_date->date().year()));
    edit_workout->set_workout_sport(ui->lineEdit_sport->text());
    edit_workout->set_workout_code(ui->comboBox_wcode->currentText());
    edit_workout->set_workout_title(ui->lineEdit_wtitle->text());
    edit_workout->set_workout_duration(ui->timeEdit_duration->time().toString("hh:mm:ss"));
    edit_workout->set_workout_distance(ui->doubleSpinBox_distance->value());
    edit_workout->set_workout_stress(ui->spinBox_stress->value());
}


void Dialog_edit::set_result(QString result_text,int result_code)
{
    QMessageBox::StandardButton reply;

    if(result_code == 1)reply = QMessageBox::warning(this,result_text + " Workout",result_text + " Workout?",QMessageBox::Yes|QMessageBox::No);
    if(result_code == 2)reply = QMessageBox::question(this,result_text + " Workout",result_text + " Workout?",QMessageBox::Yes|QMessageBox::No);
    if(result_code == 3) reply = QMessageBox::critical(this,result_text + " Workout",result_text + " Workout?",QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        edit_result = result_code;
        accept();
    }
}

void Dialog_edit::on_pushButton_edit_clicked()
{
    this->set_workout_data();
    this->set_result("Edit",1);
}

void Dialog_edit::on_pushButton_copy_clicked()
{
    this->set_workout_data();
    this->set_result("Copy",2);
}

void Dialog_edit::on_pushButton_delete_clicked()
{
    this->set_result("Delete",3);
}

void Dialog_edit::on_doubleSpinBox_distance_valueChanged(double dist)
{
    ui->label_speed->setText(edit_settings->get_workout_pace(dist,ui->timeEdit_duration->time(),ui->lineEdit_sport->text(),false));
}

void Dialog_edit::on_timeEdit_duration_timeChanged(const QTime &time)
{
    ui->label_speed->setText(edit_settings->get_workout_pace(ui->doubleSpinBox_distance->value(),time,ui->lineEdit_sport->text(),false));
}

void Dialog_edit::on_pushButton_clicked()
{
    stdWorkouts = new Dialog_workouts(this,ui->lineEdit_sport->text(),std_workouts);
    stdWorkouts->setModal(true);
    int returnCode = stdWorkouts->exec();
    if(returnCode == QDialog::Accepted)
    {
        ui->comboBox_wcode->setCurrentText(stdWorkouts->get_workout_code());
        ui->lineEdit_wtitle->setText(stdWorkouts->get_workout_title());
        ui->timeEdit_duration->setTime(stdWorkouts->get_workout_duration());
        ui->doubleSpinBox_distance->setValue(stdWorkouts->get_workout_distance());
        ui->spinBox_stress->setValue(stdWorkouts->get_workout_stress());
    }

    delete stdWorkouts;
}

void Dialog_edit::on_dateEdit_edit_date_dateChanged(const QDate &date)
{
    QList<QStandardItem*> workCount = workSched->workout_schedule->findItems(date.toString("dd.MM.yyyy"),Qt::MatchExactly,1);
    if(workCount.count() == 3)
    {
        ui->pushButton_copy->setEnabled(false);
        ui->pushButton_edit->setEnabled(false);
    }
    else
    {
        ui->pushButton_copy->setEnabled(true);
        ui->pushButton_edit->setEnabled(true);
    }
}
