#include "dialog_settings.h"
#include "ui_dialog_settings.h"
#include <QMessageBox>
#include <QColorDialog>

Dialog_settings::Dialog_settings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_settings)
{
    ui->setupUi(this);
    powerlist = settings().get_powerList();
    paceList = settings().get_paceList();
    hfList = settings().get_hfList();
    sportList << settings().isSwim << settings().isBike << settings().isRun;
    model_header << "Level" << "Low %" << "Low" << "High %" << "High";
    level_model = new QStandardItemModel();
    hf_model = new QStandardItemModel();
    ui->lineEdit_regpath->setText(settings().get_gcInfo().at(0));
    ui->lineEdit_workdir->setText(settings().get_gcInfo().at(1));
    ui->lineEdit_athlete->setText(settings().get_gcInfo().at(2));
    ui->lineEdit_activity->setText(settings().get_gcInfo().at(3));
    ui->lineEdit_schedule->setText(settings().get_schedulePath());
    ui->lineEdit_standard->setText(settings().get_workoutsPath());
    ui->lineEdit_hfThres->setText(hfList.at(0));
    ui->lineEdit_hfmax->setText(hfList.at(1));
    ui->comboBox_selInfo->addItems(settings().get_keyList());
    ui->lineEdit_saison->setText(settings().get_saisonYear());
    ui->lineEdit_saisonWeeks->setText(QString::number(settings().get_saisonWeeks()));
    ui->dateEdit_saisonStart->setDate(QDate::fromString(settings().get_saisonFDW(),"dd.MM.yyyy"));
    ui->lineEdit_startWeek->setText(QString::number(settings().get_saisonStart()));
    ui->comboBox_thresSport->addItems(sportList);
    ui->pushButton_save->setEnabled(false);
    this->set_hfmodel();
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

void Dialog_settings::writeChangedValues()
{
    QString selection = ui->comboBox_selInfo->currentText();
    QStringList updateList;

    for(int i = 0; i < ui->listWidget_selection->count();++i)
    {
        updateList << ui->listWidget_selection->item(i)->data(Qt::DisplayRole).toString();
    }

    if(ui->comboBox_thresSport->currentText() == settings().isSwim)
    {
        powerlist[0] = ui->lineEdit_thresPower->text().toDouble();
        paceList.replace(0,ui->lineEdit_thresPace->text());
    }
    if(ui->comboBox_thresSport->currentText() == settings().isBike)
    {
        powerlist[1] = ui->lineEdit_thresPower->text().toDouble();
        paceList.replace(1,ui->lineEdit_thresPace->text());
    }
    if(ui->comboBox_thresSport->currentText() == settings().isRun)
    {
        powerlist[2] = ui->lineEdit_thresPower->text().toDouble();
        paceList.replace(2,ui->lineEdit_thresPace->text());
    }

    hfList.replace(0,ui->lineEdit_hfThres->text());
    hfList.replace(1,ui->lineEdit_hfmax->text());

    settings().set_saisonInfos(ui->lineEdit_saison->text(),ui->dateEdit_saisonStart->date(),ui->lineEdit_saisonWeeks->text().toInt(),ui->lineEdit_startWeek->text().toInt());

    settings().writeSettings(selection,updateList,paceList,hfList);
    this->set_thresholdView(ui->comboBox_thresSport->currentText());
    this->set_hfmodel();
}


void Dialog_settings::on_comboBox_selInfo_currentTextChanged(const QString &value)
{
    this->set_listEntries(value);
}

void Dialog_settings::set_listEntries(QString selection)
{
    ui->listWidget_selection->clear();

    if(selection == settings().get_keyList().at(0))
    {
        ui->listWidget_selection->addItems(settings().get_sportList());
    }
    if(selection == settings().get_keyList().at(1))
    {
        ui->listWidget_selection->addItems(settings().get_levelList());
    }
    if(selection == settings().get_keyList().at(2))
    {
        ui->listWidget_selection->addItems(settings().get_phaseList());
    }
    if(selection == settings().get_keyList().at(3))
    {
        ui->listWidget_selection->addItems(settings().get_cycleList());
    }
    if(selection == settings().get_keyList().at(4))
    {
        ui->listWidget_selection->addItems(settings().get_codeList());
    }
    if(selection == settings().get_keyList().at(5))
    {
        ui->listWidget_selection->addItems(settings().get_jsoninfos());
    }
    if(selection == settings().get_keyList().at(6))
    {
        ui->listWidget_selection->addItems(settings().get_intPlanerList());
    }
}

void Dialog_settings::set_thresholdView(QString sport)
{
    if(sport == settings().isSwim)
    {
        ui->lineEdit_thresPower->setText(QString::number(powerlist[0]));
        ui->lineEdit_thresPace->setText(paceList.at(0));
        ui->lineEdit_speed->setText(settings().get_speed(QTime::fromString(ui->lineEdit_thresPace->text(),"mm:ss"),100,ui->comboBox_thresSport->currentText(),true));
        this->set_thresholdModel(settings().get_swimRange());
    }
    if(sport == settings().isBike)
    {
        ui->lineEdit_thresPower->setText(QString::number(powerlist[1]));
        ui->lineEdit_thresPace->setText(paceList.at(1));
        ui->lineEdit_speed->setText(settings().get_speed(QTime::fromString(ui->lineEdit_thresPace->text(),"mm:ss"),1000,ui->comboBox_thresSport->currentText(),true));
        this->set_thresholdModel(settings().get_bikeRange());
    }
    if(sport == settings().isRun)
    {
        ui->lineEdit_thresPower->setText(QString::number(powerlist[2]));
        ui->lineEdit_thresPace->setText(paceList.at(2));
        ui->lineEdit_speed->setText(settings().get_speed(QTime::fromString(ui->lineEdit_thresPace->text(),"mm:ss"),1000,ui->comboBox_thresSport->currentText(),true));
        this->set_thresholdModel(settings().get_runRange());
    }
}

void Dialog_settings::set_hfmodel()
{
    if(hf_model->rowCount() > 0) hf_model->clear();

    QStringList hflevel = settings().get_hfRange();
    QString range,zone_low,zone_high;

    hf_model->setHorizontalHeaderLabels(model_header);
    ui->tableView_hf->setModel(hf_model);
    ui->tableView_hf->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_hf->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_hf->verticalHeader()->hide();

    for(int i = 0; i < hflevel.count(); ++i)
    {
        range = hflevel.at(i);
        zone_low = range.split("-").first();
        zone_high = range.split("-").last();
        hf_model->insertRows(i,1,QModelIndex());
        hf_model->setData(hf_model->index(i,0,QModelIndex()),settings().get_levelList().at(i));
        hf_model->setData(hf_model->index(i,1,QModelIndex()),zone_low);
        hf_model->setData(hf_model->index(i,2,QModelIndex()),settings().get_hfvalue(zone_low));
        hf_model->setData(hf_model->index(i,3,QModelIndex()),zone_high);
        hf_model->setData(hf_model->index(i,4,QModelIndex()),settings().get_hfvalue(zone_high));
    }
}

void Dialog_settings::set_thresholdModel(QStringList levelList)
{
    if(level_model->rowCount() > 0) level_model->clear();
    level_model->setHorizontalHeaderLabels(model_header);
    ui->tableView_level->setModel(level_model);
    ui->tableView_level->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_level->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_level->verticalHeader()->hide();
    double threshold_value = ui->lineEdit_thresPower->text().toDouble();
    int threshold_pace = settings().get_timesec(ui->lineEdit_thresPace->text());
    QString range;

    for(int i = 0; i < levelList.count(); ++i)
    {
        range = levelList.at(i);
        level_model->insertRows(i,1,QModelIndex());
        level_model->setData(level_model->index(i,0,QModelIndex()),settings().get_levelList().at(i));
        level_model->setData(level_model->index(i,1,QModelIndex()),range.split("-").first());
        level_model->setData(level_model->index(i,3,QModelIndex()),range.split("-").last());

        if(ui->comboBox_thresSport->currentText() == settings().isBike)
        {
            level_model->setData(level_model->index(i,2,QModelIndex()),threshold_value * (level_model->data(level_model->index(i,1,QModelIndex())).toDouble()/100));
            level_model->setData(level_model->index(i,4,QModelIndex()),threshold_value * (level_model->data(level_model->index(i,3,QModelIndex())).toDouble()/100));
        }
        else
        {
            level_model->setData(level_model->index(i,2,QModelIndex()),settings().set_time(static_cast<int>(round(threshold_pace + ((threshold_pace/100)*(100-level_model->data(level_model->index(i,1,QModelIndex())).toDouble()))))));
            level_model->setData(level_model->index(i,4,QModelIndex()),settings().set_time(static_cast<int>(round(threshold_pace + ((threshold_pace/100)*(100-level_model->data(level_model->index(i,3,QModelIndex())).toDouble()))))));
        }
    }

}

void Dialog_settings::on_listWidget_selection_itemDoubleClicked(QListWidgetItem *item)
{
    ui->lineEdit_addedit->setText(item->data(Qt::DisplayRole).toString());
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
    delete item;
    this->enableSavebutton();
}

void Dialog_settings::on_pushButton_edit_clicked()
{
    ui->listWidget_selection->item(ui->listWidget_selection->currentRow())->setData(Qt::EditRole,ui->lineEdit_addedit->text());
    this->enableSavebutton();
}

void Dialog_settings::on_comboBox_thresSport_currentTextChanged(const QString &value)
{
    this->set_thresholdView(value);
    QPalette gback,wback;
    gback.setColor(QPalette::Base,Qt::green);
    wback.setColor(QPalette::Base,Qt::white);;

    if(value == settings().isSwim)
    {
        ui->lineEdit_thresPower->setPalette(wback);
        ui->lineEdit_thresPace->setPalette(gback);
    }
    if(value == settings().isBike)
    {
        ui->lineEdit_thresPower->setPalette(gback);
        ui->lineEdit_thresPace->setPalette(wback);
    }
    if(value == settings().isRun)
    {
        ui->lineEdit_thresPower->setPalette(wback);
        ui->lineEdit_thresPace->setPalette(gback);
    }
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
    if(ui->comboBox_thresSport->currentText() == settings().isBike)
    {
        ui->lineEdit_thresPace->setText(settings().set_time(static_cast<int>(3600/(round(ui->lineEdit_thresPower->text().toDouble()/6.5)))));
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
    QColor color = QColorDialog::getColor(Qt::white,this);
    QPalette palette = ui->pushButton_color->palette();
    palette.setColor(ui->pushButton_color->backgroundRole(),color);
    ui->pushButton_color->setAutoFillBackground(true);
    ui->pushButton_color->setPalette(palette);
}
