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

Dialog_settings::Dialog_settings(QWidget *parent,schedule *psched,foodplanner *pFood) :
    QDialog(parent),
    ui(new Ui::Dialog_settings)
{
    ui->setupUi(this);

    schedule_ptr = psched;
    food_ptr = pFood;
    saisonProxy = new QSortFilterProxyModel(this);
    saisonProxy->setSourceModel(schedule_ptr->saisonsModel);
    contestProxy = new QSortFilterProxyModel(this);
    contestProxy->setSourceModel(schedule_ptr->contestModel);
    sportList << settings::isSwim << settings::isBike << settings::isRun;
    colorMapCache = settings::get_colorMap();
    useColor = false;
    stressEdit = false;
    editContest = false;
    model_header << "Level" << "Low %" << "Low" << "High %" << "High";
    level_model = new QStandardItemModel(this);
    hf_model = new QStandardItemModel(this);
    ui->tableView_contest->setModel(contestProxy);
    ui->tableView_contest->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_contest->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView_contest->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->tableView_contest->horizontalHeader()->setVisible(false);
    ui->tableView_contest->horizontalHeader()->setSectionHidden(0,true);
    ui->tableView_contest->horizontalHeader()->setSectionHidden(1,true);
    ui->tableView_contest->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_contest->setGridStyle(Qt::PenStyle::DotLine);
    ui->lineEdit_gcpath->setText(gcValues->value("gcpath"));
    ui->lineEdit_gcpath->setEnabled(false);
    ui->lineEdit_athlete->setText(gcValues->value("athlete"));
    ui->lineEdit_activity->setText(gcValues->value("folder"));
    ui->lineEdit_schedule->setText(gcValues->value("schedule"));
    ui->lineEdit_standard->setText(gcValues->value("workouts"));
    ui->lineEdit_maps->setText(gcValues->value("maps"));
    ui->lineEdit_saisonFile->setText(gcValues->value("saisons"));
    ui->lineEdit_configfile->setText(gcValues->value("valuefile"));
    ui->lineEdit_foodFile->setText(gcValues->value("foodplanner"));
    ui->spinBox_hfThres->setValue(thresValues->value("hfthres"));
    ui->spinBox_hfMax->setValue(thresValues->value("hfmax"));
    ui->spinBox_ltsDays->setValue(ltsValues->value("ltsdays"));
    ui->spinBox_stsDays->setValue(ltsValues->value("stsdays"));
    ui->spinBox_lastLTS->setValue(ltsValues->value("lastlts"));
    ui->spinBox_lastSTS->setValue(ltsValues->value("laststs"));
    ui->comboBox_thresSport->addItems(sportList);
    ui->comboBox_thresBase->addItem("Pace");
    ui->comboBox_thresBase->addItem("Power");
    ui->dateEdit_contest->setDate(QDate::currentDate());
    ui->comboBox_contestsport->addItems(settings::get_listValues("Sport"));
    ui->pushButton_clearContest->setEnabled(false);
    ui->pushButton_delContest->setEnabled(false);
    ui->comboBox_weightmode->addItems(settings::get_listValues("Mode"));
    ui->comboBox_food->addItem("Dish");
    ui->comboBox_food->addItem("Meals");
    ui->pushButton_save->setEnabled(false);
    ui->dateEdit_stress->setDate(QDate::currentDate().addDays(1-QDate::currentDate().dayOfWeek()));
    ui->label_watttospeed->setVisible(false);
    ui->doubleSpinBox_watttospeed->setVisible(false);

    ui->lineEdit_age->setText(QString::number(QDate::currentDate().year() - athleteValues->value("yob")));
    ui->lineEdit_weight->setText(QString::number(settings::get_weightforDate(QDateTime::currentDateTime())));
    ui->lineEdit_currDayCal->setText(QString::number(current_dayCalories(QDateTime::currentDateTime())));
    ui->doubleSpinBox_bone->setValue(athleteValues->value("boneskg"));
    ui->doubleSpinBox_muscle->setValue(athleteValues->value("musclekg"));
    ui->doubleSpinBox_PALvalue->setValue(athleteValues->value("currpal"));
    ui->comboBox_methode->addItem("Mifflin-St.Jeor");
    ui->comboBox_methode->addItem("Katch-McArdle");
    ui->comboBox_methode->setCurrentIndex(athleteValues->value("methode"));
    ui->doubleSpinBox_max->setValue(settings::doubleVector.value(ui->comboBox_weightmode->currentText()).at(0));
    this->set_bottonColor(ui->toolButton_colormax,false);
    ui->doubleSpinBox_high->setValue(settings::doubleVector.value(ui->comboBox_weightmode->currentText()).at(1));
    this->set_bottonColor(ui->toolButton_colorhigh,false);
    ui->doubleSpinBox_low->setValue(settings::doubleVector.value(ui->comboBox_weightmode->currentText()).at(2));
    this->set_bottonColor(ui->toolButton_colorlow,false);
    ui->doubleSpinBox_min->setValue(settings::doubleVector.value(ui->comboBox_weightmode->currentText()).at(3));
    this->set_bottonColor(ui->toolButton_colormin,false);

    ui->spinBox_carbs->setValue(settings::doubleVector.value("Macros").at(0));
    ui->spinBox_protein->setValue(settings::doubleVector.value("Macros").at(1));
    ui->spinBox_fat->setValue(settings::doubleVector.value("Macros").at(2));

    for(int i = 0; i < 7; ++i)
    {
       tempCheck = this->findChild<QCheckBox *>("checkBox_Work_"+QString::number(i));
       tempCheck->setChecked(doubleVector.value("Moveday").at(i));
    }

    ui->listWidget_selection->setItemDelegate(&mousehover_del);
    ui->listWidget_useIn->setItemDelegate(&mousehover_del);
    ui->listWidget_stressValue->setItemDelegate(&mousehover_del);
    ui->listWidget_food->setItemDelegate(&mousehover_del);

    connect(ui->tableView_contest->verticalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(selectContest(int)));

    this->refresh_saisonCombo();
    this->checkSetup();
}

