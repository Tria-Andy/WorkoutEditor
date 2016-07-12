#include <QDebug>
#include "dialog_add.h"
#include "ui_dialog_add.h"

Dialog_add::Dialog_add(QWidget *parent,schedule *p_sched,workout *p_workout,settings *p_settings,standardWorkouts *p_stdWorkout) :
    QDialog(parent),
    ui(new Ui::Dialog_add)
{
    ui->setupUi(this);
    workSched = p_sched;
    curr_workout = p_workout;
    add_settings = p_settings;
    stdWorkouts = p_stdWorkout;
    current_date = new QDate();
    workout_date = current_date->currentDate();

    w_date = workout_date.toString("dd.MM.yyyy");
    w_time = ui->timeEdit_time->time().toString("hh:mm");
    w_distance = 0.0;
    w_stress = 0;
    w_duration = "00:00:00";

    ui->dateEdit_wdate->setDate(workout_date);
    ui->lineEdit_wkw->setText(QString::number(workout_date.weekNumber()));
    ui->comboBox_sport->addItems(add_settings->get_sportList());
    ui->comboBox_wcode->addItems(add_settings->get_codeList());

    connect(ui->dateEdit_wdate, SIGNAL(dateChanged(QDate)),this, SLOT(set_workout_date()));
    connect(ui->dateEdit_wdate, SIGNAL(dateChanged(QDate)),this, SLOT(set_workout_calweek()));
    connect(ui->timeEdit_time, SIGNAL(timeChanged(QTime)), this, SLOT(set_workout_time()));
    connect(ui->timeEdit_time, SIGNAL(timeChanged(QTime)), this, SLOT(set_workout_time()));
    connect(ui->doubleSpinBox_dist, SIGNAL(valueChanged(double)), this, SLOT(set_workout_distance()));
    connect(ui->spinBox_stress_score, SIGNAL(valueChanged(int)), this, SLOT(set_workout_stress()));

    connect(ui->pushButton_add,SIGNAL(clicked(bool)),this,SLOT(accept()));
    connect(ui->pushButton_cancel,SIGNAL(clicked(bool)),this,SLOT(reject()));

    ui->lineEdit_phase->setText(workSched->get_weekPhase(workout_date));
}

Dialog_add::~Dialog_add()
{
    delete current_date;
    delete ui;
}

void Dialog_add::set_workout_date()
{
    workout_date = ui->dateEdit_wdate->date();
    w_date = workout_date.toString("dd.MM.yyyy");
    ui->lineEdit_phase->setText(workSched->get_weekPhase(workout_date));
    QList<QStandardItem*> workCount = workSched->workout_schedule->findItems(w_date,Qt::MatchExactly,1);
    if(workCount.count() == 3)
    {
        ui->pushButton_add->setEnabled(false);
    }
    else
    {
        ui->pushButton_add->setEnabled(true);
    }
}

void Dialog_add::set_workout_time()
{
    w_time = ui->timeEdit_time->time().toString("hh:mm");
}

void Dialog_add::set_workout_calweek()
{
    ui->lineEdit_wkw->setText(QString::number(workout_date.weekNumber()));
    w_cal_week = ui->lineEdit_wkw->text() +"_"+ QString::number(ui->dateEdit_wdate->date().year());
}

void Dialog_add::set_workout_duration()
{
    w_duration = ui->timeEdit_duration->time().toString("hh:mm:ss");
}

void Dialog_add::set_workout_distance()
{
    w_distance = ui->doubleSpinBox_dist->value();
}

void Dialog_add::set_workout_stress()
{
    w_stress = ui->spinBox_stress_score->value();
}

void Dialog_add::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_add::on_pushButton_add_clicked()
{
    curr_workout->set_workout_date(w_date);
    curr_workout->set_workout_time(w_time);
    curr_workout->set_workout_calweek(w_cal_week);
    curr_workout->set_workout_sport(ui->comboBox_sport->currentText());
    curr_workout->set_workout_code(ui->comboBox_wcode->currentText());
    curr_workout->set_workout_title(ui->lineEdit_wtitle->text());
    curr_workout->set_workout_duration(w_duration);
    curr_workout->set_workout_distance(w_distance);
    curr_workout->set_workout_stress(w_stress);

    accept();
}

void Dialog_add::on_doubleSpinBox_dist_valueChanged(double dist)
{
    ui->label_avg_speed->setText(add_settings->get_workout_pace(dist,ui->timeEdit_duration->time(),ui->comboBox_sport->currentText(),false));
}

void Dialog_add::on_timeEdit_duration_timeChanged(const QTime &time)
{
    ui->label_avg_speed->setText(add_settings->get_workout_pace(ui->doubleSpinBox_dist->value(),time,ui->comboBox_sport->currentText(),false));
    this->set_workout_duration();
}

void Dialog_add::on_comboBox_sport_currentTextChanged(const QString &sport)
{
    ui->label_avg_speed->setText(add_settings->get_workout_pace(ui->doubleSpinBox_dist->value(),ui->timeEdit_duration->time(),sport,false));
}

void Dialog_add::on_pushButton_workouts_clicked()
{
    Dialog_workouts std_workouts(this,ui->comboBox_sport->currentText(),stdWorkouts);
    std_workouts.setModal(true);
    int returnCode = std_workouts.exec();
    if(returnCode == QDialog::Accepted)
    {
        ui->comboBox_wcode->setCurrentText(std_workouts.get_workout_code());
        ui->lineEdit_wtitle->setText(std_workouts.get_workout_title());
        ui->timeEdit_duration->setTime(std_workouts.get_workout_duration());
        ui->doubleSpinBox_dist->setValue(std_workouts.get_workout_distance());
        ui->spinBox_stress_score->setValue(std_workouts.get_workout_stress());
    }
}
