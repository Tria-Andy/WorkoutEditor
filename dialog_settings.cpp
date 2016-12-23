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

Dialog_settings::Dialog_settings(QWidget *parent,schedule *psched) :
    QDialog(parent),
    ui(new Ui::Dialog_settings)
{
    ui->setupUi(this);
    schedule_ptr = psched;
    sportList << settings::isSwim << settings::isBike << settings::isRun;
    keyList = settings::get_keyList();
    extkeyList = settings::get_extkeyList();
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
    ui->spinBox_hfThres->setValue(settings::get_thresValue("hfthres"));
    ui->spinBox_hfMax->setValue(settings::get_thresValue("hfmax"));
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
    ui->dateEdit_stress->setDate(QDate::currentDate().addDays(1-QDate::currentDate().dayOfWeek()));
    this->checkSetup();
}

enum {SPORT,LEVEL,PHASE,CYCLE,WCODE,JFILE,EDITOR};
enum {SPORTUSE};

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
    listMap.insert(keyList.at(SPORT),settings::get_sportList());
    listMap.insert(keyList.at(LEVEL),settings::get_levelList());
    listMap.insert(keyList.at(PHASE),settings::get_phaseList());
    listMap.insert(keyList.at(CYCLE),settings::get_cycleList());
    listMap.insert(keyList.at(WCODE),settings::get_codeList());
    listMap.insert(keyList.at(JFILE),settings::get_jsoninfos());
    listMap.insert(keyList.at(EDITOR),settings::get_intPlanerList());
    listMap.insert(extkeyList.at(SPORTUSE),settings::get_sportUseList());
    ui->comboBox_selInfo->addItems(keyList);
    this->set_hfmodel(ui->spinBox_hfThres->value());
    this->set_ltsList();
}

void Dialog_settings::updateListMap(int index,bool isKey)
{
    QStringList updateList;
    if(isKey)
    {
        for(int i = 0; i < ui->listWidget_selection->count();++i)
        {
            updateList << ui->listWidget_selection->item(i)->data(Qt::DisplayRole).toString();
        }
        listMap.insert(keyList.at(index),updateList);
    }
    else
    {
        for(int i = 0; i < ui->listWidget_useIn->count();++i)
        {
            updateList << ui->listWidget_useIn->item(i)->data(Qt::DisplayRole).toString();
        }
        listMap.insert(extkeyList.at(index),updateList);
    }
}

void Dialog_settings::writeChangedValues()
{
    QString sport = ui->comboBox_thresSport->currentText();
    double paceSec = (ui->timeEdit_thresPace->time().minute()*60) + ui->timeEdit_thresPace->time().second();

    if(sport == settings::isSwim)
    {
        settings::set_thresValue("swimpower",ui->spinBox_thresPower->value());
        settings::set_thresValue("swimpace",paceSec);
        settings::set_thresValue("swimfactor",ui->doubleSpinBox_factor->value());
        this->writeRangeValues(sport);
    }
    if(sport == settings::isBike)
    {
        settings::set_thresValue("bikepower",ui->spinBox_thresPower->value());
        settings::set_thresValue("bikepace",paceSec);
        settings::set_thresValue("bikefactor",ui->doubleSpinBox_factor->value());
        this->writeRangeValues(sport);
    }
    if(sport == settings::isRun)
    {
        settings::set_thresValue("runpower",ui->spinBox_thresPower->value());
        settings::set_thresValue("runpace",paceSec);
        settings::set_thresValue("runfactor",ui->doubleSpinBox_factor->value());
        this->writeRangeValues(sport);
    }

    settings::set_thresValue("hfthres",ui->spinBox_hfThres->value());
    settings::set_thresValue("hfmax",ui->spinBox_hfMax->value());
    this->writeRangeValues("HF");

    settings::set_gcInfo("gcpath",ui->lineEdit_gcpath->text());
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

    settings::writeListValues(&listMap);

    thresPower = ui->spinBox_thresPower->value();
    thresPace = paceSec;
    sportFactor = ui->doubleSpinBox_factor->value();

    this->set_thresholdView(ui->comboBox_thresSport->currentText());
    this->set_hfmodel(ui->spinBox_hfThres->value());
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
    ui->listWidget_selection->addItems(listMap.value(selection));


    if(selection == keyList.at(SPORT))
    {
        ui->listWidget_useIn->addItems(listMap.value(extkeyList.at(SPORTUSE)));
        ui->listWidget_useIn->setEnabled(true);
        useColor = true;
        this->checkSportUse();
    }
    else if(selection == keyList.at(LEVEL) || selection == keyList.at(PHASE))
    {
        ui->listWidget_useIn->clear();
        ui->listWidget_useIn->setEnabled(false);
        useColor = true;
    }
    else
    {
        ui->listWidget_useIn->clear();
        ui->listWidget_useIn->setEnabled(false);
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
        ui->lineEdit_speed->setText(settings::get_speed(ui->timeEdit_thresPace->time(),100,ui->comboBox_thresSport->currentText(),true));
        this->set_thresholdModel(sport);
    }
    if(sport == settings::isBike)
    {
        ui->lineEdit_speed->setText(settings::get_speed(ui->timeEdit_thresPace->time(),1000,ui->comboBox_thresSport->currentText(),true));
        this->set_thresholdModel(sport);
    }
    if(sport == settings::isRun)
    {
        ui->lineEdit_speed->setText(settings::get_speed(ui->timeEdit_thresPace->time(),1000,ui->comboBox_thresSport->currentText(),true));
        this->set_thresholdModel(sport);
    }
}

