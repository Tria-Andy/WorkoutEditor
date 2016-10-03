#include "dialog_addweek.h"
#include "ui_dialog_addweek.h"

Dialog_addweek::Dialog_addweek(QWidget *parent, QString sel_week, schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::Dialog_addweek)
{
    ui->setupUi(this);
    workSched = p_sched;
    ui->comboBox_phase->addItems(settings::get_phaseList());
    ui->comboBox_cycle->addItems(settings::get_cycleList());
    ui->dateEdit_selectDate->setDate(QDate().currentDate());
    empty = "0-0-00:00-0";
    swimValues = bikeValues = runValues = stgValues = altValues = sumValues = empty;

    this->fill_values(sel_week);
}

Dialog_addweek::~Dialog_addweek()
{
    delete ui;
}

void Dialog_addweek::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_addweek::fill_values(QString selWeek)
{
    QStringList weekInfo = selWeek.split("-");
    QList<QStandardItem*> content = workSched->week_content->findItems(weekInfo.at(1),Qt::MatchExactly,1);
    QList<QStandardItem*> meta = workSched->week_meta->findItems(weekInfo.at(1),Qt::MatchExactly,1);
    QModelIndex index;
    QString value,work,dura,dist,stress;
    QStringList values;

    if(!meta.isEmpty())
    {
        openID = weekInfo.at(1);
        index = workSched->week_meta->indexFromItem(meta.at(0));
        ui->dateEdit_selectDate->setDate(QDate::fromString(workSched->week_meta->item(index.row(),3)->text(),"dd.MM.yyyy"));
        ui->lineEdit_week->setText(QString::number(ui->dateEdit_selectDate->date().weekNumber()));
        value = weekInfo.at(3);
        ui->comboBox_phase->setCurrentText(value.split("_").first());
        ui->comboBox_cycle->setCurrentText(value.split("_").last());
        update = true;
    }
    else
    {
        update = false;
    }

    if(!content.isEmpty())
    {
        index = workSched->week_content->indexFromItem(content.at(0));
        for(int i = 2; i < 7; ++i)
        {
            value = workSched->week_content->item(index.row(),i)->text();
            values = value.split("-");
            work = values.at(0);
            dist = values.at(1);
            dura = values.at(2);
            stress = values.at(3);

            if(i == 2)
            {
                ui->spinBox_swim_work->setValue(work.toInt());
                ui->timeEdit_swim_dur->setTime(QTime::fromString(dura,"hh:mm"));
                ui->doubleSpinBox_swim_dist->setValue(dist.toDouble());
                ui->spinBox_swim_stress->setValue(stress.toInt());
            }
            if(i == 3)
            {
                ui->spinBox_bike_work->setValue(work.toInt());
                ui->timeEdit_bike_dur->setTime(QTime::fromString(dura,"hh:mm"));
                ui->doubleSpinBox_bike_dist->setValue(dist.toDouble());
                ui->spinBox_bike_stress->setValue(stress.toInt());
            }
            if(i == 4)
            {
                ui->spinBox_run_work->setValue(work.toInt());
                ui->timeEdit_run_dur->setTime(QTime::fromString(dura,"hh:mm"));
                ui->doubleSpinBox_run_dist->setValue(dist.toDouble());
                ui->spinBox_run_stress->setValue(stress.toInt());
            }
            if(i == 5)
            {
                ui->spinBox_stg_work->setValue(work.toInt());
                ui->timeEdit_stg_dur->setTime(QTime::fromString(dura,"hh:mm"));
                ui->spinBox_stg_stress->setValue(stress.toInt());
            }
            if(i == 6)
            {
                ui->spinBox_alt_work->setValue(work.toInt());
                ui->timeEdit_alt_dur->setTime(QTime::fromString(dura,"hh:mm"));
                ui->doubleSpinBox_alt_dist->setValue(dist.toDouble());
                ui->spinBox_alt_stress->setValue(stress.toInt());
            }
        }
    }

}

