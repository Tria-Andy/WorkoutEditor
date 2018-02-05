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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QTableWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //Settings
    settings::loadSettings();
    userSetup = true;
    sportCounter = settings::get_listValues("Sport").count();
    sportUse = settings::get_listValues("Sportuse").count();
    weekRange = settings::get_fontValue("weekRange");

    //Planning Mode
    graphLoaded = false;
    workSchedule = new schedule();

    schedMode << "Week" << "Year";    
    selectedDate = QDate::currentDate();
    firstdayofweek = selectedDate.addDays(1 - selectedDate.dayOfWeek());
    foodPlan = new foodplanner(workSchedule,firstdayofweek);
    weeknumber = QString::number(selectedDate.weekNumber()) +"_"+QString::number(selectedDate.year()); 
    weekpos = weekCounter = 0;;
    weekDays = 7;
    work_sum.resize(sportCounter);
    dur_sum.resize(sportCounter);
    dist_sum.resize(sportCounter);
    stress_sum.resize(sportCounter);
    isWeekMode = true;
    buttonStyle = "QToolButton:hover {color: white; border: 1px solid white; border-radius: 4px; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #00b8ff, stop: 0.5 #0086ff,stop: 1 #0064ff)}";

    ui->label_month->setText("Week " + weeknumber + " - " + QString::number(selectedDate.addDays(weekRange*weekDays).weekNumber()-1));
    planerIcon.addFile(":/images/icons/DateTime.png");
    editorIcon.addFile(":/images/icons/Editor.png");
    foodIcon.addFile(":/images/icons/Food.png");

    modules = new QComboBox(this);
    modules->addItem(planerIcon,"Planner");
    modules->addItem(editorIcon,"Editor");
    modules->addItem(foodIcon,"Food");
    modules->setToolTip("Modules");

    planerMode = new QLabel(this);
    planerMode->setText("Planer Mode:");
    planMode = new QToolButton(this);
    planMode->setCheckable(true);
    planMode->setToolTip("Change Planer Mode");
    planMode->setMinimumHeight(25);
    planMode->setMinimumWidth(75);
    planMode->setText(schedMode.at(0));
    menuSpacer = new QWidget(this);
    menuSpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    ui->mainToolBar->addWidget(modules);
    ui->mainToolBar->addWidget(menuSpacer);
    ui->mainToolBar->addWidget(planerMode);
    ui->mainToolBar->addWidget(planMode);
    ui->calendarWidget->setVisible(true);
    ui->frame_YearAvg->setVisible(false);
    ui->toolButton_weekCurrent->setEnabled(false);
    ui->toolButton_weekMinus->setEnabled(false);
    calendarModel = new QStandardItemModel(this);
    sumModel = new QStandardItemModel(0,1,this);
    avgModel = new QStandardItemModel(this);
    scheduleProxy = new QSortFilterProxyModel(this);
    scheduleProxy->setSourceModel(workSchedule->workout_schedule);
    metaProxy = new QSortFilterProxyModel(this);
    metaProxy->setSourceModel(workSchedule->week_meta);
    metaProxyFilter = new QSortFilterProxyModel(this);
    metaProxyFilter->setSourceModel(metaProxy);
    contentProxy = new QSortFilterProxyModel(this);
    contentProxy->setSourceModel(workSchedule->week_content);
    this->set_phaseButtons();
    ui->frame_phases->setVisible(false);
    cal_header << "Week";
    for(int d = 1; d < 8; ++d)
    {
        cal_header << QDate::longDayName(d);
    }
    avgHeader << "Sport" << "Workouts" << "Duration" << "Distance";

    this->refresh_saisonInfo();

    //Editor Mode
    avgCounter = 0;
    fileModel = new QStandardItemModel(this);
    avgCounter = 0;
    actLoaded = false;

    connect(ui->actionExit_and_Save, SIGNAL(triggered()), this, SLOT(close()));
    connect(planMode,SIGNAL(clicked(bool)),this,SLOT(toolButton_planMode(bool)));
    connect(phaseGroup,SIGNAL(buttonClicked(int)),this,SLOT(set_phaseFilter(int)));
    connect(modules,SIGNAL(currentIndexChanged(int)),this,SLOT(set_module(int)));
    connect(workSchedule->workout_schedule,SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),this,SLOT(refresh_model()));
    connect(workSchedule->workout_schedule,SIGNAL(layoutChanged(QList<QPersistentModelIndex>,QAbstractItemModel::LayoutChangeHint)),this,SLOT(refresh_model()));
    connect(workSchedule->workout_schedule,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(refresh_model()));
    connect(workSchedule->workout_schedule,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(refresh_model()));
    connect(workSchedule->week_meta,SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),this,SLOT(refresh_model()));
    connect(workSchedule->week_content,SIGNAL(dataChanged(QModelIndex,QModelIndex,QVector<int>)),this,SLOT(refresh_model()));

    //UI
    ui->stackedWidget->setGeometry(5,5,0,0);
    this->summery_view();
    ui->tableView_cal->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_cal->setModel(calendarModel);
    ui->tableView_cal->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_cal->horizontalHeader()->setSectionsClickable(false);
    ui->tableView_cal->verticalHeader()->hide();
    ui->tableView_yearAvg->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_yearAvg->setModel(avgModel);
    ui->tableView_yearAvg->setItemDelegate(&avgweek_del);
    ui->tableView_yearAvg->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_yearAvg->horizontalHeader()->setSectionsClickable(false);
    ui->tableView_yearAvg->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_yearAvg->verticalHeader()->hide();
    ui->toolButton_addSelect->setEnabled(false);
    ui->toolButton_clearSelect->setEnabled(false);
    ui->toolButton_clearContent->setEnabled(false);
    ui->toolButton_sync->setEnabled(false);
    ui->horizontalSlider_factor->setEnabled(false);

    ui->tableView_weekSum->setModel(foodPlan->weekSumModel);
    ui->tableView_weekSum->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_weekSum->horizontalHeader()->hide();
    ui->tableView_weekSum->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_weekSum->setItemDelegate(&foodSumWeek_del);

    ui->tableWidget_weekPlan->setRowCount(foodPlan->mealsHeader.count());
    ui->tableWidget_weekPlan->setColumnCount(foodPlan->dayHeader.count());
    ui->tableWidget_weekPlan->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_weekPlan->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_weekPlan->setVerticalHeaderLabels(foodPlan->mealsHeader);
    ui->tableWidget_weekPlan->setHorizontalHeaderLabels(foodPlan->dayHeader);
    ui->tableWidget_weekPlan->verticalHeader()->setFixedWidth(110);
    ui->tableWidget_weekPlan->setItemDelegate(&food_del);
    ui->tableWidget_weekPlan->viewport()->setMouseTracking(true);
    ui->tableWidget_weekPlan->installEventFilter(this);
    ui->tableWidget_weekPlan->viewport()->installEventFilter(this);
    this->fill_weekTable(foodPlan->set_weekID(firstdayofweek),false);

    ui->tableView_daySummery->setModel(foodPlan->daySumModel);
    ui->tableView_daySummery->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_daySummery->horizontalHeader()->hide();
    ui->tableView_daySummery->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_daySummery->verticalHeader()->setFixedWidth(110);
    ui->tableView_daySummery->setItemDelegate(&foodSum_del);
    ui->treeView_meals->setModel(foodPlan->mealModel);
    ui->treeView_meals->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView_meals->header()->hide();
    ui->treeView_meals->setSortingEnabled(true);
    ui->treeView_meals->sortByColumn(0,Qt::AscendingOrder);
    //ui->treeView_meals->setItemDelegate(&mousehover_del);
    //ui->treeView_meals->setStyleSheet(viewStyle);
    ui->treeView_meals->setEditTriggers(QAbstractItemView::NoEditTriggers);
    mealSelection = ui->treeView_meals->selectionModel();
    connect(mealSelection,SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(setSelectedMeal(QModelIndex)));
    connect(foodPlan->mealModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(mealSave(QStandardItem*)));

    ui->listWidget_weekPlans->addItems(foodPlan->planList);
    //ui->listWidget_weekPlans->setStyleSheet(viewStyle);
    //ui->listWidget_weekPlans->setItemDelegate(&mousehover_del);
    //ui->listWidget_MenuEdit->setStyleSheet(viewStyle);
    //ui->listWidget_MenuEdit->setItemDelegate(&mousehover_del);
    ui->spinBox_calories->setVisible(false);

    ui->tableView_forecast->setModel(foodPlan->estModel);
    ui->tableView_forecast->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_forecast->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_forecast->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_forecast->horizontalHeader()->hide();

    this->set_foodWeek(foodPlan->set_weekID(firstdayofweek)+" - "+firstdayofweek.toString("dd.MM.yyyy"));

    ui->toolButton_saveMeals->setEnabled(false);
    ui->toolButton_deleteMenu->setEnabled(false);
    ui->toolButton_addMenu->setEnabled(false);
    ui->toolButton_foodUp->setEnabled(false);
    ui->toolButton_foodDown->setEnabled(false);
    ui->toolButton_menuEdit->setEnabled(false);
    ui->toolButton_clear->setEnabled(false);
    selModule = 0;

    this->set_speedgraph();
    this->resetPlot();
    this->read_activityFiles();

    ui->actionSave->setEnabled(false);
    this->set_menuItems(false,true);
    this->set_phaseFilter(1);
    this->loadUISettings();
}

enum {PLANER,EDITOR,FOOD};

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::loadUISettings()
{
        settings::settingsUpdated = false;
}

void MainWindow::freeMem()
{
    if(userSetup)
    {
        workSchedule->freeMem();
        calendarModel->clear();
        delete workSchedule;
        delete sumModel;
        delete fileModel;
        delete calendarModel;
        delete avgModel;
        delete scheduleProxy;
        delete metaProxy;
        delete metaProxyFilter;
        delete contentProxy;
    }
}

