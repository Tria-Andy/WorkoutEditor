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
        gcValues = settings::getStringMapPointer(settings::stingMap::GC);
        sportUse = settings::get_listValues("Sportuse").count();
        ui->lineEdit_athlete->setText(gcValues->value("athlete"));
        weekRange = settings::get_intValue("weekrange");
        weekDays = settings::get_intValue("weekdays");
        firstdayofweek = settings::firstDayofWeek;
        dateFormat = settings::get_format("dateformat");
        longTime = settings::get_format("longtime");
        shortTime = settings::get_format("shorttime");

        //Planning Mode
        graphLoaded = false;
        workSchedule = new schedule();
        saisonValues = workSchedule->get_saisonValues();
        schedMode = settings::getHeaderMap("mode");

        selectedDate = QDate::currentDate();
        foodPlan = new foodplanner(workSchedule);
        weeknumber = this->calc_weekID(selectedDate);
        saisonWeek = weekCounter = 0;
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

        connect(ui->actionExit_and_Save, SIGNAL(triggered()), this, SLOT(close()));
        connect(planMode,SIGNAL(clicked(bool)),this,SLOT(toolButton_planMode(bool)));
        connect(phaseGroup,SIGNAL(buttonClicked(int)),this,SLOT(set_phaseFilter(int)));
        connect(modules,SIGNAL(currentIndexChanged(int)),this,SLOT(set_module(int)));
        connect(workSchedule->scheduleModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(refresh_schedule()));
        connect(workSchedule->scheduleModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(refresh_schedule()));
        connect(workSchedule->phaseModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(refresh_saison()));
        connect(workSchedule->phaseModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(refresh_saison()));

        //Planer
        ui->stackedWidget->setGeometry(5,5,0,0);
        this->set_tableWidgetItems(ui->tableWidget_schedule,weekRange,cal_header.count(),&schedule_del);
        this->set_tableHeader(ui->tableWidget_schedule,&cal_header,false);
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


        //Food Planer
        this->set_tableWidgetItems(ui->tableWidget_foodPlan,foodPlan->mealsHeader.count(),foodPlan->dayHeader.count(),&foodPlan_del);
        this->set_tableHeader(ui->tableWidget_foodPlan,&foodPlan->mealsHeader,true);
        this->set_tableHeader(ui->tableWidget_foodPlan,&foodPlan->dayHeader,false);
        ui->tableWidget_foodPlan->verticalHeader()->setFixedWidth(110);
        ui->tableWidget_foodPlan->viewport()->setMouseTracking(true);
        ui->tableWidget_foodPlan->installEventFilter(this);
        ui->tableWidget_foodPlan->viewport()->installEventFilter(this);

        this->set_tableWidgetItems(ui->tableWidget_daySum,5,foodPlan->dayHeader.count(),&foodSumDay_del);
        this->set_tableHeader(ui->tableWidget_daySum,foodPlan->foodsumHeader,true);
        ui->tableWidget_daySum->verticalHeader()->setFixedWidth(110);
        ui->tableWidget_daySum->horizontalHeader()->hide();

        this->set_tableWidgetItems(ui->tableWidget_weekSum,5,1,&foodSumWeek_del);
        this->set_tableHeader(ui->tableWidget_weekSum,foodPlan->foodsumHeader,true);
        ui->tableWidget_weekSum->horizontalHeader()->hide();

        ui->treeView_meals->setModel(foodPlan->mealModel);
        ui->treeView_meals->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->treeView_meals->setSortingEnabled(true);
        ui->treeView_meals->sortByColumn(0,Qt::AscendingOrder);
        ui->treeView_meals->hideColumn(1);
        //ui->treeView_meals->setItemDelegate(&mousehover_del);
        ui->treeView_meals->setEditTriggers(QAbstractItemView::NoEditTriggers);

        connect(foodPlan->mealModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(mealSave(QStandardItem*)));
        connect(ui->tableWidget_foodPlan->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(selectFoodDay(int)));
        connect(ui->tableWidget_foodPlan->verticalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(selectFoodSection(int)));

        ui->comboBox_weightmode->blockSignals(true);
        ui->comboBox_weightmode->addItems(settings::get_listValues("Mode"));
        ui->comboBox_weightmode->setEnabled(false);
        ui->comboBox_weightmode->blockSignals(false);
        ui->spinBox_calories->setVisible(false);
        ui->spinBox_portFactor->setVisible(false);

        this->set_tableWidgetItems(ui->tableWidget_forecast,foodPlan->foodestHeader->count(),1,nullptr);
        this->set_tableHeader(ui->tableWidget_forecast,foodPlan->foodestHeader,true);
        ui->tableWidget_forecast->horizontalHeader()->hide();

        ui->toolButton_saveMeals->setEnabled(false);
        ui->toolButton_deleteMenu->setEnabled(false);
        ui->toolButton_menuCopy->setEnabled(false);
        ui->toolButton_menuPaste->setEnabled(false);
        ui->toolButton_addMeal->setProperty("Editmode",1);
        ui->frame_dayShow->setVisible(false);

        ui->tableWidget_selectedSection->setColumnCount(4);
        ui->tableWidget_selectedSection->setItemDelegate(&foodSumSelect_del);
        ui->tableWidget_selectedSection->setRowCount(foodPlan->mealsHeader.count()+1);
        ui->tableWidget_selectedSection->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_selectedSection->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->toolButton_linePaste->setEnabled(false);
        foodcopyMode = lineSelected = dayLineSelected = false;

        this->reset_menuEdit();
        this->set_speedgraph();
        this->resetPlot();
        this->read_activityFiles();
        this->loadUISettings();
        this->set_menuItems(0);
        this->set_phaseFilter(1);   
        this->set_buttons(firstdayofweek,false);
        this->fill_foodPlanList(false);
        this->fill_foodPlanTable(firstdayofweek);
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
enum {ADD,DEL,EDIT};

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