void Dialog_settings::set_hfmodel(double hfThres)
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

    double percLow = 0,percHigh = 0;

    for(int i = 0; i < levels.count(); ++i)
    {
        range = settings::get_rangeValue("HF",levels.at(i));
        zone_low = range.split("-").first();
        zone_high = range.split("-").last();
        hf_model->insertRows(i,1,QModelIndex());
        hf_model->setData(hf_model->index(i,0,QModelIndex()),levels.at(i));
        hf_model->setData(hf_model->index(i,1,QModelIndex()),zone_low);
        hf_model->setData(hf_model->index(i,3,QModelIndex()),zone_high);

        percLow = hf_model->data(hf_model->index(i,1)).toDouble()/100;
        percHigh = hf_model->data(hf_model->index(i,3)).toDouble()/100;

        hf_model->setData(hf_model->index(i,2,QModelIndex()),round(hfThres * percLow));
        hf_model->setData(hf_model->index(i,4,QModelIndex()),round(hfThres * percHigh));
    }
}

void Dialog_settings::set_ltsList()
{
    QMap<QDate,double> *map = schedule_ptr->get_StressMap();
    QString itemValue;
    ui->listWidget_stressValue->clear();

    for(QMap<QDate,double>::const_iterator it =  map->cbegin(), end = map->cend(); it != end; ++it)
    {
        itemValue = it.key().toString("dd.MM.yyyy") +" - "+QString::number(it.value());
        ui->listWidget_stressValue->addItem(itemValue);
    }
}

void Dialog_settings::checkSportUse()
{
    if(ui->listWidget_useIn->count() >=5)
    {
        ui->listWidget_useIn->setDragDropMode(QAbstractItemView::NoDragDrop);
    }
    else
    {
        ui->listWidget_useIn->setDragDropMode(QAbstractItemView::DropOnly);
    }
}

void Dialog_settings::set_thresholdModel(QString sport)
{
    QStringList levels = settings::get_levelList();
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

        if(sport == settings::isBike)
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
    this->updateListMap(ui->comboBox_selInfo->currentIndex(),true);
    this->enableSavebutton();
}

void Dialog_settings::on_pushButton_down_clicked()
{
    int currentindex = ui->listWidget_selection->currentRow();
    QListWidgetItem *currentItem = ui->listWidget_selection->takeItem(currentindex);
    ui->listWidget_selection->insertItem(currentindex+1,currentItem);
    ui->listWidget_selection->setCurrentRow(currentindex+1);
    this->updateListMap(ui->comboBox_selInfo->currentIndex(),true);
    this->enableSavebutton();
}

void Dialog_settings::on_pushButton_add_clicked()
{
    ui->listWidget_selection->insertItem(ui->listWidget_selection->currentRow(),ui->lineEdit_addedit->text());
    this->updateListMap(ui->comboBox_selInfo->currentIndex(),true);
    this->enableSavebutton();
}

void Dialog_settings::on_pushButton_delete_clicked()
{
    QListWidgetItem *item = ui->listWidget_selection->takeItem(ui->listWidget_selection->currentRow());
    ui->lineEdit_addedit->clear();
    this->set_color(QColor(255,255,255,0),false,"");
    delete item;
    this->updateListMap(ui->comboBox_selInfo->currentIndex(),true);
    this->enableSavebutton();
    ui->pushButton_delete->setEnabled(false);
    ui->pushButton_add->setEnabled(true);
}

void Dialog_settings::on_pushButton_edit_clicked()
{
    ui->listWidget_selection->item(ui->listWidget_selection->currentRow())->setData(Qt::EditRole,ui->lineEdit_addedit->text());
    this->updateListMap(ui->comboBox_selInfo->currentIndex(),true);
    this->enableSavebutton();
}