void MainWindow::fill_weekTable(QString weekID,bool reset)
{
    ui->tableWidget_weekPlan->blockSignals(true);
    if(reset) ui->tableWidget_weekPlan->clearContents();

    QStandardItemModel *model = foodPlan->weekPlansModel;
    QModelIndex weekIndex = model->indexFromItem(model->findItems(weekID,Qt::MatchExactly,0).at(0));
    QModelIndex dayIndex;
    QModelIndex mealIndex;
    QString dayString;
    int foodCount = 0;
    int mealCal = 0;

    foodPlan->estModel->setData(foodPlan->estModel->index(0,0),model->data(model->index(weekIndex.row(),2)).toDouble());

    for(int day = 0; day < foodPlan->dayHeader.count(); ++day)
    {
        dayIndex = model->index(day,0,weekIndex);
        for(int meal = 0; meal < foodPlan->mealsHeader.count(); ++meal)
        {
            mealIndex = model->index(meal,0,dayIndex);
            foodCount = model->itemFromIndex(mealIndex)->rowCount();

            if(!ui->tableWidget_weekPlan->item(meal,day))
            {            
                QTableWidgetItem *item = new QTableWidgetItem;

                for(int cal = 0; cal < foodCount; ++cal)
                {
                    mealCal = mealCal + model->data(model->index(cal,2,mealIndex)).toInt();
                }

                item->setToolTip("Meal Cal: "+QString::number(mealCal));
                ui->tableWidget_weekPlan->setItem(meal,day,item);
            }
            mealCal = 0;

            for(int food = 0; food < foodCount; ++food)
            {
                dayString = dayString + ui->tableWidget_weekPlan->item(meal,day)->data(Qt::DisplayRole).toString() + "\n"+
                            model->data(model->index(food,0,mealIndex)).toString() + " ("+
                            model->data(model->index(food,1,mealIndex)).toString()+"-"+
                            model->data(model->index(food,2,mealIndex)).toString()+")";

            }
            if(dayString.startsWith("\n"))
            {
                dayString.remove(0,1);
            }
            ui->tableWidget_weekPlan->item(meal,day)->setData(Qt::EditRole,dayString);
            dayString.clear();
        }
    }
    ui->tableWidget_weekPlan->blockSignals(false);
}

void MainWindow::read_activityFiles()
{
    fileModel->clear();
    ui->progressBar_fileState->setValue(20);
    QStandardItem *rootItem = fileModel->invisibleRootItem();
    this->readJsonFiles(rootItem);
    ui->progressBar_fileState->setValue(75);
    this->loadfile(fileModel->data(fileModel->index(0,4)).toString());
    actLoaded = true;
    ui->progressBar_fileState->setValue(100);
    QTimer::singleShot(2000,ui->progressBar_fileState,SLOT(reset()));
}

void MainWindow::clearActivtiy()
{
    this->resetPlot();

    for(int i = 0; i < infoModel->rowCount(); ++i)
    {
        infoModel->setData(infoModel->index(i,0),"-");
    }

    if(actLoaded)
    {
        delete treeSelection;
        curr_activity->reset_avgSelection();
        delete curr_activity->intModel;
        delete curr_activity->sampleModel;
        delete curr_activity->intTreeModel;
        delete curr_activity->avgModel;
        delete curr_activity->selItemModel;
        if(curr_activity->get_sport() == settings::isSwim)
        {
            delete curr_activity->swimModel;
            delete curr_activity->swimProxy;
        }
        delete curr_activity;
    }
    actLoaded = false;

    QString viewBackground = "background-color: #e6e6e6";

    ui->lineEdit_workContent->clear();
    ui->tableView_actInfo->setStyleSheet(viewBackground);
    ui->treeView_intervall->setStyleSheet(viewBackground);
    ui->treeView_files->setStyleSheet(viewBackground);

    ui->actionSelect_File->setEnabled(true);
    ui->actionReset->setEnabled(false);
}

void MainWindow::openPreferences()
{
    Dialog_settings dia_settings(this);
    dia_settings.setModal(true);
    dia_settings.exec();
}

void MainWindow::set_menuItems(bool mEditor,bool mPlaner)
{
    //Editor
    ui->actionRefresh_Filelist->setVisible(mEditor);
    ui->actionReset->setVisible(mEditor);
    ui->actionSelect_File->setVisible(mEditor);
    ui->actionReset->setEnabled(actLoaded);

    //Schedule
    ui->menuWorkout->setEnabled(mPlaner);
    ui->actionExport_to_Golden_Cheetah->setVisible(mPlaner);
    ui->actionNew->setVisible(mPlaner);
    planerMode->setEnabled(mPlaner);
    planMode->setEnabled(mPlaner);
    planerMode->setVisible(mPlaner);

    ui->actionDelete->setVisible(false);

    if(workSchedule->get_StressMap()->count() == 0) ui->actionPMC->setEnabled(false);
}

//Planner Functions ***********************************************************************************

QString MainWindow::set_summeryString(int pos,bool week)
{
    QString sumString;
    QString sum_name = generalValues->value("sum");
    QStringList sportList = settings::get_listValues("Sport");
    double percent = 0.0;
    if(week)
    {
        if(pos == 0)
        {
            if(dur_sum[pos] != 0) percent = (static_cast<double>(dur_sum[pos]) / static_cast<double>(dur_sum[0]))*100;
            sumString = sum_name +"-"+ QString::number(work_sum[pos]) +"-"+ this->set_time(dur_sum[pos]) +"-"+ QString::number(this->set_doubleValue(percent,false)) +"-"+ QString::number(dist_sum[pos]) +"-"+ QString::number(stress_sum[pos]);
        }
        else
        {
            if(dur_sum[pos] != 0) percent = static_cast<double>(dur_sum[pos]) / (static_cast<double>(dur_sum[0]))*100;
            sumString = sportList.at(pos-1) +"-"+ QString::number(work_sum[pos]) +"-"+ this->set_time(dur_sum[pos]) +"-"+ QString::number(this->set_doubleValue(percent,false)) +"-"+ QString::number(dist_sum[pos]) +"-"+QString::number(stress_sum[pos]);
        }
    }
    else
    {
        if(pos == 0)
        {
            if(dur_sum[sportUse] != 0) percent = (static_cast<double>(dur_sum[sportUse]) / static_cast<double>(dur_sum[sportUse]))*100;
            sumString = sum_name +"-"+ QString::number(work_sum[sportUse]) +"-"+ this->set_time(dur_sum[sportUse]) +"-"+ QString::number(this->set_doubleValue(percent,false)) +"-"+ QString::number(dist_sum[sportUse]) +"-"+ QString::number(stress_sum[sportUse]);
        }
        else
        {
            if(dur_sum[pos-1] != 0) percent = (static_cast<double>(dur_sum[pos-1]) / static_cast<double>(dur_sum[sportUse]))*100;
            sumString = sportList.at(pos-1) +"-"+ QString::number(work_sum[pos-1]) +"-"+ this->set_time(dur_sum[pos-1]) +"-"+ QString::number(this->set_doubleValue(percent,false)) +"-"+ QString::number(dist_sum[pos-1]) +"-"+QString::number(stress_sum[pos-1]);
        }
    }
    return sumString;
}

void MainWindow::week_summery(int pos, int dataIndex)
{
    work_sum[pos] = work_sum[pos] + 1;
    dur_sum[pos] = dur_sum[pos] + this->get_timesec(scheduleProxy->data(scheduleProxy->index(dataIndex,6)).toString());
    dist_sum[pos] = dist_sum[pos] + scheduleProxy->data(scheduleProxy->index(dataIndex,7)).toDouble();
    stress_sum[pos] = stress_sum[pos] + scheduleProxy->data(scheduleProxy->index(dataIndex,8)).toInt();
}

void MainWindow::summery_view()
{
    sumModel->removeRows(0,sumModel->rowCount());
    ui->tableView_summery->setModel(sumModel);
    ui->tableView_summery->verticalHeader()->resetDefaultSectionSize();
    ui->tableView_summery->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_summery->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView_summery->verticalHeader()->setDefaultSectionSize(ui->tableView_summery->verticalHeader()->defaultSectionSize()*3.5);
    ui->tableView_summery->horizontalHeader()->setVisible(false);
    ui->tableView_summery->verticalHeader()->setVisible(false);
    ui->tableView_summery->setItemDelegate(&sum_del);
    ui->tableView_summery->setEditTriggers(QAbstractItemView::NoEditTriggers);
    metaProxy->setFilterRegExp("");
    metaProxy->setFilterFixedString("");
    contentProxy->setFilterRegExp("");
    scheduleProxy->setFilterFixedString("");

    QString sport,weekID;
    QStringList sumValues;
    int rowcount;

    work_sum.fill(0);
    dur_sum.fill(0);
    dist_sum.fill(0.0);
    stress_sum.fill(0);

    QStringList sportList = settings::get_listValues("Sport");
    QString selSaison = ui->comboBox_saisonName->currentText();

    if(isWeekMode)
    {
        scheduleProxy->setFilterRegExp("\\b"+weeknumber+"\\b");
        scheduleProxy->setFilterKeyColumn(0);
        rowcount = scheduleProxy->rowCount();

        if(rowcount > 0)
        {
            for(int i = 0; i < rowcount; ++i)
            {
                sport = scheduleProxy->data(scheduleProxy->index(i,3)).toString().trimmed();
                if(!sport.contains(settings::isOther))
                {
                    this->week_summery(0,i);
                    this->week_summery(sportList.indexOf(sport)+1,i);
                }
            }

            for(int i = 0; i < sportList.count(); ++i)
            {
                if(work_sum[i] > 0)
                {
                    sumValues << this->set_summeryString(i,isWeekMode);
                }
            }
        }

        QStringList weekInfo = weeknumber.split("_");
        QString year = weekInfo.at(1);
        QString week = weekInfo.at(0);
        QDate firstday,calcDay;
        calcDay.setDate(year.toInt(),1,1);
        firstday = calcDay.addDays(week.toInt()*7).addDays(1 - calcDay.dayOfWeek());
        ui->comboBox_saisonName->setCurrentText(workSchedule->saison_atDate(firstday));
        ui->label_selWeek->setText("Week: "+weeknumber+" - Phase: " +workSchedule->get_weekPhase(firstday,false));
    }
    else
    {
        QString work,dura,dist,stress;
        int sportUseSum = sportUse+1;
        avgModel->clear();
        avgModel->setColumnCount(3);
        avgModel->setRowCount(sportUseSum);
        avgModel->setHorizontalHeaderLabels(avgHeader);

        int metaRowCount = 0;
        int phaseWeeks = 0;

        metaProxy->setFilterFixedString(selSaison);
        metaProxy->setFilterKeyColumn(0);

        if(phaseFilterID > 1)
        {

            metaProxyFilter->setFilterFixedString(phaseFilter);
            metaProxyFilter->setFilterKeyColumn(3);
        }
        metaProxyFilter->sort(1);
        metaRowCount = metaProxyFilter->rowCount();

        if(metaRowCount > 0)
        {
            phaseWeeks = metaRowCount;
            for(int row = 0; row < metaRowCount; ++row)
            {
                weekID = metaProxyFilter->data(metaProxyFilter->index(row,2)).toString();
                contentProxy->setFilterRegExp("\\b"+weekID+"\\b");
                contentProxy->setFilterKeyColumn(1);

                for(int col = 0; col < sportUseSum; ++col)
                {
                    sumValues = contentProxy->data(contentProxy->index(0,col+2)).toString().split("-");
                    work = sumValues.at(0);
                    dist = sumValues.at(1);
                    dura = sumValues.at(2);
                    stress = sumValues.at(3);

                    work_sum[col] = work_sum[col] + work.toInt();
                    dur_sum[col] = dur_sum[col] + (this->get_timesec(dura)*60);
                    dist_sum[col] = dist_sum[col] + dist.toDouble();
                    stress_sum[col] = stress_sum[col] + stress.toInt();
                }
                contentProxy->invalidate();
                sumValues.clear();
            }
        }
        else
        {
            phaseWeeks = 1;
        }

        for(int i = 0; i < sportUse; ++i)
        {
            avgModel->setData(avgModel->index(i,0,QModelIndex()),sportList.at(i));
            avgModel->setData(avgModel->index(i,1,QModelIndex()),set_doubleValue(work_sum[i]/phaseWeeks,false));
            avgModel->setData(avgModel->index(i,2,QModelIndex()),set_time(dur_sum[i]/60/phaseWeeks));
            avgModel->setData(avgModel->index(i,3,QModelIndex()),set_doubleValue(dist_sum[i]/phaseWeeks,false));
        }
        avgModel->setData(avgModel->index(sportUse,0,QModelIndex()),"Phase");
        avgModel->setData(avgModel->index(sportUse,1,QModelIndex()),set_doubleValue(work_sum[sportUse]/phaseWeeks,false));
        avgModel->setData(avgModel->index(sportUse,2,QModelIndex()),set_time(dur_sum[sportUse]/60/phaseWeeks));
        avgModel->setData(avgModel->index(sportUse,3,QModelIndex()),set_doubleValue(dist_sum[sportUse]/phaseWeeks,false));

        sumValues << this->set_summeryString(0,isWeekMode);

        for(int i = 1; i < sportUseSum; ++i)
        {
            sumValues << this->set_summeryString(i,isWeekMode);
        }

        if(phaseFilterID == 1)
        {
            ui->label_selWeek->setText("All Phases - Weeks: "+QString::number(metaRowCount));
        }
        else
        {
            ui->label_selWeek->setText("Phase: "+ phaseFilter + " - Weeks: "+QString::number(metaRowCount));
        }
    }

    for(int i = 0; i < sumValues.count(); ++i)
    {
        rowcount = sumModel->rowCount();
        sumModel->insertRow(rowcount,QModelIndex());
        sumModel->setData(sumModel->index(i,0,QModelIndex()),sumValues.at(i));
    }
}