void MainWindow::set_tableWidgetItems(QTableWidget *widget, int rows, int cols,QAbstractItemDelegate *delegate)
{
    widget->blockSignals(true);
    widget->setRowCount(rows);
    widget->setColumnCount(cols);
    if(delegate != nullptr) widget->setItemDelegate(delegate);

    for(int row = 0; row < rows; ++row)
    {
        for(int col = 0; col < cols; ++col)
        {
            QTableWidgetItem *item = new QTableWidgetItem("-");
            widget->setItem(row,col,item);
        }
    }
    widget->blockSignals(false);
}

void MainWindow::set_tableHeader(QTableWidget *widget, QStringList *header, bool vert)
{
    widget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    widget->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    for(int i = 0; i < header->count(); ++i)
    {
        if(vert)
        {
            widget->setVerticalHeaderItem(i,new QTableWidgetItem(header->at(i)));
        }
        else
        {
            widget->setHorizontalHeaderItem(i,new QTableWidgetItem(header->at(i)));
        }
    }
}

void MainWindow::fill_foodPlanTable(QDate startDate)
{
    QMap<QDate,QHash<QString,QHash<QString,QVector<double>>>> *foodPlanMap = foodPlan->get_foodPlanMap();
    int day = 0;
    QString foodItem;
    QMap<int,QString> foodEntries;
    double mealCal = 0;

    ui->tableWidget_foodPlan->blockSignals(true);
    for(QMap<QDate,QHash<QString,QHash<QString,QVector<double>>>>::const_iterator datestart = foodPlanMap->find(startDate), dateend = foodPlanMap->find(startDate.addDays(7)); datestart != dateend; ++datestart,++day)
    {
        for(QHash<QString,QHash<QString,QVector<double>>>::const_iterator mealstart = datestart.value().cbegin(), mealend = datestart.value().cend(); mealstart != mealend; ++mealstart)
        {
            for(QHash<QString,QVector<double>>::const_iterator foodstart = mealstart.value().cbegin(), foodend = mealstart.value().cend(); foodstart != foodend; ++foodstart)
            {
                foodEntries.insert(static_cast<int>(foodstart.value().at(0)),foodPlan->get_mealName(foodstart.key())+" - ("+QString::number(foodstart.value().at(1)) +"-"+QString::number(foodstart.value().at(2))+")");
                mealCal = mealCal + foodstart.value().at(2);
            }

            for(QMap<int,QString>::const_iterator it = foodEntries.cbegin(), end = foodEntries.cend(); it != end; ++it)
            {
                foodItem = foodItem + it.value()+"\n";
            }
            foodEntries.clear();

            QTableWidgetItem *headerItem = ui->tableWidget_foodPlan->horizontalHeaderItem(day);
            ui->tableWidget_foodPlan->openPersistentEditor(headerItem);
            headerItem->setData(Qt::UserRole,datestart.key());
            ui->tableWidget_foodPlan->closePersistentEditor(headerItem);

            QTableWidgetItem *item = ui->tableWidget_foodPlan->item(foodPlan->mealsHeader.indexOf(mealstart.key()),day);
            ui->tableWidget_foodPlan->openPersistentEditor(item);
            item->setData(Qt::UserRole,datestart.key());
            item->setData(Qt::UserRole+1,mealstart.key());
            item->setData(Qt::EditRole,foodItem);
            item->setData(Qt::ToolTipRole,"Meal Cal: " + QString::number(mealCal));
            foodItem.clear();
            mealCal = 0;
            ui->tableWidget_foodPlan->closePersistentEditor(item);
        }
    }
    ui->tableWidget_foodPlan->blockSignals(false);

    QStringList weekInfo = ui->listWidget_weekPlans->currentIndex().data(Qt::DisplayRole).toString().split(" - ");
    ui->label_foodWeekInfo->setText(weekInfo.at(0)+" - "+weekInfo.at(1));
    ui->label_foodWeek->setText("Kw "+ ui->listWidget_weekPlans->currentIndex().data(Qt::DisplayRole).toString());
    ui->comboBox_weightmode->setCurrentText(weekInfo.at(2));
    ui->comboBox_weightmode->setEnabled(true);
    ui->actionDelete->setEnabled(true);
    ui->calendarWidget_Food->setSelectedDate(ui->listWidget_weekPlans->currentIndex().data(Qt::UserRole).toDate());
    this->reset_menuEdit();

    this->fill_foodSumTable(startDate);
}

