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

#include "dialog_week_copy.h"
#include "ui_dialog_week_copy.h"

Dialog_week_copy::Dialog_week_copy(QWidget *parent,QString selected_week,schedule *p_sched,bool hasWeek) :
    QDialog(parent),
    ui(new Ui::Dialog_week_copy)
{
    ui->setupUi(this);
    workSched = p_sched;
    for(int i = 0; i < workSched->week_meta->rowCount(); ++i)
    {
        weekList << workSched->week_meta->data(workSched->week_meta->index(i,1,QModelIndex())).toString();
    }
    fixWeek = hasWeek;
    isSaveWeek = false;
    if(fixWeek)
    {
        sourceWeek = selected_week.split("#").first();
        ui->lineEdit_currweek->setText(sourceWeek);
        ui->lineEdit_currweek->setVisible(true);
        ui->comboBox_select->setVisible(false);
    }
    else
    {
        ui->comboBox_select->addItems(weekList);
        ui->comboBox_select->setVisible(true);
        ui->lineEdit_currweek->setVisible(false);
    }

    this->setFixedHeight(400);
    ui->comboBox_copyto->addItems(weekList);
    ui->frame_copy->setVisible(true);
    ui->frame_save->setVisible(false);
    ui->frame_delete->setVisible(false);
    ui->listView_weeks->setEnabled(false);

    editMode = 0;
    saveweekFile = "workout_saveweek.xml";
    schedulePath = settings::get_gcInfo("schedule");
    weekTags << "id" << "weekday" << "time" << "sport" << "code" << "title" << "duration" << "distance" << "stress";
    saveWeekModel = new QStandardItemModel();
    listModel = new QStandardItemModel();
    saveWeekModel->setColumnCount(10);
    listModel->setColumnCount(5);
    ui->listView_weeks->setModel(listModel);
    ui->listView_weeks->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->check_File(schedulePath,saveweekFile);
    this->readSaveweeks(this->load_XMLFile(schedulePath,saveweekFile));
}

enum {COPY,CLEAR,SAVE,LOAD};

Dialog_week_copy::~Dialog_week_copy()
{
    delete saveWeekModel;
    delete listModel;
    delete ui;
}

void Dialog_week_copy::readSaveweeks(QDomDocument xmldoc)
{
    QDomElement rootTag;
    QDomNodeList xmlList,childList;
    QDomElement xmlElement,childElement;
    int rowCount = saveWeekModel->rowCount();
    QString weekName,weekDur,weekStress;
    int workCount;

    rootTag = xmldoc.firstChildElement();
    xmlList = rootTag.elementsByTagName("week");

    listModel->insertRows(0,xmlList.count());

    for(int week = 0; week < xmlList.count(); ++week)
    {
        xmlElement = xmlList.at(week).toElement();
        weekName = xmlElement.attribute("name");
        weekDur = xmlElement.attribute("duration");
        weekStress = xmlElement.attribute("stress");
        listModel->setData(listModel->index(week,1),weekName);
        listModel->setData(listModel->index(week,3),weekDur);
        listModel->setData(listModel->index(week,4),weekStress);

        childList = xmlElement.childNodes();
        workCount = childList.count();
        listModel->setData(listModel->index(week,0),weekName+" - "+QString::number(workCount)+" - "+weekDur+" - "+weekStress);
        listModel->setData(listModel->index(week,2),workCount);

        saveWeekModel->insertRows(saveWeekModel->rowCount(),workCount);
        for(int day = 0; day < workCount; ++day,++rowCount)
        {
            childElement = childList.at(day).toElement();
            saveWeekModel->setData(saveWeekModel->index(rowCount,0),weekName);
            for(int tag = 0; tag < weekTags.count(); ++tag)
            {
                saveWeekModel->setData(saveWeekModel->index(rowCount,tag+1),childElement.attribute(weekTags.at(tag)));
            }
        }
    }
    listModel->sort(0);
}