void MainWindow::workout_calendar()
{
    QModelIndex cal_index;
    QDate currentdate = QDate::currentDate();
    QDate workout_date = currentdate;
    QString delimiter = "#";
    QString w_connect = " - ";
    QString stdConnect;
    QString weekValue,cal_value,phase_value;
    int dayofweek = currentdate.dayOfWeek();
    QStringList sportuseList = settings::get_listValues("Sportuse");
    metaProxy->setFilterRegExp("");
    metaProxyFilter->setFilterRegExp("");
    scheduleProxy->setFilterFixedString("");

    if(isWeekMode)
    {
        calendarModel->setRowCount(weekRange);
        calendarModel->setColumnCount(8);
        calendarModel->setHorizontalHeaderLabels(cal_header);
        ui->tableView_cal->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView_cal->setItemDelegate(&calender_del);
        scheduleProxy->sort(2);
        int offset = (1 - dayofweek) + (weekDays*weekCounter);

        for(int week = 0; week < weekRange; ++week)
        {
            weekValue = QString::number(currentdate.addDays(offset).weekNumber()) +"_"+ QString::number(currentdate.addDays(offset).year());
            metaProxy->setFilterRegExp("\\b"+weekValue+"\\b");
            metaProxy->setFilterKeyColumn(2);

            for(int day = 0; day < 8 ; ++day)
            {
                workout_date = currentdate.addDays(offset);
                scheduleProxy->setFilterFixedString(workout_date.toString("dd.MM.yyyy"));
                scheduleProxy->setFilterKeyColumn(1);

                for(int wa = 0; wa < scheduleProxy->rowCount(); ++wa)
                {
                    if(scheduleProxy->rowCount() > 0)
                    {
                        stdConnect = scheduleProxy->data(scheduleProxy->index(wa,10)).toString().isEmpty() ? "" : "*";
                        cal_value = cal_value + (scheduleProxy->data(scheduleProxy->index(wa,3)).toString() + w_connect);
                        cal_value = cal_value + (scheduleProxy->data(scheduleProxy->index(wa,4)).toString() + stdConnect+"\n");
                        cal_value = cal_value + (scheduleProxy->data(scheduleProxy->index(wa,6)).toString().left(5) + w_connect);
                        cal_value = cal_value + (scheduleProxy->data(scheduleProxy->index(wa,7)).toString() + " km" + delimiter);
                    }
                    else
                    {
                        cal_value = QString();
                    }
                }
                cal_index = calendarModel->index(week,day,QModelIndex());

                if(day == 0)
                {
                    if(metaProxy->rowCount() > 0)
                    {
                        phase_value = workSchedule->get_weekPhase(workout_date,true);
                    }
                    else
                    {
                        phase_value = generalValues->value("empty");
                    }
                    calendarModel->setData(cal_index,weekValue + delimiter + phase_value);
                }
                else
                {
                    calendarModel->setData(cal_index,workout_date.toString("dd MMM yy") + delimiter + cal_value);
                    ++offset;
                }
                cal_value = phase_value = QString();
            }
        }
    }
    else
    {
        year_header.clear();
        QString temp;
        year_header << "Week";
        for(int i = 0; i < sportUse;++i)
        {
            temp = sportuseList.at(i);
            year_header << temp.toUpper();
        }
        year_header << generalValues->value("sum");

        calendarModel->setColumnCount(year_header.count());
        calendarModel->setHorizontalHeaderLabels(year_header);
        ui->tableView_cal->setItemDelegate(&week_del);
        QString weekInfo,weekID;
        QString selSaison = ui->comboBox_saisonName->currentText();
        QString empty = "0-0-00:00:00-0";
        int weekoffset;

        metaProxy->setFilterFixedString(selSaison);
        metaProxy->setFilterKeyColumn(0);

        if(phaseFilterID == 1)
        {
            ui->tableView_cal->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
            weekoffset = settings::get_fontValue("weekOffSet");
            calendarModel->setRowCount(weekRange);
        }
        else
        {
            ui->tableView_cal->verticalHeader()->resetDefaultSectionSize();
            ui->tableView_cal->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
            ui->tableView_cal->verticalHeader()->setDefaultSectionSize(ui->tableView_cal->verticalHeader()->defaultSectionSize()*2.5);
            metaProxyFilter->setFilterFixedString(phaseFilter);
            metaProxyFilter->setFilterKeyColumn(3);
            weekoffset = metaProxyFilter->rowCount();
            calendarModel->setRowCount(weekoffset);
        }
        metaProxyFilter->sort(1);

        for(int week = weekpos,i=0; week < weekpos+weekoffset;++week,++i)
        {
          weekID = metaProxyFilter->data(metaProxyFilter->index(week,2)).toString();
          contentProxy->setFilterRegExp("\\b"+weekID+"\\b");
          contentProxy->setFilterKeyColumn(1);

          for(int col = 0; col < sportUse+2;++col)
          {
              cal_index = calendarModel->index(i,col,QModelIndex());
              if(contentProxy->rowCount() > 0)
              {
                  if(col == 0)
                  {
                    weekInfo = metaProxyFilter->data(metaProxyFilter->index(week,1)).toString() +"#"+
                               metaProxyFilter->data(metaProxyFilter->index(week,2)).toString() +"#"+
                               metaProxyFilter->data(metaProxyFilter->index(week,4)).toString() +"#"+
                               metaProxyFilter->data(metaProxyFilter->index(week,3)).toString() +"#"+
                               metaProxyFilter->data(metaProxyFilter->index(week,5)).toString();
                  }
                  else
                  {
                    weekInfo = contentProxy->data(contentProxy->index(0,col+1)).toString();
                  }
              }
              else
              {
                weekInfo = empty;
              }
              calendarModel->setData(cal_index,weekInfo);
          }
          contentProxy->setFilterRegExp("");
        }
     }
}

void MainWindow::refresh_model()
{
    this->summery_view();
    this->workout_calendar();
}

QString MainWindow::get_weekRange()
{
    QString display_weeks;
    int phaseStart;
    if(isWeekMode)
    {
        if(weekCounter != 0)
        {
            display_weeks = QString::number(firstdayofweek.addDays(weekDays*weekCounter).weekNumber()) + " - " +
                            QString::number(firstdayofweek.addDays(((weekRange-1)*weekDays)+(weekDays*weekCounter)).weekNumber());
        }
        else
        {
            display_weeks = QString::number(firstdayofweek.addDays(weekDays*weekCounter).weekNumber()) + " - " +
                            QString::number(firstdayofweek.addDays((weekRange-1)*weekDays).weekNumber());
        }
    }
    else
    {
        if(phaseFilterID == 1)
        {
            display_weeks = QString::number(weekpos+1) + " - " + QString::number(weekpos + settings::get_fontValue("weekOffSet"));
        }
        else
        {
            metaProxy->setFilterRegExp("\\b"+workSchedule->get_currSaison()+"\\b");
            metaProxy->setFilterKeyColumn(0);
            metaProxyFilter->setFilterFixedString(phaseFilter);
            metaProxyFilter->setFilterKeyColumn(3);
            metaProxy->sort(1);
            phaseStart = metaProxyFilter->data(metaProxyFilter->index(0,1)).toInt();
            display_weeks = QString::number(phaseStart) + " - " + QString::number(phaseStart + (metaProxyFilter->rowCount()-1));
            metaProxyFilter->invalidate();
            metaProxy->invalidate();
        }
    }

    return display_weeks;
}

void MainWindow::set_buttons(bool set_value)
{
    ui->toolButton_weekMinus->setEnabled(set_value);
    ui->toolButton_weekCurrent->setEnabled(set_value);
}