void MainWindow::fill_foodSumTable(QDate startDate)
{
    QMap<QDate,QVector<double>> *daySumMap = foodPlan->get_daySumMap();
    int day = 0;
    QPair<double,double> minMax;
    QVector<double> weekValues(5,0);

    QString mode = foodPlan->get_mode(startDate);
    minMax.first = settings::doubleVector.value(mode).at(2)/100.0;
    minMax.second = settings::doubleVector.value(mode).at(1)/100.0;

    QTableWidgetItem *item;

    for(QMap<QDate,QVector<double>>::const_iterator it = daySumMap->find(startDate), end = daySumMap->find(startDate.addDays(7)); it != end; ++it,++day)
    {
        for(int row = 0; row < it.value().count(); ++row)
        {
            item = ui->tableWidget_daySum->item(row,day);
            ui->tableWidget_daySum->openPersistentEditor(item);
            item->setData(Qt::UserRole,mode);
            item->setData(Qt::EditRole,QString::number(it.value().at(row)));
            if(row == it.value().count()-1)
            {
                item->setData(Qt::ToolTipRole,"Range: "+QString::number(round(it.value().at(3) * minMax.first))+"-"+QString::number(round(it.value().at(3) * minMax.second)));
            }
            ui->tableWidget_daySum->closePersistentEditor(item);

            weekValues[row] = weekValues.at(row) + it.value().at(row);
            item = ui->tableWidget_weekSum->item(row,0);
            ui->tableWidget_weekSum->openPersistentEditor(item);
            item->setData(Qt::DisplayRole,weekValues.at(row));
            item->setData(Qt::UserRole,mode);
            ui->tableWidget_weekSum->closePersistentEditor(item);
        }
    }

    item = ui->tableWidget_forecast->item(0,0);
    item->setData(Qt::DisplayRole,ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole+1).toString()+" kg");
    item = ui->tableWidget_forecast->item(1,0);
    item->setData(Qt::DisplayRole,QString::number(round(weekValues.at(0) / weekDays))+" Cal");
    item = ui->tableWidget_forecast->item(2,0);
    item->setData(Qt::DisplayRole,QString::number(round(weekValues.at(3) / weekDays))+" Cal");
    item = ui->tableWidget_forecast->item(3,0);
    item->setData(Qt::DisplayRole,QString::number(round((weekValues.at(4) / weekDays)*-1))+" Cal");
    item = ui->tableWidget_forecast->item(4,0);
    item->setData(Qt::DisplayRole,QString::number(set_doubleValue(weekValues.at(4) / athleteValues->value("BodyFatCal") / 1000.0,true)*-1)+" kg");
    item->setData(Qt::UserRole,set_doubleValue(weekValues.at(4) / athleteValues->value("BodyFatCal") / 1000.0,true));
    item = ui->tableWidget_forecast->item(5,0);
    item->setData(Qt::DisplayRole,QString::number(set_doubleValue(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole+1).toDouble() - ui->tableWidget_forecast->item(4,0)->data(Qt::UserRole).toDouble(),false))+" kg");

}

void MainWindow::fill_foodPlanList(bool newWeek)
{
    QMap<QDate,QVector<QString>> *foodListMap = foodPlan->get_foodPlanList();
    QString weekString;

    ui->listWidget_weekPlans->clear();

    for(QMap<QDate,QVector<QString>>::const_iterator it = foodListMap->cbegin(), end = foodListMap->cend(); it != end; ++it)
    {
        weekString = it.value().at(0) +" - "+it.value().at(1)+" - "+it.value().at(2);
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole,weekString);
        item->setData(Qt::UserRole,it.key());
        item->setData(Qt::UserRole+1,it.value().at(3).toDouble());
        ui->listWidget_weekPlans->addItem(item);
    }

    if(newWeek)
    {
        ui->listWidget_weekPlans->setCurrentItem(ui->listWidget_weekPlans->item(foodListMap->count()-1));
        ui->listWidget_weekPlans->itemClicked(ui->listWidget_weekPlans->item(foodListMap->count()-1));
    }
    else
    {
        ui->listWidget_weekPlans->setCurrentItem(ui->listWidget_weekPlans->item(0));
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
        for(int day = 0; day < weekDays; ++day)
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
            int weeks = saisonValues->value(saison).at(2).toInt();
            QDate saisonStart = QDate::fromString(saisonValues->value(saison).at(0),dateFormat);

            for(int week = 0; week < weeks; ++week)
            {
                weekID = calc_weekID(saisonStart.addDays(week*static_cast<int>(weekDays)));
                calcSummery = compWeek->value(weekID);
                this->summery_calc(&calcSummery,&sportSummery,&sportUseList,sumCounter);
            }
            headerString = "All Phases - Weeks: " + saisonValues->value(saison).at(2);
        }
        else
        {
            for(int week = 0; week < phaseItem->rowCount(); ++week)
            {
                calcSummery = compWeek->value(phaseItem->child(week,0)->data(Qt::DisplayRole).toString());
                this->summery_calc(&calcSummery,&sportSummery,&sportUseList,sumCounter);
            }
            headerString = "Phase: " + phaseItem->data(Qt::DisplayRole).toString() + " - Weeks: " + QString::number(phaseItem->rowCount());
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
    uint weekComp = static_cast<uint>(summery->value(generalValues->value("sum")).at(1));
    double sportComp = summery->value(sport).at(1);
    double avgValue = weekComp/weekComp;

    avgValue = sportComp / weekComp;

    for(int i = 0; i < counter; ++i)
    {
        if(i == 1 && sport == generalValues->value("sum"))
        {
            valueString = valueString + "-" + QString::number(weekComp);
        }
        else if(i == 2)
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
            QTableWidgetItem *item = ui->tableWidget_schedule->item(row,col);
            ui->tableWidget_schedule->openPersistentEditor(item);
            if(col == 0)
            {
                weekInfo = workSchedule->get_weekMeta(calc_weekID(date.addDays(dayCounter)));
                itemValue = weekInfo.at(0) + delimiter + weekInfo.at(2) + delimiter + weekInfo.at(4) + delimiter + weekInfo.at(5);
                item->setData(Qt::UserRole,weekInfo.at(0));
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
                item->setData(Qt::UserRole,date.addDays(dayCounter));
                ++dayCounter;
                itemValue.chop(1);
            }
            item->setData(Qt::EditRole,itemValue);
            item->setTextAlignment(0);
            itemValue.clear();
            ui->tableWidget_schedule->editItem(item);
            ui->tableWidget_schedule->closePersistentEditor(item);
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
    weekitem->setData(Qt::UserRole,weekID);
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
    QString weekID;

    ui->tableWidget_saison->setColumnCount(sportUseList.count());
    ui->tableWidget_saison->setHorizontalHeaderLabels(sportUseList);

    if(phase == phaseGroup->button(1)->text())
    {
        QString saison = ui->comboBox_saisonName->currentText();
        QDate startDate = QDate::fromString(saisonValues->value(saison).at(0),dateFormat).addDays(weekDays*saisonWeek);

        ui->tableWidget_saison->setRowCount(static_cast<int>(weekRange));
        ui->tableWidget_saison->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        for(int week = 0; week < static_cast<int>(weekRange); ++week)
        {
            weekID = calc_weekID(startDate.addDays((week*static_cast<int>(weekDays))));
            this->set_saisonValues(&sportUseList,weekID,week);
        }
        this->summery_Set(startDate,nullptr);
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
    ui->label_month->setText("Week " + this->get_weekRange());
}

void MainWindow::refresh_saison()
{
    QString weekLabel = "Week ";
    if(phaseGroup->checkedId() != 1) weekLabel = "Phase:";
    ui->label_month->setText(weekLabel + this->get_weekRange());

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
        if(phaseGroup->checkedId() == 1)
        {
            display_weeks = QString::number(saisonWeek+1) + " - " + QString::number(static_cast<unsigned int>(saisonWeek) + static_cast<unsigned int>(weekRange));
        }
        else
        {
            display_weeks = phaseGroup->checkedButton()->text() + " - Weeks: "+ QString::number(workSchedule->get_phaseItem(phaseGroup->checkedButton()->text())->rowCount());
        }
    }

    return display_weeks;
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

    for(int i = 0; i < saisonValues->keys().count(); ++i)
    {
        ui->comboBox_saisonName->addItem(saisonValues->keys().at(i));
    }
    ui->comboBox_saisonName->setEnabled(false);
    saisonWeeks = saisonValues->value(ui->comboBox_saisonName->currentText()).at(2).toInt();
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
        foodPlan->insert_newWeek(ui->calendarWidget_Food->selectedDate().addDays(1 - ui->calendarWidget_Food->selectedDate().dayOfWeek()));
        this->fill_foodPlanList(true);
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
            foodPlan->save_foolPlan();
            //foodPlan->write_foodHistory();
            ui->actionSave->setEnabled(false);
        }
    }
}

