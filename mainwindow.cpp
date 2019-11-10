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
    userSetup = settings::loadSettings();

    if(userSetup == 0)
    {
        sportUse = settings::get_listValues("Sportuse").count();
        weekRange = settings::get_intValue("weekrange");
        ui->lineEdit_athlete->setText(gcValues->value("athlete"));
        //Planning Mode
        graphLoaded = false;
        workSchedule = new schedule();

        schedMode = settings::getHeaderMap("mode");

        selectedDate = QDate::currentDate();
        foodPlan = new foodplanner(workSchedule);
        weeknumber = this->calc_weekID(selectedDate);
        weekpos = weekCounter = 0;
        weekDays = 7;
        isWeekMode = true;
        buttonStyle = "QToolButton:hover {color: white; border: 1px solid white; border-radius: 4px; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #00b8ff, stop: 0.5 #0086ff,stop: 1 #0064ff)}";

        ui->label_month->setText("Week " + weeknumber + " - " + QString::number(selectedDate.addDays(weekRange*weekDays).weekNumber()-1));
        planerIcon.addFile(":/images/icons/DateTime.png");
        editorIcon.addFile(":/images/icons/Editor.png");
        foodIcon.addFile(":/images/icons/Food.png");

        modules = new QComboBox(this);
        modules->addItem(planerIcon,"Planner");
        modules->addItem(editorIcon,"Editor");
        modules->addItem(foodIcon,"Nutrition");
        modules->setToolTip("Modules");

        planerMode = new QLabel(this);
        planerMode->setText("Planer Mode:");
        planMode = new QToolButton(this);
        planMode->setCheckable(true);
        planMode->setToolTip("Change Planer Mode");
        planMode->setMinimumHeight(25);
        planMode->setMinimumWidth(75);
        planMode->setText(schedMode->at(0));
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
        sumModel = new QStandardItemModel(0,1,this);
        avgModel = new QStandardItemModel(this);
        this->set_phaseButtons();
        ui->frame_phases->setVisible(false);
        cal_header << "Week";
        for(int d = 1; d < 8; ++d)
        {
            cal_header << QLocale().dayName(d);
        }
        avgHeader = settings::getHeaderMap("average");

        //Editor Mode
        avgCounter = 0;
        fileModel = new QStandardItemModel(this);
        actLoaded = false;
        modeDelegate = &schedule_del;

        connect(ui->actionExit_and_Save, SIGNAL(triggered()), this, SLOT(close()));
        connect(planMode,SIGNAL(clicked(bool)),this,SLOT(toolButton_planMode(bool)));
        connect(phaseGroup,SIGNAL(buttonClicked(int)),this,SLOT(set_phaseFilter(int)));
        connect(modules,SIGNAL(currentIndexChanged(int)),this,SLOT(set_module(int)));
        connect(workSchedule->scheduleModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(refresh_schedule()));
        connect(workSchedule->scheduleModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(refresh_schedule()));
        connect(workSchedule->phaseModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(refresh_saison()));
        connect(workSchedule->phaseModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(refresh_saison()));

        //UI
        ui->stackedWidget->setGeometry(5,5,0,0);
        ui->tableWidget_schedule->setColumnCount(8);
        ui->tableWidget_schedule->setRowCount(static_cast<int>(weekRange));
        ui->tableWidget_schedule->setHorizontalHeaderLabels(cal_header);
        ui->tableWidget_schedule->setItemDelegate(&schedule_del);
        ui->tableWidget_schedule->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_schedule->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_schedule->verticalHeader()->hide();

        ui->tableWidget_saison->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_saison->setItemDelegate(&saison_del);
        ui->tableWidget_saison->verticalHeader()->hide();
        ui->tableWidget_saison->setVisible(false);

        ui->tableWidget_summery->setColumnCount(1);
        ui->tableWidget_summery->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_summery->verticalHeader()->setDefaultSectionSize(ui->tableWidget_summery->verticalHeader()->defaultSectionSize()*4);
        ui->tableWidget_summery->verticalHeader()->hide();
        ui->tableWidget_summery->setItemDelegate(&sum_del); 

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

        ui->tableView_daySummery->setModel(foodPlan->daySumModel);
        ui->tableView_daySummery->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView_daySummery->horizontalHeader()->hide();
        ui->tableView_daySummery->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView_daySummery->verticalHeader()->setFixedWidth(110);
        ui->tableView_daySummery->setItemDelegate(&foodSum_del);
        ui->treeView_meals->setModel(foodPlan->mealModel);
        ui->treeView_meals->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->treeView_meals->setSortingEnabled(true);
        ui->treeView_meals->sortByColumn(0,Qt::AscendingOrder);
        //ui->treeView_meals->setItemDelegate(&mousehover_del);
        //ui->treeView_meals->setStyleSheet(viewStyle);
        ui->treeView_meals->setEditTriggers(QAbstractItemView::NoEditTriggers);
        mealSelection = ui->treeView_meals->selectionModel();
        connect(mealSelection,SIGNAL(currentChanged(QModelIndex,QModelIndex)),this,SLOT(setSelectedMeal(QModelIndex)));
        connect(foodPlan->mealModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(mealSave(QStandardItem*)));
        connect(ui->tableWidget_weekPlan->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(selectFoodMealDay(int)));
        connect(ui->tableWidget_weekPlan->verticalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(selectFoodMealWeek(int)));

        ui->listWidget_weekPlans->addItems(foodPlan->planList);
        ui->comboBox_weightmode->blockSignals(true);
        ui->comboBox_weightmode->addItems(settings::get_listValues("Mode"));
        ui->comboBox_weightmode->setEnabled(false);
        ui->comboBox_weightmode->blockSignals(false);
        ui->spinBox_calories->setVisible(false);

        ui->tableView_forecast->setModel(foodPlan->estModel);
        ui->tableView_forecast->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView_forecast->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableView_forecast->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableView_forecast->horizontalHeader()->hide();

        ui->toolButton_saveMeals->setEnabled(false);
        ui->toolButton_deleteMenu->setEnabled(false);
        ui->toolButton_menuCopy->setEnabled(false);
        ui->toolButton_menuPaste->setEnabled(false);
        ui->frame_dayShow->setVisible(false);
        ui->tableWidget_daySummery->setColumnCount(4);
        ui->tableWidget_daySummery->setItemDelegate(&foodDaySum_del);
        ui->tableWidget_daySummery->setRowCount(foodPlan->mealsHeader.count()+1);
        ui->tableWidget_daySummery->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_daySummery->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->pushButton_lineCopy->setIcon(QIcon(":/images/icons/Copy.png"));
        ui->pushButton_lineCopy->setEnabled(false);
        ui->toolButton_linePaste->setEnabled(false);
        foodcopyMode = lineSelected = dayLineSelected = false;

        this->reset_menuEdit();
        this->set_speedgraph();
        this->resetPlot();
        this->read_activityFiles();
        this->refresh_saisonInfo();
        //ui->actionSave->setEnabled(false);
        this->set_menuItems(0);
        this->set_phaseFilter(1);
        this->loadUISettings();
        this->workoutSchedule(firstdayofweek);
        this->fill_weekTable(this->calc_weekID(firstdayofweek),false);
        this->set_foodWeek(ui->listWidget_weekPlans->item(0)->text());
    }
    else
    {

        if(userSetup == 1)
        {
            QMessageBox::warning(this,"Init Error","Not able to load WorkoutEditor.ini. Check WorkoutEditor.ini location. Has to be in WorkoutEditor Install Directory!",QMessageBox::Ok);
        }
        if(userSetup == 2)
        {
            QMessageBox::warning(this,"GoldenCheetah Error","Not able to load GC Config. Check GCPath (GC Home Directory) in WorkoutEditor.ini!",QMessageBox::Ok);
        }
        if(userSetup == 3)
        {
            QMessageBox::warning(this,"User Setting Error","User Settings not loaded. Check WorkoutEditor_values.ini location!",QMessageBox::Ok);
        }
    }
}