void MainWindow::set_calender()
{
    if(weekCounter == 0)
    {
        ui->calendarWidget->setSelectedDate(QDate::currentDate());
    }
    else
    {
        ui->calendarWidget->setSelectedDate(selectedDate.addDays((1-selectedDate.currentDate().dayOfWeek())+weekDays*weekCounter));
    }

    weeknumber = QString::number(ui->calendarWidget->selectedDate().weekNumber())+"_"+QString::number(ui->calendarWidget->selectedDate().year());
    this->summery_view();
}

void MainWindow::set_phaseButtons()
{
    QStringList phases;
    phases << "All" <<  settings::get_listValues("Phase");
    QHBoxLayout *layout = new QHBoxLayout(ui->frame_phases);
    layout->setContentsMargins(1,1,1,1);
    layout->addSpacing(2);
    phaseGroup = new QButtonGroup(this);

    for(int i = 0;i < phases.count(); ++i)
    {
        QToolButton *pButton = new QToolButton();
        pButton->setText(phases.at(i));
        pButton->setFixedHeight(25);
        pButton->setFixedWidth(50);
        pButton->setAutoRaise(true);
        pButton->setCheckable(true);
        //pButton->setStyleSheet(buttonStyle);
        QFrame *sline = new QFrame();
        sline->setFrameShape(QFrame::VLine);
        sline->setFrameShadow(QFrame::Sunken);
        layout->addWidget(sline);
        layout->addWidget(pButton);
        phaseGroup->addButton(pButton,i+1);
    }
    phaseGroup->button(1)->setChecked(true);
}

void MainWindow::refresh_saisonInfo()
{
    ui->comboBox_saisonName->clear();

    for(int i = 0; i < workSchedule->saisonsModel->rowCount(); ++i)
    {
        ui->comboBox_saisonName->addItem(workSchedule->saisonsModel->data(workSchedule->saisonsModel->index(i,0)).toString());
    }
    ui->comboBox_saisonName->setEnabled(false);
    saisonWeeks = workSchedule->get_saisonInfo(ui->comboBox_saisonName->currentText(),"weeks").toInt();
}

//ACTIONS**********************************************************************

void MainWindow::on_actionNew_triggered()
{
    int dialog_code;
    if(selModule == PLANER)
    {
        if(isWeekMode)
        {
            day_popup day_pop(this,QDate::currentDate(),workSchedule);
            day_pop.setModal(true);
            dialog_code = day_pop.exec();
            if(dialog_code == QDialog::Rejected)
            {
                ui->actionSave->setEnabled(workSchedule->get_isUpdated());
                ui->actionPMC->setEnabled(true);
            }
        }
    }
    if(selModule == FOOD)
    {
        QDate selDate = ui->calendarWidget_Food->selectedDate();
        foodPlan->insert_newWeek(selDate.addDays(1 - selDate.dayOfWeek()));
        ui->listWidget_weekPlans->clear();
        ui->listWidget_weekPlans->addItems(foodPlan->planList);
    }
}

void MainWindow::on_actionStress_Calculator_triggered()
{
    Dialog_stresscalc stressCalc(this);
    stressCalc.setModal(true);
    stressCalc.exec();
}

void MainWindow::on_actionSave_triggered()
{
    if(selModule == PLANER)
    {
        if(isWeekMode)
        {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          tr("Save Workouts"),
                                          "Save Week Workout Schedule?",
                                          QMessageBox::Yes|QMessageBox::No
                                          );
            if (reply == QMessageBox::Yes)
            {
                workSchedule->save_dayWorkouts();
                workSchedule->set_isUpdated(false);
                ui->actionSave->setEnabled(false);
            }
        }
        else
        {
            QMessageBox::StandardButton reply;
                reply = QMessageBox::question(this,
                                              tr("Save Schedule"),
                                              "Save Current Year Schedule?",
                                              QMessageBox::Yes|QMessageBox::No
                                              );
            if (reply == QMessageBox::Yes)
            {
                workSchedule->save_weekPlan();
                workSchedule->set_isUpdated(false);
                ui->actionSave->setEnabled(false);
            }
        }
    }
    else if(selModule == EDITOR)
    {
        ui->progressBar_fileState->setValue(10);
        QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          tr("Save File"),
                                          "Save Changes to Golden Cheetah?",
                                          QMessageBox::Yes|QMessageBox::No
                                          );
        if (reply == QMessageBox::Yes)
        {
            if(curr_activity->get_sport() == settings::isSwim)
            {
                curr_activity->updateXDataModel();
            }
            else
            {
                curr_activity->updateIntModel(2,1);
            }
            ui->progressBar_fileState->setValue(25);
            curr_activity->writeChangedData();
            ui->progressBar_fileState->setValue(75);
        }
        ui->progressBar_fileState->setValue(100);
        QTimer::singleShot(2000,ui->progressBar_fileState,SLOT(reset()));
    }
    else if(selModule == FOOD)
    {
        QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          tr("Save Food Plan"),
                                          "Save Food Plan and Meal List?",
                                          QMessageBox::Yes|QMessageBox::No
                                          );
        if (reply == QMessageBox::Yes)
        {
            foodPlan->write_foodPlan();
            ui->actionSave->setEnabled(false);
        }
    }
}

void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
   weeknumber = QString::number(date.weekNumber())+"_"+QString::number(date.year());
   this->summery_view();
}

void MainWindow:: on_tableView_cal_clicked(const QModelIndex &index)
{
    int dialog_code;

    if(isWeekMode)
    {
        if(index.column() != 0)
        {
            QString getdate = calendarModel->data(index,Qt::DisplayRole).toString().left(9);
            QDate selectDate = QDate::fromString(getdate,"dd MMM yy").addYears(100);
            day_popup day_pop(this,selectDate,workSchedule);
            day_pop.setModal(true);
            dialog_code = day_pop.exec();
            if(dialog_code == QDialog::Rejected)
            {
                ui->actionSave->setEnabled(workSchedule->get_isUpdated());
                foodPlan->update_sumBySchedule(selectDate);
            }
        }
        else
        {
            QString selected_week =  calendarModel->data(index,Qt::DisplayRole).toString();
            weeknumber = selected_week.split("#").at(0);

            this->summery_view();

            week_popup week_pop(this,selected_week,workSchedule);
            week_pop.setModal(true);
            dialog_code = week_pop.exec();

            if(dialog_code == QDialog::Accepted)
            {
                Dialog_week_copy week_copy(this,selected_week,workSchedule,true);
                week_copy.setModal(true);
                dialog_code = week_copy.exec();

                if(dialog_code == QDialog::Accepted)
                {
                    this->workout_calendar();
                    this->set_calender();
                    ui->actionSave->setEnabled(workSchedule->get_isUpdated());
                }
            }

            if(dialog_code == QDialog::Rejected)
            {
                this->set_calender();
            }
        }
    }
    else
    {
        if(index.column() == 0)
        {
            QString selected_week = calendarModel->data(index,Qt::DisplayRole).toString();
            Dialog_addweek new_week(this,selected_week,workSchedule);
            new_week.setModal(true);
            int dialog_code = new_week.exec();

            if(dialog_code == QDialog::Rejected)
            {
                ui->actionSave->setEnabled(workSchedule->get_isUpdated());
            }
        }
    }
}