void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
   weeknumber = this->calc_weekID(date);
   this->summery_Set(date,nullptr);
}

bool MainWindow::check_Date(QDate currentDate)
{
    QDate lastDate = workSchedule->get_stressMap()->lastKey();

    if(currentDate == firstdayofweek)
    {
        return false;
    }
    else if(currentDate >= lastDate)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void MainWindow::set_buttons(QDate currentDate,bool phase)
{
    if(isWeekMode)
    {
        ui->toolButton_weekMinus->setEnabled(this->check_Date(currentDate));
        ui->toolButton_weekCurrent->setEnabled(this->check_Date(currentDate));
        ui->toolButton_weekPlus->setEnabled(this->check_Date(currentDate.addDays(weekDays*weekRange)));
        ui->toolButton_weekFour->setEnabled(this->check_Date(currentDate.addDays(weekDays*(weekRange+4))));
        this->set_calender();
        this->refresh_schedule();
    }
    else
    {
        if(phase)
        {
            ui->toolButton_weekMinus->setEnabled(false);
            ui->toolButton_weekCurrent->setEnabled(false);
            ui->toolButton_weekFour->setEnabled(false);
            ui->toolButton_weekPlus->setEnabled(false);
        }
        else
        {
            if(saisonWeek == 0)
            {
                ui->toolButton_weekMinus->setEnabled(false);
                ui->toolButton_weekCurrent->setEnabled(false);
                ui->toolButton_weekFour->setEnabled(true);
                ui->toolButton_weekPlus->setEnabled(true);
            }
            else if(saisonWeek+weekRange >= saisonValues->value(ui->comboBox_saisonName->currentText()).at(2).toInt())
            {
                ui->toolButton_weekFour->setEnabled(false);
                ui->toolButton_weekPlus->setEnabled(false);
            }
            else
            {
                ui->toolButton_weekMinus->setEnabled(true);
                ui->toolButton_weekCurrent->setEnabled(true);
                ui->toolButton_weekFour->setEnabled(true);
                ui->toolButton_weekPlus->setEnabled(true);
            }
        }
        this->refresh_saison();
    }
}

void MainWindow::on_toolButton_weekCurrent_clicked()
{
    if(isWeekMode)
    {
        weekCounter = 0;
        this->set_buttons(firstdayofweek.addDays(weekCounter*weekDays),false);
    }
    else
    {
        saisonWeek = 0;
        this->set_buttons(QDate(),false);
    }
}

void MainWindow::on_toolButton_weekMinus_clicked()
{
    if(isWeekMode)
    {
        --weekCounter;
        this->set_buttons(firstdayofweek.addDays(weekCounter*weekDays),false);
    }
    else
    {
        --saisonWeek;
        this->set_buttons(QDate(),false);
    }
}

void MainWindow::on_toolButton_weekPlus_clicked()
{
    if(isWeekMode)
    {
        ++weekCounter;
        this->set_buttons(firstdayofweek.addDays(weekCounter*weekDays),false);
    }
    else
    {
        ++saisonWeek;
        this->set_buttons(QDate(),false);
    }
}

void MainWindow::on_toolButton_weekFour_clicked()
{
    if(isWeekMode)
    {
        weekCounter = weekCounter+4;
        this->set_buttons(firstdayofweek.addDays(weekCounter*weekDays),false);
    }
    else
    {
        saisonWeek = saisonWeek+4;
        this->set_buttons(QDate(),false);
    }
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
        year_popup year_pop(this,index.data(Qt::DisplayRole).toString(),workSchedule,phaseGroup->checkedButton()->text());
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
        this->set_buttons(QDate(),false);
    }
    else
    {
        this->set_buttons(QDate(),true);
        saisonWeek = 0;
    }

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
    }
    else
    {  
        planMode->setText(schedMode->at(1));
        this->saisonSchedule(phaseGroup->checkedButton()->text());
        this->set_phaseFilter(phaseGroup->checkedId());
    }
    ui->comboBox_saisonName->setEnabled(checked);
    ui->calendarWidget->setVisible(!checked);
    ui->frame_YearAvg->setVisible(checked);
    ui->frame_phases->setVisible(checked);
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