enum {ADD,DEL,EDIT};
enum {SPORT,LEVEL,PHASE,CYCLE,WCODE,JFILE,EDITOR,MISC};
enum {SPORTUSE};

Dialog_settings::~Dialog_settings()
{
    delete ui;
}

void Dialog_settings::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_settings::checkSetup()
{
    if(ui->lineEdit_athlete->text().isEmpty()) ui->pushButton_save->setEnabled(true);

    listMap.insert(extkeyList.at(SPORTUSE),settings::get_listValues("Sportuse"));

    ui->comboBox_selInfo->addItems(keyList);
    ui->toolButton_color->setEnabled(false);
    ui->toolButton_add->setEnabled(false);
    ui->toolButton_edit->setEnabled(false);
    ui->toolButton_delete->setEnabled(false);

    this->set_thresholdModel(ui->comboBox_thresSport->currentText(),ui->comboBox_thresBase->currentIndex());
    this->set_hfmodel(ui->spinBox_hfThres->value());
    this->set_ltsList();
    this->set_saisonInfo(ui->comboBox_saisons->currentText());
}

void Dialog_settings::set_saisonInfo(QString saisonName)
{
    saisonProxy->invalidate();
    saisonProxy->setFilterFixedString(saisonName);
    saisonProxy->setFilterKeyColumn(0);

    ui->dateEdit_saisonStart->setDate(saisonProxy->data(saisonProxy->index(0,1)).toDate());
    ui->dateEdit_saisonEnd->setDate(saisonProxy->data(saisonProxy->index(0,2)).toDate());
    ui->lineEdit_startWeek->setText(QString::number(ui->dateEdit_saisonStart->date().weekNumber()));
    ui->lineEdit_saisonWeeks->setText(QString::number(saisonProxy->data(saisonProxy->index(0,3)).toInt()));

    this->refresh_contestTable(saisonName);
}

void Dialog_settings::refresh_contestTable(QString saisonName)
{
    contestProxy->invalidate();
    contestProxy->setFilterFixedString(saisonName);
    contestProxy->setFilterKeyColumn(1);
    contestProxy->sort(2);

    for(int i = 0; i <= contestProxy->rowCount(); ++i)
    {
        contestProxy->setHeaderData(i,Qt::Orientation::Vertical,i+1);
    }

    this->reset_contest(false);
}

void Dialog_settings::refresh_macros()
{
    QPalette sumBox;
    int value = ui->spinBox_carbs->value()+ui->spinBox_protein->value()+ui->spinBox_fat->value();

    if(value > 100)
    {
        sumBox.setColor(QPalette::Base,Qt::red);
        sumBox.setColor(QPalette::Text,Qt::white);
    }
    else
    {
        sumBox.setColor(QPalette::NoRole,Qt::NoBrush);
        sumBox.setColor(QPalette::Text,Qt::black);
    }
    ui->lineEdit_macroSum->setPalette(sumBox);
    ui->lineEdit_macroSum->setText(QString::number(value));
}


void Dialog_settings::set_SelectControls(QString selection)
{
    if(selection == keyList.at(SPORT))
    {
        ui->listWidget_useIn->clear();
        ui->toolButton_add->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        ui->toolButton_edit->setEnabled(true);
        ui->pushButton_up->setEnabled(true);
        ui->pushButton_down->setEnabled(true);
        ui->listWidget_useIn->addItems(listMap.value(extkeyList.at(SPORTUSE)));
        ui->listWidget_useIn->setEnabled(true);
        useColor = true;
        this->checkSportUse();
    }
    else if(selection == keyList.at(LEVEL) || selection == keyList.at(PHASE))
    {
        ui->toolButton_add->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        ui->toolButton_edit->setEnabled(true);
        ui->pushButton_up->setEnabled(true);
        ui->pushButton_down->setEnabled(true);
        ui->listWidget_useIn->clear();
        ui->listWidget_useIn->setEnabled(false);
        useColor = true;
    }
    else if(selection == keyList.at(MISC))
    {
        ui->toolButton_add->setEnabled(false);
        ui->toolButton_delete->setEnabled(false);
        ui->toolButton_edit->setEnabled(true);
        ui->pushButton_up->setEnabled(false);
        ui->pushButton_down->setEnabled(false);
        ui->listWidget_useIn->clear();
        ui->listWidget_useIn->setEnabled(false);
        useColor = true;
    }
    else
    {
        ui->toolButton_add->setEnabled(true);
        ui->toolButton_delete->setEnabled(true);
        ui->toolButton_edit->setEnabled(true);
        ui->pushButton_up->setEnabled(true);
        ui->pushButton_down->setEnabled(true);
        ui->listWidget_useIn->clear();
        ui->listWidget_useIn->setEnabled(false);
        useColor = false;
    }
}