void MainWindow::on_toolButton_weekCurrent_clicked()
{
    if(isWeekMode)
    {
        weekCounter = 0;
        this->set_calender();
        this->set_buttons(false);
        emit workSchedule->workout_schedule->layoutChanged();
    }
    else
    {
        weekpos = 0;
        ui->toolButton_weekFour->setEnabled(true);
        ui->toolButton_weekPlus->setEnabled(true);
        this->set_buttons(false);
        this->workout_calendar();
    }
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::on_toolButton_weekMinus_clicked()
{
    if(isWeekMode)
    {
        --weekCounter;
        if(weekCounter == 0)
        {
           this->set_buttons(false);
        }
        this->set_calender();
        emit workSchedule->workout_schedule->layoutChanged();
    }
    else
    {
        --weekpos;
        if(weekpos == 0)
        {
            this->set_buttons(false);
        }
        if(weekpos < 52)
        {
            ui->toolButton_weekFour->setEnabled(true);
            ui->toolButton_weekPlus->setEnabled(true);
        }
        this->workout_calendar();
    }
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::on_toolButton_weekPlus_clicked()
{
    if(isWeekMode)
    {
        ++weekCounter;
        this->set_calender();
        this->set_buttons(true);
        emit workSchedule->workout_schedule->layoutChanged();
    }
    else
    {
        ++weekpos;
        if(weekpos + settings::get_fontValue("weekOffSet") == saisonWeeks)
        {
            ui->toolButton_weekFour->setEnabled(false);
            ui->toolButton_weekPlus->setEnabled(false);
            this->workout_calendar();
        }
        else
        {
            this->set_buttons(true);
            this->workout_calendar();
        }
    }
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::on_toolButton_weekFour_clicked()
{
    if(isWeekMode)
    {
        weekCounter = weekCounter+4;
        this->set_calender();
        this->set_buttons(true);
        emit workSchedule->workout_schedule->layoutChanged();
    }
    else
    {
        int offSet = settings::get_fontValue("weekOffSet");
        weekpos = weekpos+4;
        if(weekpos + offSet >= saisonWeeks)
        {
            weekpos = saisonWeeks-offSet;
            ui->toolButton_weekFour->setEnabled(false);
            ui->toolButton_weekPlus->setEnabled(false);
            this->workout_calendar();
        }
        else
        {
            this->set_buttons(true);
            this->workout_calendar();
        }
    }
    ui->label_month->setText("Week " + this->get_weekRange());
}
void MainWindow::on_actionExport_to_Golden_Cheetah_triggered()
{
    Dialog_export export_workout(this,workSchedule);
    export_workout.setModal(true);
    export_workout.exec();
}

//EDITOR Functions *****************************************************************************

void MainWindow::select_activityFile()
{
    QMessageBox::StandardButton reply;
    QString filename = QFileDialog::getOpenFileName(
                this,
                tr("Select GC JSON File"),
                gcValues->value("actpath"),
                "JSON Files (*.json)"
                );

    if(filename != "")
    {
        reply = QMessageBox::question(this,
                                  tr("Open Selected File!"),
                                  filename,
                                  QMessageBox::Yes|QMessageBox::No
                                  );
        if (reply == QMessageBox::Yes)
        {
            this->loadfile(filename);
        }
    }
}

void MainWindow::loadfile(const QString &filename)
{
    QFile file(filename);
    QFileInfo fileinfo(filename);
    QString filecontent;

    if(fileinfo.suffix() == "json")
    {
        if (!file.open(QFile::ReadOnly | QFile::Text))
        {
            QMessageBox::warning(this, tr("Application"),
                                     tr("Cannot read file %1:\n%2.")
                                     .arg(filename)
                                     .arg(file.errorString()));
           return;
        }
        filecontent = file.readAll();
        curr_activity = new Activity(filecontent,true);
        actLoaded = true;
        file.close();

        ui->actionSelect_File->setEnabled(false);
        ui->actionReset->setEnabled(true);
        intSelect_del.sport = avgSelect_del.sport = curr_activity->get_sport();
        this->set_menuItems(true,false);

        this->init_editorViews();
        this->update_infoModel();
     }
}

void MainWindow::init_editorViews()
{
    QStringList infoHeader = settings::get_listValues("JsonFile");
    infoModel = new QStandardItemModel(infoHeader.count(),1,this);
    infoModel->setVerticalHeaderLabels(infoHeader);

    ui->treeView_files->setModel(fileModel);
    ui->treeView_files->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView_files->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView_files->header()->setVisible(false);
    ui->treeView_files->setItemDelegate(&fileList_del);
    ui->treeView_files->hideColumn(4);

    ui->tableView_actInfo->setModel(infoModel);
    ui->tableView_actInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_actInfo->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_actInfo->horizontalHeader()->setVisible(false);
    ui->tableView_actInfo->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_actInfo->verticalHeader()->setSectionsClickable(false);
    ui->tableView_actInfo->setItemDelegate(&avgSelect_del);
    ui->tableView_actInfo->setFixedHeight(infoHeader.count()*25);

    ui->treeView_intervall->setModel(curr_activity->intTreeModel);
    ui->treeView_intervall->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->treeView_intervall->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeView_intervall->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->treeView_intervall->setItemDelegate(&tree_del);

    treeSelection = ui->treeView_intervall->selectionModel();
    connect(treeSelection,SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(setSelectedIntRow(QModelIndex)));

    ui->tableView_selectInt->setModel(curr_activity->selItemModel);
    ui->tableView_selectInt->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui->tableView_selectInt->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_selectInt->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView_selectInt->verticalHeader()->setFixedWidth(100);
    ui->tableView_selectInt->verticalHeader()->setMaximumSectionSize(25);
    ui->tableView_selectInt->verticalHeader()->setSectionsClickable(false);
    ui->tableView_selectInt->horizontalHeader()->setVisible(false);
    ui->tableView_selectInt->hideColumn(1);
    ui->tableView_selectInt->setItemDelegate(&intSelect_del);

    ui->tableView_avgValues->setModel(curr_activity->avgModel);
    ui->tableView_avgValues->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_avgValues->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_avgValues->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->tableView_avgValues->verticalHeader()->setFixedWidth(100);
    ui->tableView_avgValues->verticalHeader()->setMaximumSectionSize(25);
    ui->tableView_avgValues->verticalHeader()->setSectionsClickable(false);
    ui->tableView_avgValues->horizontalHeader()->setVisible(false);
    ui->tableView_avgValues->setItemDelegate(&avgSelect_del);
    this->init_controlStyleSheets();
}

void MainWindow::init_controlStyleSheets()
{
    QString viewBackground = "background-color: #e6e6e6";

    ui->tableView_selectInt->setStyleSheet(viewBackground);
    ui->tableView_avgValues->setStyleSheet(viewBackground);
    ui->treeView_intervall->setStyleSheet(viewBackground);
    ui->treeView_files->setStyleSheet(viewBackground);
    ui->tableView_cal->setStyleSheet(viewBackground);
    ui->tableView_summery->setStyleSheet(viewBackground);
    /*
    ui->toolButton_addSelect->setStyleSheet(buttonStyle);
    ui->toolButton_clearSelect->setStyleSheet(buttonStyle);
    ui->toolButton_add->setStyleSheet(buttonStyle);
    ui->toolButton_delete->setStyleSheet(buttonStyle);
    ui->toolButton_update->setStyleSheet(buttonStyle);
    ui->toolButton_downInt->setStyleSheet(buttonStyle);
    ui->toolButton_upInt->setStyleSheet(buttonStyle);
    planMode->setStyleSheet(buttonStyle);
    */
}

void MainWindow::update_infoModel()
{
    for(int i = 0; i < infoModel->rowCount();++i)
    {
        infoModel->setData(infoModel->index(i,0),curr_activity->ride_info.value(settings::get_listValues("JsonFile").at(i)));
    }
    ui->actionSave->setEnabled(true);
}

void MainWindow::setSelectedIntRow(QModelIndex index)
{
    QStringList intLabel;
    intLabel << "Swim Lap" << "Interval";
    bool isInt = true;
    bool isSwim = false;
    if(curr_activity->get_sport() == settings::isSwim) isSwim = true;

    treeSelection->select(index,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    QString lapIdent = treeSelection->selectedRows(0).at(0).data().toString().trimmed();
    curr_activity->set_selectedItem(treeSelection);

    if(isSwim)
    {
        if(treeSelection->selectedRows(2).at(0).data().toInt() == 1)
        {
            isInt = false;
        }
        else if(curr_activity->intTreeModel->itemFromIndex(index)->parent() == nullptr || lapIdent.contains(generalValues->value("breakname")))
        {
            isInt = true;
        }
        else
        {
            isInt = false;
        }
        curr_activity->set_editRow(lapIdent,isInt);
        curr_activity->showSwimLap(isInt);
    }
    else
    {
        if(curr_activity->get_sport() != settings::isTria)
        {
            ui->horizontalSlider_factor->setEnabled(true);
        }
        curr_activity->set_editRow(lapIdent,isInt);
        curr_activity->showInterval(true);
    }

    if(curr_activity->intTreeModel->itemFromIndex(index)->parent() == nullptr)
    {
        this->set_speedValues(index.row());
    }

    intSelect_del.intType = isInt;
    ui->label_lapType->setText(intLabel.at(isInt));
    ui->tableView_selectInt->setCurrentIndex(curr_activity->selItemModel->index(0,0));
}

void MainWindow::selectAvgValues(QModelIndex index, int avgCol)
{
    treeSelection->select(index,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    QStandardItem *avgItem = curr_activity->intTreeModel->itemFromIndex(treeSelection->selectedRows(avgCol).at(0));

    bool checkAvg = avgItem->data().toBool();
    curr_activity->set_selectedItem(treeSelection);

    if(checkAvg == false)
    {
        curr_activity->avgItems.insert(index.row(),index);
        curr_activity->intTreeModel->setData(index,"+");
        curr_activity->intTreeModel->setData(index,1,Qt::UserRole+1);
        curr_activity->set_avgValues(++avgCounter,1);
    }
    else
    {
        curr_activity->avgItems.remove(index.row());
        curr_activity->intTreeModel->setData(index,"-");
        curr_activity->intTreeModel->setData(index,0,Qt::UserRole+1);
        curr_activity->set_avgValues(--avgCounter,-1);
    }

    if(avgCounter > 0)
    {
        ui->toolButton_addSelect->setEnabled(true);
        ui->toolButton_clearSelect->setEnabled(true);
        ui->toolButton_clearContent->setEnabled(true);
    }
    else
    {
        ui->toolButton_addSelect->setEnabled(false);
        ui->toolButton_clearSelect->setEnabled(false);
        ui->toolButton_clearContent->setEnabled(false);
        ui->toolButton_sync->setEnabled(false);
    }

    treeSelection->clearSelection();
}

void MainWindow::on_treeView_intervall_clicked(const QModelIndex &index)
{
    int avgCol = curr_activity->intTreeModel->columnCount()-1;

    if(index.column() == avgCol)
    {
        this->selectAvgValues(index,avgCol);
    }
    else
    {
        treeSelection->setCurrentIndex(index,QItemSelectionModel::Select);
    }
}


void MainWindow::set_polishValues(int lap,double intDist, double avgSpeed,double factor,int ypos)
{

    for(int i = 0; i < speedValues.count(); ++i)
    {
        if(lap == 0 && i < 5)
        {
            polishValues[i] = speedValues[i];
        }
        else
        {
            polishValues[i] = curr_activity->polish_SpeedValues(speedValues[i],avgSpeed,0.10-factor,true);
        }
    }

    this->set_speedPlot(avgSpeed,intDist,ypos);
}

void MainWindow::on_horizontalSlider_factor_valueChanged(int value)
{
    ui->label_factorValue->setText(QString::number(10-value) + "%");
    double factor = static_cast<double>(value)/100;
    curr_activity->set_polishFactor(0.1-factor);

    double intSpeed = treeSelection->selectedRows(6).at(0).data().toDouble();
    double intDist = treeSelection->selectedRows(4).at(0).data().toDouble();

    this->set_polishValues(ui->treeView_intervall->currentIndex().row(),intDist,intSpeed,factor,0);
    rangeMinMax[0] = curr_activity->polish_SpeedValues(1.0,intSpeed,0.1-factor,false);
    rangeMinMax[1] = curr_activity->polish_SpeedValues(50.0,intSpeed,0.1-factor,false);

    ui->lineEdit_polMin->setText(QString::number(rangeMinMax[0]));
    ui->lineEdit_polMax->setText(QString::number(rangeMinMax[1]));
}

void MainWindow::resetPlot()
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();
    speedValues.resize(100);
    secTicker.resize(100);
    secTicker.fill(0);
    speedValues.fill(0);
    QCPGraph *resetLine = ui->widget_plot->addGraph();
    resetLine->setPen(QPen(QColor(255,255,255),2));
    resetLine->setData(secTicker,speedValues);
    resetLine->setName("-");
    ui->widget_plot->xAxis->setRange(0,100);
    ui->widget_plot->xAxis2->setRange(0,1);
    ui->widget_plot->yAxis->setRange(0,5);
    ui->widget_plot->plotLayout()->setRowStretchFactor(1,0.0001);
    ui->widget_plot->replot();
}

void MainWindow::set_speedValues(int index)
{
    int lapLen;
    int vSize;
    double current = 0;
    double second = 0;
    int yPos = 0;
    double intSpeed,intDist;

    int start = curr_activity->intModel->data(curr_activity->intModel->index(index,1,QModelIndex())).toInt();
    int stop = curr_activity->intModel->data(curr_activity->intModel->index(index,2,QModelIndex())).toInt();
    speedMinMax.resize(2);
    secondMinMax.resize(2);
    rangeMinMax.resize(2);
    speedMinMax[0] = 40.0;
    speedMinMax[1] = 0.0;
    secondMinMax[0] = 10.0;
    secondMinMax[1] = 0.0;
    lapLen = stop-start;
    vSize = lapLen+1;
    polishValues.clear();

    speedValues.resize(vSize);
    secondValues.resize(vSize);
    polishValues.resize(vSize);
    secTicker.resize(vSize);

    for(int i = start, pos=0; i <= stop; ++i,++pos)
    {
        current = curr_activity->sampSpeed[i];
        second = curr_activity->sampSecond[i];
        secTicker[pos] = pos;
        speedValues[pos] = current;
        secondValues[pos] = second;
        if(speedMinMax[0] > current) rangeMinMax[0] = speedMinMax[0] = current;
        if(speedMinMax[1] < current) rangeMinMax[1] = speedMinMax[1] = current;
        if(secondMinMax[0] > second) secondMinMax[0] = second;
        if(secondMinMax[1] < second) secondMinMax[1] = second;
    }

    if(curr_activity->get_sport() != settings::isSwim)
    {
        intSpeed = treeSelection->selectedRows(6).at(0).data().toDouble();
        intDist = treeSelection->selectedRows(4).at(0).data().toDouble();

        if(curr_activity->get_sport() == settings::isRun)
        {
            ui->horizontalSlider_factor->setEnabled(true);
            double factor = static_cast<double>(ui->horizontalSlider_factor->value())/100;
            this->set_polishValues(index,intDist,intSpeed,factor,yPos);
        }
        if(curr_activity->isIndoor)
        {
            ui->horizontalSlider_factor->setEnabled(false);
            yPos = 1;
        }
    }
    else
    {
        ui->horizontalSlider_factor->setEnabled(false);
        intSpeed = treeSelection->selectedRows(7).at(0).data().toDouble();
        intDist = treeSelection->selectedRows(3).at(0).data().toDouble();
        yPos = 2;
    }

    this->set_speedPlot(intSpeed,intDist,yPos);
}

void MainWindow::set_speedgraph()
{
    QFont plotFont;
    plotFont.setBold(true);
    plotFont.setPointSize(8);
    y2Label << "HF" << "Watts" << "Strokes";

    ui->widget_plot->xAxis->setLabel("Seconds");
    ui->widget_plot->xAxis->setLabelFont(plotFont);
    ui->widget_plot->xAxis2->setVisible(true);
    ui->widget_plot->xAxis2->setLabelFont(plotFont);
    ui->widget_plot->xAxis2->setLabel("Distance");
    ui->widget_plot->xAxis2->setTickLabels(true);
    ui->widget_plot->yAxis->setLabel("Speed");
    ui->widget_plot->yAxis->setLabelFont(plotFont);
    ui->widget_plot->yAxis2->setVisible(true);
    ui->widget_plot->yAxis2->setLabelFont(plotFont);
    ui->widget_plot->legend->setVisible(true);
    ui->widget_plot->legend->setFont(plotFont);

    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    ui->widget_plot->plotLayout()->addElement(1,0,subLayout);
    subLayout->setMargins(QMargins(200,0,200,5));
    subLayout->addElement(0,0,ui->widget_plot->legend);
}

void MainWindow::set_speedPlot(double avgSpeed,double intdist,int yPos)
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();
    ui->widget_plot->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->widget_plot->plotLayout()->setRowStretchFactor(1,0.0001);

    QCPGraph *speedLine = ui->widget_plot->addGraph();
    speedLine->setName("Speed");
    speedLine->setLineStyle(QCPGraph::lsLine);
    speedLine->setData(secTicker,speedValues);
    speedLine->setPen(QPen(QColor(0,255,0),2));

    QCPGraph *secondLine = ui->widget_plot->addGraph(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    secondLine->setName(y2Label.at(yPos));
    secondLine->setLineStyle(QCPGraph::lsLine);
    secondLine->setData(secTicker,secondValues);
    secondLine->setPen(QPen(QColor(255,0,0),2));

    QCPItemLine *avgLine = new QCPItemLine(ui->widget_plot);
    avgLine->start->setCoords(0,avgSpeed);
    avgLine->end->setCoords(speedValues.count(),avgSpeed);
    avgLine->setPen(QPen(QColor(0,0,255),2));

    QCPGraph *avgLineP = ui->widget_plot->addGraph();
    avgLineP->setName("Avg Speed");
    avgLineP->setPen(QPen(QColor(0,0,255),2));

    if(curr_activity->get_sport() != settings::isSwim)
    {
        QCPGraph *polishLine = ui->widget_plot->addGraph();
        polishLine->setName("Polished Speed");
        polishLine->setLineStyle(QCPGraph::lsLine);
        polishLine->setData(secTicker,polishValues);
        polishLine->setPen(QPen(QColor(255,200,0),2));

        QCPGraph *polishRangeP = ui->widget_plot->addGraph();
        polishRangeP->setName("Polish Range");
        polishRangeP->setPen(QPen(QColor(225,225,0),2));

        QCPItemRect *polishRange = new QCPItemRect(ui->widget_plot);
        polishRange->topLeft->setCoords(0,rangeMinMax[1]);
        polishRange->bottomRight->setCoords(speedValues.count(),rangeMinMax[0]);
        polishRange->setPen(QPen(QColor(225,225,0),2));
        polishRange->setBrush(QBrush(QColor(255,255,0,50)));
    }

    double yMin = 0,yMax = 0,y2Min = 0, y2Max = 0;

    if(speedMinMax[0] > 0) yMin = speedMinMax[0]*0.1;
    yMax =  speedMinMax[1]*0.1;

    if(secondMinMax[0] > 0) y2Min = secondMinMax[0]*0.1;
    y2Max = secondMinMax[1]*0.1;

    ui->widget_plot->xAxis->setRange(0,speedValues.count());
    ui->widget_plot->xAxis2->setRange(0,intdist);
    ui->widget_plot->yAxis->setRange(speedMinMax[0]-yMin,speedMinMax[1]+yMax);
    ui->widget_plot->yAxis2->setRange(secondMinMax[0]-y2Min,secondMinMax[1]+y2Max);

    ui->widget_plot->replot();
}

void MainWindow::fill_WorkoutContent()
{
    QStandardItemModel *avgModel = curr_activity->avgModel;
    QString content,newEntry,contentValue,label;
    content = ui->lineEdit_workContent->text();

    QString avgTime = avgModel->data(avgModel->index(1,0)).toString();
    QString avgPace = avgModel->data(avgModel->index(2,0)).toString();
    double dist = avgModel->data(avgModel->index(3,0)).toDouble();
    int time = 0;

    if(ui->radioButton_time->isChecked())
    {
        if(ui->checkBox_exact->isChecked())
        {
            time = this->get_timesec(avgTime);
        }
        else
        {
            time = (ceil(this->get_timesec(avgTime)/10.0)*10);
        }

        if(time >= 60)
        {
            label = "Min";
        }
        else
        {
            label = "Sec";
        }

        contentValue = this->set_time(time)+label;
    }

    if(ui->radioButton_distance->isChecked())
    {
        if(ui->checkBox_exact->isChecked())
        {
            dist = round(dist*1000)/1000.0;
        }
        else
        {
            dist = ceil(dist*10)/10.0;
        }

        if(dist < 1)
        {
            label = "M";
            dist = dist*1000.0;
        }
        else
        {
            label = "Km";
        }

        contentValue = QString::number(dist)+label;
    }

    if(curr_activity->get_sport() == settings::isSwim)
    {        
        if(avgCounter > 1)
        {
            newEntry = QString::number(avgCounter)+"x"+QString::number(dist)+"/"+avgTime;
        }
        else
        {
            newEntry = QString::number(dist)+"-"+avgTime;
        }
    }

    if(curr_activity->get_sport() == settings::isBike)
    {
        QString watts = avgModel->data(avgModel->index(4,0)).toString();

        if(avgCounter > 1)
        {
            newEntry = QString::number(avgCounter)+"x"+contentValue+"/" +watts+"W";
        }
        else
        {
            newEntry = contentValue+"-" +watts+"W";
        }
    }

    if(curr_activity->get_sport() == settings::isRun)
    {
        if(avgCounter > 1)
        {
            newEntry = QString::number(avgCounter)+"x"+contentValue+"-" +avgPace+"/km";
        }
        else
        {
            newEntry = contentValue+"-" +avgPace+"/km";
        }
    }

    if(ui->lineEdit_workContent->text() == "")
    {
        ui->lineEdit_workContent->setText(content+newEntry);
        ui->toolButton_clearContent->setEnabled(true);
    }
    else
    {
        ui->lineEdit_workContent->setText(content+" | "+newEntry);
    }

    curr_activity->set_workoutContent(ui->lineEdit_workContent->text());

}

void MainWindow::unselect_intRow(bool setToolButton)
{
    curr_activity->reset_avgSelection();
    avgCounter = 0;
    ui->toolButton_addSelect->setEnabled(setToolButton);
    ui->toolButton_clearSelect->setEnabled(setToolButton);
    ui->toolButton_clearContent->setEnabled(setToolButton);
    ui->toolButton_sync->setEnabled(setToolButton);
}

void MainWindow::on_toolButton_update_clicked()
{
    ui->treeView_intervall->setFocus();
    curr_activity->updateRow_intTree(treeSelection);
    this->update_infoModel();
    ui->treeView_intervall->setCurrentIndex(treeSelection->currentIndex());
    this->set_speedValues(treeSelection->currentIndex().row());
}

void MainWindow::on_toolButton_delete_clicked()
{
    curr_activity->removeRow_intTree(treeSelection);
    this->update_infoModel();
}

void MainWindow::on_toolButton_add_clicked()
{
    curr_activity->addRow_intTree(treeSelection);
    treeSelection->setCurrentIndex(ui->treeView_intervall->indexAbove(treeSelection->currentIndex()),QItemSelectionModel::Select);
}

void MainWindow::on_toolButton_upInt_clicked()
{
    this->setCurrentTreeIndex(true);
}

void MainWindow::on_toolButton_downInt_clicked()
{
    this->setCurrentTreeIndex(false);
}

void MainWindow::setCurrentTreeIndex(bool up)
{
    QModelIndex index;

    if(up)
    {
        index = ui->treeView_intervall->indexAbove(treeSelection->currentIndex());
    }
    else
    {
        index = ui->treeView_intervall->indexBelow(treeSelection->currentIndex());
    }

    int currRow = index.row();

    if(currRow == 0)
    {
        ui->toolButton_upInt->setEnabled(false);
    }
    else if(currRow == curr_activity->intTreeModel->rowCount()-1)
    {
        ui->toolButton_downInt->setEnabled(false);
    }
    else
    {
        ui->toolButton_upInt->setEnabled(true);
        ui->toolButton_downInt->setEnabled(true);
    }

    treeSelection->setCurrentIndex(index,QItemSelectionModel::Select);
}

void MainWindow::on_actionEditor_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);
    this->set_menuItems(true,false);
}

void MainWindow::on_actionPlaner_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
    this->set_menuItems(false,true);
}

void MainWindow::on_actionExit_triggered()
{
    if(workSchedule->get_isUpdated())
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      tr("Save and Exit"),
                                      "Save Changes on Workouts?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            workSchedule->save_dayWorkouts();
            this->freeMem();
            close();
        }
        else
        {
            this->freeMem();
            close();
        }
    }
    else
    {
        this->freeMem();
        close();
    }
}

