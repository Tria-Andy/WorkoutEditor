#include <QDebug>
#include "dialog_add.h"
#include "ui_dialog_add.h"

Dialog_add::Dialog_add(QWidget *parent,schedule *p_sched,settings *p_settings,standardWorkouts *p_stdWorkout) :
    QDialog(parent),
    ui(new Ui::Dialog_add)
{
    ui->setupUi(this);
    workSched = p_sched;
    add_settings = p_settings;
    stdWorkouts = p_stdWorkout;
    workout_date = QDate().currentDate();

    w_date = workout_date.toString("dd.MM.yyyy");
    w_time = ui->timeEdit_time->time().toString("hh:mm");
    w_distance = 0.0;
    w_stress = 0;
    w_duration = "00:00:00";

    ui->dateEdit_wdate->setDate(workout_date);
    ui->lineEdit_wkw->setText(QString::number(workout_date.weekNumber()));
    ui->comboBox_sport->addItems(add_settings->get_sportList());
    ui->comboBox_wcode->addItems(add_settings->get_codeList());
    w_cal_week = ui->lineEdit_wkw->text() +"_"+ QString::number(ui->dateEdit_wdate->date().year());

    ui->lineEdit_phase->setText(workSched->get_weekPhase(workout_date));
}

Dialog_add::~Dialog_add()
{
    delete ui;
}

void Dialog_add::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_add::on_pushButton_add_clicked()
{
    workSched->set_workout_date(w_date);
    workSched->set_workout_time(w_time);
    workSched->set_workout_calweek(w_cal_week);
    workSched->set_workout_sport(ui->comboBox_sport->currentText());
    workSched->set_workout_code(ui->comboBox_wcode->currentText());
    workSched->set_workout_title(ui->lineEdit_wtitle->text());
    workSched->set_workout_duration(w_duration);
    workSched->set_workout_distance(w_distance);
    workSched->set_workout_stress(w_stress);

    accept();
}

void Dialog_add::on_doubleSpinBox_dist_valueChanged(double dist)
{
    ui->label_avg_speed->setText(add_settings->get_workout_pace(dist,ui->timeEdit_duration->time(),ui->comboBox_sport->currentText(),false));
    w_distance = dist;
}

void Dialog_add::on_timeEdit_duration_timeChanged(const QTime &time)
{
    ui->label_avg_speed->setText(add_settings->get_workout_pace(ui->doubleSpinBox_dist->value(),time,ui->comboBox_sport->currentText(),false));
    w_duration = time.toString("hh:mm:ss");
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

void Dialog_add::on_dateEdit_wdate_dateChanged(const QDate &date)
{
    workout_date = date;
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
    ui->lineEdit_wkw->setText(QString::number(workout_date.weekNumber()));
    w_cal_week = ui->lineEdit_wkw->text() +"_"+ QString::number(workout_date.year());
}

void Dialog_add::on_timeEdit_time_timeChanged(const QTime &time)
{
    w_time = time.toString("hh:mm");
}

void Dialog_add::on_spinBox_stress_score_valueChanged(int stressValue)
{
    w_stress = stressValue;
}