void Dialog_settings::reset_contest(bool setButton)
{
    ui->lineEdit_contest->clear();
    ui->dateEdit_contest->setDate(QDate::currentDate());
    ui->doubleSpinBox_contest->setValue(0);
    ui->spinBox_contestStress->setValue(0);
    ui->comboBox_contestsport->setCurrentIndex(-1);
    ui->tableView_contest->clearSelection();
    ui->tableView_contest->setCurrentIndex(QModelIndex());
    ui->pushButton_clearContest->setEnabled(setButton);
    ui->pushButton_delContest->setEnabled(setButton);
    ui->pushButton_addContest->setText("Add");
    editContest = setButton;
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
    QVector<double> saveVector;


    if(sport == settings::isSwim)
    {        
        thresholdMap.insert("swimpower",ui->spinBox_thresPower->value());
        thresholdMap.insert("swimpace",paceSec);
        thresholdMap.insert("swimfactor",ui->doubleSpinBox_factor->value());
        thresholdMap.insert("swimlimit",ui->doubleSpinBox_lowLimit->value());
        thresholdMap.insert("swimpm",ui->checkBox_usepm->isChecked());
        this->writeRangeValues(sport);
    }
    if(sport == settings::isBike)
    {
        thresholdMap.insert("bikepower",ui->spinBox_thresPower->value());
        thresholdMap.insert("bikepace",paceSec);
        thresholdMap.insert("bikespeed",this->set_doubleValue(ui->lineEdit_speed->text().toDouble(),true));
        thresholdMap.insert("bikefactor",ui->doubleSpinBox_factor->value());
        thresholdMap.insert("bikelimit",ui->doubleSpinBox_lowLimit->value());
        thresholdMap.insert("bikepm",ui->checkBox_usepm->isChecked());
        thresholdMap.insert("wattfactor",ui->doubleSpinBox_watttospeed->value());
        this->writeRangeValues(sport);
    }
    if(sport == settings::isRun)
    {
        thresholdMap.insert("runpower",ui->spinBox_thresPower->value());
        thresholdMap.insert("runpace",paceSec);
        thresholdMap.insert("runpm",ui->checkBox_usepm->isChecked());
        thresholdMap.insert("runlimit",ui->doubleSpinBox_lowLimit->value());
        thresholdMap.insert("runfactor",ui->doubleSpinBox_factor->value());
        this->writeRangeValues(sport);
    }

    thresholdMap.insert("hfthres",ui->spinBox_hfThres->value());
    thresholdMap.insert("hfmax",ui->spinBox_hfMax->value());
    this->writeRangeValues("HF");

    gcInfo.insert("gcpath",ui->lineEdit_gcpath->text());
    gcInfo.insert("athlete",ui->lineEdit_athlete->text());
    gcInfo.insert("folder",ui->lineEdit_activity->text());
    gcInfo.insert("schedule",ui->lineEdit_schedule->text());
    gcInfo.insert("workouts",ui->lineEdit_standard->text());
    gcInfo.insert("maps",ui->lineEdit_maps->text());
    gcInfo.insert("valuefile",ui->lineEdit_configfile->text());

    ltsMap.insert("ltsdays",ui->spinBox_ltsDays->value());
    ltsMap.insert("stsdays",ui->spinBox_stsDays->value());
    ltsMap.insert("lastlts",ui->spinBox_lastLTS->value());
    ltsMap.insert("laststs",ui->spinBox_lastSTS->value());

    if(generalValues->value("sum") != listMap.value("Misc").at(0)) generalMap.insert("sum",listMap.value("Misc").at(0));
    if(generalValues->value("empty") != listMap.value("Misc").at(1)) generalMap.insert("empty",listMap.value("Misc").at(1));
    if(generalValues->value("breakname") != listMap.value("Misc").at(2)) generalMap.insert("breakname",listMap.value("Misc").at(2));

    athleteMap.insert("currpal",ui->doubleSpinBox_PALvalue->value());

    for(QHash<QString,QColor>::const_iterator it = colorMapCache.cbegin(), end = colorMapCache.cend(); it != end; ++it)
    {
        colorMap.insert(it.key(),it.value());
    }

    saveVector.resize(7);
    for(int i = 0; i < 7; ++i)
    {
       tempCheck = this->findChild<QCheckBox *>("checkBox_Work_"+QString::number(i));
       saveVector[i] = tempCheck->isChecked();
    }
    doubleVector.insert("Moveday",saveVector);

    saveVector.resize(3);
    saveVector[0] = ui->spinBox_carbs->value();
    saveVector[1] = ui->spinBox_protein->value();
    saveVector[2] = ui->spinBox_fat->value();
    doubleVector.insert("Macros",saveVector);

    settings::writeListValues(&listMap);

    schedule_ptr->write_saisonInfo();

    if(stressEdit) schedule_ptr->save_ltsFile(ui->spinBox_ltsDays->value());

    thresPower = ui->spinBox_thresPower->value();
    thresPace = paceSec;
    sportFactor = ui->doubleSpinBox_factor->value();

    this->set_thresholdView(ui->comboBox_thresSport->currentText());
    this->set_hfmodel(ui->spinBox_hfThres->value());
}