void MainWindow::set_menuList(QDate selectDay,QString section)
{
    QHash<QString, QPair<QString, QPair<int, double>>> foodUpdateMap = foodPlan->get_mealtoUpdate(true,selectDay,section);
    QMap<int,QVector<QString>> foodEntries;
    QVector<QString> foodData(3);
    QVector<double> menuSum(6,0);
    QVector<double> mealValues(7,0);
    QString foodString;

    ui->listWidget_menuEdit->clear();

    for(QHash<QString, QPair<QString, QPair<int, double>>>::const_iterator it = foodUpdateMap.cbegin(), end = foodUpdateMap.cend(); it != end; ++it)
    {
            mealValues = foodPlan->get_mealValues(it.key(),it.value().second.second);
            foodData[0] = it.value().first + " ("+QString::number(it.value().second.second)+" - "+QString::number(mealValues.at(1))+")";
            foodData[1] = it.key();
            foodData[2] = QString::number(it.value().second.second);
            menuSum[0] = menuSum[0] + mealValues.at(1);
            menuSum[1] = menuSum[1] + mealValues.at(2);
            menuSum[2] = menuSum[2] + mealValues.at(3);
            menuSum[3] = menuSum[3] + mealValues.at(4);
            menuSum[4] = menuSum[4] + mealValues.at(5);
            menuSum[5] = menuSum[5] + mealValues.at(6);
            foodEntries.insert(static_cast<int>(it.value().second.first),foodData);

    }

    for(QMap<int,QVector<QString>>::const_iterator it = foodEntries.cbegin(), end = foodEntries.cend(); it != end; ++it)
    {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole,it.value().at(0));
        item->setData(Qt::UserRole,it.value().at(1));
        item->setData(Qt::UserRole+1,it.value().at(2));
        ui->listWidget_menuEdit->addItem(item);
    }

    QStringList macroShort;
    macroShort << "C:" << "P:" << "F:" << "Fi:" << "S:";

    for(int i = 0; i < macroShort.count(); ++i)
    {
        foodString = foodString + " - "+macroShort.at(i)+QString::number(menuSum.at(i+1));
    }
    ui->label_menuCal->setText("Summery: "+QString::number(menuSum[0]) +" KCal - " + foodString);
    ui->toolButton_addMeal->setProperty("Editmode",DEL);
}

void MainWindow::reset_menuEdit()
{
    ui->toolButton_foodUp->setEnabled(false);
    ui->toolButton_foodDown->setEnabled(false);
    ui->toolButton_menuEdit->setEnabled(false);
    ui->toolButton_menuClear->setEnabled(false);
    ui->listWidget_menuEdit->clear();
}

void MainWindow::on_listWidget_weekPlans_itemClicked(QListWidgetItem *item)
{
    if(foodPlan->updateMap_hasData())
    {
        if(ui->dateEdit_copyDay->date().addDays(1 - ui->dateEdit_copyDay->date().dayOfWeek()) != item->data(Qt::UserRole).toDate())
        {
            ui->toolButton_linePaste->setEnabled(true);
        }
        else
        {
            ui->toolButton_linePaste->setEnabled(false);
        }
    }

    this->fill_foodPlanTable(item->data(Qt::UserRole).toDate());
}

void MainWindow::on_toolButton_addMenu_clicked()
{
    ui->toolButton_saveMeals->setEnabled(true);

}

void MainWindow::on_listWidget_menuEdit_itemDoubleClicked(QListWidgetItem *item)
{
    ui->listWidget_menuEdit->clearSelection();
    foodPlan->edit_updateMap(DEL,qMakePair(ui->dateEdit_selectDay->date(),ui->lineEdit_editSection->text()),item->data(Qt::UserRole).toString(),0);
    this->set_menuList(ui->dateEdit_selectDay->date(),ui->lineEdit_editSection->text());
    this->set_foodOrder();
}

void MainWindow::on_toolButton_menuEdit_clicked()
{
    foodPlan->update_foodPlanData(true,ui->dateEdit_selectDay->date(),QDate());
    this->fill_foodPlanTable(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate());

    ui->label_menuCal->setText(" : 0 KCal");
    ui->label_menuEdit->setText("Edit: -");
    ui->actionSave->setEnabled(true);
    this->reset_menuEdit();

}

void MainWindow::on_toolButton_saveMeals_clicked()
{
    ui->progressBar_saveMeals->setValue(0);
    foodPlan->save_mealList();
    ui->progressBar_saveMeals->setValue(100);
    ui->actionSave->setEnabled(true);
    QTimer::singleShot(2000,ui->progressBar_saveMeals,SLOT(reset()));
    ui->toolButton_saveMeals->setEnabled(false);
}

void MainWindow::on_calendarWidget_Food_clicked(const QDate &date)
{
    QDate firstDay = date.addDays(1 - date.dayOfWeek());

    for(int week = 0; week < ui->listWidget_weekPlans->count(); ++week)
    {
        if(firstDay == ui->listWidget_weekPlans->item(week)->data(Qt::UserRole).toDate())
        {
            ui->listWidget_weekPlans->setCurrentItem(ui->listWidget_weekPlans->item(week));
            ui->listWidget_weekPlans->itemClicked(ui->listWidget_weekPlans->item(week));
        }
        else
        {
            ui->listWidget_weekPlans->clearSelection();
            ui->label_foodWeekInfo->setText("Add Week: " + calc_weekID(date));
            ui->comboBox_weightmode->setCurrentIndex(0);
            ui->actionNew->setEnabled(true);
        }
    }
}