void Dialog_settings::on_comboBox_thresSport_currentTextChanged(const QString &value)
{
    QTime paceTime(0,0,0);
    QPalette gback,wback;
    gback.setColor(QPalette::Base,Qt::green);
    wback.setColor(QPalette::Base,Qt::white);

    ui->timeEdit_thresPace->setTime(paceTime);

    if(value == settings::isSwim)
    {
        thresPower = settings::get_thresValue("swimpower");
        thresPace = settings::get_thresValue("swimpace");
        sportFactor = settings::get_thresValue("swimfactor");
        ui->spinBox_thresPower->setPalette(wback);
        ui->timeEdit_thresPace->setPalette(gback);
    }
    if(value == settings::isBike)
    {
        thresPower = settings::get_thresValue("bikepower");
        thresPace = settings::get_thresValue("bikepace");
        sportFactor = settings::get_thresValue("bikefactor");
        ui->spinBox_thresPower->setPalette(gback);
        ui->timeEdit_thresPace->setPalette(wback);
    }
    if(value == settings::isRun)
    {
        thresPower = settings::get_thresValue("runpower");
        thresPace = settings::get_thresValue("runpace");
        sportFactor = settings::get_thresValue("runfactor");
        ui->spinBox_thresPower->setPalette(wback);
        ui->timeEdit_thresPace->setPalette(gback);
    }
    paceTime = paceTime.addSecs(static_cast<int>(thresPace));
    ui->spinBox_thresPower->setValue(thresPower);
    ui->timeEdit_thresPace->setTime(paceTime);
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

void Dialog_settings::on_lineEdit_gcpath_textChanged(const QString &value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
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

void Dialog_settings::on_listWidget_useIn_doubleClicked(const QModelIndex &index)
{
    QListWidgetItem *item = ui->listWidget_useIn->takeItem(index.row());
    delete item;
    this->updateListMap(SPORTUSE,false);
    this->checkSportUse();
    this->enableSavebutton();
}

void Dialog_settings::on_listWidget_useIn_itemChanged(QListWidgetItem *item)
{
    Q_UNUSED(item)
    this->checkSportUse();
    this->updateListMap(SPORTUSE,false);
    this->enableSavebutton();
}

void Dialog_settings::on_toolButton_gcPath_clicked()
{
    QString dir = this->getDirectory("Set GC Activities Directory");
    if(!dir.isEmpty())
    {
        ui->lineEdit_gcpath->setText(QDir::toNativeSeparators(dir));
    }
}

void Dialog_settings::on_doubleSpinBox_factor_valueChanged(double value)
{
    sportFactor = value;
    this->enableSavebutton();
}

void Dialog_settings::on_spinBox_thresPower_valueChanged(int value)
{
    if(ui->comboBox_thresSport->currentText() == settings::isBike)
    {
        thresPower = value;
        QTime paceTime(0,0,0);
        paceTime = paceTime.addSecs(static_cast<int>(3600/(round(static_cast<double>(value)/6.5))));
        ui->timeEdit_thresPace->setTime(paceTime);
        this->set_thresholdView(ui->comboBox_thresSport->currentText());
    }
    this->enableSavebutton();
}

void Dialog_settings::on_timeEdit_thresPace_timeChanged(const QTime &time)
{
    thresPace = time.minute()*60 + time.second();
    this->set_thresholdView(ui->comboBox_thresSport->currentText());
    this->enableSavebutton();
}

void Dialog_settings::on_toolButton_schedulePath_clicked()
{
    QString dir = this->getDirectory("Select Schedule File Dir");
    if(!dir.isEmpty())
    {
        ui->lineEdit_schedule->setText(QDir::toNativeSeparators(dir));
    }
}

QString Dialog_settings::getDirectory(QString getdir)
{
    QFileDialog dialogSched;
    return dialogSched.getExistingDirectory(this,getdir,"C:\\",QFileDialog::ShowDirsOnly);
}

void Dialog_settings::on_toolButton_workoutsPath_clicked()
{
    QString dir = this->getDirectory("Select StandardWorkouts File Dir");
    if(!dir.isEmpty())
    {
        ui->lineEdit_standard->setText(QDir::toNativeSeparators(dir));
    }
}

void Dialog_settings::on_spinBox_hfThres_valueChanged(int value)
{
    this->set_hfmodel(static_cast<double>(value));
    this->enableSavebutton();
}

void Dialog_settings::on_spinBox_hfMax_valueChanged(int value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_dateEdit_stress_dateChanged(const QDate &date)
{
    ui->spinBox_stress->setValue(schedule_ptr->get_StressMap()->value(date));
}

void Dialog_settings::on_pushButton_stressEdit_clicked()
{
    schedule_ptr->set_stressMap(ui->dateEdit_stress->date(),ui->spinBox_stress->value());
    this->set_ltsList();
}