void Dialog_addweek::store_meta()
{
    weekMeta = QStringList();
    weekID = ui->lineEdit_week->text()+"_"+selYear;
    int currID = workSched->week_meta->rowCount()+1;
    if(update)
    {
        weekMeta << weekID
                 << ui->comboBox_phase->currentText()+"_"+ui->comboBox_cycle->currentText()
                 << ui->dateEdit_selectDate->date().toString("dd.MM.yyyy");

        weekContent << weekID
                    << swimValues
                    << bikeValues
                    << runValues
                    << stgValues
                    << altValues
                    << sumValues;
    }
    else
    {
        weekMeta << QString::number(currID)
                 << weekID
                 << ui->comboBox_phase->currentText()+"_"+ui->comboBox_cycle->currentText()
                 << ui->dateEdit_selectDate->date().toString("dd.MM.yyyy");

        weekContent << QString::number(currID)
                    << weekID
                    << swimValues
                    << bikeValues
                    << runValues
                    << stgValues
                    << altValues
                    << sumValues;
    }

}

void Dialog_addweek::store_values(int pos)
{
    weekContent = QStringList();

    if(pos == 1)
    {
        swimValues = QString::number(ui->spinBox_swim_work->value()) +"-";
        swimValues = swimValues + QString::number(ui->doubleSpinBox_swim_dist->value()) +"-";
        swimValues = swimValues + ui->timeEdit_swim_dur->time().toString("hh:mm") +"-";
        swimValues = swimValues + QString::number(ui->spinBox_swim_stress->value());
    }
    if(pos == 2)
    {
        bikeValues = QString::number(ui->spinBox_bike_work->value()) +"-";
        bikeValues = bikeValues + QString::number(ui->doubleSpinBox_bike_dist->value()) +"-";
        bikeValues = bikeValues + ui->timeEdit_bike_dur->time().toString("hh:mm") +"-";
        bikeValues = bikeValues + QString::number(ui->spinBox_bike_stress->value());
    }
    if(pos == 3)
    {
        runValues = QString::number(ui->spinBox_run_work->value()) +"-";
        runValues = runValues + QString::number(ui->doubleSpinBox_run_dist->value()) +"-";
        runValues = runValues + ui->timeEdit_run_dur->time().toString("hh:mm") +"-";
        runValues = runValues + QString::number(ui->spinBox_run_stress->value());
    }
    if(pos == 4)
    {
        stgValues = QString::number(ui->spinBox_stg_work->value()) +"-";
        stgValues = stgValues + "0" +"-";
        stgValues = stgValues + ui->timeEdit_stg_dur->time().toString("hh:mm") +"-";
        stgValues = stgValues + QString::number(ui->spinBox_stg_stress->value());
    }
    if(pos == 5)
    {
        altValues = QString::number(ui->spinBox_alt_work->value()) +"-";
        altValues = altValues + QString::number(ui->doubleSpinBox_alt_dist->value()) +"-";
        altValues = altValues + ui->timeEdit_alt_dur->time().toString("hh:mm") +"-";
        altValues = altValues + QString::number(ui->spinBox_alt_stress->value());
    }

    sumValues = ui->lineEdit_sum_work->text() +"-";
    sumValues = sumValues + ui->lineEdit_sum_distance->text() +"-";
    sumValues = sumValues + ui->lineEdit_sum_duration->text() +"-";
    sumValues = sumValues + ui->lineEdit_sum_stress->text();

}

void Dialog_addweek::sum_workouts()
{
    int work;
    work = ui->spinBox_swim_work->value() + ui->spinBox_bike_work->value() + ui->spinBox_run_work->value() + ui->spinBox_stg_work->value() + ui->spinBox_alt_work->value();
    ui->lineEdit_sum_work->setText(QString::number(work));
}

void Dialog_addweek::sum_distance()
{
    double dist;
    dist = ui->doubleSpinBox_swim_dist->value() + ui->doubleSpinBox_bike_dist->value() + ui->doubleSpinBox_run_dist->value() + ui->doubleSpinBox_alt_dist->value();
    ui->lineEdit_sum_distance->setText(QString::number(dist));
}

void Dialog_addweek::sum_duration()
{
    int duration;
    duration = settings::get_timesec(ui->timeEdit_swim_dur->time().toString("hh:mm"));
    duration = duration + settings::get_timesec(ui->timeEdit_bike_dur->time().toString("hh:mm"));
    duration = duration + settings::get_timesec(ui->timeEdit_run_dur->time().toString("hh:mm"));
    duration = duration + settings::get_timesec(ui->timeEdit_stg_dur->time().toString("hh:mm"));
    duration = duration + settings::get_timesec(ui->timeEdit_alt_dur->time().toString("hh:mm"));
    ui->lineEdit_sum_duration->setText(settings::set_time(duration));
}