void MainWindow::on_toolButton_deleteMenu_clicked()
{
    ui->toolButton_saveMeals->setEnabled(true);
}

void MainWindow::on_doubleSpinBox_portion_valueChanged(double value)
{
    ui->lineEdit_calories->setText(QString::number(round(value *ui->spinBox_calories->value()/ui->spinBox_portFactor->value())));
}


void MainWindow::on_toolButton_addMeal_clicked()
{
    if(ui->toolButton_addMeal->property("Editmode") == EDIT)
    {
        foodPlan->edit_updateMap(EDIT,qMakePair(ui->dateEdit_selectDay->date(),ui->lineEdit_editSection->text()),ui->listWidget_menuEdit->currentItem()->data(Qt::UserRole).toString(),ui->doubleSpinBox_portion->value());
    }
    else if(ui->toolButton_addMeal->property("Editmode") == ADD)
    {
        foodPlan->edit_updateMap(ADD,qMakePair(ui->dateEdit_selectDay->date(),ui->lineEdit_editSection->text()),ui->treeView_meals->currentIndex().siblingAtColumn(1).data(Qt::DisplayRole).toString(),ui->doubleSpinBox_portion->value());
    }
    this->set_menuList(ui->dateEdit_selectDay->date(),ui->lineEdit_editSection->text());
}

void MainWindow::change_foodOrder(int moveID)
{
    int currentindex = ui->listWidget_menuEdit->currentRow();
    QListWidgetItem *currentItem = ui->listWidget_menuEdit->takeItem(currentindex);

    ui->listWidget_menuEdit->insertItem(currentindex+moveID,currentItem);
    ui->listWidget_menuEdit->setCurrentRow(currentindex+moveID);
    this->set_foodOrder();

}

void MainWindow::set_foodOrder()
{
    QMap<QString,int> orderMap;

    for(int row = 0; row < ui->listWidget_menuEdit->count(); ++row)
    {
        orderMap.insert(ui->listWidget_menuEdit->item(row)->data(Qt::UserRole).toString(),row);
    }
    foodPlan->change_updateMapOrder(qMakePair(ui->dateEdit_selectDay->date(),ui->lineEdit_editSection->text()),orderMap);
}


void MainWindow::on_toolButton_foodUp_clicked()
{
    this->change_foodOrder(-1);
}

void MainWindow::on_toolButton_foodDown_clicked()
{
    this->change_foodOrder(1);
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
        ui->actionDelete->setEnabled(false);
        ui->actionSave->setEnabled(true);
    }
}

void MainWindow::on_listWidget_menuEdit_itemClicked(QListWidgetItem *item)
{
    QPair<QString,QVector<int>> mealData = foodPlan->get_mealData(item->data(Qt::UserRole).toString());
    this->set_selectedMeal(mealData,item->data(Qt::UserRole+1).toDouble());
    ui->toolButton_addMeal->setProperty("Editmode",EDIT);
}

void MainWindow::on_treeView_meals_clicked(const QModelIndex &index)
{
    ui->listWidget_menuEdit->clearSelection();

    if(index.parent() == foodPlan->mealModel->invisibleRootItem()->index())
    {
        ui->treeView_meals->expand(index);
    }
    else
    {
        QPair<QString,QVector<int>> mealData = foodPlan->get_mealData(index.siblingAtColumn(1).data(Qt::DisplayRole).toString());

        for(int row = 0; row < ui->listWidget_menuEdit->count(); ++row)
        {
            if(ui->listWidget_menuEdit->item(row)->data(Qt::UserRole).toString() == index.siblingAtColumn(1).data(Qt::DisplayRole).toString())
            {
                ui->listWidget_menuEdit->setCurrentItem(ui->listWidget_menuEdit->item(row));
                ui->listWidget_menuEdit->itemClicked(ui->listWidget_menuEdit->currentItem());
                ui->listWidget_menuEdit->item(row)->setSelected(true);
                ui->toolButton_addMeal->setProperty("Editmode",EDIT);
                break;
            }
        }

        if(ui->toolButton_addMeal->property("Editmode") == DEL)
        {
            ui->toolButton_addMeal->setProperty("Editmode",ADD);
            this->set_selectedMeal(mealData,index.siblingAtColumn(2).data(Qt::DisplayRole).toDouble());
            ui->listWidget_menuEdit->clearSelection();
        }
    }
}

void MainWindow::set_selectedMeal(QPair<QString,QVector<int>> mealData,double portion)
{
    ui->doubleSpinBox_portion->setEnabled(true);
    ui->lineEdit_Mealname->setText(mealData.first);

    if(mealData.second.at(0) == 1)
    {
        ui->doubleSpinBox_portion->setRange(0.0,50.0);
        ui->doubleSpinBox_portion->setSingleStep(0.05);
        ui->label_portInfo->setText("Portion");
    }
    else if(mealData.second.at(0) == 100)
    {
        ui->doubleSpinBox_portion->setRange(0.0,500.0);
        ui->doubleSpinBox_portion->setSingleStep(5.0);
        ui->label_portInfo->setText("Gramm");
    }
    else
    {
        ui->doubleSpinBox_portion->setEnabled(false);
        ui->label_portInfo->setText("N/A");
    }
    ui->spinBox_portFactor->setValue(mealData.second.at(0));
    ui->spinBox_calories->setValue(mealData.second.at(1));
    ui->lineEdit_calories->setText(QString::number(mealData.second.at(1)));
    ui->doubleSpinBox_portion->setValue(portion);
}

