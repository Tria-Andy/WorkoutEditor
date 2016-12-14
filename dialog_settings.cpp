/*
 * Copyright (c) 2016 Andreas Hunner (andy-atech@gmx.net)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include "dialog_settings.h"
#include "ui_dialog_settings.h"
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>

Dialog_settings::Dialog_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_settings)
{
    ui->setupUi(this);
    sportList << settings::isSwim << settings::isBike << settings::isRun;
    useColor = false;
    model_header << "Level" << "Low %" << "Low" << "High %" << "High";
    level_model = new QStandardItemModel();
    hf_model = new QStandardItemModel();
    ui->lineEdit_gcpath->setText(settings::get_gcInfo("gcpath"));
    ui->lineEdit_gcpath->setEnabled(false);
    ui->lineEdit_athlete->setText(settings::get_gcInfo("athlete"));
    ui->lineEdit_yob->setText(settings::get_gcInfo("yob"));
    ui->lineEdit_activity->setText(settings::get_gcInfo("folder"));
    ui->lineEdit_schedule->setText(settings::get_gcInfo("schedule"));
    ui->lineEdit_standard->setText(settings::get_gcInfo("workouts"));
    ui->lineEdit_configfile->setText(settings::get_gcInfo("valuefile"));
    ui->lineEdit_hfThres->setText(QString::number(settings::get_thresValue("hfthres")));
    ui->lineEdit_hfmax->setText(QString::number(settings::get_thresValue("hfmax")));
    ui->comboBox_selInfo->addItems(settings::get_keyList());
    ui->lineEdit_saison->setText(settings::get_saisonInfo("saison"));
    ui->lineEdit_saisonWeeks->setText(settings::get_saisonInfo("weeks"));
    ui->dateEdit_saisonStart->setDate(QDate::fromString(settings::get_saisonInfo("startDate"),"dd.MM.yyyy"));
    ui->lineEdit_startWeek->setText(settings::get_saisonInfo("startkw"));
    ui->dateEdit_saisonEnd->setDate(QDate::fromString(settings::get_saisonInfo("endDate"),"dd.MM.yyyy"));
    ui->comboBox_thresSport->addItems(sportList);
    ui->pushButton_save->setEnabled(false);
    ui->pushButton_color->setEnabled(false);
    ui->pushButton_add->setEnabled(false);
    ui->pushButton_edit->setEnabled(false);
    ui->pushButton_delete->setEnabled(false);
    this->checkSetup();
}

Dialog_settings::~Dialog_settings()
{
    delete ui;
}

void Dialog_settings::on_pushButton_cancel_clicked()
{
    delete level_model;
    delete hf_model;
    reject();
}

void Dialog_settings::checkSetup()
{
    if(ui->lineEdit_athlete->text().isEmpty()) ui->pushButton_save->setEnabled(true);
    this->set_hfmodel();
}

void Dialog_settings::writeChangedValues()
{
    QString selection = ui->comboBox_selInfo->currentText();
    QStringList updateList;
    QString sport = ui->comboBox_thresSport->currentText();
<<<<<<< HEAD
=======

>>>>>>> refs/remotes/origin/develop
    for(int i = 0; i < ui->listWidget_selection->count();++i)
    {
        updateList << ui->listWidget_selection->item(i)->data(Qt::DisplayRole).toString();
    }

    if(sport == settings::isSwim)
    {
        settings::set_thresValue("swimpower",ui->lineEdit_thresPower->text().toDouble());
        settings::set_thresValue("swimpace",settings::get_timesec(ui->lineEdit_thresPace->text()));
        settings::set_thresValue("swimfactor",ui->doubleSpinBox_factor->value());
        this->writeRangeValues(sport);
    }
    if(sport == settings::isBike)
    {
        settings::set_thresValue("bikepower",ui->lineEdit_thresPower->text().toDouble());
        settings::set_thresValue("bikepace",settings::get_timesec(ui->lineEdit_thresPace->text()));
        settings::set_thresValue("bikefactor",ui->doubleSpinBox_factor->value());
        this->writeRangeValues(sport);
    }
    if(sport == settings::isRun)
    {
        settings::set_thresValue("runpower",ui->lineEdit_thresPower->text().toDouble());
        settings::set_thresValue("runpace",settings::get_timesec(ui->lineEdit_thresPace->text()));
        settings::set_thresValue("runfactor",ui->doubleSpinBox_factor->value());
        this->writeRangeValues(sport);
    }

    settings::set_thresValue("hfthres",ui->lineEdit_hfThres->text().toDouble());
    settings::set_thresValue("hfmax",ui->lineEdit_hfmax->text().toDouble());
    this->writeRangeValues("HF");

    settings::set_gcInfo("athlete",ui->lineEdit_athlete->text());
    settings::set_gcInfo("yob",ui->lineEdit_yob->text());
    settings::set_gcInfo("folder",ui->lineEdit_activity->text());
    settings::set_gcInfo("schedule",ui->lineEdit_schedule->text());
    settings::set_gcInfo("workouts",ui->lineEdit_standard->text());
    settings::set_gcInfo("valuefile",ui->lineEdit_configfile->text());

    settings::set_saisonInfos("saison",ui->lineEdit_saison->text());
    settings::set_saisonInfos("startDate",ui->dateEdit_saisonStart->date().toString("dd.MM.yyyy"));
    settings::set_saisonInfos("startkw",ui->lineEdit_startWeek->text());
    settings::set_saisonInfos("endDate",ui->dateEdit_saisonEnd->date().toString("dd.MM.yyyy"));
    settings::set_saisonInfos("weeks",ui->lineEdit_saisonWeeks->text());

    settings::writeSettings(selection,updateList);

    thresPower = ui->lineEdit_thresPower->text().toDouble();
    thresPace = settings::get_timesec(ui->lineEdit_thresPace->text());
    sportFactor = ui->doubleSpinBox_factor->value();

    this->set_thresholdView(ui->comboBox_thresSport->currentText());
    this->set_hfmodel();
}

void Dialog_settings::writeRangeValues(QString sport)
{
    QStringList levels = settings::get_levelList();
    QString min,max;
    QStandardItemModel *model;
    if(sport == "HF")
    {
        model = hf_model;
    }
    else
    {
        model = level_model;
    }

    for(int i = 0; i < levels.count();++i)
    {
        min = model->data(model->index(i,1,QModelIndex())).toString();
        max = model->data(model->index(i,3,QModelIndex())).toString();
        settings::set_rangeValue(sport,levels.at(i),min+"-"+max);
    }
}

void Dialog_settings::on_comboBox_selInfo_currentTextChanged(const QString &value)
{
    this->set_listEntries(value);
    ui->lineEdit_addedit->clear();
    ui->pushButton_add->setEnabled(false);
    ui->pushButton_delete->setEnabled(false);
    ui->pushButton_edit->setEnabled(false);
}

void Dialog_settings::set_listEntries(QString selection)
{
    ui->listWidget_selection->clear();

    if(selection == settings::get_keyList().at(0))
    {
        ui->listWidget_selection->addItems(settings::get_sportList());
        useColor = true;
    }
    if(selection == settings::get_keyList().at(1))
    {
        ui->listWidget_selection->addItems(settings::get_levelList());
        useColor = true;
    }
    if(selection == settings::get_keyList().at(2))
    {
        ui->listWidget_selection->addItems(settings::get_phaseList());
        useColor = true;
    }
    if(selection == settings::get_keyList().at(3))
    {
        ui->listWidget_selection->addItems(settings::get_cycleList());
        useColor = false;
    }
    if(selection == settings::get_keyList().at(4))
    {
        ui->listWidget_selection->addItems(settings::get_codeList());
        useColor = false;
    }
    if(selection == settings::get_keyList().at(5))
    {
        ui->listWidget_selection->addItems(settings::get_jsoninfos());
        useColor = false;
    }
    if(selection == settings::get_keyList().at(6))
    {
        ui->listWidget_selection->addItems(settings::get_intPlanerList());
        useColor = false;
    }

    QColor color;
    color.setRgb(255,255,255,0);
    this->set_color(color,false,"");
}

void Dialog_settings::set_thresholdView(QString sport)
{
    if(sport == settings::isSwim)
    {
        ui->lineEdit_speed->setText(settings::get_speed(QTime::fromString(ui->lineEdit_thresPace->text(),"mm:ss"),100,ui->comboBox_thresSport->currentText(),true));
        this->set_thresholdModel(sport);
    }
    if(sport == settings::isBike)
    {
        ui->lineEdit_speed->setText(settings::get_speed(QTime::fromString(ui->lineEdit_thresPace->text(),"mm:ss"),1000,ui->comboBox_thresSport->currentText(),true));
        this->set_thresholdModel(sport);
    }
    if(sport == settings::isRun)
    {
        ui->lineEdit_speed->setText(settings::get_speed(QTime::fromString(ui->lineEdit_thresPace->text(),"mm:ss"),1000,ui->comboBox_thresSport->currentText(),true));
        this->set_thresholdModel(sport);
    }
}

void Dialog_settings::set_hfmodel()
{
    QStringList levels = settings::get_levelList();
    if(hf_model->rowCount() > 0) hf_model->clear();
    QString range,zone_low,zone_high;

    hf_model->setHorizontalHeaderLabels(model_header);
    ui->tableView_hf->setModel(hf_model);
    ui->tableView_hf->setItemDelegate(&level_del);
    ui->tableView_hf->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_hf->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_hf->verticalHeader()->hide();

    for(int i = 0; i < levels.count(); ++i)
    {
        range = settings::get_rangeValue("HF",levels.at(i));
        zone_low = range.split("-").first();
        zone_high = range.split("-").last();
        hf_model->insertRows(i,1,QModelIndex());
        hf_model->setData(hf_model->index(i,0,QModelIndex()),levels.at(i));
        hf_model->setData(hf_model->index(i,1,QModelIndex()),zone_low);
        hf_model->setData(hf_model->index(i,2,QModelIndex()),settings::get_hfvalue(zone_low));
        hf_model->setData(hf_model->index(i,3,QModelIndex()),zone_high);
        hf_model->setData(hf_model->index(i,4,QModelIndex()),settings::get_hfvalue(zone_high));
    }
}

void Dialog_settings::set_thresholdModel(QString sport)
{
    QStringList levels = settings::get_levelList();
<<<<<<< HEAD

=======
>>>>>>> refs/remotes/origin/develop
    if(level_model->rowCount() > 0) level_model->clear();
    level_model->setHorizontalHeaderLabels(model_header);
    ui->tableView_level->setModel(level_model);
    ui->tableView_level->setItemDelegate(&level_del);
    ui->tableView_level->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_level->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_level->verticalHeader()->hide();

    double percLow = 0,percHigh = 0;
    QString range;

    for(int i = 0; i < levels.count(); ++i)
    {
        range = settings::get_rangeValue(sport,levels.at(i));
        level_model->insertRows(i,1,QModelIndex());
        level_model->setData(level_model->index(i,0,QModelIndex()),levels.at(i));
        level_model->setData(level_model->index(i,1,QModelIndex()),range.split("-").first());
        level_model->setData(level_model->index(i,3,QModelIndex()),range.split("-").last());

        percLow = level_model->data(level_model->index(i,1,QModelIndex())).toDouble()/100;
        percHigh = level_model->data(level_model->index(i,3,QModelIndex())).toDouble()/100;

        if(ui->comboBox_thresSport->currentText() == settings::isBike)
        {
            level_model->setData(level_model->index(i,2,QModelIndex()),round(thresPower * percLow));
            level_model->setData(level_model->index(i,4,QModelIndex()),round(thresPower * percHigh));
        }
        else
        {
            level_model->setData(level_model->index(i,2,QModelIndex()),settings::set_time(static_cast<int>(round(thresPace / percLow))));
            level_model->setData(level_model->index(i,4,QModelIndex()),settings::set_time(static_cast<int>(round(thresPace / percHigh))));
        }
    }
}

void Dialog_settings::set_color(QColor color,bool write,QString key)
{
    QPalette palette = ui->pushButton_color->palette();
    palette.setColor(ui->pushButton_color->backgroundRole(),color);
    ui->pushButton_color->setAutoFillBackground(true);
    ui->pushButton_color->setPalette(palette);

    if(write)
    {
        settings::set_itemColor(key,color);
    }
}

void Dialog_settings::on_listWidget_selection_itemDoubleClicked(QListWidgetItem *item)
{
    QString listValue = item->data(Qt::DisplayRole).toString();
    QColor color;
    ui->lineEdit_addedit->setText(listValue);
    ui->pushButton_edit->setEnabled(true);
    ui->pushButton_delete->setEnabled(true);

    color = settings::get_itemColor(listValue);

    if(useColor)
    {
        this->set_color(color,false,listValue);
        ui->pushButton_color->setEnabled(true);
    }
    else
    {
        color.setRgb(255,255,255,0);
        this->set_color(color,false,listValue);
        ui->pushButton_color->setEnabled(false);
    }
}

void Dialog_settings::on_pushButton_up_clicked()
{
    int currentindex = ui->listWidget_selection->currentRow();
    QListWidgetItem *currentItem = ui->listWidget_selection->takeItem(currentindex);
    ui->listWidget_selection->insertItem(currentindex-1,currentItem);
    ui->listWidget_selection->setCurrentRow(currentindex-1);
}

void Dialog_settings::on_pushButton_down_clicked()
{
    int currentindex = ui->listWidget_selection->currentRow();
    QListWidgetItem *currentItem = ui->listWidget_selection->takeItem(currentindex);
    ui->listWidget_selection->insertItem(currentindex+1,currentItem);
    ui->listWidget_selection->setCurrentRow(currentindex+1);
}

void Dialog_settings::on_pushButton_add_clicked()
{
    int index = ui->listWidget_selection->count();
    ui->listWidget_selection->insertItem(index,ui->lineEdit_addedit->text());
    this->enableSavebutton();
}

void Dialog_settings::on_pushButton_delete_clicked()
{
    QListWidgetItem *item = ui->listWidget_selection->takeItem(ui->listWidget_selection->currentRow());
    ui->lineEdit_addedit->clear();
    this->set_color(QColor(255,255,255,0),false,"");
    delete item;
    this->enableSavebutton();
    ui->pushButton_delete->setEnabled(false);
    ui->pushButton_add->setEnabled(true);
}

void Dialog_settings::on_pushButton_edit_clicked()
{
    ui->listWidget_selection->item(ui->listWidget_selection->currentRow())->setData(Qt::EditRole,ui->lineEdit_addedit->text());
    this->enableSavebutton();
}

void Dialog_settings::on_comboBox_thresSport_currentTextChanged(const QString &value)
{
    QPalette gback,wback;
    gback.setColor(QPalette::Base,Qt::green);
    wback.setColor(QPalette::Base,Qt::white);;

    if(value == settings::isSwim)
    {
        thresPower = settings::get_thresValue("swimpower");
        thresPace = settings::get_thresValue("swimpace");
        sportFactor = settings::get_thresValue("swimfactor");
        ui->lineEdit_thresPower->setPalette(wback);
        ui->lineEdit_thresPace->setPalette(gback);
    }
    if(value == settings::isBike)
    {
        thresPower = settings::get_thresValue("bikepower");
        thresPace = settings::get_thresValue("bikepace");
        sportFactor = settings::get_thresValue("bikefactor");
        ui->lineEdit_thresPower->setPalette(gback);
        ui->lineEdit_thresPace->setPalette(wback);
    }
    if(value == settings::isRun)
    {
        thresPower = settings::get_thresValue("runpower");
        thresPace = settings::get_thresValue("runpace");
        sportFactor = settings::get_thresValue("runfactor");
        ui->lineEdit_thresPower->setPalette(wback);
        ui->lineEdit_thresPace->setPalette(gback);
    }

    ui->lineEdit_thresPower->setText(QString::number(thresPower));
    ui->lineEdit_thresPace->setText(settings::set_time(thresPace));
    ui->doubleSpinBox_factor->setValue(sportFactor);

    this->set_thresholdView(value);
}

void Dialog_settings::enableSavebutton()
{
    ui->pushButton_save->setEnabled(true);
}

void Dialog_settings::on_pushButton_save_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  tr("Save Settings"),
                                  "Save Stetting changes?",
                                  QMessageBox::Yes|QMessageBox::No
                                  );
    if (reply == QMessageBox::Yes)
    {
        this->writeChangedValues();
    }
}


void Dialog_settings::on_lineEdit_thresPace_textEdited(const QString &value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_lineEdit_thresPower_textEdited(const QString &value)
{
    Q_UNUSED(value)
    if(ui->comboBox_thresSport->currentText() == settings::isBike)
    {
        ui->lineEdit_thresPace->setText(settings::set_time(static_cast<int>(3600/(round(ui->lineEdit_thresPower->text().toDouble()/6.5)))));
    }
    this->enableSavebutton();
}

void Dialog_settings::on_lineEdit_hfThres_textEdited(const QString &value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_lineEdit_hfmax_textEdited(const QString &value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_dateEdit_saisonStart_dateChanged(const QDate &date)
{
    ui->lineEdit_startWeek->setText(QString::number(date.weekNumber()));
    this->enableSavebutton();
}


void Dialog_settings::on_pushButton_color_clicked()
{
    QColor color = QColorDialog::getColor(ui->pushButton_color->palette().color(ui->pushButton_color->backgroundRole()),this);
    if(color.isValid())
    {
        this->set_color(color,true,ui->lineEdit_addedit->text());
        this->enableSavebutton();
    }
}

void Dialog_settings::on_dateEdit_saisonEnd_dateChanged(const QDate &enddate)
{
    QDate lastweek;
    lastweek.setDate(ui->dateEdit_saisonStart->date().year(),12,31);
    int weeksStartYear = (lastweek.weekNumber() - (ui->dateEdit_saisonStart->date().weekNumber()-1));
    ui->lineEdit_saisonWeeks->setText(QString::number(weeksStartYear + enddate.weekNumber()));
    this->enableSavebutton();
}

void Dialog_settings::on_pushButton_schedulePath_clicked()
{
    QFileDialog dialogSched;
    QString directory = dialogSched.getExistingDirectory(0,"Caption",QString(),QFileDialog::ShowDirsOnly);
    ui->lineEdit_schedule->setText(QDir::toNativeSeparators(directory));
}

void Dialog_settings::on_pushButton_workoutsPath_clicked()
{
    QFileDialog dialogWork;
    QString directory = dialogWork.getExistingDirectory(0,"Caption",QString(),QFileDialog::ShowDirsOnly);
    ui->lineEdit_standard->setText(QDir::toNativeSeparators(directory));
}

void Dialog_settings::on_lineEdit_schedule_textChanged(const QString &value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_lineEdit_standard_textChanged(const QString &value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_lineEdit_athlete_textChanged(const QString &value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_lineEdit_yob_textChanged(const QString &value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_lineEdit_addedit_textChanged(const QString &value)
{
    if(!value.isEmpty())
    {
        ui->pushButton_add->setEnabled(true);
    }
    else
    {
        ui->pushButton_add->setEnabled(false);
    }
}

void Dialog_settings::on_tableView_level_doubleClicked(const QModelIndex &index)
{
    if(index.column() == 1)
    {
        this->enableSavebutton();
    }
}

void Dialog_settings::on_tableView_hf_doubleClicked(const QModelIndex &index)
{
    if(index.column() == 1)
    {
        this->enableSavebutton();
    }
}