void Dialog_addweek::sum_stress()
{
    int stress;
    stress = ui->spinBox_swim_stress->value() + ui->spinBox_bike_stress->value() + ui->spinBox_run_stress->value() + ui->spinBox_stg_stress->value() + ui->spinBox_alt_stress->value();
    ui->lineEdit_sum_stress->setText(QString::number(stress));
}

void Dialog_addweek::on_dateEdit_selectDate_dateChanged(const QDate &date)
{
    ui->lineEdit_week->setText(QString::number(date.weekNumber()));
    selYear = QString::number(date.year());
}

void Dialog_addweek::on_pushButton_ok_clicked()
{
    this->store_meta();
    if(update)
    {
        QList<QStandardItem*> openItem = workSched->week_meta->findItems(openID,Qt::MatchExactly,1);

        if(!openItem.isEmpty())
        {
            QModelIndex index = workSched->week_meta->indexFromItem(openItem.at(0));
            workSched->week_meta->setData(workSched->week_meta->index(index.row(),1,QModelIndex()),weekMeta.at(0));
            workSched->week_meta->setData(workSched->week_meta->index(index.row(),2,QModelIndex()),weekMeta.at(1));
            workSched->week_meta->setData(workSched->week_meta->index(index.row(),3,QModelIndex()),weekMeta.at(2));
        }
        openItem.clear();

        openItem = workSched->week_content->findItems(openID,Qt::MatchExactly,1);

        if(!openItem.isEmpty())
        {
            QModelIndex index = workSched->week_content->indexFromItem(openItem.at(0));
            for(int i = 1; i <= weekContent.count(); ++i)
            {
                workSched->week_content->setData(workSched->week_content->index(index.row(),i,QModelIndex()),weekContent.at(i-1));
            }
        }
    }
    else
    {
        int rowcount;
        rowcount = workSched->week_meta->rowCount();
        workSched->week_meta->insertRow(rowcount,QModelIndex());

        for(int i = 0; i < weekMeta.count(); ++i)
        {
            workSched->week_meta->setData(workSched->week_meta->index(rowcount,i,QModelIndex()),weekMeta.at(i));
        }

        rowcount = workSched->week_content->rowCount();
        workSched->week_content->insertRow(rowcount,QModelIndex());

        for(int i = 0; i < weekContent.count(); ++i)
        {
            workSched->week_content->setData(workSched->week_content->index(rowcount,i,QModelIndex()),weekContent.at(i));
        }
    }
    accept();
}

void Dialog_addweek::calc_percent()
{
    double percent;
    QString sDuration = ui->lineEdit_sum_duration->text();
    percent = (static_cast<double>(settings::get_timesec(ui->timeEdit_swim_dur->time().toString("hh:mm"))) / static_cast<double>(settings::get_timesec(sDuration)))*100.0;
    ui->lineEdit_swim_perc->setText(QString::number(static_cast<int>((percent *100+.5)/100.0)));
    percent = (static_cast<double>(settings::get_timesec(ui->timeEdit_bike_dur->time().toString("hh:mm"))) / static_cast<double>(settings::get_timesec(sDuration)))*100.0;
    ui->lineEdit_bike_perc->setText(QString::number(static_cast<int>((percent *100+.5)/100.0)));
    percent = (static_cast<double>(settings::get_timesec(ui->timeEdit_run_dur->time().toString("hh:mm"))) / static_cast<double>(settings::get_timesec(sDuration)))*100.0;
    ui->lineEdit_run_perc->setText(QString::number(static_cast<int>((percent *100+.5)/100.0)));
    percent = (static_cast<double>(settings::get_timesec(ui->timeEdit_stg_dur->time().toString("hh:mm"))) / static_cast<double>(settings::get_timesec(sDuration)))*100.0;
    ui->lineEdit_stg_perc->setText(QString::number(static_cast<int>((percent *100+.5)/100.0)));
    percent = (static_cast<double>(settings::get_timesec(ui->timeEdit_alt_dur->time().toString("hh:mm"))) / static_cast<double>(settings::get_timesec(sDuration)))*100.0;
    ui->lineEdit_alt_perc->setText(QString::number(static_cast<int>((percent *100+.5)/100.0)));

}