enum {PLANER,EDITOR,FOOD};

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::loadUISettings()
{
    this->refresh_saisonInfo();
    settings::settingsUpdated = false;
}

void MainWindow::freeMem()
{
    if(userSetup == 0)
    {
        workSchedule->freeMem();
        delete workSchedule;
        delete sumModel;
        delete fileModel;
        delete avgModel;
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

void MainWindow::fill_dayTable(int daySelected)
{
    QVector<int> foodMacros(5);
    QVector<int> sumMacros(5);
    QVector<int> summery(5);
    sumMacros.fill(0);
    summery.fill(0);

    double dayCal = foodPlan->daySumModel->data(foodPlan->daySumModel->index(0,daySelected)).toDouble();
    double mealCal = 0;
    double percent = 0;

    QStringList mealList;
    QString calString;

    for(int i = 0; i < foodPlan->mealsHeader.count(); ++i)
    {
        mealList = ui->tableWidget_weekPlan->item(i,daySelected)->data(Qt::DisplayRole).toString().split("\n");

        for(int x = 0; x < mealList.count(); ++x)
        {
            foodMacros = this->calc_menuCal(mealList.at(x));
            for(int y = 0; y < 5; ++y)
            {
                sumMacros[y] = sumMacros[y] + foodMacros.at(y);
            }

            for(int val = 0; val < 5; ++val)
            {
                 QTableWidgetItem *item = new QTableWidgetItem();
                 if(val == 0)
                 {
                    mealCal = sumMacros.at(val);
                    percent = (sumMacros.at(val) / dayCal)*100.0;
                    calString = QString::number(sumMacros.at(val));
                 }
                 else if(val == 1 || val == 2)
                 {
                    mealCal = sumMacros.at(val) * 4.1;
                    if(val == 1) mealCal = mealCal - sumMacros.at(4);
                    percent = (mealCal / sumMacros.at(0))*100.0;
                    calString = QString::number(sumMacros.at(val))+"-"+QString::number(round(mealCal));
                 }
                 else if(val == 3)
                 {
                    mealCal = sumMacros.at(val) * 9.3;
                    percent = (mealCal / sumMacros.at(0))*100.0;
                    calString = QString::number(sumMacros.at(val))+"-"+QString::number(round(mealCal));
                 }

                 if(mealCal == 0.0) percent = 0;

                 if(x == mealList.count()-1)
                 {
                     if(val == 0)
                     {
                         summery[val] = summery.at(val) + sumMacros.at(val);
                     }
                     if(val == 1 || val == 2)
                     {
                         summery[val] = summery.at(val) + (sumMacros.at(val)*4.1);
                         if(val == 1) summery[val] = summery[val] - sumMacros.at(4);
                     }
                     if(val == 3)
                     {
                         summery[val] = summery.at(val) + (sumMacros.at(val)*9.3);
                     }

                 }
                 item->setText(calString+" ("+QString::number(set_doubleValue(percent,false))+")");
                 ui->tableWidget_daySummery->setItem(i,val,item);
            }
        }

        sumMacros.fill(0);
    }

    for(int i = 0; i < summery.count(); ++i)
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setText(QString::number(summery.at(i))+" ("+QString::number(set_doubleValue((summery.at(i)/dayCal)*100.0,false))+")");
        ui->tableWidget_daySummery->setItem(foodPlan->mealsHeader.count(),i,item);
    }
}

void MainWindow::read_activityFiles()
{
    fileModel->clear();
    ui->lineEdit_workContent->clear();
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
        curr_activity->reset_avgSelection();
        delete treeSelection;
        delete curr_activity->intModel;
        delete curr_activity->sampleModel;
        delete curr_activity->intTreeModel;
        delete curr_activity->avgModel;
        delete curr_activity->selItemModel;
        if(hasXdata)
        {
            //delete curr_activity->xdataModel;
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

void MainWindow::set_menuItems(int module)
{
    if(module == PLANER)
    {
        ui->menuWorkout->setEnabled(true);
        ui->actionPMC->setVisible(true);
        ui->actionIntervall_Editor->setVisible(true);
        ui->actionExport_to_Golden_Cheetah->setVisible(true);
        ui->actionPace_Calculator->setVisible(true);
        ui->actionStress_Calculator->setVisible(true);
        ui->actionNew->setVisible(true);
        planerMode->setEnabled(true);
        planMode->setEnabled(true);
        planerMode->setVisible(true);

        if(workSchedule->get_stressMap()->count() == 0) ui->actionPMC->setEnabled(false);

        ui->actionfood_History->setVisible(false);
        ui->actionFood_Macros->setVisible(false);
        ui->actionSelect_File->setVisible(false);
        ui->actionDelete->setVisible(false);
        ui->actionRefresh_Filelist->setVisible(false);
        ui->actionReset->setVisible(false);
    }
    if(module == EDITOR)
    {
        ui->actionRefresh_Filelist->setVisible(true);
        ui->actionIntervall_Editor->setVisible(true);
        ui->actionReset->setVisible(true);
        ui->actionSelect_File->setVisible(true);
        ui->actionReset->setEnabled(actLoaded);
        ui->actionPMC->setVisible(true);
        ui->actionPace_Calculator->setVisible(true);
        ui->actionStress_Calculator->setVisible(true);

        ui->actionfood_History->setVisible(false);
        ui->actionFood_Macros->setVisible(false);
        ui->actionSelect_File->setVisible(false);
        ui->actionDelete->setVisible(false);
    }
    if(module == FOOD)
    { 
        ui->actionFood_Macros->setVisible(true);
        ui->actionfood_History->setVisible(true);
        ui->actionDelete->setVisible(true);
        ui->actionDelete->setEnabled(false);
        ui->actionNew->setVisible(true);
        ui->actionNew->setEnabled(false);

        ui->actionPMC->setVisible(false);
        ui->actionReset->setVisible(false);
        ui->actionPace_Calculator->setVisible(false);
        ui->actionStress_Calculator->setVisible(false);
        ui->actionIntervall_Editor->setVisible(false);
        ui->actionRefresh_Filelist->setVisible(false);
        planerMode->setVisible(false);
        planMode->setEnabled(false);
    }
}

//Planner Functions ***********************************************************************************

void MainWindow::summery_Set(QDate date,QStandardItem *phaseItem)
{
    QStringList sportUseList,headerInfo;
    QString sportUse,headerString;
    sportUse = generalValues->value("sum");
    sportUseList << sportUse;
    sportUseList << settings::get_listValues("Sportuse");
    int sumCounter = 0;
    QMap<QString,QVector<double>> sportSummery;
    QMap<QString,QVector<double>> calcSummery;

    if(isWeekMode)
    {
        sumCounter = settings::getHeaderMap("summery")->count();
        sportSummery.insert(sportUse,QVector<double>(sumCounter,0));

        QHash<QDate,QMap<QString,QVector<double> >> *comp = workSchedule->get_compValues();
        for(unsigned int day = 0; day < weekDays; ++day)
        {
            calcSummery = comp->value(date.addDays(day));
            this->summery_calc(&calcSummery,&sportSummery,&sportUseList,sumCounter);
        }
        headerInfo = workSchedule->get_weekMeta(this->calc_weekID(date));
        headerString = "Week: " + headerInfo.at(0) + " - " + "Phase: " + headerInfo.at(2);
    }
    else
    {
        sportUseList.removeLast();
        sumCounter = settings::getHeaderMap("summery")->count()-1;
        sportSummery.insert(sportUse,QVector<double>(sumCounter,0));

        QHash<QString,QMap<QString,QVector<double> >> *compWeek = workSchedule->get_compWeekValues();
        QString weekID;
        QString saison = ui->comboBox_saisonName->currentText();

        if(phaseGroup->checkedId() == 1)
        {
            int weeks = workSchedule->get_saisonValues()->value(saison).at(2).toInt();
            QDate saisonStart = QDate::fromString(workSchedule->get_saisonValues()->value(saison).at(0),dateFormat);

            for(int week = 0; week < weeks; ++week)
            {
                weekID = calc_weekID(saisonStart.addDays(week*static_cast<int>(weekDays)));
                calcSummery = compWeek->value(weekID);
                this->summery_calc(&calcSummery,&sportSummery,&sportUseList,sumCounter);
            }
            headerString = "All Phases - Weeks:" + workSchedule->get_saisonValues()->value(saison).at(2);
        }
        else
        {
            for(int week = 0; week < phaseItem->rowCount(); ++week)
            {
                calcSummery = compWeek->value(phaseItem->child(week,0)->data(Qt::DisplayRole).toString());
                this->summery_calc(&calcSummery,&sportSummery,&sportUseList,sumCounter);
            }
            headerString = "Phase: " + phaseItem->data(Qt::DisplayRole).toString() + " - Weeks" + QString::number(phaseItem->rowCount());
        }
    }

    QMap<int,QString> sumValues;
    ui->tableWidget_summery->clear();
    ui->tableWidget_summery->setRowCount(sportSummery.count());
    ui->tableWidget_summery->setHorizontalHeaderItem(0,new QTableWidgetItem());
    ui->tableWidget_summery->horizontalHeaderItem(0)->setData(Qt::EditRole,headerString);
    ui->tableWidget_summery->horizontalHeaderItem(0)->setTextAlignment(Qt::AlignLeft);

    for(int sports = 0; sports < sportUseList.count(); ++sports)
    {
        sportUse = sportUseList.at(sports);

        if(sportSummery.contains(sportUse))
        {
            sumValues.insert(sports,this->set_summeryString(sumCounter,&sportSummery,sportUse));
        }
    }

    for(QMap<int,QString>::const_iterator it = sumValues.cbegin(), end = sumValues.cend(); it != end; ++it)
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setData(Qt::EditRole,it.value());
        ui->tableWidget_summery->setItem(it.key(),0,item);
    }
}

void MainWindow::summery_calc(QMap<QString,QVector<double>> *calcSummery,QMap<QString,QVector<double>> *sportSummery,QStringList *sportUseList,int sumCounter)
{
    QString sportUse = generalValues->value("sum");
    QVector<double> calcSum(sumCounter,0),sportSum(sumCounter,0);
    QMap<int,QString> sumValues;
    calcSum = sportSummery->value(sportUse);

    for(QMap<QString,QVector<double>>::const_iterator it = calcSummery->cbegin(), end = calcSummery->cend(); it != end; ++it)
    {
        sportSum.fill(0);
        sportUse = it.key();
        if(sportUseList->contains(sportUse))
        {
            for(int values = 0; values < sumCounter; ++values)
            {
                sportSum[values] = it.value().at(values);
                calcSum[values] = calcSum.at(values) + sportSum.at(values);
            }
            if(sportSummery->contains(sportUse))
            {
                for(int sum = 0; sum < sumCounter; ++sum)
                {
                    sportSum[sum] = sportSum.at(sum) + sportSummery->value(sportUse).at(sum);
                }
            }
            sportSummery->insert(sportUse,sportSum);
            sportSummery->insert(generalValues->value("sum"),calcSum);
        }
     }
}

QString MainWindow::set_summeryString(int counter, QMap<QString,QVector<double>> *summery,QString sport)
{
    QString valueString = sport;
    double weekComp = summery->value(generalValues->value("sum")).at(1);
    double sportComp = summery->value(sport).at(1);
    double avgValue = weekComp/weekComp;

    //qDebug() << sportComp << weekComp;

    avgValue = sportComp / weekComp;

    for(int i = 0; i < counter; ++i)
    {
        if(i == 2)
        {
            valueString = valueString + "-" + QString::number(this->set_doubleValue(avgValue*100.0,false));
        }
        else
        {
            valueString = valueString + "-" + QString::number(summery->value(sport).at(i));
        }
    }
    return valueString;
}

void MainWindow::workoutSchedule(QDate date)
{
    int dayCounter = 0;
    QMap<int, QStringList> dayWorkouts;
    QStringList weekInfo;
    QString conString = " - ";
    QString itemValue = QString();
    QString delimiter = "#";
    QString stdConnect;

    for(int row = 0; row < weekRange; ++row)
    {
        for(int col = 0; col < weekDays+1; ++col)
        {
            QTableWidgetItem *item = new QTableWidgetItem();
            if(col == 0)
            {
                weekInfo = workSchedule->get_weekMeta(calc_weekID(date.addDays(dayCounter)));
                itemValue = weekInfo.at(0) + delimiter + weekInfo.at(2) + delimiter + weekInfo.at(4) + delimiter + weekInfo.at(5);
            }
            else
            {
                dayWorkouts = workSchedule->get_workouts(true,date.addDays(dayCounter).toString(dateFormat));
                itemValue = date.addDays(dayCounter).toString("dd MMM yy") +delimiter;
                for(QMap<int,QStringList>::const_iterator it = dayWorkouts.cbegin(), end = dayWorkouts.cend(); it != end; ++it)
                {
                    stdConnect = it.value().count() == 9 ? "\n" : "*\n";
                    itemValue = itemValue + it.value().at(1) + conString + it.value().at(2) + stdConnect;
                    itemValue = itemValue + set_time(it.value().at(5).toInt()) + conString + it.value().at(6) + " km" + delimiter;
                }
                ++dayCounter;
                itemValue.chop(1);
            }
            item->setData(Qt::DisplayRole,itemValue);
            item->setTextAlignment(0);
            itemValue.clear();
            ui->tableWidget_schedule->setItem(row,col,item);
        }
    }
    this->summery_Set(date,nullptr);
}

void MainWindow::set_saisonValues(QStringList *sportList,QString weekID, int week)
{
    QMap<QString, QVector<double>> compValues;
    QHash<QString,QMap<QString,QVector<double>>> *compWeekMap = workSchedule->get_compWeekValues();
    QStringList weekInfo;
    QString conString = " - ";
    QString itemValue = QString();
    QString delimiter = "#";
    QVector<double> sumValues(5,0);
    int col = 0;

    QTableWidgetItem *weekitem = new QTableWidgetItem();
    weekInfo = workSchedule->get_weekMeta(weekID);
    itemValue = weekInfo.at(0) + conString + weekInfo.at(1) + conString + weekInfo.at(3) + delimiter + weekInfo.at(2) + delimiter + weekInfo.at(4);
    weekitem->setData(Qt::EditRole,itemValue);
    weekitem->setTextAlignment(0);
    itemValue.clear();
    ui->tableWidget_saison->setItem(week,col++,weekitem);

    compValues = compWeekMap->value(weekInfo.at(0));

    for(int sportValues = 0; sportValues < compValues.count(); ++sportValues)
    {
        QTableWidgetItem *sportitem = new QTableWidgetItem();
        for(int mapValues = 0; mapValues < sumValues.count(); ++mapValues)
        {
            itemValue = itemValue + QString::number(compValues.value(sportList->at(sportValues+1)).at(mapValues)) + delimiter;
            sumValues[mapValues] = sumValues.at(mapValues) + compValues.value(sportList->at(sportValues+1)).at(mapValues);
        }
        itemValue.chop(1);
        sportitem->setData(Qt::EditRole,itemValue);
        sportitem->setTextAlignment(0);
        itemValue.clear();
        ui->tableWidget_saison->setItem(week,col++,sportitem);
    }

    QTableWidgetItem *sumItem = new QTableWidgetItem();
    for(int sum = 0; sum < sumValues.count(); ++sum)
    {
        itemValue = itemValue + QString::number(sumValues.at(sum)) + delimiter;
    }
    itemValue.chop(1);
    sumItem->setData(Qt::EditRole,itemValue);
    itemValue.clear();
    sumValues.fill(0);
    ui->tableWidget_saison->setItem(week,col,sumItem);
    col = 0;

}

void MainWindow::saisonSchedule(QString phase)
{
    QStringList sportUseList;
    sportUseList << "Week";
    sportUseList << settings::get_listValues("Sportuse");
    sportUseList.removeLast();
    sportUseList << generalValues->value("sum");
    QString saison = ui->comboBox_saisonName->currentText();
    QDate saisonStart = QDate::fromString(workSchedule->get_saisonValues()->value(saison).at(0),dateFormat);
    QString weekID;

    ui->tableWidget_saison->setColumnCount(sportUseList.count());
    ui->tableWidget_saison->setHorizontalHeaderLabels(sportUseList);

    if(phase == phaseGroup->button(1)->text())
    {
        ui->tableWidget_saison->setRowCount(static_cast<int>(weekRange));
        ui->tableWidget_saison->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        for(int week = 0; week < static_cast<int>(weekRange); ++week)
        {
            weekID = calc_weekID(saisonStart.addDays(week*static_cast<int>(weekDays)));
            this->set_saisonValues(&sportUseList,weekID,week);
        }
        this->summery_Set(saisonStart,nullptr);
    }
    else
    {
        QStandardItem *phaseItem = workSchedule->get_phaseItem(phase);

        if(phaseItem->hasChildren())
        {
            ui->tableWidget_saison->setRowCount(phaseItem->rowCount());
            ui->tableWidget_saison->verticalHeader()->setDefaultSectionSize(ui->tableWidget_saison->height() / static_cast<int>(weekRange));
            ui->tableWidget_saison->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
            for(int week = 0; week < phaseItem->rowCount(); ++week)
            {
                weekID = phaseItem->child(week,0)->data(Qt::DisplayRole).toString();
                this->set_saisonValues(&sportUseList,weekID,week);
            }
            this->summery_Set(QDate(),phaseItem);
        }
    }
}

void MainWindow::refresh_schedule()
{
    this->workoutSchedule(firstdayofweek.addDays(weekCounter*weekDays));
}

void MainWindow::refresh_saison()
{
    this->saisonSchedule(phaseGroup->button(phaseGroup->checkedId())->text());
}

QString MainWindow::get_weekRange()
{
    QString display_weeks;
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
            display_weeks = QString::number(weekpos+1) + " - " + QString::number(static_cast<unsigned int>(weekpos) + weekRange);
        }
        else
        {

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
    QDate setDate;

    if(weekCounter == 0)
    {
        setDate = QDate::currentDate();
    }
    else
    {
        setDate = selectedDate.addDays((1-selectedDate.currentDate().dayOfWeek())+weekDays*weekCounter);
    }

    weeknumber = this->calc_weekID(ui->calendarWidget->selectedDate());
    ui->calendarWidget->setSelectedDate(setDate);
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

    for(int i = 0; i < workSchedule->get_saisonValues()->keys().count(); ++i)
    {
        ui->comboBox_saisonName->addItem(workSchedule->get_saisonValues()->keys().at(i));
    }
    ui->comboBox_saisonName->setEnabled(false);
    saisonWeeks = workSchedule->get_saisonValues()->value(ui->comboBox_saisonName->currentText()).at(2).toInt();
}

//ACTIONS**********************************************************************

void MainWindow::on_actionNew_triggered()
{
    int dialog_code;
    if(ui->stackedWidget->currentIndex() == PLANER)
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
    if(ui->stackedWidget->currentIndex() == FOOD)
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
    if(ui->stackedWidget->currentIndex() == PLANER)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this,
                                      tr("Save Workouts"),
                                      "Save Workout Schedule?",
                                      QMessageBox::Yes|QMessageBox::No
                                      );
        if (reply == QMessageBox::Yes)
        {
            workSchedule->save_workouts(isWeekMode);
            ui->actionSave->setEnabled(workSchedule->get_isUpdated());
        }
    }
    else if(ui->stackedWidget->currentIndex() == EDITOR)
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
    else if(ui->stackedWidget->currentIndex() == FOOD)
    {
        QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this,
                                          tr("Save Food Plan"),
                                          "Save Food Plan?",
                                          QMessageBox::Yes|QMessageBox::No
                                          );
        if (reply == QMessageBox::Yes)
        {
            foodPlan->write_foodPlan();
            foodPlan->write_foodHistory();
            ui->actionSave->setEnabled(false);
        }
    }
}

