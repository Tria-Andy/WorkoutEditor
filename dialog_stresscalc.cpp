#include "dialog_stresscalc.h"
#include "ui_dialog_stresscalc.h"
#include <QDebug>

Dialog_stresscalc::Dialog_stresscalc(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_stresscalc)
{
    ui->setupUi(this);
    t_power = new double[4];
    t_pace = new double[4];
    combo_sport << "--" << settings::isSwim << settings::isBike << settings::isRun;
    ui->comboBox_sport->addItems(combo_sport);
    lab_power = "Threshold ----:";
    lab_workout = "Workout ----:";
    ui->lineEdit_power->setText("0");
    ui->label_threshold->setText(lab_power);
    ui->label_workout_power->setText(lab_workout);
    this->read_threshold_values();
}

Dialog_stresscalc::~Dialog_stresscalc()
{
    delete ui;
}

void Dialog_stresscalc::read_threshold_values()
{
    QVector<double> *p = settings::get_powerList();
    QString pace;

    for(int i = 1; i < 4; ++i)
    {
        t_power[i] = (*p)[i-1];
        pace = settings::get_paceList().at(i-1);
        t_pace[i] = settings::get_timesec(pace);
    }
}

void Dialog_stresscalc::estimate_stress()
{
    double stressScore,current;

    if(sport == settings::isSwim)
    {
        current = settings::get_timesec(ui->lineEdit_goal_power->text());
        current = t_pace[sport_index] / current;
        current = pow(current,3.0);
        ui->lineEdit_intensity->setText(QString::number(current));

    }
    if(sport == settings::isBike)
    {   
        current = ui->lineEdit_goal_power->text().toDouble();
        ui->lineEdit_intensity->setText(QString::number(current / t_power[sport_index]));
    }
    if(sport == settings::isRun)
    {
        current = settings::get_timesec(ui->lineEdit_goal_power->text());
        ui->lineEdit_intensity->setText(QString::number(t_pace[sport_index] / current));
    }
    stressScore = settings::estimate_stress(sport,ui->lineEdit_goal_power->text(),settings::get_timesec(ui->timeEdit_duration->time().toString("hh:mm:ss")));
    ui->lineEdit_stressScore->setText(QString::number(stressScore));
}

void Dialog_stresscalc::set_sport_threshold()
{
    if(sport == settings::isSwim || sport == settings::isRun)
    {
        ui->label_threshold->setText("Threshold Pace:");
        ui->label_workout_power->setText("Workout Pace:");
        ui->lineEdit_power->setText(settings::set_time(t_pace[sport_index]));
        ui->lineEdit_goal_power->setInputMask("00:00");
    }
    if(sport == settings::isBike)
    {
        ui->label_threshold->setText("Threshold Power:");
        ui->label_workout_power->setText("Workout Power:");
        ui->lineEdit_power->setText(QString::number(t_power[sport_index]));
        ui->lineEdit_goal_power->setInputMask("000");
    }
}

void Dialog_stresscalc::on_pushButton_close_clicked()
{
    delete t_pace;
    delete t_power;
    reject();
}

void Dialog_stresscalc::reset_calc()
{
    ui->label_threshold->setText(lab_power);
    ui->label_workout_power->setText(lab_workout);
    ui->timeEdit_duration->setTime(QTime::fromString("00:00:00"));
    ui->lineEdit_power->clear();
    ui->lineEdit_stressScore->clear();
    ui->lineEdit_goal_power->clear();
    ui->lineEdit_intensity->clear();
}

void Dialog_stresscalc::on_pushButton_calc_clicked()
{
    this->estimate_stress();
}

void Dialog_stresscalc::on_comboBox_sport_currentIndexChanged(int index)
{
    this->reset_calc();
    sport_index = index;
    sport = ui->comboBox_sport->currentText();
    this->set_sport_threshold();
}


void Dialog_stresscalc::on_pushButton_clear_clicked()
{
    ui->comboBox_sport->setCurrentIndex(0);
    this->reset_calc();

}