void Dialog_addweek::on_spinBox_swim_work_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_workouts();
    this->store_values(1);
}

void Dialog_addweek::on_spinBox_bike_work_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_workouts();
    this->store_values(2);
}

void Dialog_addweek::on_spinBox_run_work_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_workouts();
    this->store_values(3);
}

void Dialog_addweek::on_spinBox_stg_work_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_workouts();
    this->store_values(4);
}

void Dialog_addweek::on_spinBox_alt_work_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_workouts();
    this->store_values(5);
}

void Dialog_addweek::on_timeEdit_swim_dur_timeChanged(const QTime &time)
{
    this->sum_duration();
    this->store_values(1);
    this->calc_percent();
    ui->lineEdit_swim_pace->setText(settings::get_workout_pace(ui->doubleSpinBox_swim_dist->value(),time,settings::isSwim,false));
    ui->spinBox_swim_stress->setValue(round(settings::get_timesec(time.toString("hh:mm"))*settings::get_factorList()->at(0)));
}

void Dialog_addweek::on_timeEdit_bike_dur_timeChanged(const QTime &time)
{
    this->sum_duration();
    this->store_values(2);
    this->calc_percent();
    ui->lineEdit_bike_pace->setText(settings::get_workout_pace(ui->doubleSpinBox_bike_dist->value(),time,settings::isBike,false));
    ui->spinBox_bike_stress->setValue(round(settings::get_timesec(time.toString("hh:mm"))*settings::get_factorList()->at(1)));
}

void Dialog_addweek::on_timeEdit_run_dur_timeChanged(const QTime &time)
{
    this->sum_duration();
    this->store_values(3);
    this->calc_percent();
    ui->lineEdit_run_pace->setText(settings::get_workout_pace(ui->doubleSpinBox_run_dist->value(),time,settings::isRun,false));
    ui->spinBox_run_stress->setValue(round(settings::get_timesec(time.toString("hh:mm"))*settings::get_factorList()->at(2)));
}

void Dialog_addweek::on_timeEdit_stg_dur_timeChanged(const QTime &time)
{
    this->sum_duration();
    this->store_values(4);
    this->calc_percent();
    ui->spinBox_stg_stress->setValue(round(settings::get_timesec(time.toString("hh:mm"))));
}

void Dialog_addweek::on_timeEdit_alt_dur_timeChanged(const QTime &time)
{
    Q_UNUSED(time)
    this->sum_duration();
    this->store_values(5);
    this->calc_percent();
}

void Dialog_addweek::on_doubleSpinBox_swim_dist_valueChanged(double dist)
{
    this->sum_distance();
    this->store_values(1);
    ui->lineEdit_swim_pace->setText(settings::get_workout_pace(dist,ui->timeEdit_swim_dur->time(),settings::isSwim,false));
}

void Dialog_addweek::on_doubleSpinBox_bike_dist_valueChanged(double dist)
{
    this->sum_distance();
    this->store_values(2);
    ui->lineEdit_bike_pace->setText(settings::get_workout_pace(dist,ui->timeEdit_bike_dur->time(),settings::isBike,false));
}

void Dialog_addweek::on_doubleSpinBox_run_dist_valueChanged(double dist)
{
    this->sum_distance();
    this->store_values(3);
    ui->lineEdit_run_pace->setText(settings::get_workout_pace(dist,ui->timeEdit_run_dur->time(),settings::isRun,false));
}

void Dialog_addweek::on_doubleSpinBox_alt_dist_valueChanged(double arg1)
{
    Q_UNUSED(arg1)
    this->sum_distance();
    this->store_values(5);
}

void Dialog_addweek::on_spinBox_swim_stress_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_stress();
    this->store_values(1);
}

void Dialog_addweek::on_spinBox_bike_stress_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_stress();
    this->store_values(2);
}

void Dialog_addweek::on_spinBox_run_stress_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_stress();
    this->store_values(3);
}

void Dialog_addweek::on_spinBox_stg_stress_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_stress();
    this->store_values(4);
}

void Dialog_addweek::on_spinBox_alt_stress_valueChanged(int arg1)
{
    Q_UNUSED(arg1)
    this->sum_stress();
    this->store_values(5);
}