void MainWindow::on_actionExit_and_Save_triggered()
{
    workSchedule->save_dayWorkouts();
    this->freeMem();
}

void MainWindow::on_actionSelect_File_triggered()
{
    this->select_activityFile();
}

void MainWindow::on_actionReset_triggered()
{
    this->clearActivtiy();
    avgSelect_del.sport = QString();
    actLoaded = false;
    ui->horizontalSlider_factor->setEnabled(false);
}

void MainWindow::on_toolButton_clearSelect_clicked()
{
    this->unselect_intRow(false);
}

void MainWindow::on_actionIntervall_Editor_triggered()
{
    int dialog_code;
    Dialog_workCreator workCreator(this,workSchedule);
    workCreator.setModal(true);
    dialog_code = workCreator.exec();

    if(dialog_code == QDialog::Accepted)
    {
        this->summery_view();
        this->workout_calendar();
        ui->actionSave->setEnabled(workSchedule->get_isUpdated());
    }
}

void MainWindow::on_actionPreferences_triggered()
{
    int dialog_code;
    Dialog_settings dia_settings(this,workSchedule,foodPlan);
    dia_settings.setModal(true);
    dialog_code = dia_settings.exec();
    if(dialog_code == QDialog::Rejected)
    {
        if(settings::get_listValues("Sportuse").count() != sportUse)
        {
            planMode->setChecked(false);
            planMode->setEnabled(false);

            QMessageBox::StandardButton reply;
            reply = QMessageBox::information(this,
                                          tr("Restart!"),
                                          "You changed Sport List used for Year Planning.\n WorkoutEditor has to be restarted!",
                                          QMessageBox::Ok
                                          );
            if (reply == QMessageBox::Ok)
            {
                close();
            }
        }
        this->summery_view();
        this->workout_calendar();

        if(settings::settingsUpdated)
        {
            this->loadUISettings();
        }
    }

    if(workSchedule->newSaison)
    {
        this->refresh_saisonInfo();
    }
}