void Dialog_week_copy::write_weekList()
{
    QSortFilterProxyModel *weekProxy = new QSortFilterProxyModel;
    weekProxy->setSourceModel(saveWeekModel);
    QString weekName;

    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement,childElement;

    xmlRoot = xmlDoc.createElement("saveweek");
    xmlDoc.appendChild(xmlRoot);

    for(int week = 0; week < listModel->rowCount(); ++week)
    {
        weekName = listModel->data(listModel->index(week,1)).toString();
        xmlElement = xmlDoc.createElement("week");
        xmlElement.setAttribute("name",weekName);
        xmlElement.setAttribute("duration",listModel->data(listModel->index(week,3)).toString());
        xmlElement.setAttribute("stress",listModel->data(listModel->index(week,4)).toString());

        weekProxy->setFilterRegExp("\\b"+weekName+"\\b");
        weekProxy->setFilterKeyColumn(0);
        for(int day = 0; day < weekProxy->rowCount(); ++day)
        {
            childElement = xmlDoc.createElement("day");
            for(int tag = 0; tag < weekTags.count(); ++tag)
            {
                childElement.setAttribute(weekTags.at(tag),weekProxy->data(weekProxy->index(day,tag+1)).toString());
            }
            xmlElement.appendChild(childElement);
            childElement.clear();
        }
        xmlRoot.appendChild(xmlElement);
        xmlElement.clear();
        weekProxy->setFilterRegExp("");
    }
    this->write_XMLFile(schedulePath,&xmlDoc,saveweekFile);
    xmlDoc.clear();
}

void Dialog_week_copy::addWeek()
{
    QString weekName = listModel->data(listModel->index(listIndex.row(),1)).toString();
    QSortFilterProxyModel *workProxy = new QSortFilterProxyModel;
    workProxy->setSourceModel(saveWeekModel);
    workProxy->setFilterRegExp("\\b"+weekName+"\\b");
    workProxy->setFilterKeyColumn(0);
    workProxy->sort(1);
    int workCount = workProxy->rowCount();
    int schedCount = workSched->workout_schedule->rowCount();

    QModelIndex index = workSched->week_meta->findItems(sourceWeek,Qt::MatchExactly,1).at(0)->index();

    QDate sourceFirstDay = QDate::fromString(workSched->week_meta->data(workSched->week_meta->index(index.row(),3)).toString(),"dd.MM.yyyy");
    workSched->workout_schedule->insertRows(schedCount,workCount,QModelIndex());

    int dayofweek;

    for(int work = 0; work < workCount; ++work)
    {
        dayofweek = workProxy->data(workProxy->index(work,2)).toInt();

        for(int col = 1; col < workProxy->columnCount(); ++col)
        {
            qDebug() << sourceFirstDay.addDays(dayofweek-1) << workProxy->data(workProxy->index(work,col));
        }
    }
}


void Dialog_week_copy::processWeek()
{
    if(editMode == COPY)
    {
        QString targetWeek = ui->comboBox_copyto->currentText();

        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Copy Week",
                                      "Copy Week "+sourceWeek+" to Week "+targetWeek+"?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            workSched->copyWeek(sourceWeek,targetWeek);
            accept();
        }
    }
    if(editMode == CLEAR)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Clear Week",
                                      "Clear all Workouts of Week "+sourceWeek+"?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            workSched->deleteWeek(sourceWeek);
            accept();
        }
    }
    if(editMode == SAVE)
    {
        if(isSaveWeek)
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          "Save Week",
                                          "Save Changes at Week List?",
                                          QMessageBox::Yes|QMessageBox::No
                                          );
            if (reply == QMessageBox::Yes)
            {
                this->write_weekList();
                accept();
            }
        }
    }
    if(editMode == LOAD)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      "Load Week",
                                      "Load selected Week into " +sourceWeek+"?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            qDebug() << "Call AddWeek";
            this->addWeek();
            accept();
        }
    }
}


void Dialog_week_copy::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_week_copy::on_pushButton_ok_clicked()
{
    this->processWeek();
}

void Dialog_week_copy::on_radioButton_copy_clicked()
{
    editMode = COPY;
    ui->frame_copy->setVisible(true);
    ui->frame_save->setVisible(false);
    ui->frame_delete->setVisible(false);
    ui->listView_weeks->setEnabled(false);
    ui->listView_weeks->clearSelection();
}

void Dialog_week_copy::on_radioButton_clear_clicked()
{
    editMode = CLEAR;
    ui->frame_copy->setVisible(false);
    ui->frame_save->setVisible(false);
    ui->frame_delete->setVisible(true);
    ui->listView_weeks->setEnabled(false);
    ui->listView_weeks->clearSelection();
}

