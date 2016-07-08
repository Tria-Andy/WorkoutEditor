#include "dialog_export.h"
#include "ui_dialog_export.h"
#include <QDebug>
#include <math.h>
#include <QDir>

Dialog_export::Dialog_export(QWidget *parent,QStandardItemModel *w_model,settings *p_settings) :
    QDialog(parent),
    ui(new Ui::Dialog_export)
{
    ui->setupUi(this);
    QModelIndex index;
    workout_model = w_model;
    export_settings = p_settings;
    current_date = new QDate();
    ui->dateEdit_export->setDate(current_date->currentDate());
    workout_time = workout_model->findItems(current_date->currentDate().toString("dd.MM.yyyy"),Qt::MatchExactly,1);
    workout_model->sort(0);
    QSet<QString> uniqueWeek;
    for(int i = 0; i < workout_model->rowCount();++i)
    {
        index = workout_model->index(i,1,QModelIndex());
        QString week = workout_model->item(index.row(),0)->text();
        if(!uniqueWeek.contains(week))
        {
            ui->comboBox_week_export->addItem(week);
            uniqueWeek.insert(week);
        }
    }
    export_mode = 0;
    ui->comboBox_week_export->setEnabled(false);
    this->set_comboBox_time();
    this->set_infolabel(workout_time.count());

}

Dialog_export::~Dialog_export()
{
    delete current_date;
    delete ui;
}

void Dialog_export::set_comboBox_time()
{
    ui->comboBox_time_export->clear();
    ui->comboBox_time_export->addItem("all");

    for(int i = 0; i < workout_time.count();++i)
    {
        ui->comboBox_time_export->addItem(workout_model->item(workout_model->indexFromItem(workout_time.at(i)).row(),2)->text());
    }
}

void Dialog_export::set_filecontent(QModelIndex index)
{
    QDate workoutDate;
    QTime workoutTime;
    QDateTime workoutDateTime;
    QString tempDate,tempTime,sport,stressType;

    tempDate = workout_model->item(index.row(),1)->text();
    workoutDate = QDate::fromString(tempDate,"dd.MM.yyyy");

    tempTime = workout_model->item(index.row(),2)->text();
    workoutTime = QTime::fromString(tempTime,"hh:mm");

    fileName = workoutDate.toString("yyyy_MM_dd_") + workoutTime.toString("hh_mm_ss") +".json";
    workoutDateTime = QDateTime::fromString(tempDate+"T"+tempTime+":00","dd.MM.yyyyThh:mm:ss").toUTC();

    sport = workout_model->item(index.row(),3)->text();
    if(sport == export_settings->isSwim) stressType = "swimscore";
    if(sport == export_settings->isBike) stressType = "skiba_bike_score";
    if(sport == export_settings->isRun) stressType = "govss";
    if(sport == export_settings->isAlt || sport == export_settings->isStrength) stressType = "triscore";

    fileContent = "{ \n \"RIDE\":{\n";
    fileContent.append("\"STARTTIME\":\"" + workoutDateTime.toString("yyyy\\/MM\\/dd hh:mm:ss UTC ") +"\",\n");
    fileContent.append("\"RECINTSECS\":0,\n");
    fileContent.append("\"DEVICETYPE\":\"Manual \",\n");
    fileContent.append("\"IDENTIFIER\":\" \",\n");
    fileContent.append("\"OVERRIDES\":[\n");
    fileContent.append("{ \"time_riding\":{ \"value\":\""+QString::number(export_settings->get_timesec(workout_model->item(index.row(),6)->text()))+"\" }},\n");
    fileContent.append("{ \"workout_time\":{ \"value\":\""+QString::number(export_settings->get_timesec(workout_model->item(index.row(),6)->text()))+"\" }},\n");
    fileContent.append("{ \""+stressType+"\":{ \"value\":\""+workout_model->item(index.row(),8)->text()+"\" }}\n ],\n");
    fileContent.append("\"TAGS\":{\n");
    fileContent.append("\"Sport\":\""+workout_model->item(index.row(),3)->text()+" \",\n");
    fileContent.append("\"Workout Code\":\""+workout_model->item(index.row(),4)->text()+" \",\n");
    fileContent.append("\"Workout Title\":\""+workout_model->item(index.row(),5)->text()+" \"\n } \n } \n }");

    this->write_file(fileName,fileContent);
}