void MainWindow::on_actionPace_Calculator_triggered()
{
    Dialog_paceCalc dia_pace(this);
    dia_pace.setModal(true);
    dia_pace.exec();
}

void MainWindow::on_tableView_summery_clicked(const QModelIndex &index)
{
    if(!isWeekMode)
    {
        int dialog_code;
        year_popup year_pop(this,sumModel->data(index,Qt::DisplayRole).toString(),index.row(),workSchedule,phaseFilter,phaseFilterID-1);
        year_pop.setModal(true);
        dialog_code = year_pop.exec();
        if(dialog_code == QDialog::Rejected)
        {
            this->set_calender();
        }
    }
}

void MainWindow::set_phaseFilter(int phaseID)
{
    phaseFilterID = phaseID;

    if(phaseID == 1)
    {
        this->set_buttons(false);
        ui->toolButton_weekFour->setEnabled(true);
        ui->toolButton_weekPlus->setEnabled(true);
        phaseFilter = "All";
    }
    else
    {
        this->set_buttons(false);
        ui->toolButton_weekFour->setEnabled(false);
        ui->toolButton_weekPlus->setEnabled(false);
        phaseFilter = settings::get_listValues("Phase").at(phaseID-2);
        weekpos = 0;
    }

    this->workout_calendar();
    this->summery_view();
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::on_actionVersion_triggered()
{
    Dialog_version versionBox(this);
    versionBox.setModal(true);
    versionBox.exec();
}

void MainWindow::on_lineEdit_workContent_textChanged(const QString &value)
{
    Q_UNUSED(value)
    ui->toolButton_sync->setEnabled(true);
    ui->actionSave->setEnabled(true);
}

void MainWindow::on_toolButton_addSelect_clicked()
{
    this->fill_WorkoutContent();
}

void MainWindow::on_toolButton_sync_clicked()
{
    curr_activity->set_workoutContent(ui->lineEdit_workContent->text());
    ui->toolButton_sync->setEnabled(false);
}

void MainWindow::on_toolButton_clearContent_clicked()
{
    ui->lineEdit_workContent->clear();
    ui->toolButton_clearContent->setEnabled(false);
    ui->toolButton_sync->setEnabled(false);
}

void MainWindow::on_actionPMC_triggered()
{
    stress_popup stressPop(this,workSchedule,ui->calendarWidget->selectedDate());
    stressPop.setModal(true);
    stressPop.exec();
}

void MainWindow::set_module(int modID)
{
    ui->stackedWidget->setCurrentIndex(modID);
    selModule = modID;

    if(modID == PLANER)
    {
        this->set_menuItems(false,true);
    }
    else if(modID == EDITOR)
    {
        this->set_menuItems(true,false);
    }
    else if(modID == FOOD)
    {
        this->set_menuItems(false,false);
        ui->actionDelete->setVisible(true);
        ui->actionDelete->setEnabled(false);
        ui->actionNew->setVisible(true);
        ui->actionNew->setEnabled(false);
    }
}

void MainWindow::on_actionEdit_Week_triggered()
{
    int dialogCode;
    Dialog_week_copy week_copy(this,QString(),workSchedule,false);
    week_copy.setModal(true);
    dialogCode = week_copy.exec();

    if(dialogCode == QDialog::Accepted)
    {
        this->workout_calendar();
        this->set_calender();
        ui->actionSave->setEnabled(workSchedule->get_isUpdated());
    }
    if(dialogCode == QDialog::Rejected)
    {
        this->set_calender();
    }
}

void MainWindow::toolButton_planMode(bool checked)
{
    isWeekMode = !checked;

    if(!checked)
    {
        planMode->setText(schedMode.at(0));
        weekCounter = 0;
        ui->actionNew->setEnabled(!checked);
        this->set_buttons(checked);
        this->set_phaseFilter(1);
    }
    else
    {
        planMode->setText(schedMode.at(1));
        if(weekpos == 0)
        {
             this->set_buttons(false);
        }
        else
        {
            this->set_buttons(true);
        }
        this->set_phaseFilter(phaseGroup->checkedId());
    }
    ui->comboBox_saisonName->setEnabled(checked);
    ui->calendarWidget->setVisible(!checked);
    ui->frame_YearAvg->setVisible(checked);
    ui->frame_phases->setVisible(checked);
    ui->label_month->setText("Week " + this->get_weekRange());
    this->workout_calendar();
    this->summery_view();
}

void MainWindow::on_treeView_files_clicked(const QModelIndex &index)
{
    this->clearActivtiy();
    this->loadfile(fileModel->data(fileModel->index(index.row(),4)).toString());
}

void MainWindow::on_actionRefresh_Filelist_triggered()
{
    ui->progressBar_fileState->setValue(10);
    this->read_activityFiles();
    ui->progressBar_fileState->setValue(100);
    QTimer::singleShot(2000,ui->progressBar_fileState,SLOT(reset()));

}

void MainWindow::on_comboBox_saisonName_currentIndexChanged(const QString &value)
{
    if(isWeekMode)
    {
        workSchedule->set_selSaison(value);
    }
    else
    {
        workSchedule->set_selSaison(value);
        this->summery_view();
        this->workout_calendar();
    }
}

void MainWindow::set_foodWeek(QString weekID)
{
    foodPlan->firstDayofWeek = QDate::fromString(weekID.split(" - ").last(),"dd.MM.yyyy");
    this->fill_weekTable(weekID.split(" - ").first(),true);
    foodPlan->update_sumByMenu(foodPlan->firstDayofWeek,0,NULL,false);
    foodPlan->update_sumBySchedule(foodPlan->firstDayofWeek);
    ui->label_foodWeek->setText("Kw "+ weekID);
}

void MainWindow::calc_foodCalories(int portion,double factor,int calories)
{
    if(portion >= 1 && portion < 100)
    {
        ui->lineEdit_calories->setText(QString::number(round(calories*(portion*factor))));
    }
    else if(portion >= 100)
    {
        ui->lineEdit_calories->setText(QString::number(round(calories*(portion*factor)/100)));
    }
}

int MainWindow::checkFoodString(QString checkString)
{
    QRegExp rxp("\\b([A-Za-zäöü]*\\s){1,3}(\\s?\\-{1}\\s{1}\\d{1,3}){2}\\b");
    QRegExpValidator rxpVal(rxp,0);
    int pos = 0;

    return rxpVal.validate(checkString,pos);
}

void MainWindow::calc_menuCal()
{
    QString foodString;
    int listCount = ui->listWidget_MenuEdit->count();
    int calPos = 0;
    int mealCal = 0;

    if(listCount != 0)
    {
        for(int i = 0; i < listCount; ++i)
        {
            foodString = ui->listWidget_MenuEdit->item(i)->data(Qt::DisplayRole).toString();
            calPos = foodString.indexOf("-")+1;
            mealCal = mealCal + foodString.mid(calPos,foodString.indexOf(")")-calPos).toInt();
        }
    }
    ui->label_menuCal->setText("Meal Cal: "+QString::number(mealCal));
}

void MainWindow::on_listWidget_weekPlans_clicked(const QModelIndex &index)
{
    QString weekId = index.data(Qt::DisplayRole).toString();
    this->set_foodWeek(weekId);
    ui->actionDelete->setEnabled(true);
}

void MainWindow::on_toolButton_addMenu_clicked()
{
    ui->toolButton_saveMeals->setEnabled(true);
    foodPlan->add_meal(mealSelection);
}

void MainWindow::on_listWidget_MenuEdit_doubleClicked(const QModelIndex &index)
{
    delete ui->listWidget_MenuEdit->takeItem(index.row());
    ui->listWidget_MenuEdit->clearSelection();
    this->calc_menuCal();
}

void MainWindow::on_toolButton_menuEdit_clicked()
{
    QString setString,foodString;
    QStringList updateList;
    QModelIndex index = ui->tableWidget_weekPlan->currentIndex();
    QTableWidgetItem *selItem = ui->tableWidget_weekPlan->currentItem();
    int listCount = ui->listWidget_MenuEdit->count();
    int calPos = 0;
    int mealCal = 0;

    if(listCount != 0)
    {
        for(int i = 0; i < listCount; ++i)
        {
            foodString = ui->listWidget_MenuEdit->item(i)->data(Qt::DisplayRole).toString();
            calPos = foodString.indexOf("-")+1;
            mealCal = mealCal + foodString.mid(calPos,foodString.indexOf(")")-calPos).toInt();
            setString = setString + ui->listWidget_MenuEdit->item(i)->data(Qt::DisplayRole).toString()+"\n";
            updateList << ui->listWidget_MenuEdit->item(i)->data(Qt::DisplayRole).toString();
        }
    }
    else
    {
        setString = "No Food\n";
    }

    setString.remove(setString.length()-1,1);

    selItem->setData(Qt::EditRole,setString);
    selItem->setToolTip("Meal Cal: "+QString::number(mealCal));
    ui->label_menuCal->setText("Meal Cal:");

    foodPlan->update_sumByMenu(foodPlan->firstDayofWeek.addDays(index.column()),index.row(),&updateList,true);

    ui->label_menuEdit->setText("Edit: -");
    ui->listWidget_MenuEdit->clear();
    ui->actionSave->setEnabled(true);
    ui->toolButton_foodUp->setEnabled(false);
    ui->toolButton_foodDown->setEnabled(false);
    ui->toolButton_menuEdit->setEnabled(false);
    ui->toolButton_clear->setEnabled(false);
}

void MainWindow::on_toolButton_saveMeals_clicked()
{
    ui->progressBar_saveMeals->setValue(0);
    foodPlan->write_meals();
    ui->progressBar_saveMeals->setValue(100);
    ui->actionSave->setEnabled(true);
    QTimer::singleShot(2000,ui->progressBar_saveMeals,SLOT(reset()));
    ui->toolButton_saveMeals->setEnabled(false);
}

void MainWindow::on_calendarWidget_Food_clicked(const QDate &date)
{
    ui->label_foodWeekInfo->setText("Add Week: " + QString::number(date.weekNumber())+ " - " +QString::number(date.year()));
    ui->actionNew->setEnabled(true);
}

void MainWindow::on_toolButton_deleteMenu_clicked()
{
    foodPlan->remove_meal(mealSelection);
    ui->toolButton_saveMeals->setEnabled(true);
}

void MainWindow::on_doubleSpinBox_portion_valueChanged(double value)
{
    this->calc_foodCalories(ui->spinBox_portion->value(),value,ui->spinBox_calories->value());
}

void MainWindow::on_spinBox_portion_valueChanged(int value)
{
    this->calc_foodCalories(value,ui->doubleSpinBox_portion->value(),ui->spinBox_calories->value());
}

void MainWindow::on_toolButton_addMeal_clicked()
{
    QString mealPort = QString::number(ui->spinBox_portion->value()*ui->doubleSpinBox_portion->value());
    QString mealString = ui->lineEdit_Mealname->text()+" ("+mealPort+"-"+ui->lineEdit_calories->text()+")";
    bool mealFlag = false;


    if(ui->listWidget_MenuEdit->selectedItems().count() == 1)
    {
        ui->listWidget_MenuEdit->currentItem()->setData(Qt::EditRole,mealString);
    }
    else if(ui->listWidget_MenuEdit->selectedItems().count() == 0)
    {
        QString checkMeal;
        for(int i = 0; i < ui->listWidget_MenuEdit->count(); ++i)
        {
            checkMeal = ui->listWidget_MenuEdit->item(i)->data(Qt::DisplayRole).toString().split(" (").first();

            if(checkMeal == ui->lineEdit_Mealname->text())
            {
                ui->listWidget_MenuEdit->item(i)->setData(Qt::EditRole,mealString);
                mealFlag = true;
                break;
            }
        }
        if(!mealFlag) ui->listWidget_MenuEdit->addItem(mealString);
    }

    this->calc_menuCal();
}

void MainWindow::on_toolButton_foodUp_clicked()
{
    int currentindex = ui->listWidget_MenuEdit->currentRow();
    QListWidgetItem *currentItem = ui->listWidget_MenuEdit->takeItem(currentindex);
    ui->listWidget_MenuEdit->insertItem(currentindex-1,currentItem);
    ui->listWidget_MenuEdit->setCurrentRow(currentindex-1);
}

void MainWindow::on_toolButton_foodDown_clicked()
{
    int currentindex = ui->listWidget_MenuEdit->currentRow();
    QListWidgetItem *currentItem = ui->listWidget_MenuEdit->takeItem(currentindex);
    ui->listWidget_MenuEdit->insertItem(currentindex+1,currentItem);
    ui->listWidget_MenuEdit->setCurrentRow(currentindex+1);
}

void MainWindow::on_actionDelete_triggered()
{
    QModelIndex index = ui->listWidget_weekPlans->currentIndex();
    QString weekID = index.data().toString();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  tr("Delete Week"),
                                  "Delete selected week "+weekID,
                                  QMessageBox::Yes|QMessageBox::No
                                  );
    if (reply == QMessageBox::Yes)
    {
        foodPlan->remove_week(weekID);
        ui->listWidget_weekPlans->takeItem(index.row());
        this->set_foodWeek(ui->listWidget_weekPlans->currentItem()->text());
        ui->actionDelete->setEnabled(false);
        ui->actionSave->setEnabled(true);
    }
}