void Dialog_week_copy::on_radioButton_save_clicked()
{
    editMode = SAVE;
    ui->frame_copy->setVisible(false);
    ui->label_saveload->setText("Save as:");
    ui->lineEdit_saveas->setEnabled(true);
    ui->lineEdit_saveas->setText("");
    ui->toolButton_addweek->setVisible(true);
    ui->toolButton_addweek->setEnabled(false);
    ui->toolButton_delete->setVisible(false);
    ui->frame_save->setVisible(true);
    ui->frame_delete->setVisible(false);
    ui->listView_weeks->setEnabled(false);
    ui->listView_weeks->clearSelection();
}
void Dialog_week_copy::on_radioButton_load_clicked()
{
    editMode = LOAD;
    ui->frame_copy->setVisible(false);
    ui->frame_save->setVisible(true);
    ui->label_saveload->setText("Selected Week:");
    ui->lineEdit_saveas->setText("");
    ui->toolButton_addweek->setVisible(false);
    ui->toolButton_delete->setVisible(true);
    ui->toolButton_delete->setEnabled(false);
    ui->lineEdit_saveas->setEnabled(false);
    ui->frame_delete->setVisible(false);
    ui->listView_weeks->setEnabled(true);
}

void Dialog_week_copy::on_listView_weeks_clicked(const QModelIndex &index)
{
    QString selectWeek = listModel->data(index).toString();
    ui->lineEdit_saveas->setText(selectWeek);
    ui->toolButton_delete->setEnabled(true);
    listIndex = index;
}

void Dialog_week_copy::on_toolButton_addweek_clicked()
{
    QString weekName;
    QList<QDate> workouts;
    int timeSum = 0;
    int stressSum = 0;

    QSortFilterProxyModel *scheduleProxy = new QSortFilterProxyModel;
    scheduleProxy->setSourceModel(workSched->workout_schedule);
    scheduleProxy->setFilterRegExp("\\b"+sourceWeek+"\\b");
    scheduleProxy->setFilterKeyColumn(0);
    int idCount = 1;
    int proxyCount = scheduleProxy->rowCount();
    int modelCount = saveWeekModel->rowCount();

    saveWeekModel->insertRows(modelCount,proxyCount,QModelIndex());

    for(int i = 0; i < proxyCount; ++i)
    {
        QDate dateofWeek = QDate::fromString(scheduleProxy->data(scheduleProxy->index(i,1)).toString(),"dd.MM.yyyy");

        timeSum =  timeSum+this->get_timesec(scheduleProxy->data(scheduleProxy->index(i,6)).toString());
        stressSum = stressSum+scheduleProxy->data(scheduleProxy->index(i,8)).toInt();
        if(!workouts.contains(dateofWeek)) workouts.append(dateofWeek);
    }
    qSort(workouts);

    weekName = ui->lineEdit_saveas->text();

    for(int day = 0; day < workouts.count(); ++day)
    {
        for(int work = 0; work < proxyCount; ++work)
        {
            if(QDate::fromString(scheduleProxy->data(scheduleProxy->index(work,1)).toString(),"dd.MM.yyyy") == workouts.at(day))
            {
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,0),weekName); //Name
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,1),idCount); //LfdNr
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,2),day+1); // DayofWeek
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,3),scheduleProxy->data(scheduleProxy->index(work,2)).toString()); //Time
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,4),scheduleProxy->data(scheduleProxy->index(work,3)).toString()); //Sport
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,5),scheduleProxy->data(scheduleProxy->index(work,4)).toString()); //Code
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,6),scheduleProxy->data(scheduleProxy->index(work,5)).toString()); //Title
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,7),scheduleProxy->data(scheduleProxy->index(work,6)).toString()); //Duration
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,8),scheduleProxy->data(scheduleProxy->index(work,7)).toString()); //Distance
                saveWeekModel->setData(saveWeekModel->index(modelCount+work,9),scheduleProxy->data(scheduleProxy->index(work,8)).toString()); //Stress
                ++idCount;
            }
        }
    }
    int listCount = listModel->rowCount();
    listModel->insertRow(listCount,QModelIndex());
    listModel->setData(listModel->index(listCount,0),weekName+" - "+QString::number(proxyCount)+" - "+this->set_time(timeSum)+" - "+QString::number(stressSum));
    listModel->setData(listModel->index(listCount,1),weekName);
    listModel->setData(listModel->index(listCount,2),proxyCount);
    listModel->setData(listModel->index(listCount,3),this->set_time(timeSum));
    listModel->setData(listModel->index(listCount,4),stressSum);

    ui->lineEdit_saveas->setText("");
    ui->toolButton_addweek->setEnabled(false);
    isSaveWeek = true;
}

void Dialog_week_copy::on_lineEdit_saveas_textChanged(const QString &value)
{
    Q_UNUSED(value)
    if(editMode == SAVE) ui->toolButton_addweek->setEnabled(true);
}

void Dialog_week_copy::on_comboBox_select_currentIndexChanged(const QString &value)
{
    sourceWeek = value;
}