void Dialog_export::workout_export()
{
    fileName = QString();
    fileContent = QString();
    QList<QStandardItem*> list;

    if(export_mode == 2)
    {
        QString weeknumber = ui->comboBox_week_export->currentText();
        list = workout_model->findItems(weeknumber,Qt::MatchExactly,0);

        for(int i = 0; i < list.count(); ++i)
        {
            this->set_filecontent(workout_model->indexFromItem(list.at(i)));
            ui->progressBar->setValue((100/list.count())*i);
        }
        ui->progressBar->setValue(100);

    }
    if(export_mode == 0 || export_mode == 1)
    {
        QString workoutdate = ui->dateEdit_export->date().toString("dd.MM.yyyy");
        list = workout_model->findItems(workoutdate,Qt::MatchExactly,1);

        for(int i = 0; i < list.count(); ++i)
        {
            if(export_mode == 0)
            {
                this->set_filecontent(workout_model->indexFromItem(list.at(i)));
                ui->progressBar->setValue((100/list.count())*i);
            }
            if(export_mode == 1)
            {
                QString time_value = workout_model->data(workout_model->index(workout_model->indexFromItem(list.at(i)).row(),2,QModelIndex()),Qt::DisplayRole).toString();
                if(time_value == ui->comboBox_time_export->currentText())
                {
                    this->set_filecontent(workout_model->indexFromItem(list.at(i)));
                    ui->progressBar->setValue((100/list.count())*i);
                }
            }
        }
        ui->progressBar->setValue(100);
    }
}

void Dialog_export::write_file(QString filename, QString filecontent)
{
    QFile file(export_settings->get_gcPath() + QDir::separator() + filename);
    if(!file.open(QFile::WriteOnly | QFile::Text))
    {
        qDebug() << "File not open!";
        return;
    }

    QTextStream stream(&file);
    stream << filecontent;
    file.flush();
    file.close();
}

void Dialog_export::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_export::on_pushButton_export_clicked()
{
    this->workout_export();
}

void Dialog_export::set_exportselection(bool b_day,bool b_week)
{
    ui->comboBox_week_export->setEnabled(b_week);
    ui->comboBox_time_export->setEnabled(b_day);
    ui->dateEdit_export->setEnabled(b_day);
}

void Dialog_export::get_exportinfo(QString v_week,QString v_date,bool week)
{
    QList<QStandardItem*> list;

    if(week)
    {
        list = workout_model->findItems(v_week,Qt::MatchExactly,0);
    }
    else
    {
        list = workout_model->findItems(v_date,Qt::MatchExactly,1);
    }

    if(export_mode == 0 || export_mode == 2)
    {
        this->set_infolabel(list.count());
    }
    else
    {
        this->set_infolabel(1);
    }
}

void Dialog_export::on_radioButton_day_clicked()
{
    export_mode = 0;
    ui->progressBar->setValue(0);
    this->set_exportselection(true,false);
    this->get_exportinfo("NULL",ui->dateEdit_export->date().toString("dd.MM.yyyy"),false);
}

void Dialog_export::on_radioButton_week_clicked()
{
    export_mode = 2;
    ui->progressBar->setValue(0);
    this->set_exportselection(false,true);
    this->get_exportinfo(ui->comboBox_week_export->currentText(),"NULL",true);
}

void Dialog_export::set_infolabel(int value)
{
    ui->label_exportinfo->setText(QString::number(value) + " Workout(s) selected for Export");
}

void Dialog_export::on_comboBox_week_export_currentIndexChanged(const QString &weekvalue)
{
    ui->progressBar->setValue(0);
    this->get_exportinfo(weekvalue,"NULL",true);
}

void Dialog_export::on_dateEdit_export_dateChanged(const QDate &date)
{
    ui->progressBar->setValue(0);
    workout_time = workout_model->findItems(date.toString("dd.MM.yyyy"),Qt::MatchExactly,1);
    this->set_comboBox_time();
    this->get_exportinfo("NULL",date.toString("dd.MM.yyyy"),false);
}

void Dialog_export::on_comboBox_time_export_currentIndexChanged(const QString &time)
{
    ui->progressBar->setValue(0);
    if(time == "all")
    {
        export_mode = 0;
    }
    else
    {
        export_mode = 1;
    }
    this->get_exportinfo("NULL",ui->dateEdit_export->date().toString("dd.MM.yyyy"),false);
}