void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
   weeknumber = this->calc_weekID(date);
   this->summery_Set(date,nullptr);
}


void MainWindow::on_toolButton_weekCurrent_clicked()
{
    if(isWeekMode)
    {
        weekCounter = 0;
        this->set_calender();
        this->set_buttons(false);
        this->refresh_schedule();
    }
    else
    {
        weekpos = 0;
        ui->toolButton_weekFour->setEnabled(true);
        ui->toolButton_weekPlus->setEnabled(true);
        this->set_buttons(false);
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
        this->refresh_schedule();
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
        this->refresh_schedule();
    }
    else
    {
        ++weekpos;
        if(weekpos + weekRange == saisonWeeks)
        {
            ui->toolButton_weekFour->setEnabled(false);
            ui->toolButton_weekPlus->setEnabled(false);
        }
        else
        {
            this->set_buttons(true);
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
        this->refresh_schedule();
    }
    else
    {
        weekpos = weekpos+4;
        if(weekpos + weekRange >= saisonWeeks)
        {
            weekpos = saisonWeeks-weekRange;
            ui->toolButton_weekFour->setEnabled(false);
            ui->toolButton_weekPlus->setEnabled(false);
        }
        else
        {
            this->set_buttons(true);
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
        this->set_menuItems(EDITOR);

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
    ui->tableWidget_summery->setStyleSheet(viewBackground);
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
        }
        yPos = calculation::usePMData ? 1 : 0;
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
        if(usePMData)
        {
            QString watts = avgModel->data(avgModel->index(4,0)).toString()+"W";
            newEntry = newEntry+"-"+watts;
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

void MainWindow::on_actionPlaner_triggered()
{
    ui->stackedWidget->setCurrentIndex(0);
    this->set_menuItems(PLANER);
}

void MainWindow::on_actionEditor_triggered()
{
    ui->stackedWidget->setCurrentIndex(1);
    this->set_menuItems(EDITOR);
}

void MainWindow::on_actionFood_triggered()
{
    ui->stackedWidget->setCurrentIndex(2);
    this->set_menuItems(FOOD);
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
            workSchedule->save_workouts(true);
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
        //close();
        QApplication::exit(EXIT_FAILURE);
    }
}

void MainWindow::on_actionExit_and_Save_triggered()
{
    workSchedule->save_workouts(true);
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
    avgCounter = 0;
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

        if(settings::settingsUpdated)
        {
            this->loadUISettings();
        }
    }
}

void MainWindow::on_actionPace_Calculator_triggered()
{
    Dialog_paceCalc dia_pace(this);
    dia_pace.setModal(true);
    dia_pace.exec();
}

void MainWindow::on_tableWidget_summery_clicked(const QModelIndex &index)
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
    if(phaseID == 1)
    {
        this->set_buttons(false);
        ui->toolButton_weekFour->setEnabled(true);
        ui->toolButton_weekPlus->setEnabled(true);
    }
    else
    {
        this->set_buttons(false);
        ui->toolButton_weekFour->setEnabled(false);
        ui->toolButton_weekPlus->setEnabled(false);
        weekpos = 0;
    }
    ui->label_month->setText("Week " + this->get_weekRange());
    this->saisonSchedule(phaseGroup->button(phaseID)->text());
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
    this->set_menuItems(modID);
}

void MainWindow::on_actionEdit_Week_triggered()
{
    int dialogCode;
    Dialog_week_copy week_copy(this,QString(),workSchedule,false);
    week_copy.setModal(true);
    dialogCode = week_copy.exec();

    if(dialogCode == QDialog::Accepted)
    {
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

    ui->tableWidget_schedule->setVisible(!checked);
    ui->tableWidget_saison->setVisible(checked);

    if(!checked)
    {
        planMode->setText(schedMode->at(0));
        ui->actionNew->setEnabled(!checked);

        this->workoutSchedule(firstdayofweek);
        this->set_buttons(checked);
    }
    else
    {  
        planMode->setText(schedMode->at(1));
        this->saisonSchedule(phaseGroup->checkedButton()->text());
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
}

void MainWindow::on_treeView_files_clicked(const QModelIndex &index)
{
    this->unselect_intRow(false);
    ui->lineEdit_workContent->clear();
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
        this->saisonSchedule(phaseGroup->button(1)->text());
    }
}

void MainWindow::set_foodWeek(QString weekID)
{
    foodPlan->dayMacros.clear();
    foodPlan->dayTarget.clear();
    QStringList weekInfo = weekID.split(" - ");
    foodPlan->firstDayWeek = QDate::fromString(weekInfo.at(1),dateFormat);
    this->fill_weekTable(weekInfo.at(0),true);
    foodPlan->calPercent = settings::doubleVector.value(weekInfo.at(2));
    foodSum_del.percent = foodPlan->calPercent;
    foodSumWeek_del.percent = foodPlan->calPercent;
    foodPlan->update_sumByMenu(foodPlan->firstDayWeek,0,nullptr,false);
    foodPlan->update_sumBySchedule(foodPlan->firstDayWeek);
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
    QRegExpValidator rxpVal(rxp,nullptr);
    int pos = 0;

    return rxpVal.validate(checkString,pos);
}

QVector<int> MainWindow::calc_menuCal(QString foodString)
{
    QString calcString;
    QVector<int> foodMacros(5);
    int vPos = 0;
    int mealCal = 0;
    double mealPort = 0;

    vPos = foodString.indexOf("-")+1;
    mealCal = mealCal + foodString.mid(vPos,foodString.indexOf(")")-vPos).toInt();
    vPos = foodString.indexOf("(")+1;
    mealPort = foodString.mid(vPos,foodString.indexOf("-")-vPos).toDouble();
    calcString = foodString.mid(vPos,foodString.indexOf("-")-vPos);

    foodString = foodString.left(foodString.indexOf("(")-1);
    foodMacros = foodPlan->calc_FoodMacros(foodString,mealPort);

    foodMacros.append(mealCal);
    foodMacros.move(foodMacros.count()-1,0);

    return  foodMacros;
}

void MainWindow::set_menuList()
{
    QString foodString,temp;
    QVector<int> foodMacros(5);
    QVector<int> sumMacros(5);
    sumMacros.fill(0);
    QStringList macroShort;
    macroShort << "C:" << "P:" << "F:" << "Fi:" << "S:";

    int listCount = ui->listWidget_MenuEdit->count();
    int vPos = 0;
    int mealCal = 0;
    double mealPort = 0;

    if(listCount != 0)
    {
        for(int i = 0; i < listCount; ++i)
        {
            foodString = ui->listWidget_MenuEdit->item(i)->data(Qt::DisplayRole).toString();
            vPos = foodString.indexOf("-")+1;
            mealCal = mealCal + foodString.mid(vPos,foodString.indexOf(")")-vPos).toInt();
            vPos = foodString.indexOf("(")+1;
            mealPort = foodString.mid(vPos,foodString.indexOf("-")-vPos).toDouble();
            temp = foodString.mid(vPos,foodString.indexOf("-")-vPos);

            foodString = foodString.left(foodString.indexOf("(")-1);
            foodMacros = foodPlan->calc_FoodMacros(foodString,mealPort);

            for(int x = 0; x < foodMacros.count(); ++x)
            {
                sumMacros[x] = sumMacros[x] + foodMacros.at(x);
            }
        }
        foodString.clear();

        for(int y = 0; y < sumMacros.count(); ++y)
        {
            foodString = foodString + macroShort.at(y)+QString::number(sumMacros.at(y)) + " - ";
        }
    }
    foodString.remove(foodString.length()-3,3);
    ui->label_menuCal->setText("Summery: "+QString::number(mealCal) +" KCal - " + foodString);
}

void MainWindow::reset_menuEdit()
{
    ui->toolButton_foodUp->setEnabled(false);
    ui->toolButton_foodDown->setEnabled(false);
    ui->toolButton_menuEdit->setEnabled(false);
    ui->toolButton_clear->setEnabled(false);
    ui->listWidget_MenuEdit->clear();
}

void MainWindow::on_listWidget_weekPlans_clicked(const QModelIndex &index)
{
    QString weekString = index.data(Qt::DisplayRole).toString();
    QStringList weekInfo = weekString.split(" - ");

    QString weekId = weekInfo.at(0)+" - "+weekInfo.at(1);
    ui->label_foodWeekInfo->setText(weekId);
    ui->comboBox_weightmode->setCurrentText(weekInfo.at(2));
    ui->comboBox_weightmode->setEnabled(true);
    this->set_foodWeek(weekString);
    ui->actionDelete->setEnabled(true);
    ui->calendarWidget_Food->setSelectedDate(QDate::fromString(weekInfo.at(1),dateFormat));
    this->reset_menuEdit();
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
    this->set_menuList();
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
        setString.remove(setString.length()-1,1);
    }

    selItem->setData(Qt::EditRole,setString);
    selItem->setToolTip("Meal Cal: "+QString::number(mealCal));
    ui->label_menuCal->setText(" : 0 KCal");

    foodPlan->update_sumByMenu(foodPlan->firstDayWeek.addDays(index.column()),index.row(),&updateList,true);

    ui->label_menuEdit->setText("Edit: -");
    ui->actionSave->setEnabled(true);
    this->reset_menuEdit();

}

void MainWindow::on_toolButton_saveMeals_clicked()
{
    ui->progressBar_saveMeals->setValue(0);
    foodPlan->write_meals(true);
    ui->progressBar_saveMeals->setValue(100);
    ui->actionSave->setEnabled(true);
    QTimer::singleShot(2000,ui->progressBar_saveMeals,SLOT(reset()));
    ui->toolButton_saveMeals->setEnabled(false);
}

void MainWindow::on_calendarWidget_Food_clicked(const QDate &date)
{
    QString weekID = this->calc_weekID(date);

    if(foodPlan->weekPlansModel->findItems(weekID,Qt::MatchExactly,0).count() == 0 && date > firstdayofweek)
    {
        ui->listWidget_weekPlans->clearSelection();
        ui->label_foodWeekInfo->setText("Add Week: " + weekID);
        ui->comboBox_weightmode->setCurrentIndex(0);
        ui->actionNew->setEnabled(true);
    }
    else
    {
        ui->listWidget_weekPlans->setCurrentItem(ui->listWidget_weekPlans->findItems(weekID,Qt::MatchStartsWith).first());
        this->set_foodWeek(ui->listWidget_weekPlans->currentItem()->data(Qt::DisplayRole).toString());
    }
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

    if(ui->listWidget_MenuEdit->count() > 0)
    {
        if(ui->listWidget_MenuEdit->item(0)->data(Qt::DisplayRole).toString().contains("Default"))
        {
            ui->listWidget_MenuEdit->clear();
        }
    }

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

    this->set_menuList();
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

    foodPlan->update_sumByMenu(foodPlan->firstDayWeek.addDays(item->column()),item->row(),&itemList,true);
    ui->actionSave->setEnabled(true);
}

void MainWindow::on_listWidget_MenuEdit_itemClicked(QListWidgetItem *item)
{
    foodcopyMode = false;
    QString foodString,foodName,mealValues,mealCal,mealPort;
    QVector<int> mealData(2);
    foodString = item->data(Qt::DisplayRole).toString();
    foodName = foodString.split(" (").first();
    mealValues = foodString.split(" (").last();
    mealValues = mealValues.remove(")");
    mealPort = mealValues.split("-").first();
    mealCal = mealValues.split("-").last();
    mealData = foodPlan->get_mealData(foodName,false);

    double pFactor = mealPort.toDouble()/mealData.at(0);
    double d_mealCal = mealData.at(1)*pFactor;

    ui->lineEdit_Mealname->setText(foodName);
    ui->spinBox_portion->setValue(mealData.at(0));
    ui->doubleSpinBox_portion->setValue(pFactor);
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
    ui->frame_dayShow->setVisible(false);
    ui->pushButton_lineCopy->setEnabled(false);
    ui->toolButton_linePaste->setEnabled(false);
    ui->frame_menuEdit->setVisible(true);
    ui->listWidget_MenuEdit->clear();
    if(!item->data(Qt::DisplayRole).toString().isEmpty())
    {
        ui->listWidget_MenuEdit->addItems(item->data(Qt::DisplayRole).toString().split("\n"));
        this->set_menuList();
    }
    ui->label_menuEdit->setText("Edit: "+ foodPlan->dayHeader.at(item->column()) + " - " + foodPlan->mealsHeader.at(item->row()));
    ui->toolButton_foodDown->setEnabled(true);
    ui->toolButton_foodUp->setEnabled(true);
    ui->toolButton_menuEdit->setEnabled(true);
    ui->toolButton_clear->setEnabled(true);
    ui->toolButton_menuCopy->setEnabled(true);
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
    this->set_menuList();
}

void MainWindow::on_toolButton_menuCopy_clicked()
{
    menuCopy.clear();
    for(int i = 0; i < ui->listWidget_MenuEdit->count(); ++i)
    {
        menuCopy << ui->listWidget_MenuEdit->item(i)->text();
    }

    ui->toolButton_menuPaste->setEnabled(true);
}

void MainWindow::on_toolButton_menuPaste_clicked()
{

    ui->listWidget_MenuEdit->clear();
    ui->listWidget_MenuEdit->addItems(menuCopy);

    ui->toolButton_menuCopy->setEnabled(false);

    this->set_menuList();
}

void MainWindow::on_comboBox_weightmode_currentIndexChanged(const QString &value)
{
    if(ui->listWidget_weekPlans->selectedItems().count() == 1)
    {
        int modelRow = ui->listWidget_weekPlans->currentRow();
        ui->listWidget_weekPlans->currentItem()->setData(Qt::EditRole,ui->label_foodWeekInfo->text()+" - "+value);
        this->set_foodWeek(ui->listWidget_weekPlans->currentItem()->text());
        foodPlan->weekPlansModel->setData(foodPlan->weekPlansModel->index(modelRow,1),value);
        ui->comboBox_weightmode->setEnabled(false);
        ui->actionSave->setEnabled(true);
    }
}

void MainWindow::fill_selectLine(int line, int lineCounter)
{
    QStringList tempValues;
    selectedLine.clear();

    for(int i = 0; i < lineCounter;++i)
    {
        if(dayLineSelected)
        {
            tempValues = ui->tableWidget_weekPlan->item(i,line)->data(Qt::DisplayRole).toString().split("\n");
        }
        else
        {
            tempValues = ui->tableWidget_weekPlan->item(line,i)->data(Qt::DisplayRole).toString().split("\n");
        }

        selectedLine.insert(i,tempValues);
        tempValues.clear();
    }

    ui->toolButton_menuCopy->setEnabled(true);
}

void MainWindow::selectFoodMealWeek(int selectedMeal)
{
    ui->tableWidget_daySummery->clearContents();
    ui->label_dayShow->setText("Selected: "+foodPlan->mealsHeader.at(selectedMeal));

    if(!foodcopyMode)
    {
        ui->frame_dayShow->setVisible(true);
        ui->frame_menuEdit->setVisible(false);
        ui->tableWidget_daySummery->clearContents();
        ui->tableWidget_daySummery->verticalHeader()->setVisible(false);
        ui->tableWidget_daySummery->horizontalHeader()->setVisible(false);
    }
    else
    {
        if(!dayLineSelected)
        {
            ui->label_dayShow->setText(ui->label_dayShow->text()+" - Insert");
        }
        else
        {
            ui->toolButton_linePaste->setEnabled(false);
        }
    }

    ui->pushButton_lineCopy->setEnabled(true);
    foodcopyLine = selectedMeal;
    dayLineSelected = false;
}

void MainWindow::selectFoodMealDay(int selectedDay)
{
    ui->label_dayShow->setText("Selected: "+QLocale().dayName(selectedDay+1));

    if(!foodcopyMode)
    {
        ui->frame_dayShow->setVisible(true);
        ui->frame_menuEdit->setVisible(false);
        ui->tableWidget_daySummery->setVerticalHeaderLabels(foodPlan->mealsHeader);
        ui->tableWidget_daySummery->setHorizontalHeaderLabels(foodPlan->dayListHeader);
        ui->tableWidget_daySummery->setVerticalHeaderItem(foodPlan->mealsHeader.count(),new QTableWidgetItem(generalValues->value("sum")));
        ui->tableWidget_daySummery->verticalHeader()->setVisible(true);
        ui->tableWidget_daySummery->horizontalHeader()->setVisible(true);
        this->fill_dayTable(selectedDay);
    }
    else
    {
        if(dayLineSelected)
        {
            ui->label_dayShow->setText(ui->label_dayShow->text()+" - Insert");
        }
        else
        {
            ui->toolButton_linePaste->setEnabled(false);
        }
    }

    ui->pushButton_lineCopy->setEnabled(true);
    foodcopyLine = selectedDay;
    dayLineSelected = true;
}

void MainWindow::on_actionFood_Macros_triggered()
{
    foodmacro_popup foodPop(this,foodPlan,ui->calendarWidget_Food->selectedDate());
    foodPop.setModal(true);
    foodPop.exec();
}

void MainWindow::on_toolButton_switch_clicked()
{
    int firstVal = ui->spinBox_portion->value();
    double secVal = ui->doubleSpinBox_portion->value();

    ui->spinBox_portion->setValue(static_cast<int>(secVal));
    ui->doubleSpinBox_portion->setValue(firstVal);
}


void MainWindow::on_pushButton_lineCopy_toggled(bool checked)
{
    foodcopyMode = checked;

    if(dayLineSelected)
    {
        this->fill_selectLine(foodcopyLine,ui->tableWidget_weekPlan->rowCount());
    }
    else
    {
        this->fill_selectLine(foodcopyLine,ui->tableWidget_weekPlan->columnCount());
    }

    if(checked)
    {
        ui->pushButton_lineCopy->setIcon(QIcon(":/images/icons/No.png"));
    }
    else
    {
        ui->pushButton_lineCopy->setIcon(QIcon(":/images/icons/Copy.png"));
    }

    ui->toolButton_linePaste->setEnabled(checked);

}

void MainWindow::on_toolButton_linePaste_clicked()
{
    int counter = 0;
    QString tempValue;

    if(dayLineSelected)
    {
        counter = ui->tableWidget_weekPlan->rowCount();
    }
    else
    {
        counter = ui->tableWidget_weekPlan->columnCount();
    }

    for(int i = 0; i < counter; ++i)
    {
        for(int x = 0; x < selectedLine.value(i).count(); ++x)
        {
            tempValue = tempValue + selectedLine.value(i).at(x) + "\n";
        }

        tempValue.remove(tempValue.length()-1,1);

        if(dayLineSelected)
        {
            ui->tableWidget_weekPlan->item(i,foodcopyLine)->setData(Qt::EditRole,tempValue);
        }
        else
        {
            ui->tableWidget_weekPlan->item(foodcopyLine,i)->setData(Qt::EditRole,tempValue);
        }
        tempValue.clear();
    }
}

void MainWindow::on_actionfood_History_triggered()
{
    int dialog_code;
    foodhistory_popup foodHistory(this,foodPlan);
    foodHistory.setModal(true);
    dialog_code = foodHistory.exec();

    if(dialog_code == QDialog::Accepted)
    {
        ui->actionSave->setEnabled(true);
    }
}

void MainWindow::on_tableWidget_schedule_itemClicked(QTableWidgetItem *item)
{
    int dialog_code;

    if(item->column() == 0)
    {
       QString selected_week =  item->data(Qt::DisplayRole).toString();
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
               this->refresh_schedule();
               ui->actionSave->setEnabled(workSchedule->get_isUpdated());
           }
       }
    }
    else
    {
        QString getdate = item->data(Qt::DisplayRole).toString().split("#").first();
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

}

void MainWindow::on_tableWidget_saison_itemClicked(QTableWidgetItem *item)
{
    if(item->column() == 0)
    {
       QString selected_week =  item->data(Qt::DisplayRole).toString();
       Dialog_addweek addweek(this,selected_week,workSchedule);
       addweek.setModal(true);
       int dialog_code = addweek.exec();
    }
}