void MainWindow::on_tableWidget_weekPlan_itemChanged(QTableWidgetItem *item)
{
    QStringList itemList = item->data(Qt::DisplayRole).toString().split("\n");
    QString foodString;
    int calPos = 0;
    int mealCal = 0;

    for(int i = 0; i < itemList.count(); ++i)
    {
        foodString = itemList.at(i);
        calPos = foodString.indexOf("-")+1;
        mealCal = mealCal + foodString.mid(calPos,foodString.indexOf(")")-calPos).toInt();
    }

    foodPlan->update_sumByMenu(foodPlan->firstDayofWeek.addDays(item->column()),item->row(),&itemList,true);
    ui->actionSave->setEnabled(true);
}

void MainWindow::on_listWidget_MenuEdit_itemClicked(QListWidgetItem *item)
{
    QString foodString,foodName,portCal,portion;
    QVector<int> mealData(2);
    foodString = item->data(Qt::DisplayRole).toString();
    foodName = foodString.split(" (").first();
    portCal = foodString.split(" (").last();
    portCal = portCal.remove(")");

    portion = portCal.split("-").first();
    mealData = foodPlan->get_mealData(foodName);

    double d_portion = portion.toDouble()/mealData.at(0);
    double d_mealCal = mealData.at(1)*d_portion;

    ui->lineEdit_Mealname->setText(foodName);
    ui->doubleSpinBox_portion->setValue(d_portion);
    ui->spinBox_portion->setValue(mealData.at(0));
    ui->spinBox_calories->setValue(mealData.at(1));
    ui->lineEdit_calories->setText(QString::number(round(d_mealCal)));
}

void MainWindow::on_treeView_meals_clicked(const QModelIndex &index)
{
    mealSelection->setCurrentIndex(index,QItemSelectionModel::Select);
    ui->listWidget_MenuEdit->clearSelection();
}

void MainWindow::setSelectedMeal(QModelIndex index)
{
    mealSelection->select(index,QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);

    if(foodPlan->mealModel->itemFromIndex(index)->parent() == nullptr)
    {
        ui->treeView_meals->setEditTriggers(QAbstractItemView::NoEditTriggers);
        if(ui->treeView_meals->isExpanded(index))
        {
            ui->treeView_meals->collapse(index);
            ui->toolButton_addMenu->setEnabled(false);
        }
        else
        {
            ui->treeView_meals->expand(index);
            ui->toolButton_addMenu->setEnabled(true);
        }
        ui->toolButton_deleteMenu->setEnabled(false);
    }
    else
    {
        ui->treeView_meals->setEditTriggers(QAbstractItemView::DoubleClicked);
        int port = mealSelection->selectedRows(1).at(0).data().toInt();
        int cal = mealSelection->selectedRows(2).at(0).data().toInt();

        ui->lineEdit_Mealname->setText(mealSelection->selectedRows(0).at(0).data().toString());
        ui->spinBox_portion->setValue(port);
        ui->doubleSpinBox_portion->setValue(1);
        ui->spinBox_calories->setValue(cal);
        ui->toolButton_deleteMenu->setEnabled(true);
        ui->toolButton_addMenu->setEnabled(false);
        this->calc_foodCalories(port,1.0,cal);
    }
}

void MainWindow::on_treeView_meals_collapsed(const QModelIndex &index)
{
    Q_UNUSED(index)
    ui->toolButton_addMenu->setEnabled(false);
    ui->toolButton_deleteMenu->setEnabled(false);
}

void MainWindow::on_tableWidget_weekPlan_itemClicked(QTableWidgetItem *item)
{
    ui->listWidget_MenuEdit->clear();
    ui->listWidget_MenuEdit->addItems(item->data(Qt::DisplayRole).toString().split("\n"));
    this->calc_menuCal();
    ui->label_menuEdit->setText("Edit: "+ foodPlan->dayHeader.at(item->column()) + " - " + foodPlan->mealsHeader.at(item->row()));
    ui->toolButton_foodDown->setEnabled(true);
    ui->toolButton_foodUp->setEnabled(true);
    ui->toolButton_menuEdit->setEnabled(true);
    ui->toolButton_clear->setEnabled(true);
}

void MainWindow::on_treeView_meals_expanded(const QModelIndex &index)
{
    Q_UNUSED(index)
    ui->toolButton_addMenu->setEnabled(true);
    ui->toolButton_deleteMenu->setEnabled(false);
}

void MainWindow::mealSave(QStandardItem *item)
{
    Q_UNUSED(item)
    ui->toolButton_saveMeals->setEnabled(true);
}

void MainWindow::on_toolButton_mealreset_clicked()
{
    ui->treeView_meals->collapseAll();
}

void MainWindow::on_toolButton_clear_clicked()
{
    ui->listWidget_MenuEdit->clear();
    this->calc_menuCal();
}