void MainWindow::on_treeView_meals_collapsed(const QModelIndex &index)
{
    Q_UNUSED(index)
    ui->toolButton_addMenu->setEnabled(false);
    ui->toolButton_deleteMenu->setEnabled(false);
}

void MainWindow::on_tableWidget_foodPlan_itemClicked(QTableWidgetItem *item)
{
    foodPlan->fill_updateMap(true,false,item->data(Qt::UserRole).toDate(),foodPlan->mealsHeader.at(item->row()));

    ui->frame_dayShow->setVisible(false);
    ui->toolButton_linePaste->setEnabled(false);
    ui->frame_menuEdit->setVisible(true);
    ui->listWidget_menuEdit->clear();

    ui->label_menuEdit->setText("Edit: "+ foodPlan->dayHeader.at(item->column()));
    ui->lineEdit_editSection->setText(foodPlan->mealsHeader.at(item->row()));
    ui->dateEdit_selectDay->setDate(item->data(Qt::UserRole).toDate());
    ui->toolButton_foodDown->setEnabled(true);
    ui->toolButton_foodUp->setEnabled(true);
    ui->toolButton_menuEdit->setEnabled(true);
    ui->toolButton_menuClear->setEnabled(true);
    ui->toolButton_menuCopy->setEnabled(true);

    this->set_menuList(item->data(Qt::UserRole).toDate(),foodPlan->mealsHeader.at(item->row()));
}