void Dialog_settings::writeRangeValues(QString sport)
{
    QStringList levels = listMap.value("Level");
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

void Dialog_settings::refresh_saisonCombo()
{
    saisonProxy->setFilterFixedString("");
    saisonProxy->setFilterKeyColumn(0);

    disconnect(ui->comboBox_saisons,SIGNAL(currentIndexChanged(QString)),this,SLOT(on_comboBox_saisons_currentIndexChanged(QString)));

    ui->comboBox_saisons->clear();
    for(int i = 0; i < saisonProxy->rowCount(); ++i)
    {
        ui->comboBox_saisons->addItem(saisonProxy->data(saisonProxy->index(i,0)).toString());
    }
    ui->comboBox_saisons->setEditable(true);

    connect(ui->comboBox_saisons,SIGNAL(currentIndexChanged(QString)),this,SLOT(on_comboBox_saisons_currentIndexChanged(QString)));
    ui->comboBox_saisons->setCurrentIndex(0);
}

void Dialog_settings::on_comboBox_selInfo_currentTextChanged(const QString &value)
{
    QColor color;
    ui->lineEdit_addedit->clear();
    ui->listWidget_selection->clear();
    ui->listWidget_selection->addItems(listMap.value(value));
    this->set_SelectControls(ui->comboBox_selInfo->currentText());
    color.setRgb(255,255,255,0);
    this->set_color(color,"");
    ui->toolButton_add->setEnabled(false);
    ui->toolButton_delete->setEnabled(false);
    ui->toolButton_edit->setEnabled(false);

}

void Dialog_settings::set_thresholdView(QString sport)
{
    if(sport == settings::isSwim)
    {
        ui->lineEdit_speed->setText(QString::number(this->get_speed(ui->timeEdit_thresPace->time(),100,ui->comboBox_thresSport->currentText(),true)));
        this->set_thresholdModel(sport,ui->comboBox_thresBase->currentIndex());
    }
    if(sport == settings::isBike)
    {
        ui->lineEdit_speed->setText(QString::number(this->set_doubleValue(ui->spinBox_thresPower->value()/(athleteValues->value("ridercw")*(athleteValues->value("riderfrg")*thresValues->value("wattfactor"))),false)));
        this->set_thresholdModel(sport,ui->comboBox_thresBase->currentIndex());
    }
    if(sport == settings::isRun)
    {
        ui->lineEdit_speed->setText(QString::number(this->get_speed(ui->timeEdit_thresPace->time(),1000,ui->comboBox_thresSport->currentText(),true)));
        this->set_thresholdModel(sport,ui->comboBox_thresBase->currentIndex());
    }
}

void Dialog_settings::set_hfmodel(double hfThres)
{
    QStringList levels = listMap.value("Level");
    if(hf_model->rowCount() > 0) hf_model->clear();
    QString range,zone_low,zone_high;

    hf_model->setHorizontalHeaderLabels(model_header);
    ui->tableView_hf->setModel(hf_model);
    ui->tableView_hf->setItemDelegate(&level_del);
    ui->tableView_hf->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_hf->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_hf->verticalHeader()->hide();

    level_del.thresSelect = "HF";
    level_del.threshold = ui->spinBox_hfThres->value();

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
    QMap<QDate,QPair<double,double> > *map = schedule_ptr->get_StressMap();
    QString itemValue;
    ui->listWidget_stressValue->clear();

    for(QMap<QDate,QPair<double,double>>::const_iterator it =  map->cbegin(), end = map->cend(); it != end; ++it)
    {
        itemValue = it.key().toString("dd.MM.yyyy") +" - "+QString::number(it.value().first);
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

void Dialog_settings::set_thresholdModel(QString sport,int thresBase)
{
    QStringList levels = listMap.value("Level");
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

        if(thresBase == 0)
        {
            level_model->setData(level_model->index(i,2,QModelIndex()),this->set_time(static_cast<int>(round(thresPace / percLow))));
            level_model->setData(level_model->index(i,4,QModelIndex()),this->set_time(static_cast<int>(round(thresPace / percHigh))));
        }
        else
        {
            level_model->setData(level_model->index(i,2,QModelIndex()),round(thresPower * percLow));
            level_model->setData(level_model->index(i,4,QModelIndex()),round(thresPower * percHigh));
        }
    }
}

void Dialog_settings::set_color(QColor color,QString key)
{
    QPalette palette = ui->toolButton_color->palette();
    palette.setColor(ui->toolButton_color->backgroundRole(),color);
    ui->toolButton_color->setAutoFillBackground(true);
    ui->toolButton_color->setPalette(palette);
    colorMapCache.insert(key,color);
}

void Dialog_settings::set_bottonColor(QToolButton *button,bool changeColor)
{
    QPalette palette = button->palette();
    QString colorKey = button->objectName().remove("toolButton_color");

    if(changeColor)
    {
        QColor color = this->openColor(button->palette().color(button->backgroundRole()));
        if(color.isValid()) palette.setColor(button->backgroundRole(),color);
    }
    else
    {
        palette.setColor(button->backgroundRole(),colorMap.value(colorKey));
    }
    button->setAutoFillBackground(true);
    button->setPalette(palette);
}

void Dialog_settings::on_listWidget_selection_itemDoubleClicked(QListWidgetItem *item)
{
    QString listValue = item->data(Qt::DisplayRole).toString();
    QColor color;
    ui->lineEdit_addedit->setText(listValue);

    color = colorMapCache.value(listValue);

    if(useColor)
    {
        this->set_color(color,listValue);
        ui->toolButton_color->setEnabled(true);
    }
    else
    {
        color.setRgb(255,255,255,0);
        this->set_color(color,listValue);
        ui->toolButton_color->setEnabled(false);
    }
    this->set_SelectControls(ui->comboBox_selInfo->currentText());
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

void Dialog_settings::on_comboBox_thresSport_currentTextChanged(const QString &value)
{
    QTime paceTime(0,0,0);
    QPalette gback,wback;
    gback.setColor(QPalette::Base,Qt::green);
    wback.setColor(QPalette::Base,Qt::white);
    level_del.thresSelect = value;
    ui->timeEdit_thresPace->setTime(paceTime);

    if(value == settings::isSwim)
    {
        thresPower = thresValues->value("swimpower");
        thresPace = thresValues->value("swimpace");
        sportFactor = thresValues->value("swimfactor");
        ui->doubleSpinBox_lowLimit->setValue(thresValues->value("swimlimit"));
        ui->checkBox_usepm->setChecked(thresValues->value("swimpm"));
        ui->spinBox_thresPower->setPalette(wback);
        ui->timeEdit_thresPace->setPalette(gback);
        level_del.threshold = thresPace;
        ui->label_watttospeed->setVisible(false);
        ui->doubleSpinBox_watttospeed->setVisible(false);

    }
    if(value == settings::isBike)
    {
        thresPower = thresValues->value("bikepower");
        thresPace = thresValues->value("bikepace");
        sportFactor = thresValues->value("bikefactor");
        ui->doubleSpinBox_lowLimit->setValue(thresValues->value("bikelimit"));
        ui->checkBox_usepm->setChecked(thresValues->value("bikepm"));
        ui->doubleSpinBox_watttospeed->setValue(thresValues->value("wattfactor"));
        ui->spinBox_thresPower->setPalette(gback);
        ui->timeEdit_thresPace->setPalette(wback);
        level_del.threshold = thresPower;
        ui->label_watttospeed->setVisible(true);
        ui->doubleSpinBox_watttospeed->setVisible(true);

    }
    if(value == settings::isRun)
    {
        if(ui->comboBox_thresBase->currentIndex() == 0)
        {
            thresPower = thresValues->value("runpower");
        }
        else
        {
            thresPower = thresValues->value("runcp");
        }

        thresPace = thresValues->value("runpace");
        ui->doubleSpinBox_lowLimit->setValue(thresValues->value("runlimit"));
        ui->checkBox_usepm->setChecked(thresValues->value("runpm"));
        sportFactor = thresValues->value("runfactor");
        ui->spinBox_thresPower->setPalette(wback);
        ui->timeEdit_thresPace->setPalette(gback);
        level_del.threshold = thresPace;
        ui->label_watttospeed->setVisible(false);
        ui->doubleSpinBox_watttospeed->setVisible(false);
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


void Dialog_settings::on_toolButton_color_clicked()
{
    QColor color = this->openColor(ui->toolButton_color->palette().color(ui->toolButton_color->backgroundRole()));
    if(color.isValid())
    {
        this->set_color(color,ui->lineEdit_addedit->text());
        this->enableSavebutton();
    }
}

void Dialog_settings::on_dateEdit_saisonEnd_dateChanged(const QDate &enddate)
{
    QDate lastweek;
    lastweek.setDate(ui->dateEdit_saisonStart->date().year(),12,31);
    if(lastweek.weekNumber() == 1) lastweek = lastweek.addDays(-lastweek.dayOfWeek());
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

void Dialog_settings::on_lineEdit_addedit_textChanged(const QString &value)
{
    if(!value.isEmpty())
    {
        ui->toolButton_add->setEnabled(true);
    }
    else
    {
        ui->toolButton_add->setEnabled(false);
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
        paceTime = paceTime.addSecs(static_cast<int>(round(3600/ui->lineEdit_speed->text().toDouble())));
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

QColor Dialog_settings::openColor(QColor mappedColor)
{
    return QColorDialog::getColor(mappedColor,this);
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
    ui->spinBox_stress->setValue(schedule_ptr->get_StressMap()->value(date).first);
}

void Dialog_settings::on_pushButton_stressEdit_clicked()
{
    QPair<double,double> stressMap;
    //schedule_ptr->set_stressMap(ui->dateEdit_stress->date(),ui->spinBox_stress->value());
    stressMap.first = ui->spinBox_stress->value();
    stressMap.second = 0.0;
    schedule_ptr->updateStress(ui->dateEdit_stress->date().toString("dd.MM.yyyy"),stressMap,1);
    stressEdit = true;
    this->set_ltsList();
    this->enableSavebutton();
}

void Dialog_settings::on_toolButton_add_clicked()
{
    ui->listWidget_selection->insertItem(ui->listWidget_selection->currentRow(),ui->lineEdit_addedit->text());
    this->updateListMap(ui->comboBox_selInfo->currentIndex(),true);
    this->enableSavebutton();
}

void Dialog_settings::on_toolButton_delete_clicked()
{
    QListWidgetItem *item = ui->listWidget_selection->takeItem(ui->listWidget_selection->currentRow());
    ui->lineEdit_addedit->clear();
    this->set_color(QColor(255,255,255,0),"");
    delete item;
    this->updateListMap(ui->comboBox_selInfo->currentIndex(),true);
    this->enableSavebutton();
    ui->toolButton_delete->setEnabled(false);
    ui->toolButton_add->setEnabled(true);
}

void Dialog_settings::on_toolButton_edit_clicked()
{
    ui->listWidget_selection->item(ui->listWidget_selection->currentRow())->setData(Qt::EditRole,ui->lineEdit_addedit->text());
    this->updateListMap(ui->comboBox_selInfo->currentIndex(),true);
    this->set_color(ui->toolButton_color->palette().color(ui->toolButton_color->backgroundRole()),ui->lineEdit_addedit->text());
    this->enableSavebutton();
}

void Dialog_settings::on_spinBox_ltsDays_valueChanged(int value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_spinBox_stsDays_valueChanged(int value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_spinBox_lastLTS_valueChanged(int value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_spinBox_lastSTS_valueChanged(int value)
{
    Q_UNUSED(value)
    this->enableSavebutton();
}

void Dialog_settings::on_listWidget_stressValue_itemClicked(QListWidgetItem *item)
{
    QString values = item->data(Qt::DisplayRole).toString();
    QString stress = values.split(" - ").last();
    ui->dateEdit_stress->setDate(QDate::fromString(values.split(" - ").first(),"dd.MM.yyyy"));
    ui->spinBox_stress->setValue(stress.toInt());
}

void Dialog_settings::on_tabWidget_tabBarClicked(int index)
{
    if(index == 2)
    {
        level_del.thresSelect = ui->comboBox_thresSport->currentText();
        if(ui->comboBox_thresSport->currentText() == settings::isBike)
        {
            level_del.threshold = thresPower;
        }
        else
        {
            level_del.threshold = thresPace;
        }
    }
    if(index == 3)
    {
        level_del.thresSelect = "HF";
        level_del.threshold = ui->spinBox_hfThres->value();
    }
}

void Dialog_settings::on_pushButton_calcFat_clicked()
{
    int age = ui->lineEdit_age->text().toInt();
    double weight = ui->lineEdit_weight->text().toDouble();
    double height = athleteValues->value("height");
    double sum7 = 0;
    double k0 = 1.112;
    double k1 = - 0.00043499;
    double k2 = 0.00000055;
    double ka = - 0.00028826 * age;
    double fatCalc = 0.0;
    double fatPercent;
    double bodyFreeFat;
    double ffmi;
    QString fatComment;

    sum7 = ui->spinBox_breast->value() * 1.0
         + ui->spinBox_back->value() * 1.0
         + ui->spinBox_armpit->value() * 1.0
         + ui->spinBox_hip->value() * 1.0
         + ui->spinBox_leg->value() * 1.0
         + ui->spinBox_stomach->value() * 1.0
         + ui->spinBox_trizeps->value() * 1.0;

    //7 Point Methode Jackson & Pollock
    fatPercent = k0 + k1*sum7 + k2*sum7*sum7 + ka;
    fatPercent = 495 / fatPercent - 450;

    fatCalc = (fatPercent * weight) / 100;
    bodyFreeFat = weight - fatCalc;
    ffmi = bodyFreeFat / (height*height)+6.3*(1.8-height);

    ui->lineEdit_fatfree->setText(QString::number(set_doubleValue(bodyFreeFat,false)));
    ui->lineEdit_fatweight->setText(QString::number(set_doubleValue(fatCalc,false)));
    ui->lineEdit_fatpercent->setText(QString::number(set_doubleValue(fatPercent,false)));
    ui->lineEdit_ffmi->setText(QString::number(set_doubleValue(ffmi,false)));
    athleteValues->insert("bodyfat",fatPercent);
    ui->lineEdit_currDayCal->setText(QString::number(round(current_dayCalories(QDateTime::currentDateTime()) * ui->doubleSpinBox_PALvalue->value())));

    fatComment = QString::number(ui->spinBox_breast->value())+"-"+
                 QString::number(ui->spinBox_stomach->value())+"-"+
                 QString::number(ui->spinBox_leg->value())+"-"+
                 QString::number(ui->spinBox_hip->value())+"-"+
                 QString::number(ui->spinBox_armpit->value())+"-"+
                 QString::number(ui->spinBox_trizeps->value())+"-"+
                 QString::number(ui->spinBox_back->value());

    ui->lineEdit_comment->setText(fatComment);
}

void Dialog_settings::on_pushButton_clearFat_clicked()
{
    ui->spinBox_breast->clear();
    ui->spinBox_back->clear();
    ui->spinBox_armpit->clear();
    ui->spinBox_hip->clear();
    ui->spinBox_leg->clear();
    ui->spinBox_stomach->clear();
    ui->spinBox_trizeps->clear();
    ui->lineEdit_fatfree->clear();
    ui->lineEdit_fatweight->clear();
    ui->lineEdit_fatpercent->clear();
    ui->lineEdit_comment->clear();
}

void Dialog_settings::on_pushButton_addContest_clicked()
{
    int row = 0;

    if(editContest)
    {
        row = ui->tableView_contest->currentIndex().row();
        contestProxy->setData(contestProxy->index(row,1),ui->comboBox_saisons->currentText());
        contestProxy->setData(contestProxy->index(row,2),ui->dateEdit_contest->date());
        contestProxy->setData(contestProxy->index(row,3),ui->comboBox_contestsport->currentText());
        contestProxy->setData(contestProxy->index(row,4),ui->lineEdit_contest->text());
        contestProxy->setData(contestProxy->index(row,5),ui->doubleSpinBox_contest->value());
        contestProxy->setData(contestProxy->index(row,6),ui->spinBox_contestStress->value());
    }
    else
    {
        schedule_ptr->contestModel->insertRow(schedule_ptr->contestModel->rowCount(),QModelIndex());
        row = schedule_ptr->contestModel->rowCount()-1;
        schedule_ptr->contestModel->setData(schedule_ptr->contestModel->index(row,0),schedule_ptr->contestModel->rowCount());
        schedule_ptr->contestModel->setData(schedule_ptr->contestModel->index(row,1),ui->comboBox_saisons->currentText());
        schedule_ptr->contestModel->setData(schedule_ptr->contestModel->index(row,2),ui->dateEdit_contest->date());
        schedule_ptr->contestModel->setData(schedule_ptr->contestModel->index(row,3),ui->comboBox_contestsport->currentText());
        schedule_ptr->contestModel->setData(schedule_ptr->contestModel->index(row,4),ui->lineEdit_contest->text());
        schedule_ptr->contestModel->setData(schedule_ptr->contestModel->index(row,5),ui->doubleSpinBox_contest->value());
        schedule_ptr->contestModel->setData(schedule_ptr->contestModel->index(row,6),ui->spinBox_contestStress->value());
    }
    /*
    schedule_ptr->scheduleProxy->setFilterFixedString(ui->dateEdit_contest->date().toString("dd.MM.yyyy"));
    schedule_ptr->scheduleProxy->setFilterKeyColumn(1);
    if(schedule_ptr->scheduleProxy->rowCount() == 0)
    {
        schedule_ptr->scheduleProxy->setData(schedule_ptr->scheduleProxy->index(0,0),);
        schedule_ptr->scheduleProxy->setData(schedule_ptr->scheduleProxy->index(0,1),);
        schedule_ptr->scheduleProxy->setData(schedule_ptr->scheduleProxy->index(0,2),);
        schedule_ptr->scheduleProxy->setData(schedule_ptr->scheduleProxy->index(0,3),);
        schedule_ptr->scheduleProxy->setData(schedule_ptr->scheduleProxy->index(0,4),);
        schedule_ptr->scheduleProxy->setData(schedule_ptr->scheduleProxy->index(0,5),);
    }
    schedule_ptr->scheduleProxy->invalidate();
    */

    this->reset_contest(false);
    this->refresh_contestTable(ui->comboBox_saisons->currentText());
}

void Dialog_settings::on_pushButton_delContest_clicked()
{
    contestProxy->removeRow(ui->tableView_contest->currentIndex().row());

    this->reset_contest(false);
}

void Dialog_settings::on_toolButton_addSaison_clicked()
{
    ui->comboBox_saisons->insertItem(ui->comboBox_saisons->count(),"New-Saison");
    ui->comboBox_saisons->setCurrentIndex(ui->comboBox_saisons->count()-1);
    ui->dateEdit_saisonStart->setDate(QDate(QDate().currentDate().year(),1,1));
    ui->dateEdit_saisonEnd->setDate(QDate(QDate().currentDate().year(),12,31));
    ui->toolButton_addSaison->setEnabled(false);
}

void Dialog_settings::on_comboBox_saisons_currentIndexChanged(const QString &value)
{
    this->set_saisonInfo(value);
    schedule_ptr->newSaison = true;
}

void Dialog_settings::on_comboBox_saisons_editTextChanged(const QString &value)
{
    ui->comboBox_saisons->setItemText(ui->comboBox_saisons->currentIndex(),value);
}

void Dialog_settings::on_toolButton_updateSaison_clicked()
{
    schedule_ptr->update_saison(schedule_ptr->newSaison,
                                ui->comboBox_saisons->currentIndex(),
                                ui->comboBox_saisons->currentText(),
                                ui->dateEdit_saisonStart->date(),
                                ui->dateEdit_saisonEnd->date(),
                                ui->lineEdit_saisonWeeks->text().toInt()
                );

    if(schedule_ptr->newSaison)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      tr("New Saison"),
                                      "Create and Save new Saison?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            schedule_ptr->add_newSaison(ui->comboBox_saisons->currentText());
            schedule_ptr->write_saisonInfo();
            schedule_ptr->save_weekPlan();
        }
    }

    ui->toolButton_addSaison->setEnabled(true);
    this->set_saisonInfo(ui->comboBox_saisons->currentText());
}

void Dialog_settings::on_toolButton_deleteSaison_clicked()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  tr("Delete Saison"),
                                  "Delete selected Saison? All Saison Information will be removed!",
                                  QMessageBox::Yes|QMessageBox::No
                                  );
    if (reply == QMessageBox::Yes)
    {
        schedule_ptr->remove_saison(ui->comboBox_saisons->currentIndex());
        schedule_ptr->delete_Saison(ui->comboBox_saisons->currentText());
        schedule_ptr->write_saisonInfo();
        schedule_ptr->save_weekPlan();
        this->refresh_saisonCombo();
        this->set_saisonInfo(ui->comboBox_saisons->currentText());
    }
}

void Dialog_settings::on_doubleSpinBox_PALvalue_valueChanged(double value)
{
    ui->lineEdit_currDayCal->setText(QString::number(round(current_dayCalories(QDateTime::currentDateTime()) * value)));
}

void Dialog_settings::on_comboBox_weightmode_currentIndexChanged(const QString &mode)
{
    ui->doubleSpinBox_max->setValue(settings::doubleVector.value(mode).at(0));
    ui->doubleSpinBox_high->setValue(settings::doubleVector.value(mode).at(1));
    ui->doubleSpinBox_low->setValue(settings::doubleVector.value(mode).at(2));
    ui->doubleSpinBox_min->setValue(settings::doubleVector.value(mode).at(3));
}

void Dialog_settings::on_toolButton_colormax_clicked()
{
    this->set_bottonColor(ui->toolButton_colormax,true);
}

void Dialog_settings::on_toolButton_colorhigh_clicked()
{
    this->set_bottonColor(ui->toolButton_colorhigh,true);
}

void Dialog_settings::on_toolButton_colorlow_clicked()
{
    this->set_bottonColor(ui->toolButton_colorlow,true);
}

void Dialog_settings::on_toolButton_colormin_clicked()
{
    this->set_bottonColor(ui->toolButton_colormin,true);
}

void Dialog_settings::on_comboBox_food_currentIndexChanged(const QString &value)
{
    ui->listWidget_food->clear();
    ui->listWidget_food->addItems(listMap.value(value));
}

void Dialog_settings::on_listWidget_food_itemClicked(QListWidgetItem *item)
{
    ui->lineEdit_mealEdit->setText(item->data(Qt::DisplayRole).toString());
    ui->toolButton_mealDelete->setEnabled(true);
    ui->toolButton_mealEdit->setEnabled(true);
}

void Dialog_settings::on_toolButton_mealAdd_clicked()
{
    ui->listWidget_food->addItem(ui->lineEdit_mealEdit->text());
    QStringList tempList = listMap.value(ui->comboBox_food->currentText());
    tempList.append(ui->lineEdit_mealEdit->text());
    listMap.insert(ui->comboBox_food->currentText(),tempList);
    food_ptr->edit_mealSection(ui->lineEdit_mealEdit->text(),ADD);
    ui->lineEdit_mealEdit->clear();
    this->enableSavebutton();
}

void Dialog_settings::on_toolButton_mealDelete_clicked()
{
    QListWidgetItem *item = ui->listWidget_food->takeItem(ui->listWidget_food->currentRow());
    food_ptr->edit_mealSection(ui->lineEdit_mealEdit->text(),DEL);
    ui->lineEdit_mealEdit->clear();
    delete item;

    QStringList tempList;
    for(int i = 0; i < ui->listWidget_food->count(); ++i)
    {
        tempList.append(ui->listWidget_food->item(i)->data(Qt::DisplayRole).toString());
    }
    listMap.insert(ui->comboBox_food->currentText(),tempList);
    this->enableSavebutton();

}

void Dialog_settings::on_toolButton_mealEdit_clicked()
{
    QListWidgetItem *item = ui->listWidget_food->currentItem();
    item->setData(Qt::EditRole,ui->lineEdit_mealEdit->text());
    food_ptr->edit_mealSection(ui->lineEdit_mealEdit->text(),EDIT);
    ui->lineEdit_mealEdit->clear();
    QStringList tempList;
    for(int i = 0; i < ui->listWidget_food->count(); ++i)
    {
        tempList.append(ui->listWidget_food->item(i)->data(Qt::DisplayRole).toString());
    }
    listMap.insert(ui->comboBox_food->currentText(),tempList);
    this->enableSavebutton();
}

void Dialog_settings::on_comboBox_thresBase_currentIndexChanged(int index)
{
    if(ui->comboBox_thresSport->currentText() == settings::isRun)
    {
        if(index == 0)
        {
            thresPower = thresValues->value("runpower");
        }
        else
        {
            thresPower = thresValues->value("runcp");
        }

        ui->spinBox_thresPower->setValue(thresPower);
    }

    this->set_thresholdModel(ui->comboBox_thresSport->currentText(),index);
}

void Dialog_settings::on_toolButton_seasonPath_clicked()
{
    QString dir = this->getDirectory("Select Season File Dir");
    if(!dir.isEmpty())
    {
        ui->lineEdit_saisonFile->setText(QDir::toNativeSeparators(dir));
    }
}

void Dialog_settings::on_toolButton_foodPath_clicked()
{
    QString dir = this->getDirectory("Select FoodPlanner File Dir");
    if(!dir.isEmpty())
    {
        ui->lineEdit_foodFile->setText(QDir::toNativeSeparators(dir));
    }
}

void Dialog_settings::on_toolButton_mapPath_clicked()
{
    QString dir = this->getDirectory("Select Maps File Dir");
    if(!dir.isEmpty())
    {
        ui->lineEdit_maps->setText(QDir::toNativeSeparators(dir));
    }
}

void Dialog_settings::on_pushButton_clearContest_clicked()
{
    this->reset_contest(false);

}

void Dialog_settings::selectContest(int row)
{
    ui->dateEdit_contest->setDate(contestProxy->data(contestProxy->index(row,2)).toDate());
    ui->comboBox_contestsport->setCurrentText(contestProxy->data(contestProxy->index(row,3)).toString());
    ui->lineEdit_contest->setText(contestProxy->data(contestProxy->index(row,4)).toString());
    ui->doubleSpinBox_contest->setValue(contestProxy->data(contestProxy->index(row,5)).toDouble());
    ui->spinBox_contestStress->setValue(contestProxy->data(contestProxy->index(row,6)).toInt());
    ui->pushButton_addContest->setText("Edit");
    ui->pushButton_clearContest->setEnabled(true);
    ui->pushButton_delContest->setEnabled(true);
    editContest = true;
}

void Dialog_settings::on_spinBox_carbs_valueChanged(int value)
{
    Q_UNUSED(value)
    refresh_macros();
}

void Dialog_settings::on_spinBox_protein_valueChanged(int value)
{
     Q_UNUSED(value)
    refresh_macros();
}

void Dialog_settings::on_spinBox_fat_valueChanged(int value)
{
     Q_UNUSED(value)
    refresh_macros();
}