void MainWindow::on_tableWidget_foodPlan_itemChanged(QTableWidgetItem *item)
{
    if(item->data(Qt::UserRole).toDate().isValid())
    {
        foodPlan->fill_updateMap(false,true,item->data(Qt::UserRole).toDate(),item->data(Qt::UserRole+1).toString());
        foodPlan->fill_updateMap(true,true,ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate().addDays(item->column()),ui->tableWidget_foodPlan->verticalHeaderItem(item->row())->data(Qt::DisplayRole).toString());
        foodPlan->update_foodPlanData(true,item->data(Qt::UserRole).toDate(),ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate().addDays(item->column()));
        this->fill_foodPlanTable(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate());
    }
    ui->actionSave->setEnabled(true);
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

void MainWindow::on_toolButton_menuClear_clicked()
{
    ui->listWidget_menuEdit->clearSelection();

    for(int row = 0; row < ui->listWidget_menuEdit->count(); ++row)
    {
        foodPlan->edit_updateMap(DEL,qMakePair(ui->dateEdit_selectDay->date(),ui->lineEdit_editSection->text()),ui->listWidget_menuEdit->item(row)->data(Qt::UserRole).toString(),0);
    }
    this->set_menuList(ui->dateEdit_selectDay->date(),ui->lineEdit_editSection->text());

}

void MainWindow::on_toolButton_menuCopy_clicked()
{
    menuCopy.clear();
    for(int i = 0; i < ui->listWidget_menuEdit->count(); ++i)
    {
        menuCopy << ui->listWidget_menuEdit->item(i)->text();
    }

    ui->toolButton_menuPaste->setEnabled(true);
}

void MainWindow::on_toolButton_menuPaste_clicked()
{
    ui->listWidget_menuEdit->clear();
    ui->listWidget_menuEdit->addItems(menuCopy);
    ui->toolButton_menuCopy->setEnabled(false);
}

void MainWindow::on_comboBox_weightmode_currentIndexChanged(const QString &value)
{
    if(ui->listWidget_weekPlans->selectedItems().count() == 1)
    {
        int modelRow = ui->listWidget_weekPlans->currentRow();
        ui->listWidget_weekPlans->currentItem()->setData(Qt::EditRole,ui->label_foodWeekInfo->text()+" - "+value);
        foodPlan->foodPlanModel->setData(foodPlan->foodPlanModel->index(modelRow,1),value);
        ui->comboBox_weightmode->setEnabled(false);
        ui->actionSave->setEnabled(true);
    }
}

void MainWindow::selectFoodSection(int selectedSection)
{
    ui->tableWidget_selectedSection->clearContents();
    ui->label_CopyInfo->setText("Selected Meal:");
    ui->dateEdit_copyDay->setVisible(false);
    ui->dateEdit_copyDay->setDate(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate());
    ui->lineEdit_copySection->setVisible(true);
    ui->lineEdit_copySection->setText(foodPlan->mealsHeader.at(selectedSection));

    foodPlan->clear_updateMap();
    foodPlan->dayMealCopy.first = false;
    foodPlan->dayMealCopy.second = true;
    for(int day = 0; day < weekDays; ++day)
    {
        foodPlan->fill_updateMap(false,false,ui->tableWidget_foodPlan->horizontalHeaderItem(day)->data(Qt::UserRole).toDate(),foodPlan->mealsHeader.at(selectedSection));
    }

    ui->frame_dayShow->setVisible(true);
    ui->frame_menuEdit->setVisible(false);
    ui->tableWidget_selectedSection->clearContents();
    ui->tableWidget_selectedSection->verticalHeader()->setVisible(false);
    ui->tableWidget_selectedSection->horizontalHeader()->setVisible(false);
}

void MainWindow::selectFoodDay(int selectedDay)
{
    ui->label_CopyInfo->setText("Selected "+ QLocale().dayName(selectedDay+1)+":");
    ui->lineEdit_copySection->setVisible(false);
    ui->dateEdit_copyDay->setVisible(true);
    ui->dateEdit_copyDay->setDate(ui->tableWidget_foodPlan->horizontalHeaderItem(selectedDay)->data(Qt::UserRole).toDate());

    foodPlan->clear_updateMap();
    foodPlan->dayMealCopy.first = true;
    foodPlan->dayMealCopy.second = false;
    for(int section = 0; section < foodPlan->mealsHeader.count(); ++section)
    {
        foodPlan->fill_updateMap(false,false,ui->tableWidget_foodPlan->horizontalHeaderItem(selectedDay)->data(Qt::UserRole).toDate(),foodPlan->mealsHeader.at(section));
    }

    ui->frame_dayShow->setVisible(true);
    ui->frame_menuEdit->setVisible(false);
    ui->tableWidget_selectedSection->clearContents();
    ui->tableWidget_selectedSection->setVerticalHeaderLabels(foodPlan->mealsHeader);
    ui->tableWidget_selectedSection->setHorizontalHeaderLabels(foodPlan->dayListHeader);
    ui->tableWidget_selectedSection->setVerticalHeaderItem(foodPlan->mealsHeader.count(),new QTableWidgetItem(generalValues->value("sum")));
    ui->tableWidget_selectedSection->verticalHeader()->setVisible(true);
    ui->tableWidget_selectedSection->horizontalHeader()->setVisible(true);

    //Fill SelectionWidget
    QHash<QString,QHash<QString,QVector<double>>> dayFoodValues = foodPlan->get_foodPlanMap()->value(ui->tableWidget_foodPlan->horizontalHeaderItem(selectedDay)->data(Qt::UserRole).toDate());
    QHash<QString,QVector<double>> dayFoodMap,dayMacroSum;
    QVector<double> dayValues(4,0);

    for(int mealSection = 0; mealSection < foodPlan->mealsHeader.count(); ++mealSection)
    {
        dayFoodMap = dayFoodValues.value(foodPlan->mealsHeader.at(mealSection));

        for(QHash<QString,QVector<double>>::const_iterator it = dayFoodMap.cbegin(), end = dayFoodMap.cend(); it != end; ++it)
        {
            dayValues[0] = round(dayValues[0] + it.value().at(2));
            dayValues[1] = round(dayValues[1] + (it.value().at(3)*4.1));
            dayValues[2] = round(dayValues[2] + (it.value().at(4)*4.1));
            dayValues[3] = round(dayValues[3] + (it.value().at(5)*9.3));
        }
        dayMacroSum.insert(foodPlan->mealsHeader.at(mealSection),dayValues);
        dayValues.fill(0);
    }

    for(QHash<QString,QVector<double>>::const_iterator it = dayMacroSum.cbegin(), end = dayMacroSum.cend(); it != end; ++it)
    {
        dayValues[0] = dayValues[0] + it.value().at(0);
        dayValues[1] = dayValues[1] + it.value().at(1);
        dayValues[2] = dayValues[2] + it.value().at(2);
        dayValues[3] = dayValues[3] + it.value().at(3);
    }
    dayMacroSum.insert(generalValues->value("sum"),dayValues);

    QString macroValue,macroPercent;
    double percent;

    for(int mealSection = 0; mealSection < foodPlan->mealsHeader.count()+1; ++mealSection)
    {
        for(int col = 0; col < foodPlan->dayListHeader.count(); ++col)
        {
            if(mealSection == foodPlan->mealsHeader.count())
            {
                macroValue = QString::number(dayMacroSum.value(generalValues->value("sum")).at(col));
                if(col == 0)
                {
                    percent = dayMacroSum.value(generalValues->value("sum")).at(col) / dayMacroSum.value(generalValues->value("sum")).at(col);
                }
                else
                {
                    percent = dayMacroSum.value(generalValues->value("sum")).at(col) / dayMacroSum.value(generalValues->value("sum")).at(0);
                }
            }
            else
            {
                macroValue = QString::number(dayMacroSum.value(foodPlan->mealsHeader.at(mealSection)).at(col));
                if(col == 0)
                {
                    percent = dayMacroSum.value(foodPlan->mealsHeader.at(mealSection)).at(col) / dayMacroSum.value(generalValues->value("sum")).at(col);
                }
                else
                {
                    percent = dayMacroSum.value(foodPlan->mealsHeader.at(mealSection)).at(col) / dayMacroSum.value(foodPlan->mealsHeader.at(mealSection)).at(0);
                }
            }
            QTableWidgetItem *item = new QTableWidgetItem();
            item->setData(Qt::DisplayRole,macroValue + " ("+QString::number(set_doubleValue(percent*100.0,false))+")");
            item->setData(Qt::UserRole,set_doubleValue(percent*100.0,false));
            ui->tableWidget_selectedSection->setItem(mealSection,col,item);
        }
    }
}

void MainWindow::on_actionFood_Macros_triggered()
{
    foodmacro_popup foodPop(this,foodPlan,ui->calendarWidget_Food->selectedDate());
    foodPop.setModal(true);
    foodPop.exec();
}

void MainWindow::on_toolButton_linePaste_clicked()
{
    foodPlan->update_foodPlanData(false,ui->dateEdit_copyDay->date(),
                                  ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate().addDays(ui->dateEdit_copyDay->date().dayOfWeek()-1));
    this->fill_foodPlanTable(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate());
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
       QString selected_week =  item->data(Qt::UserRole).toString();
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
        day_popup day_pop(this,item->data(Qt::UserRole).toDate(),workSchedule);
        day_pop.setModal(true);
        dialog_code = day_pop.exec();

        if(dialog_code == QDialog::Rejected)
        {
            ui->actionSave->setEnabled(workSchedule->get_isUpdated());
        }
    }

}

void MainWindow::on_tableWidget_saison_itemClicked(QTableWidgetItem *item)
{
    if(item->column() == 0)
    {
       Dialog_addweek addweek(this,item->data(Qt::UserRole).toString(),workSchedule);
       addweek.setModal(true);
       addweek.exec();
    }
}
