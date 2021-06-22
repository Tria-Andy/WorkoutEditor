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
        generalValues = settings::getStringMapPointer(settings::stingMap::General);
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
        stdWorkouts = new standardWorkouts();
        workSchedule = new schedule(stdWorkouts);
        currActivity = new Activity();

        saisonValues = workSchedule->get_saisonValues();
        schedMode = settings::getHeaderMap("mode");

        selectedDate = QDate::currentDate();
        foodPlan = new foodplanner(workSchedule);
        weeknumber = this->calc_weekID(selectedDate);
        saisonWeek = weekCounter = 0;
        isWeekMode = true;
        buttonStyle = "QToolButton:hover {color: white; border: 1px solid white; border-radius: 4px; background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,stop: 0 #00b8ff, stop: 0.5 #0086ff,stop: 1 #0064ff)}";

        ui->label_month->setText("Week " + weeknumber + " - " + QString::number(selectedDate.addDays(weekRange*weekDays).weekNumber()-1));
        iconMap.insert("Planner",QIcon(":/images/icons/DateTime.png"));
        iconMap.insert("Editor",QIcon(":/images/icons/Editor.png"));
        iconMap.insert("Food",QIcon(":/images/icons/Food.png"));
        iconMap.insert("PaneUp",QIcon(":/images/icons/Arrow_up.png"));
        iconMap.insert("PaneDown",QIcon(":/images/icons/Arrow_down.png"));
        iconMap.insert("AddAvg",QIcon(":/images/icons/AddAvg.png"));
        iconMap.insert("MinAvg",QIcon(":/images/icons/MinAvg.png"));

        modules = new QComboBox(this);
        modules->addItem(iconMap.value("Planner"),"Planner");
        modules->addItem(iconMap.value("Editor"),"Editor");
        modules->addItem(iconMap.value("Food"),"Nutrition");
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
        ui->toolButton_weekCurrent->setEnabled(false);
        ui->toolButton_weekMinus->setEnabled(false);
        this->set_phaseButtons();
        ui->frame_phases->setVisible(false);
        cal_header << "Week";
        for(int d = 1; d < 8; ++d)
        {
            cal_header << QLocale().dayName(d);
        }
        avgHeader = settings::getHeaderMap("average");

        statusLabel = new QLabel(this);
        statusLabel->setText("Status");
        ui->statusBar->addWidget(statusLabel);
        statusProgress = new QProgressBar(this);
        statusProgress->setRange(0,100);
        statusProgress->setTextVisible(false);
        statusProgress->setMaximumSize(10000,10);

        ui->statusBar->addPermanentWidget(statusProgress,1);

        //Editor Mode
        actLoaded = false;

        this->set_tableWidgetItems(ui->tableWidget_actInfo,currActivity->infoHeader->count(),1,&avgSelect_del);
        ui->tableWidget_actInfo->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableWidget_actInfo->horizontalHeader()->setVisible(false);
        ui->tableWidget_actInfo->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_actInfo->verticalHeader()->setSectionsClickable(false);
        this->set_tableHeader(ui->tableWidget_actInfo,currActivity->infoHeader,true);

        ui->treeWidget_activityfiles->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->treeWidget_activityfiles->header()->setSectionResizeMode(QHeaderView::Stretch);
        ui->treeWidget_activityfiles->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
        ui->treeWidget_activityfiles->setColumnCount(currActivity->gcActivtiesMap.last().count());
        ui->treeWidget_activityfiles->setHeaderHidden(true);
        ui->treeWidget_activityfiles->setItemDelegate(&fileList_del);

        ui->treeWidget_activity->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->treeWidget_activity->header()->setSectionResizeMode(QHeaderView::Stretch);
        ui->treeWidget_activity->header()->setVisible(false);
        ui->treeWidget_activity->setItemDelegate(&tree_del);

        ui->tableWidget_avgValues->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->tableWidget_avgValues->verticalHeader()->setFixedWidth(100);
        ui->tableWidget_avgValues->verticalHeader()->setMaximumSectionSize(25);
        ui->tableWidget_avgValues->verticalHeader()->setSectionsClickable(false);
        ui->tableWidget_avgValues->horizontalHeader()->setVisible(false);
        ui->tableWidget_avgValues->verticalHeader()->setVisible(false);
        ui->tableWidget_avgValues->setItemDelegate(&avgSelect_del);

        ui->comboBox_swimType->addItems(settings::get_listValues("SwimStyle"));
        ui->comboBox_swimType->setVisible(false);
        ui->label_swimType->setVisible(false);
        ui->toolButton_split->setVisible(false);
        ui->toolButton_merge->setVisible(false);

        //Planer
        ui->stackedWidget->setGeometry(5,5,0,0);
        this->set_tableWidgetItems(ui->tableWidget_schedule,weekRange,cal_header.count(),&schedule_del);
        this->set_tableHeader(ui->tableWidget_schedule,&cal_header,false);
        ui->tableWidget_schedule->verticalHeader()->hide();
        ui->frame_PMC->setVisible(false);
        ui->frame_saisonEstimate->setVisible(false);
        ui->pushButton_stressPlot->setChecked(false);
        ui->pushButton_stressPlot->setIcon(iconMap.value("PaneUp"));
        ui->spinBox_extWeeks->setRange(3,12);

        //ui->tableWidget_schedule->setMouseTracking(true);
        ui->tableWidget_saison->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_saison->setItemDelegate(&saison_del);
        ui->tableWidget_saison->verticalHeader()->hide();
        ui->tableWidget_saison->setVisible(false);

        ui->tableWidget_summery->setColumnCount(1);
        ui->tableWidget_summery->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_summery->verticalHeader()->setDefaultSectionSize(ui->tableWidget_summery->verticalHeader()->defaultSectionSize()*4);
        ui->tableWidget_summery->verticalHeader()->hide();
        ui->tableWidget_summery->setItemDelegate(&sum_del); 

        ui->tableWidget_weekAvg->setColumnCount(1);
        ui->tableWidget_weekAvg->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_weekAvg->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->tableWidget_weekAvg->horizontalHeader()->hide();
        ui->tableWidget_weekAvg->verticalHeader()->hide();
        ui->tableWidget_weekAvg->setItemDelegate(&avgweek_del);
        ui->tableWidget_weekAvg->setVisible(false);
        ui->comboBox_saisonSport->addItem(generalValues->value("sum"));
        ui->comboBox_saisonSport->addItems(settings::get_listValues("Sportuse"));

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

        this->set_tableWidgetItems(ui->tableWidget_daySum,6,foodPlan->dayHeader.count(),&foodSumDay_del);
        this->set_tableHeader(ui->tableWidget_daySum,foodPlan->foodsumHeader,true);
        ui->tableWidget_daySum->verticalHeader()->setFixedWidth(110);
        ui->tableWidget_daySum->horizontalHeader()->hide();

        this->set_tableWidgetItems(ui->tableWidget_weekSum,foodPlan->foodweekHeader->count(),1,&foodSumWeek_del);
        this->set_tableHeader(ui->tableWidget_weekSum,foodPlan->foodweekHeader,true);
        ui->tableWidget_weekSum->horizontalHeader()->hide();

        QStringList headerItems;
        for(int i = 0; i < settings::getHeaderMap("recipeheader")->count(); ++i)
        {
            headerItems << settings::getHeaderMap("recipeheader")->at(i);
        }
        ui->treeWidget_recipe->setHeaderLabels(headerItems);
        ui->treeWidget_recipe->header()->setSectionResizeMode(QHeaderView::Stretch);
        ui->treeWidget_recipe->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
        ui->treeWidget_ingred->setHeaderLabels(headerItems);
        ui->treeWidget_ingred->header()->setSectionResizeMode(QHeaderView::Stretch);
        ui->treeWidget_ingred->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
        ui->treeWidget_drink->setHeaderLabels(headerItems);
        ui->treeWidget_drink->header()->setSectionResizeMode(QHeaderView::Stretch);
        ui->treeWidget_drink->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);

        connect(ui->actionExit_and_Save, SIGNAL(triggered()), this, SLOT(close()));
        connect(planMode,SIGNAL(clicked(bool)),this,SLOT(toolButton_planMode(bool)));
        connect(phaseGroup,SIGNAL(buttonClicked(int)),this,SLOT(set_phaseFilter(int)));
        connect(modules,SIGNAL(currentIndexChanged(int)),this,SLOT(set_module(int)));
        connect(workSchedule->scheduleModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(refresh_schedule()));
        connect(workSchedule->scheduleModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(refresh_schedule()));
        connect(workSchedule->phaseModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(refresh_saison()));
        connect(workSchedule->phaseModel,SIGNAL(rowsRemoved(QModelIndex,int,int)),this,SLOT(refresh_saison()));
        connect(workSchedule->stressPlot,SIGNAL(selectionChangedByUser()),this,SLOT(selectionChanged_stressPlot()));
        connect(workSchedule,&schedule::compValueChanged,this,SLOT(refresh_summery()));
        connect(ui->tableWidget_foodPlan->horizontalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(selectFoodDay(int)));
        connect(ui->tableWidget_foodPlan->verticalHeader(),SIGNAL(sectionClicked(int)),this,SLOT(selectFoodSection(int)));
        connect(foodPlan->foodPlanModel,SIGNAL(rowsInserted(QModelIndex,int,int)),this,SLOT(refresh_foodTables()));
        connect(foodPlan->summeryModel,SIGNAL(itemChanged(QStandardItem*)),this,SLOT(refresh_summeryTables()));

        ui->comboBox_weightmode->blockSignals(true);
        ui->comboBox_weightmode->addItems(settings::get_listValues("Mode"));
        ui->comboBox_weightmode->setEnabled(false);
        ui->comboBox_weightmode->blockSignals(false);
        ui->spinBox_calories->setVisible(false);
        ui->spinBox_portFactor->setVisible(false);

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

        ui->stressPlot_Layout->addWidget(workSchedule->stressPlot,1);
        ui->stressPlot_Layout->addWidget(workSchedule->compPlot,1);
        ui->stressPlot_Layout->itemAt(1)->widget()->setVisible(false);
        ui->distribution_Layout->addWidget(workSchedule->levelPlot,1);

        this->reset_menuEdit();
        this->init_polishgraph();

        this->resetPlot();
        this->loadUISettings();
        this->activityList();
        this->set_menuItems(0);
        this->set_phaseFilter(1);   
        this->fill_foodTrees();
        this->set_buttons(firstdayofweek,false);
        this->fill_foodPlanList(false,0);
        this->fill_foodPlanTable(firstdayofweek);
        this->fill_foodSumTable(firstdayofweek);
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

void MainWindow::set_progress(int value)
{
    statusProgress->setValue(value);

    if(value == 100)
    {
        QTimer::singleShot(2000,statusProgress,SLOT(reset()));
        statusLabel->setText("Status");
    }
}

void MainWindow::freeMem()
{
    this->clearActivtiy();

    if(userSetup == 0)
    {
        delete workSchedule;
        delete currActivity;
        delete stdWorkouts;
        delete foodPlan;
    }
}

bool MainWindow::eventFilter(QObject *object, QEvent *event)
{
    if( object == ui->tableWidget_foodPlan->viewport() && event->type() == QEvent::Drop )
    {
        //ui->tableWidget_foodPlan->blockSignals(true);
    }

    return false;
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
    QStandardItem *weekItem = foodPlan->get_proxyItem(0);
    QStandardItem *dayItem,*sectionItem;

    QMultiMap<int,QString> foodItemMap;

    QTableWidgetItem *planItem;
    QString foodName,foodString;

    double portion = 0;
    double calories = 0;
    double mealCalories = 0;

    for(int day = 0; day < weekItem->rowCount(); ++day)
    {
        ui->tableWidget_foodPlan->horizontalHeaderItem(day)->setData(Qt::UserRole,startDate.addDays(day));

        dayItem = weekItem->child(day);

        if(dayItem->hasChildren())
        {
            for(int section = 0; section < dayItem->rowCount(); ++section)
            {
                sectionItem = dayItem->child(section);

                if(sectionItem->hasChildren())
                {
                    foodString.clear();
                    for(int meal = 0; meal < sectionItem->rowCount(); ++meal)
                    {                        
                        foodName = sectionItem->child(meal,1)->data(Qt::DisplayRole).toString();
                        portion = sectionItem->child(meal,2)->data(Qt::DisplayRole).toDouble();
                        calories = sectionItem->child(meal,3)->data(Qt::DisplayRole).toDouble();
                        mealCalories = mealCalories + calories;
                        foodString = foodName +" ("+QString::number(portion)+"-"+QString::number(calories)+")"+"\n";
                        foodItemMap.insert(sectionItem->child(meal,9)->data(Qt::DisplayRole).toInt(),foodString);
                    }
                }

                foodString.clear();

                for(QMultiMap<int,QString>::const_iterator it = foodItemMap.cbegin(); it != foodItemMap.cend(); ++it)
                {
                    foodString = foodString + it.value();           
                }

                planItem = ui->tableWidget_foodPlan->item(section,day);
                planItem->setData(Qt::DisplayRole,foodString);
                planItem->setData(Qt::AccessibleTextRole,dayItem->data(Qt::DisplayRole));
                planItem->setData(Qt::AccessibleDescriptionRole,sectionItem->data(Qt::DisplayRole));
                planItem->setData(Qt::UserRole,startDate.addDays(day));
                planItem->setData(Qt::ToolTipRole,"Meal Cal: " + QString::number(mealCalories));

                mealCalories = 0;
                foodItemMap.clear();
            }
        }
    }
}

void MainWindow::fill_foodSumTable(QDate startDate)
{
    QStandardItem *weekItem;
    QPair<double,double> minMax,calcWeight;
    QString mode = foodPlan->get_mode(startDate);
    QHash<QString,double> lossMap = settings::modeMap.value(mode).value("losspercent");
    minMax.first = settings::doubleVector.value(mode).at(2)/100.0;
    minMax.second = settings::doubleVector.value(mode).at(1)/100.0;
    QVector<double> weekValues(5,0);
    const int timeOffset = 72000;
    double weightLoss = 0;
    int sumCal = 0;
    double slideSum = 0;
    double dayDiff = 0;

    weekItem = foodPlan->get_proxyItem(1);

    for(int day = 0; day < weekItem->rowCount(); ++day)
    {
        sumCal = weekItem->child(day,0)->data(Qt::UserRole+3).toInt();
        dayDiff = weekItem->child(day,0)->data(Qt::UserRole+4).toInt();
        slideSum = foodPlan->get_slideValue(startDate.addDays(day));
        calcWeight = foodPlan->get_estimateWeight(startDate.addDays(day).startOfDay().addSecs(timeOffset));

        for(int cal = 0; cal < weekValues.count(); ++cal)
        {
            weekValues[cal] = weekValues.at(cal) + weekItem->child(day,0)->data(Qt::UserRole+cal).toDouble();

            if(cal < weekValues.count()-1)
            {
                ui->tableWidget_daySum->item(cal,day)->setData(Qt::DisplayRole,weekItem->child(day,0)->data(Qt::UserRole+cal).toInt());
                ui->tableWidget_daySum->item(cal,day)->setData(Qt::UserRole,mode);
            }
            else
            {
                ui->tableWidget_daySum->item(cal,day)->setData(Qt::DisplayRole,QString::number(slideSum)+" - " +QString::number(round(dayDiff)));
                ui->tableWidget_daySum->item(cal,day)->setData(Qt::UserRole,mode);
                ui->tableWidget_daySum->item(cal,day)->setData(Qt::UserRole+1,slideSum);
                ui->tableWidget_daySum->item(cal,day)->setData(Qt::UserRole+2,weekItem->child(day,0)->data(Qt::UserRole+4).toInt());
                ui->tableWidget_daySum->item(cal,day)->setData(Qt::ToolTipRole,"Range: "+QString::number(round(sumCal * minMax.first))+" - "+QString::number(round(sumCal * minMax.second)));
            }
        }
        ui->tableWidget_daySum->item(5,day)->setData(Qt::DisplayRole,QString::number(set_doubleValue(calcWeight.first,false))+" -> "+QString::number(set_doubleValue(calcWeight.second,false)));
    }

    double startWeight = 0;

    if(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate().daysTo(firstdayofweek) == 0)
    {
        startWeight = set_doubleValue(foodPlan->get_estimateWeight(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate().startOfDay()).second,false);
    }
    else
    {
        startWeight = set_doubleValue(foodPlan->get_estimateWeight(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate().startOfDay().addSecs(timeOffset)).second,false);
    }

    weightLoss = set_doubleValue(weekValues.at(4) / athleteValues->value("BodyFatCal") / 1000.0,true)*-1;
    double weekAvg = round(weekValues.at(3) / weekDays);

    ui->tableWidget_weekSum->item(0,0)->setData(Qt::DisplayRole,QString::number(startWeight));
    ui->tableWidget_weekSum->item(0,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(1,0)->setData(Qt::DisplayRole,QString::number(weekValues.at(0)));
    ui->tableWidget_weekSum->item(1,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(2,0)->setData(Qt::DisplayRole,QString::number(round(weekValues.at(0) / weekDays)));
    ui->tableWidget_weekSum->item(2,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(3,0)->setData(Qt::DisplayRole,QString::number(weekValues.at(1)));
    ui->tableWidget_weekSum->item(3,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(4,0)->setData(Qt::DisplayRole,QString::number(weekValues.at(2)));
    ui->tableWidget_weekSum->item(4,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(5,0)->setData(Qt::DisplayRole,QString::number(weekAvg));
    ui->tableWidget_weekSum->item(5,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(6,0)->setData(Qt::DisplayRole,QString::number(weekValues.at(3)));
    ui->tableWidget_weekSum->item(6,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(7,0)->setData(Qt::DisplayRole,QString::number(weekValues.at(4)));
    ui->tableWidget_weekSum->item(7,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(7,0)->setData(Qt::ToolTipRole,"Range: "+QString::number(round(weekValues.at(3) * minMax.first))+" - "+QString::number(round(weekValues.at(3) * minMax.second)));
    ui->tableWidget_weekSum->item(8,0)->setData(Qt::DisplayRole,QString::number(round((weekValues.at(4) / weekDays)*-1)));
    ui->tableWidget_weekSum->item(8,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(8,0)->setData(Qt::ToolTipRole,"Range: "+QString::number(round(weekAvg * minMax.first))+" - "+QString::number(round(weekAvg * minMax.second)));
    ui->tableWidget_weekSum->item(9,0)->setData(Qt::DisplayRole,QString::number(weightLoss));
    ui->tableWidget_weekSum->item(9,0)->setData(Qt::ToolTipRole,"Range: "+QString::number(set_doubleValue((startWeight * lossMap.value("low"))/100.0,true))+" - "+QString::number(set_doubleValue((startWeight * lossMap.value("high"))/100.0,true)));
    ui->tableWidget_weekSum->item(9,0)->setData(Qt::UserRole,mode);
    ui->tableWidget_weekSum->item(10,0)->setData(Qt::DisplayRole,QString::number(set_doubleValue(foodPlan->get_estimateWeight(startDate.addDays(6).startOfDay().addSecs(timeOffset)).second,false)));
}

void MainWindow::fill_foodPlanList(bool newWeek,int setIndex)
{
    ui->listWidget_weekPlans->clear();

    QString weekString;
    QListWidgetItem *item;
    QDate firstDay;

    for(int week = 1; week < foodPlan->foodPlanModel->rowCount(); ++week)
    {
        firstDay = QDate().fromString(foodPlan->foodPlanModel->item(week,1)->data(Qt::DisplayRole).toString(),"yyyy-MM-dd");

        item = new QListWidgetItem();
        weekString = foodPlan->foodPlanModel->item(week,0)->data(Qt::DisplayRole).toString()+ " - "+
                     firstDay.toString(dateFormat)+ " - "+
                     foodPlan->foodPlanModel->item(week,2)->data(Qt::DisplayRole).toString();
        item->setData(Qt::DisplayRole,weekString);
        item->setData(Qt::AccessibleTextRole,foodPlan->foodPlanModel->item(week,0)->data(Qt::DisplayRole).toString());
        item->setData(Qt::AccessibleDescriptionRole,foodPlan->foodPlanModel->item(week,2)->data(Qt::DisplayRole).toString());
        item->setData(Qt::UserRole,firstDay);

        ui->listWidget_weekPlans->addItem(item);
    }

    if(newWeek)
    {
        ui->listWidget_weekPlans->setCurrentItem(ui->listWidget_weekPlans->item(ui->listWidget_weekPlans->count()-1));
    }
    else
    {
        ui->listWidget_weekPlans->setCurrentItem(ui->listWidget_weekPlans->item(setIndex));  
    }

    emit ui->listWidget_weekPlans->itemClicked(ui->listWidget_weekPlans->currentItem());
}

bool MainWindow::clearActivtiy()
{
    this->resetPlot();
    ui->treeWidget_activity->clear();
    ui->treeWidget_activity->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidget_activity->header()->setVisible(false);
    ui->tableWidget_avgValues->clear();
    ui->tableWidget_avgValues->verticalHeader()->setVisible(false);
    ui->tableWidget_actInfo->clearContents();

    QString viewBackground = "background-color: #e6e6e6";

    ui->lineEdit_workContent->clear();
    ui->tableWidget_actInfo->setStyleSheet(viewBackground);
    ui->treeWidget_activity->setStyleSheet(viewBackground);
    ui->treeWidget_activityfiles->setStyleSheet(viewBackground);

    ui->actionSelect_File->setEnabled(true);
    ui->actionReset->setEnabled(false);

    return currActivity->clear_loadedActivity();
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
        ui->actionIntervall_Editor->setVisible(true);
        ui->actionPace_Calculator->setVisible(true);
        ui->actionStress_Calculator->setVisible(true);
        ui->actionNew->setVisible(true);
        planerMode->setEnabled(true);
        planMode->setEnabled(true);
        planerMode->setVisible(true);

        ui->actionFood_Macros->setVisible(false);
        ui->actionSelect_File->setVisible(false);
        ui->actionDelete->setVisible(false);
        ui->actionRefresh_Filelist->setVisible(false);
        ui->actionReset->setVisible(false);
        ui->actionExpand_Activity_Tree->setVisible(false);
    }
    if(module == EDITOR)
    {
        ui->actionRefresh_Filelist->setVisible(true);
        ui->actionIntervall_Editor->setVisible(true);
        ui->actionExpand_Activity_Tree->setVisible(true);
        ui->actionReset->setVisible(true);
        ui->actionSelect_File->setVisible(true);
        ui->actionReset->setEnabled(actLoaded);
        ui->actionPace_Calculator->setVisible(true);
        ui->actionStress_Calculator->setVisible(true);

        ui->actionFood_Macros->setVisible(false);
        ui->actionDelete->setVisible(false);
    }
    if(module == FOOD)
    { 
        ui->actionFood_Macros->setVisible(true);
        ui->actionDelete->setVisible(true);
        ui->actionDelete->setEnabled(false);
        ui->actionNew->setVisible(true);
        ui->actionNew->setEnabled(false);

        ui->actionReset->setVisible(false);
        ui->actionPace_Calculator->setVisible(false);
        ui->actionStress_Calculator->setVisible(false);
        ui->actionIntervall_Editor->setVisible(false);
        ui->actionRefresh_Filelist->setVisible(false);
        ui->actionExpand_Activity_Tree->setVisible(false);
        planerMode->setVisible(false);
        planMode->setEnabled(false);

        //this->fill_foodSumTable(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate());
    }
}

//Planner Functions ***********************************************************************************

void MainWindow::summery_Set(QDate date, int weekCount)
{
    QStringList sportUseList,headerInfo;
    QString sportUse,headerString;
    sportUse = generalValues->value("sum");
    sportUseList << sportUse;

    int sumCounter = 0;

    QMap<QString,QVector<double>> sportSummery;
    QMap<QString,QVector<double>> calcSummery;

    if(isWeekMode)
    {
        sumCounter = settings::getHeaderMap("summery")->count();
        sportSummery.insert(sportUse,QVector<double>(sumCounter,0));
        ui->label_selection->setText("Week:");

        QHash<QDate,QMap<QString,QVector<double> >> *comp = workSchedule->get_compValues();

        for(int day = 0; day < weekDays; ++day)
        {
            calcSummery = comp->value(date.addDays(day));

            for(QMap<QString,QVector<double>>::const_iterator workout = calcSummery.cbegin(); workout != calcSummery.cend(); ++workout)
            {
                    if(workout.key() != settings::OtherLabel)
                    {
                        if(!sportUseList.contains(workout.key())) sportUseList << workout.key();
                    }
            }
            this->summery_calc(&calcSummery,&sportSummery,&sportUseList,sumCounter);
        }

        headerInfo = workSchedule->get_weekScheduleMeta(this->calc_weekID(date));
        headerString = "Week: " + headerInfo.at(0) + " - " + "Phase: " + headerInfo.at(1);
        ui->lineEdit_currentSelect->setText(headerInfo.at(0));
    }
    else
    {
        sportUseList << settings::get_listValues("Sportuse");
        sumCounter = settings::getHeaderMap("summery")->count()-1;
        sportSummery.insert(sportUse,QVector<double>(sumCounter,0));
        ui->label_selection->setText("Phase:");
        QString saison = ui->comboBox_saisonName->currentText();

        QMap<QString,QMap<QString,QVector<double>>> *compWeek = workSchedule->get_compWeekValues();
        QMap<QDate,QPair<QString,QString>> saisonWeekMap = workSchedule->get_saisonWeekMap()->value(saison);

        if(phaseGroup->checkedId() == 1)
        {
            for(QMap<QDate,QPair<QString,QString>>::const_iterator week = saisonWeekMap.cbegin(); week != saisonWeekMap.cend(); ++week)
            {
                    calcSummery = compWeek->value(week.value().first);
                    this->summery_calc(&calcSummery,&sportSummery,&sportUseList,sumCounter);
            }

            headerString = "All Phases - Weeks: " + QString::number(weekCount);
            ui->lineEdit_currentSelect->setText("All Phases");
        }
        else
        {
            QString phaseName = phaseGroup->checkedButton()->text();

            for(QMap<QDate,QPair<QString,QString>>::iterator week = saisonWeekMap.find(date); week != saisonWeekMap.find(date.addDays(weekCount*weekDays)); ++week)
            {
                calcSummery = compWeek->value(week.value().first);
                this->summery_calc(&calcSummery,&sportSummery,&sportUseList,sumCounter);
            }

            headerString = "Phase: " + phaseName + " - Weeks: " + QString::number(weekCount);
            ui->lineEdit_currentSelect->setText(phaseName);
        }

        ui->tableWidget_weekAvg->clear();
        ui->tableWidget_weekAvg->setRowCount(sportUseList.count());

        QString avgValue;

        for(int sports = 0; sports < sportUseList.count(); ++sports)
        {
            sportUse = sportUseList.at(sports);

            if(sportSummery.contains(sportUse))
            {
                QTableWidgetItem *item = new QTableWidgetItem();
                avgValue = QString::number(set_doubleValue(sportSummery.value(sportUse).at(0)/weekCount,false))+"-"+
                           set_sectoTime(round(sportSummery.value(sportUse).at(1)/weekCount)).toString()+"-"+
                           QString::number(set_doubleValue(sportSummery.value(sportUse).at(3)/weekCount,false))+"-"+
                           QString::number(set_doubleValue(sportSummery.value(sportUse).at(4)/weekCount,false));
                item->setData(Qt::DisplayRole,avgValue);
                item->setData(Qt::AccessibleTextRole,sportUse);
                ui->tableWidget_weekAvg->setItem(sports,0,item);
            }
        }
    }

    QMap<int,QString> sumValues;
    ui->tableWidget_summery->clear();
    ui->tableWidget_summery->setRowCount(sportSummery.count());
    ui->tableWidget_summery->setHorizontalHeaderItem(0,new QTableWidgetItem());
    ui->tableWidget_summery->horizontalHeaderItem(0)->setData(Qt::DisplayRole,headerString);
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
        item->setData(Qt::DisplayRole,it.value());
        ui->tableWidget_summery->setItem(it.key(),0,item);
    }
}

void MainWindow::summery_calc(QMap<QString,QVector<double>> *calcSummery,QMap<QString,QVector<double>> *sportSummery,QStringList *sportUseList,int sumCounter)
{
    QString sportUse = generalValues->value("sum");
    QVector<double> calcSum(sumCounter,0),sportSum(sumCounter,0);
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
    double avgValue = sportComp/weekComp;

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

    this->set_tableWidgetItems(ui->tableWidget_schedule,weekRange,cal_header.count(),&schedule_del);

    for(int row = 0; row < weekRange; ++row)
    {
        for(int col = 0; col < weekDays+1; ++col)
        {
            QTableWidgetItem *item = ui->tableWidget_schedule->item(row,col);
            ui->tableWidget_schedule->openPersistentEditor(item);
            if(col == 0)
            {
                weekInfo = workSchedule->get_weekScheduleMeta(calc_weekID(date.addDays(dayCounter)));
                itemValue = weekInfo.at(0) + delimiter + weekInfo.at(1) + delimiter + weekInfo.at(2) + delimiter + foodPlan->get_mode(date.addDays(dayCounter)) +" "+weekInfo.at(3)+"kg - ("+QString::number(set_doubleValue(foodPlan->get_estimateWeight(date.addDays(dayCounter).startOfDay().addSecs(72000)).second,false))+")";
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
                    itemValue = itemValue + it.value().at(4) +"\n";
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
    this->summery_Set(date,0);
    ui->comboBox_saisonName->setCurrentText(workSchedule->get_saisonDate(date));
}

void MainWindow::set_saisonValues(QStringList *sportList,QString weekID, int week)
{
    QMap<QString, QVector<double>> compValues;
    QMap<QString,QMap<QString,QVector<double>>> *compWeekMap = workSchedule->get_compWeekValues();
    QStringList weekInfo = workSchedule->get_weekMeta(weekID);

    QString conString = " - ";
    QString itemValue = QString();
    QString delimiter = "#";
    QVector<double> sumValues(5,0);
    int col = 0;

    QTableWidgetItem *weekitem = new QTableWidgetItem();

    itemValue = weekInfo.at(0) + conString + weekInfo.at(1) + conString + weekInfo.at(3) + delimiter + weekInfo.at(2) + delimiter + weekInfo.at(4);
    weekitem->setData(Qt::DisplayRole,itemValue);
    weekitem->setData(Qt::UserRole,weekID);
    weekitem->setTextAlignment(0);
    itemValue.clear();
    ui->tableWidget_saison->insertRow(week);
    ui->tableWidget_saison->setItem(week,col++,weekitem);

    compValues = compWeekMap->value(weekID);

    for(int sportValues = 0; sportValues < compValues.count(); ++sportValues)
    {
        QTableWidgetItem *sportitem = new QTableWidgetItem();
        for(int mapValues = 0; mapValues < sumValues.count(); ++mapValues)
        {
            itemValue = itemValue + QString::number(compValues.value(sportList->at(sportValues+1)).at(mapValues)) + delimiter;
            sumValues[mapValues] = sumValues.at(mapValues) + compValues.value(sportList->at(sportValues+1)).at(mapValues);
        }
        itemValue.chop(1);
        sportitem->setData(Qt::DisplayRole,itemValue);
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
    sportUseList << generalValues->value("sum");
    QDate startDate,endDate;

    int weekCount = 0;
    int phaseWeeks = 0;

    ui->tableWidget_saison->setColumnCount(sportUseList.count());
    ui->tableWidget_saison->setRowCount(0);
    ui->tableWidget_saison->setHorizontalHeaderLabels(sportUseList);

    QString saison = ui->comboBox_saisonName->currentText();
    QMap<QDate,QPair<QString,QString>> saisonWeekMap = workSchedule->get_saisonWeekMap()->value(saison);

    if(phaseGroup->button(1)->isChecked())
    {
        startDate = saisonWeekMap.firstKey().addDays(saisonWeek*weekDays);
        endDate = startDate.addDays(weekDays*weekRange);

        for(QMap<QDate,QPair<QString,QString>>::iterator week = saisonWeekMap.find(startDate); week != saisonWeekMap.find(endDate); ++week)
        {
            this->set_saisonValues(&sportUseList,calc_weekID(week.key()),weekCount++);
            phaseWeeks = saisonWeekMap.count();
        }
        ui->tableWidget_saison->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }
    else
    {
        for(QMap<QDate,QPair<QString,QString>>::const_iterator week = saisonWeekMap.cbegin(); week != saisonWeekMap.cend(); ++week)
        {
            if(phase == week.value().second)
            {
                this->set_saisonValues(&sportUseList,calc_weekID(week.key()),weekCount++);
                if(weekCount == 1) startDate = week.key();
                ++phaseWeeks;
            }
        }
        ui->tableWidget_saison->verticalHeader()->setDefaultSectionSize(ui->tableWidget_saison->height() / static_cast<int>(weekRange));
        ui->tableWidget_saison->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    }

    this->summery_Set(startDate,phaseWeeks);

    ui->spinBox_phaseWeeks->setValue(phaseWeeks);
    ui->dateEdit_phaseStart->setDate(startDate);
    workSchedule->calc_compPlot(weekCount,startDate,ui->comboBox_saisonSport->currentText());
}

void MainWindow::refresh_schedule()
{
    int addDays = weekCounter*weekDays;

    this->workoutSchedule(firstdayofweek.addDays(addDays));
    ui->label_month->setText("Week " + this->get_weekRange());

    workSchedule->calc_levelPlot(firstdayofweek.addDays(addDays));
    workSchedule->calc_pmcPlot(firstdayofweek.addDays(addDays),ui->pushButton_currentWeek->isChecked(),ui->spinBox_extWeeks->value());
}

void MainWindow::refresh_saison()
{
    QString weekLabel = "Week ";
    if(phaseGroup->checkedId() != 1) weekLabel = "Phase:";
    ui->label_month->setText(weekLabel + this->get_weekRange());

    this->saisonSchedule(phaseGroup->button(phaseGroup->checkedId())->text());
}

void MainWindow::selectionChanged_stressPlot()
{
    for(int i = 0; i < workSchedule->stressPlot->graphCount(); ++i)
    {
        QCPGraph *graph = workSchedule->stressPlot->graph(i);
        QCPPlottableLegendItem *item = workSchedule->stressPlot->legend->itemWithPlottable(graph);
        if(item->selected() || graph->selected())
        {
            item->setSelected(true);
            graph->layer()->setVisible(false);
        }
        else
        {
            item->setSelected(false);
            graph->layer()->setVisible(true);
        }
    }
}

void MainWindow::refresh_summery()
{
    qDebug() << "Refresh CompMap";
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
    ui->comboBox_saisonName->addItems(saisonValues->keys());
    ui->comboBox_saisonName->setEnabled(false);
}

//ACTIONS**********************************************************************

void MainWindow::on_actionNew_triggered()
{
    int dialog_code;
    if(ui->stackedWidget->currentIndex() == PLANER)
    {
        if(isWeekMode)
        {
            day_popup day_pop(this,QDate::currentDate());
            day_pop.setModal(true);
            dialog_code = day_pop.exec();
            if(dialog_code == QDialog::Rejected)
            {
                ui->actionSave->setEnabled(workSchedule->get_isUpdated());
            }
        }
    }
    if(ui->stackedWidget->currentIndex() == FOOD)
    {
        foodPlan->insert_newWeek(ui->calendarWidget_Food->selectedDate().addDays(1 - ui->calendarWidget_Food->selectedDate().dayOfWeek()));
        this->fill_foodPlanList(true,0);
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
    set_progress(10);
    if(ui->stackedWidget->currentIndex() == PLANER)
    {
        statusLabel->setText("Save Schedule");
        set_progress(50);
        workSchedule->save_workouts(isWeekMode);
        ui->actionSave->setEnabled(workSchedule->get_isUpdated());
        set_progress(100);
    }
    else if(ui->stackedWidget->currentIndex() == EDITOR)
    {
        statusLabel->setText("Save GC File");
        set_progress(50);
        currActivity->prepare_save();
        this->save_activity();
        set_progress(100);
    }
    else if(ui->stackedWidget->currentIndex() == FOOD)
    {
        statusLabel->setText("Save Food Plan");
        set_progress(50);
        foodPlan->save_foolPlan();
        ui->actionSave->setEnabled(false);
        set_progress(100);
    }
}

void MainWindow::on_calendarWidget_clicked(const QDate &date)
{
   weeknumber = this->calc_weekID(date);
   this->summery_Set(date,0);
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

//EDITOR Functions *****************************************************************************

void MainWindow::reset_avgSelection()
{
    int col = currActivity->activityHeader.count()-1;

    for(int row = 0; row < ui->treeWidget_activity->topLevelItemCount(); ++row)
    {
        if(ui->treeWidget_activity->topLevelItem(row)->data(0,Qt::DisplayRole).toString() != currActivity->breakName)
        {
            if(ui->treeWidget_activity->topLevelItem(row)->data(col,Qt::UserRole).toBool())
            {
                currActivity->set_averageMap(ui->treeWidget_activity->topLevelItem(row),col);
            }
        }
    }

    for(int i = 0; i < currActivity->averageHeader.count(); ++i)
    {
        ui->tableWidget_avgValues->item(i,0)->setData(Qt::DisplayRole,"-");
    }

    ui->toolButton_clearSelect->setEnabled(false);
}

void MainWindow::activityList()
{
    QString dateString;

    QMapIterator<QDateTime,QVector<QString>> it(currActivity->gcActivtiesMap);
    it.toBack();
    while(it.hasPrevious())
    {
        it.previous();
        QTreeWidgetItem *item = new QTreeWidgetItem();
        dateString = it.key().date().toString("dd.MM.yyyy")+ " "+ it.key().time().toString("hh:mm");

        item->setData(0,Qt::DisplayRole,it.value().at(0));
        item->setData(0,Qt::UserRole,it.value().last());
        item->setData(1,Qt::DisplayRole,dateString);
        item->setData(2,Qt::DisplayRole,it.value().at(1));
        item->setData(3,Qt::DisplayRole,it.value().at(2));

        ui->treeWidget_activityfiles->addTopLevelItem(item);

    }
}

void MainWindow::load_activity(const QString &filename,bool fullPath)
{
    QFileInfo fileinfo(filename);

    if(fileinfo.suffix() == "json")
    {
        actLoaded = currActivity->read_jsonFile(filename,fullPath);

        if(actLoaded)
        {
            qDebug() << "Loaded";

            currActivity->prepare_baseData();
            currActivity->set_activityData();
            ui->actionSelect_File->setEnabled(false);
            ui->actionReset->setEnabled(true);
            avgSelect_del.sport = currActivity->get_currentSport();

            this->set_menuItems(EDITOR);
            this->set_activityTree();
            this->set_activityInfo();

            ui->treeWidget_activity->setHeaderLabels(currActivity->activityHeader);
            ui->treeWidget_activity->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
            ui->treeWidget_activity->header()->setVisible(true);
            ui->tableWidget_avgValues->verticalHeader()->setVisible(true);
            ui->comboBox_swimType->setVisible(currActivity->isSwim);
            ui->label_swimType->setVisible(currActivity->isSwim);
            ui->doubleSpinBox_intDistance->setEnabled(!currActivity->isSwim);
            this->set_tableWidgetItems(ui->tableWidget_avgValues,currActivity->averageHeader.count(),1,nullptr);
            this->set_tableHeader(ui->tableWidget_avgValues,&currActivity->averageHeader,true);

            this->init_controlStyleSheets();
        }
     }
}

void MainWindow::init_controlStyleSheets()
{
    QString viewBackground = "background-color: #e6e6e6";

    ui->tableWidget_avgValues->setStyleSheet(viewBackground);
    ui->treeWidget_activity->setStyleSheet(viewBackground);
    ui->treeWidget_activityfiles->setStyleSheet(viewBackground);
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

void MainWindow::set_activityInfo()
{
    ui->tableWidget_actInfo->clearContents();

    for(int row = 0; row < currActivity->activityInfo.count();++row)
    {
        QTableWidgetItem *item = new QTableWidgetItem();
        item->setData(Qt::DisplayRole,currActivity->activityInfo.value(currActivity->infoHeader->at(row)));
        ui->tableWidget_actInfo->setItem(row,0,item);
    }
    ui->actionSave->setEnabled(true);
}

void MainWindow::recalc_selectedInt(QTime lapTime, double lapDist)
{
    ui->timeEdit_intPace->setTime(set_sectoTime(currActivity->calc_lapPace(get_secFromTime(lapTime),lapDist)));
    ui->doubleSpinBox_intSpeed->setValue(currActivity->get_speed(ui->timeEdit_intPace->time(),lapDist,true));

    this->set_polishMinMax(ui->doubleSpinBox_intSpeed->value());
}


void MainWindow::set_polishMinMax(double speed)
{
    QPair<double,double> avgMinMax;
    if(currActivity->isBike || currActivity->isRun)
    {
        avgMinMax = currActivity->get_polishMinMax(speed);
    }
    else
    {
        avgMinMax.first = 0;
        avgMinMax.second = 0;
    }
    ui->doubleSpinBox_polMin->setValue(avgMinMax.first);
    ui->doubleSpinBox_polMax->setValue(avgMinMax.second);
}


void MainWindow::on_horizontalSlider_factor_valueChanged(int value)
{
    ui->label_factorValue->setText(QString::number(10-value) + "%");
    currActivity->set_polishFactor(value);

    this->set_polishMinMax(ui->doubleSpinBox_intSpeed->value());
    this->set_polishPlot(selectedInt);
}

void MainWindow::resetPlot()
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();
    QCPGraph *resetLine = ui->widget_plot->addGraph();
    resetLine->setPen(QPen(QColor(255,255,255),2));
    resetLine->setName("-");
    ui->widget_plot->xAxis->setRange(0,100);
    ui->widget_plot->xAxis2->setRange(0,1);
    ui->widget_plot->yAxis->setRange(0,5);
    ui->widget_plot->plotLayout()->setRowStretchFactor(1,0.0001);
    ui->widget_plot->replot();
}

QTreeWidgetItem* MainWindow::set_activityLaps(QPair<int,QString> lapKey, QVector<double> lapData,int lastSplit)
{
    QTreeWidgetItem *lapItem = new QTreeWidgetItem();
    int lapTime = round(lapData.at(2));
    int lapPace = currActivity->calc_lapPace(lapTime,currActivity->poolLength);

    lapItem->setData(0,Qt::AccessibleTextRole,lapKey.first);
    lapItem->setData(0,Qt::UserRole,lapData.at(0));
    lapItem->setData(0,Qt::DisplayRole,lapKey.second);

    lapItem->setData(1,Qt::DisplayRole,currActivity->get_swimType(lapData.at(1)));
    lapItem->setData(1,Qt::UserRole,lapData.at(1));

    lapItem->setData(3,Qt::DisplayRole,currActivity->poolLength);

    lapItem->setData(4,Qt::DisplayRole,set_time(lapTime));
    lapItem->setData(4,Qt::UserRole,lapTime);

    lapItem->setData(5,Qt::DisplayRole,set_time(lapTime+lastSplit));
    lapItem->setData(5,Qt::UserRole,lapTime+lastSplit);

    lapItem->setData(6,Qt::DisplayRole,set_time(lapPace));
    lapItem->setData(6,Qt::UserRole,lapPace);

    lapItem->setData(7,Qt::DisplayRole,currActivity->get_speed(QTime::fromString(set_time(lapPace),shortTime),currActivity->poolLength,true));

    lapItem->setData(8,Qt::DisplayRole,lapData.at(3));

    lapItem->setData(9,Qt::DisplayRole,currActivity->calc_totalWork(lapPace,lapTime,lapData.at(1)));

    return lapItem;
}


void MainWindow::set_activityTree()
{
    ui->treeWidget_activity->clear();
    QTreeWidgetItem *rootItem = ui->treeWidget_activity->invisibleRootItem();

    QMap<QPair<int,QString>,QMap<QPair<int,QString>,QVector<double>>> *activityMap = currActivity->get_activityMap();
    QPair<int,int> intStartStop;
    double distSplit = 0;
    double totalWork = 0;
    int lapPace = 0;

    for(QMap<QPair<int,QString>,QMap<QPair<int,QString>,QVector<double>>>::const_iterator intStart = activityMap->cbegin(), intEnd = activityMap->cend(); intStart != intEnd; ++intStart)
    {
        QTreeWidgetItem *intItem = new QTreeWidgetItem();
        intStartStop = currActivity->get_intervalData(intStart.key().first);
        double lapWork = 0;

        if(currActivity->isSwim)
        {
            QVector<int> lapValues(6,0);
            double completeDist = 0;
            int lapStyle = intStart.value().first().at(1);
            int lastSplit = 0;

            if(intStart.key().second != currActivity->breakName)
            {
                for(QMap<QPair<int,QString>,QVector<double>>::const_iterator lapsStart = intStart.value().cbegin(), lapsEnd = intStart.value().cend(); lapsStart != lapsEnd; ++lapsStart)
                {
                    intItem->addChild(this->set_activityLaps(lapsStart.key(),lapsStart.value(),lastSplit));
                    lapValues[0] = lapValues.at(0) + lapsStart.value().at(2);
                    lapValues[3] = lapValues.at(3) + lapsStart.value().at(3);
                    lastSplit = lastSplit + lapsStart.value().at(2);
                    lapWork = lapWork + intItem->child(lapsStart.key().first-1)->data(9,Qt::DisplayRole).toDouble();
                    if(lapStyle == lapsStart.value().at(1))
                    {
                        lapStyle = lapsStart.value().at(1);
                    }
                    else
                    {
                        lapStyle = 6;
                    }
                }

                lapValues[1] = intStart.value().count();
                lapValues[2] = lapValues.at(1)*currActivity->poolLength;
                lapValues[4] = currActivity->calc_lapPace(intStartStop.second - intStartStop.first,lapValues.at(2));

                completeDist = completeDist + (lapValues.at(2) / 1000.0);

                intItem->setData(0,Qt::DisplayRole,intStart.key().second+"_"+currActivity->checkRangeLevel(lapValues.at(4)));
                intItem->setData(0,Qt::UserRole,completeDist);

                intItem->setData(1,Qt::DisplayRole,currActivity->get_swimType(lapStyle));

                intItem->setData(2,Qt::DisplayRole,lapValues.at(1));

                intItem->setData(3,Qt::DisplayRole,lapValues.at(2));

                intItem->setData(4,Qt::DisplayRole,set_time(lapValues.at(0)));
                intItem->setData(4,Qt::UserRole,intStartStop.second);

                intItem->setData(5,Qt::DisplayRole,set_time(intStartStop.first));
                intItem->setData(5,Qt::UserRole,intStartStop.first);

                intItem->setData(6,Qt::DisplayRole,set_time(lapValues.at(4)));
                intItem->setData(6,Qt::UserRole,lapValues.at(4));

                intItem->setData(7,Qt::DisplayRole,currActivity->get_speed(QTime::fromString(set_time(lapValues.at(4)),shortTime),lapValues.at(2),true));

                intItem->setData(8,Qt::DisplayRole,lapValues.at(3));

                intItem->setData(9,Qt::DisplayRole,set_doubleValue(lapWork,false));

                intItem->setData(10,Qt::DisplayRole,"-");
                intItem->setData(10,Qt::UserRole,false);
            }
            else
            {
                lapWork = currActivity->calc_totalWork(0,intStartStop.second - intStartStop.first,0);
                intItem->setData(0,Qt::DisplayRole,intStart.key().second);
                intItem->setData(0,Qt::UserRole,completeDist);

                intItem->setData(4,Qt::DisplayRole,set_time(intStartStop.second - intStartStop.first));

                intItem->setData(5,Qt::DisplayRole,set_time(intStartStop.first));

                intItem->setData(9,Qt::DisplayRole,lapWork);
            }
            intItem->setData(0,Qt::AccessibleTextRole,QString::number(intStart.key().first)+"-"+intStart.key().second);
            totalWork = totalWork + lapWork;
        }
        else
        {
            for(QMap<QPair<int,QString>,QVector<double>>::const_iterator lapsStart = intStart.value().cbegin(), lapsEnd = intStart.value().cend(); lapsStart != lapsEnd; ++lapsStart)
            {
                lapPace = currActivity->calc_lapPace(lapsStart.value().at(0),lapsStart.value().at(1));

                intItem->setData(0,Qt::AccessibleTextRole,QString::number(intStart.key().first)+"-"+intStart.key().second);
                intItem->setData(0,Qt::UserRole,lapsStart.key().first);

                intItem->setData(1,Qt::DisplayRole,set_time(lapsStart.value().at(0)));
                intItem->setData(1,Qt::UserRole,lapsStart.value().at(0));

                intItem->setData(2,Qt::DisplayRole,set_time(intStartStop.first));
                intItem->setData(2,Qt::UserRole,intStartStop.first);

                intItem->setData(3,Qt::DisplayRole,set_doubleValue(lapsStart.value().at(1)+distSplit,true));
                intItem->setData(3,Qt::UserRole,intStartStop.second);

                if(currActivity->usePMData)
                {
                    lapWork = currActivity->calc_totalWork(lapsStart.value().at(3),lapsStart.value().at(0),0);
                }
                else
                {
                    lapWork = currActivity->calc_totalWork(lapPace,lapsStart.value().at(0),0);
                }

                if(currActivity->isBike || currActivity->isRun)
                {
                    intItem->setData(4,Qt::DisplayRole,set_doubleValue(lapsStart.value().at(1),true));

                    intItem->setData(5,Qt::DisplayRole,set_time(lapPace));
                    intItem->setData(5,Qt::UserRole,lapPace);

                    intItem->setData(6,Qt::DisplayRole,set_doubleValue(lapsStart.value().at(2),false));

                    intItem->setData(7,Qt::DisplayRole,round(lapsStart.value().at(3)));

                    intItem->setData(8,Qt::DisplayRole,round(lapsStart.value().at(4)));

                    intItem->setData(9,Qt::DisplayRole,lapWork);

                    intItem->setData(10,Qt::DisplayRole,"-");
                    intItem->setData(10,Qt::UserRole,false);
                }
                else
                {
                    intItem->setData(4,Qt::DisplayRole,lapWork);

                    intItem->setData(5,Qt::DisplayRole,"-");
                    intItem->setData(5,Qt::UserRole,false);
                }
                intItem->setData(0,Qt::DisplayRole,currActivity->set_intervalName(intItem,true));
                distSplit = distSplit + lapsStart.value().at(1);
            }
            totalWork = totalWork + lapWork;
        }
        rootItem->addChild(intItem);
    }
    currActivity->activityInfo.insert("Total Work",QString::number(ceil(totalWork)));
    this->refresh_activityTree();
}

void MainWindow::set_selecteditem(QTreeWidgetItem *selItem, int column)
{
    if(column != currActivity->activityHeader.count()-1)
    {
        selectedInt = ui->treeWidget_activity->invisibleRootItem()->indexOfChild(selItem);
        ui->label_lapType->setText(selItem->data(0,Qt::DisplayRole).toString());

        if(currActivity->isSwim)
        {
            if(selItem->childCount() > 0)
            {
                ui->timeEdit_intDuration->setEnabled(false);
                if(selItem->isExpanded())
                {
                    selItem->setExpanded(false);
                }
                else
                {
                    selItem->setExpanded(true);
                }

                ui->toolButton_split->setVisible(false);
                ui->toolButton_merge->setVisible(false);
                ui->toolButton_add->setVisible(true);
                ui->toolButton_delete->setVisible(true);
            }
            else
            {
                ui->timeEdit_intDuration->setEnabled(true);
                ui->toolButton_split->setVisible(true);
                ui->toolButton_merge->setVisible(true);
                ui->toolButton_add->setVisible(false);
                ui->toolButton_delete->setVisible(false);
            }

            ui->comboBox_swimType->setCurrentText(selItem->data(1,Qt::DisplayRole).toString());
            ui->doubleSpinBox_intDistance->setValue(selItem->data(3,Qt::DisplayRole).toDouble());
            ui->timeEdit_intDuration->setTime(QTime::fromString(selItem->data(4,Qt::DisplayRole).toString(),shortTime));
            ui->timeEdit_intPace->setTime(set_sectoTime(selItem->data(6,Qt::UserRole).toInt()));
            ui->doubleSpinBox_intSpeed->setValue(selItem->data(7,Qt::DisplayRole).toDouble());
            ui->spinBox_intCAD->setValue(selItem->data(8,Qt::DisplayRole).toUInt());
            ui->timeEdit_intDuration->setFocus();
        }
        else
        {
            ui->doubleSpinBox_intDistance->blockSignals(true);
            ui->timeEdit_intDuration->blockSignals(true);
            ui->doubleSpinBox_intDistance->setValue(selItem->data(4,Qt::DisplayRole).toDouble());
            ui->spinBox_intWatts->setValue(selItem->data(7,Qt::DisplayRole).toInt());
            ui->timeEdit_intDuration->setTime(QTime::fromString(selItem->data(1,Qt::DisplayRole).toString(),shortTime));
            ui->spinBox_intCAD->setValue(selItem->data(8,Qt::DisplayRole).toUInt());
            ui->doubleSpinBox_intDistance->setFocus();
            if(currActivity->usePMData)
            {

            }
        }
        this->recalc_selectedInt(ui->timeEdit_intDuration->time(),ui->doubleSpinBox_intDistance->value());
        this->set_polishPlot(selectedInt);
        ui->doubleSpinBox_intDistance->blockSignals(false);
        ui->timeEdit_intDuration->blockSignals(false);
    }
    else
    {
        QPair<int,QVector<double>> averageData = currActivity->set_averageMap(selItem,column);

        bool avgData = averageData.first > 0 ? true : false;

        ui->toolButton_addSelect->setEnabled(avgData);
        ui->toolButton_clearSelect->setEnabled(avgData);
        ui->toolButton_clearContent->setEnabled(avgData);
        ui->toolButton_sync->setEnabled(avgData);

        if(avgData)
        {
            ui->tableWidget_avgValues->item(0,0)->setData(Qt::DisplayRole,averageData.first);
            ui->tableWidget_avgValues->item(1,0)->setData(Qt::DisplayRole,set_time(averageData.second.at(0)));
            ui->tableWidget_avgValues->item(1,0)->setData(Qt::UserRole,averageData.second.at(0));
            ui->tableWidget_avgValues->item(2,0)->setData(Qt::DisplayRole,set_time(averageData.second.at(1)));
            ui->tableWidget_avgValues->item(2,0)->setData(Qt::UserRole,averageData.second.at(1));
            ui->tableWidget_avgValues->item(4,0)->setData(Qt::DisplayRole,round(averageData.second.at(3)));

            if(currActivity->isSwim)
            {
                ui->tableWidget_avgValues->item(3,0)->setData(Qt::DisplayRole,round(averageData.second.at(2)));
                ui->tableWidget_avgValues->item(5,0)->setData(Qt::DisplayRole,round(averageData.second.at(4)));
                ui->tableWidget_avgValues->item(6,0)->setData(Qt::DisplayRole,set_doubleValue(averageData.second.at(5),false));
            }
            else
            {
                ui->tableWidget_avgValues->item(3,0)->setData(Qt::DisplayRole,set_doubleValue(averageData.second.at(2),true));

                if(currActivity->isRun)
                {
                    ui->tableWidget_avgValues->item(5,0)->setData(Qt::DisplayRole,round(averageData.second.at(4)*2));
                }
                else
                {
                    ui->tableWidget_avgValues->item(5,0)->setData(Qt::DisplayRole,round(averageData.second.at(4)));
                }
            }
        }
    }
}

void MainWindow::set_polishPlot(int lapIndex)
{
    QPair<int,int> intStartStop = currActivity->get_intervalData(lapIndex);
    QMap<int,QVector<double>> *polishData = currActivity->get_polishData();
    QVector<double> secTicker,speedValues,powerValues,hfValues,polishSpeed,polishPower;
    QPair<double,double> speedMinMax,powerMinMax,hfMinMax,rangeMinMax;
    QStringList label;

    int pos = 0;
    double intDist = 0;
    double avgSpeed = 0;
    double avgPower = 0;

    speedMinMax.first = 50.0;
    speedMinMax.second = 0.0;
    powerMinMax.first = 500.0;
    powerMinMax.second = 0.0;
    hfMinMax.first = 80.0;
    hfMinMax.second = 0.0;

    int vSize = intStartStop.second-intStartStop.first+1;

    secTicker.resize(vSize);
    speedValues.resize(vSize);
    powerValues.resize(vSize);
    hfValues.resize(vSize);
    polishSpeed.resize(vSize);
    polishPower.resize(vSize);

    for(QMap<int,QVector<double>>::iterator intStart = polishData->lowerBound(intStartStop.first), intEnd = polishData->lowerBound(intStartStop.second); intStart != intEnd; ++intStart)
    {
        secTicker[pos] = pos;
        speedValues[pos] = intStart.value().at(0);
        powerValues[pos] = intStart.value().at(1);
        hfValues[pos] = intStart.value().at(2);

        if(speedMinMax.first > intStart.value().at(0))  speedMinMax.first = intStart.value().at(0);
        if(speedMinMax.second < intStart.value().at(0)) speedMinMax.second = intStart.value().at(0);
        if(powerMinMax.first > intStart.value().at(1)) powerMinMax.first = intStart.value().at(1);
        if(powerMinMax.second < intStart.value().at(1)) powerMinMax.second = intStart.value().at(1);
        ++pos;
    }

    if(!currActivity->isSwim)
    {
        label << "Speed" << "Watts" << "HF";
        avgSpeed = ui->doubleSpinBox_intSpeed->value();
        avgPower = ui->spinBox_intWatts->value();
        intDist = ui->doubleSpinBox_intDistance->value();

        if(currActivity->isRun)
        {
            ui->horizontalSlider_factor->setEnabled(true);
            for(int i = 0; i < speedValues.count(); ++i)
            {
                if(lapIndex == 0 && i < 5)
                {
                    polishSpeed[i] = speedValues[i];
                }
                else
                {
                    polishSpeed[i] = currActivity->polish_SpeedValues(speedValues[i],avgSpeed,true);
                    polishPower[i] = currActivity->polish_powerValues(polishSpeed.at(i),powerValues.at(i),avgPower);
                }
            }
        }

        rangeMinMax.first = ui->doubleSpinBox_polMin->value();
        rangeMinMax.second = ui->doubleSpinBox_polMax->value();

        if(currActivity->isIndoor)
        {
            ui->horizontalSlider_factor->setEnabled(false);
        }
        ui->widget_plot->yAxis2->setLabel("Watts / HF");
    }
    else
    {
        label << "Speed" << "Strokes" << "-";
        ui->widget_plot->yAxis2->setLabel(label.at(1));
        ui->horizontalSlider_factor->setEnabled(false);
        avgSpeed = ui->doubleSpinBox_intSpeed->value();
        intDist = ui->doubleSpinBox_intDistance->value();
    }

    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();
    ui->widget_plot->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->widget_plot->plotLayout()->setRowStretchFactor(1,0.0001);

    QCPGraph *speedLine = ui->widget_plot->addGraph();
    speedLine->setName(label.at(0));
    speedLine->setLineStyle(QCPGraph::lsLine);
    speedLine->setData(secTicker,speedValues);
    speedLine->setPen(QPen(QColor(0,255,0),2));

    QCPGraph *powerLine = ui->widget_plot->addGraph(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    powerLine->setName(label.at(1));
    powerLine->setLineStyle(QCPGraph::lsLine);
    powerLine->setData(secTicker,powerValues);
    powerLine->setPen(QPen(QColor(255,0,0),2));

    QCPGraph *hfLine = ui->widget_plot->addGraph(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    hfLine->setName(label.at(2));
    hfLine->setLineStyle(QCPGraph::lsLine);
    hfLine->setData(secTicker,hfValues);
    hfLine->setPen(QPen(QColor(0,170,255),2));

    QCPItemLine *avgLine = new QCPItemLine(ui->widget_plot);
    avgLine->start->setCoords(0,avgSpeed);
    avgLine->end->setCoords(speedValues.count(),avgSpeed);
    avgLine->setPen(QPen(QColor(0,0,255),2));

    QCPGraph *avgLineP = ui->widget_plot->addGraph();
    avgLineP->setName("Avg Speed");
    avgLineP->setPen(QPen(QColor(0,0,255),2));

    if(currActivity->get_currentSport() != settings::SwimLabel)
    {
        QCPGraph *corrSpeed = ui->widget_plot->addGraph();
        corrSpeed->setName("Polished Speed");
        corrSpeed->setLineStyle(QCPGraph::lsLine);
        corrSpeed->setData(secTicker,polishSpeed);
        corrSpeed->setPen(QPen(QColor(255,200,0),2));

        QCPGraph *corrPower = ui->widget_plot->addGraph(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
        corrPower->setName("Corrected Watts");
        corrPower->setLineStyle(QCPGraph::lsLine);
        corrPower->setData(secTicker,polishPower);
        corrPower->setPen(QPen(QColor(255,125,0),2));

        QCPGraph *polishRangeP = ui->widget_plot->addGraph();
        polishRangeP->setName("Polish Range");
        polishRangeP->setPen(QPen(QColor(225,225,0),2));

        QCPItemRect *polishRange = new QCPItemRect(ui->widget_plot);
        polishRange->topLeft->setCoords(0,rangeMinMax.second);
        polishRange->bottomRight->setCoords(speedValues.count(),rangeMinMax.first);
        polishRange->setPen(QPen(QColor(225,225,0),2));
        polishRange->setBrush(QBrush(QColor(255,255,0,50)));
    }

    double yAxisMin = 0,yAxisMax = 0,yAxix2Min = 0, yAxix2Max = 0;

    if(speedMinMax.first > 0) yAxisMin = speedMinMax.first*0.1;
    yAxisMax = speedMinMax.second*0.1;

    if(powerMinMax.first > 0) yAxix2Min = powerMinMax.second*0.25;
    yAxix2Max = powerMinMax.second*0.1;

    ui->widget_plot->xAxis->setRange(0,speedValues.count());
    ui->widget_plot->xAxis2->setRange(0,intDist);
    ui->widget_plot->yAxis->setRange(speedMinMax.first-yAxisMin,speedMinMax.second+yAxisMax);
    ui->widget_plot->yAxis2->setRange(powerMinMax.first-yAxix2Min,powerMinMax.second+yAxix2Max);

    ui->widget_plot->replot();

}

void MainWindow::init_polishgraph()
{
    QFont plotFont;
    plotFont.setBold(true);
    plotFont.setPointSize(8);

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

void MainWindow::fill_WorkoutContent()
{
    QString content,newEntry,contentValue,label;
    content = ui->lineEdit_workContent->text();

    int intCount = ui->tableWidget_avgValues->item(0,0)->data(Qt::DisplayRole).toInt();

    double dist = 0;
    int time = 0;

    if(ui->radioButton_time->isChecked())
    {
        if(ui->checkBox_exact->isChecked())
        {
            time = ui->tableWidget_avgValues->item(1,0)->data(Qt::UserRole).toInt();
        }
        else
        {
            time = round(ui->tableWidget_avgValues->item(1,0)->data(Qt::UserRole).toDouble() /10.0)*10.0;
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
            dist = round(ui->tableWidget_avgValues->item(3,0)->data(Qt::DisplayRole).toDouble()*1000)/1000.0;
        }
        else
        {
            dist = ceil(ui->tableWidget_avgValues->item(3,0)->data(Qt::DisplayRole).toDouble()*10)/10.0;
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

    if(currActivity->isSwim)
    {
        dist = ui->tableWidget_avgValues->item(3,0)->data(Qt::DisplayRole).toInt();

        if(intCount > 1)
        {
            newEntry = QString::number(intCount)+"x"+QString::number(dist)+"/"+ui->tableWidget_avgValues->item(1,0)->data(Qt::DisplayRole).toString();
        }
        else
        {
            newEntry = QString::number(dist)+"-"+ui->tableWidget_avgValues->item(1,0)->data(Qt::DisplayRole).toString();
        }
    }

    if(currActivity->isBike)
    {
        if(intCount > 1)
        {
            newEntry = QString::number(intCount)+"x"+contentValue+"/" +ui->tableWidget_avgValues->item(4,0)->data(Qt::DisplayRole).toString()+"W";
        }
        else
        {
            newEntry = contentValue+"-" +ui->tableWidget_avgValues->item(4,0)->data(Qt::DisplayRole).toString()+"W";
        }
    }

    if(currActivity->isRun)
    {
        if(intCount > 1)
        {
            newEntry = QString::number(intCount)+"x"+contentValue+"-" +ui->tableWidget_avgValues->item(2,0)->data(Qt::DisplayRole).toString()+"/km";
        }
        else
        {
            newEntry = contentValue+"-" +ui->tableWidget_avgValues->item(2,0)->data(Qt::DisplayRole).toString()+"/km";
        }
        if(currActivity->usePMData)
        {
            newEntry = newEntry+"-"+ui->tableWidget_avgValues->item(4,0)->data(Qt::DisplayRole).toString()+"W";
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

    currActivity->set_workoutContent(ui->lineEdit_workContent->text());

}

void MainWindow::save_activity()
{
    QTreeWidgetItem *rootItem = ui->treeWidget_activity->invisibleRootItem();
    QTreeWidgetItem *intItem;
    int intCount = 100 / rootItem->childCount();

    QPair<int, QString> intKey,lapKey;
    QMap<QPair<int, QString>, QVector<double>> intValues;
    QPair<int,int> intStartStop;
    QVector<double> lapValues(7);

    for(int row = 0; row < rootItem->childCount(); ++row)
    {
       intItem = rootItem->child(row);
       intKey.first = row;
       intKey.second = intItem->data(0,Qt::DisplayRole).toString();

       lapValues.fill(0);
       intValues.clear();

       if(currActivity->isSwim)
       {
           intStartStop.first = intItem->data(5,Qt::UserRole).toInt();
           intStartStop.second = intItem->data(4,Qt::UserRole).toInt();
           currActivity->update_intervalMap(row,intItem->data(0,Qt::DisplayRole).toString(),intStartStop);

           if(intItem->childCount() > 0)
           {
               for(int child = 0; child < intItem->childCount(); ++child)
               {
                   lapKey.first = child;
                   lapKey.second = intItem->child(child)->data(0,Qt::DisplayRole).toString();

                   lapValues[0] = intItem->child(child)->data(5,Qt::UserRole).toInt();
                   lapValues[1] = intItem->child(child)->data(0,Qt::UserRole).toDouble();
                   lapValues[2] = intItem->child(child)->data(1,Qt::UserRole).toInt();
                   lapValues[3] = intItem->child(child)->data(4,Qt::UserRole).toInt();
                   lapValues[4] = intItem->child(child)->data(8,Qt::DisplayRole).toInt();
                   lapValues[5] = intItem->child(child)->data(7,Qt::DisplayRole).toDouble();
                   lapValues[6] = currActivity->poolLength / intItem->child(child)->data(4,Qt::UserRole).toInt() / 1000.0;

                   intValues.insert(lapKey,lapValues);
               }
           }
           else
           {
                lapKey.first = row;
                lapKey.second = intItem->data(0,Qt::AccessibleTextRole).toString();

                lapValues[0] = intItem->data(5,Qt::UserRole).toInt();
                lapValues[1] = intItem->data(0,Qt::UserRole).toDouble();
                lapValues[2] = 0;
                lapValues[3] = intItem->data(5,Qt::UserRole+1).toInt();
                lapValues[4] = 0;

                intValues.insert(lapKey,lapValues);
           }  
           currActivity->update_activityMap(intKey,intValues);
       }
       else
       {
           intStartStop.first = intItem->data(2,Qt::UserRole).toInt();
           intStartStop.second = intItem->data(3,Qt::UserRole).toInt();
           currActivity->update_intervalMap(row,intItem->data(0,Qt::DisplayRole).toString(),intStartStop);

           lapKey.first = row;
           lapKey.second = intItem->data(0,Qt::AccessibleTextRole).toString();

           lapValues[0] = intItem->data(4,Qt::DisplayRole).toDouble();
           lapValues[1] = (intItem->data(4,Qt::UserRole).toDouble() / intItem->data(1,Qt::UserRole).toInt()) / 1000.0;
           lapValues[2] = intItem->data(6,Qt::DisplayRole).toDouble();
           lapValues[3] = intItem->data(7,Qt::DisplayRole).toDouble();

           intValues.insert(lapKey,lapValues);

           currActivity->update_activityMap(intKey,intValues);
       }
       set_progress(row*intCount);
    }
    currActivity->prepare_mapToJson();
}

void MainWindow::updated_changedInterval(QTreeWidgetItem *updateItem)
{
    if(currActivity->isSwim)
    {
        QTreeWidgetItem *parentItem = updateItem->parent();
        //Set Lap
        int calcTime = updateItem->data(4,Qt::UserRole).toInt();
        int calcPace = currActivity->calc_lapPace(calcTime,currActivity->poolLength);

        QPair<QString,QString> levelChange;
        levelChange.first = updateItem->data(0,Qt::DisplayRole).toString().split("_").last();
        levelChange.second = currActivity->checkRangeLevel(calcPace);
        currActivity->update_paceInZone(levelChange,calcTime);

        QStringList intKey = parentItem->data(0,Qt::AccessibleTextRole).toString().split("-");
        QString intCount = intKey.at(1).split("_").first();

        updateItem->setData(6,Qt::DisplayRole,set_time(calcPace));
        updateItem->setData(6,Qt::UserRole,calcPace);
        updateItem->setData(9,Qt::DisplayRole,currActivity->calc_totalWork(calcPace,calcTime,updateItem->data(1,Qt::UserRole).toInt()));

        calcTime = 0;
        int lapCount = parentItem->childCount();

        for(int lap = 0; lap < lapCount; ++lap)
        {
            parentItem->child(lap)->setData(0,Qt::DisplayRole,currActivity->set_intervalName(parentItem->child(lap),false));
            calcTime = calcTime + parentItem->child(lap)->data(4,Qt::UserRole).toInt();
        }

        //Set Interval
        int intDistance = lapCount*currActivity->poolLength;
        calcPace = currActivity->calc_lapPace(calcTime,intDistance);

        parentItem->setData(2,Qt::DisplayRole,parentItem->childCount());
        parentItem->setData(3,Qt::DisplayRole,intDistance);
        parentItem->setData(6,Qt::DisplayRole,set_time(calcPace));
        parentItem->setData(6,Qt::UserRole,calcPace);
        parentItem->setData(7,Qt::DisplayRole,currActivity->get_speed(QTime::fromString(set_time(calcPace),shortTime),intDistance,true));

        QString lapName = currActivity->set_intervalName(parentItem,true);

        parentItem->setData(0,Qt::AccessibleTextRole,intKey.first()+"-"+lapName);
        parentItem->setData(0,Qt::DisplayRole,lapName);
    }
}

void MainWindow::on_toolButton_update_clicked()
{
    QTreeWidgetItem *selItem = ui->treeWidget_activity->currentItem();

    selItem->setData(currActivity->activityHeader.indexOf("Pace"),Qt::DisplayRole,ui->timeEdit_intPace->time().toString(shortTime));

    if(currActivity->isSwim)
    {
        if(selItem->childCount() > 0)
        {
            selItem->setData(1,Qt::DisplayRole,ui->comboBox_swimType->currentText());
            for(int childs = 0; childs < selItem->childCount(); ++childs)
            {
                selItem->child(childs)->setData(1,Qt::DisplayRole,ui->comboBox_swimType->currentText());
                selItem->child(childs)->setData(1,Qt::UserRole,ui->comboBox_swimType->currentIndex());
                this->updated_changedInterval(selItem->child(childs));
            }
        }
        else
        {
            selItem->setData(1,Qt::DisplayRole,ui->comboBox_swimType->currentText());
            selItem->setData(1,Qt::UserRole,ui->comboBox_swimType->currentIndex());
            selItem->setData(4,Qt::DisplayRole,ui->timeEdit_intDuration->time().toString(shortTime));
            selItem->setData(4,Qt::UserRole,get_secFromTime(ui->timeEdit_intDuration->time()));
            selItem->setData(7,Qt::DisplayRole,ui->doubleSpinBox_intSpeed->value());
            selItem->setData(8,Qt::DisplayRole,ui->spinBox_intCAD->value());
            this->updated_changedInterval(selItem);
        }
    }
    else
    {
        selItem->setData(5,Qt::UserRole,get_timesec(selItem->data(5,Qt::DisplayRole).toString()));
        selItem->setData(currActivity->activityHeader.indexOf("Distance (Int)"),Qt::DisplayRole,ui->doubleSpinBox_intDistance->value());
        selItem->setData(0,Qt::DisplayRole,currActivity->set_intervalName(selItem,true));
    }

    this->refresh_activityTree();
}

void MainWindow::on_toolButton_split_clicked()
{
    QTreeWidgetItem *selItem = ui->treeWidget_activity->currentItem();

    int lapTime = selItem->data(4,Qt::UserRole).toInt() / 2;
    double lapSpeed = selItem->data(7,Qt::DisplayRole).toDouble() * 2.0;
    int splitTime = get_timesec(selItem->data(5,Qt::DisplayRole).toString());
    int strokes = selItem->data(8,Qt::DisplayRole).toInt() / 2;

    selItem->setData(4,Qt::DisplayRole,set_time(lapTime));
    selItem->setData(4,Qt::UserRole,lapTime);
    selItem->setData(7,Qt::DisplayRole,set_doubleValue(lapSpeed,true));
    selItem->setData(8,Qt::DisplayRole,strokes);

    QTreeWidgetItem *newItem = selItem->clone();
    selItem->parent()->addChild(newItem);

    selItem->setData(5,Qt::DisplayRole,set_time(splitTime-lapTime));
    newItem->setData(0,Qt::AccessibleTextRole,selItem->parent()->indexOfChild(newItem)+1);

    this->updated_changedInterval(selItem);
    this->updated_changedInterval(newItem);
    this->refresh_activityTree();
    this->set_selecteditem(selItem,0);
}

void MainWindow::refresh_activityTree()
{
    QTreeWidgetItem *rootItem = ui->treeWidget_activity->invisibleRootItem();
    QTreeWidgetItem *lapItem;
    QPair<int,int> lapStartStop;
    QString swimType;

    double poolLength = currActivity->isSwim ? (currActivity->poolLength / 1000.0) : 0;
    double completeDist = 0,totalWork = 0,intWork = 0;
    int completeTime = 0,breakTime = 0,intTime = 0,lapTime = 0,intStrokes = 0;

    for(int row = 0; row < rootItem->childCount(); ++row)
    {
        lapItem = rootItem->child(row);
        lapStartStop = currActivity->get_intervalData(lapItem->data(0,Qt::AccessibleTextRole).toString().split("-").first().toInt());

        if(currActivity->isSwim)
        {
            lapItem->setData(5,Qt::DisplayRole,set_time(completeTime));
            lapItem->setData(5,Qt::UserRole,completeTime);
            intTime = intWork = intStrokes = 0;

            if(lapItem->childCount() > 0)
            {
                swimType = lapItem->data(1,Qt::DisplayRole).toString();

                for(int child = 0; child < lapItem->childCount(); ++child)
                {
                    if(swimType ==  lapItem->child(child)->data(1,Qt::DisplayRole).toString())
                    {
                        swimType = lapItem->child(child)->data(1,Qt::DisplayRole).toString();
                    }
                    else
                    {
                        swimType = currActivity->swimType.at(6);
                    }

                    if(child > 0) completeDist = completeDist + poolLength;

                    lapItem->child(child)->setData(5,Qt::UserRole,completeTime);
                    lapTime = lapItem->child(child)->data(4,Qt::UserRole).toInt();
                    completeTime += lapTime;
                    totalWork = totalWork + lapItem->child(child)->data(9,Qt::DisplayRole).toDouble();
                    intTime += lapTime;

                    intWork += lapItem->child(child)->data(9,Qt::DisplayRole).toDouble();
                    intStrokes += lapItem->child(child)->data(8,Qt::DisplayRole).toInt();

                    lapItem->child(child)->setData(0,Qt::UserRole,completeDist);
                    lapItem->child(child)->setData(5,Qt::DisplayRole,set_time(intTime));
                }

                lapItem->setData(1,Qt::DisplayRole,swimType);
                lapItem->setData(1,Qt::UserRole,currActivity->swimType.indexOf(swimType));
                lapItem->setData(4,Qt::DisplayRole,set_time(intTime));
                lapItem->setData(4,Qt::UserRole,completeTime);
                lapItem->setData(8,Qt::DisplayRole,intStrokes);
                lapItem->setData(9,Qt::DisplayRole,set_doubleValue(intWork,false));
            }
            else
            {

                completeDist = completeDist + poolLength;
                intTime = (lapStartStop.second - lapStartStop.first) + (lapStartStop.first - completeTime);
                intWork = currActivity->calc_totalWork(0,intTime,0);
                totalWork = totalWork + intWork;
                lapItem->setData(0,Qt::UserRole,completeDist);
                lapItem->setData(4,Qt::DisplayRole,set_time(intTime));
                lapItem->setData(5,Qt::UserRole,completeTime);
                lapItem->setData(5,Qt::UserRole+1,intTime);
                lapItem->setData(9,Qt::DisplayRole,intWork);
                completeTime = completeTime + get_timesec(lapItem->data(4,Qt::DisplayRole).toString())+1;
                lapItem->setData(4,Qt::UserRole,completeTime);
                breakTime = breakTime + get_timesec(lapItem->data(4,Qt::DisplayRole).toString())+1;
            }
            ++completeTime;
        }
        else
        {
            lapItem->setData(2,Qt::DisplayRole,set_time(completeTime));
            lapItem->setData(2,Qt::UserRole,completeTime);
            intTime = get_timesec(lapItem->data(1,Qt::DisplayRole).toString());
            completeTime = completeTime + intTime;
            totalWork = totalWork + lapItem->data(9,Qt::DisplayRole).toDouble();
            completeDist = completeDist + lapItem->data(4,Qt::DisplayRole).toDouble();

            lapItem->setData(1,Qt::UserRole,intTime);
            lapItem->setData(3,Qt::DisplayRole,set_doubleValue(completeDist,true));
            lapItem->setData(3,Qt::UserRole,completeTime);
            lapItem->setData(4,Qt::UserRole,set_doubleValue(lapItem->data(4,Qt::DisplayRole).toDouble()*1000.0,true));
        }
    }

    if(currActivity->isSwim)
    {
        currActivity->update_moveTime(completeTime-breakTime);
        currActivity->set_overrideData("total_work",QString::number(ceil(totalWork)));
        currActivity->set_swimTimeInZone(true);
    }

    currActivity->activityInfo.insert("Distance",QString::number(set_doubleValue(completeDist+poolLength,true)));
    currActivity->activityInfo.insert("Total Work",QString::number(ceil(totalWork)));

    this->set_activityInfo();
}

void MainWindow::on_toolButton_merge_clicked()
{
    QTreeWidgetItem *item = ui->treeWidget_activity->currentItem();
    QTreeWidgetItem *mergeItem = ui->treeWidget_activity->itemBelow(item);
    QTreeWidgetItem *parent = item->parent();

    int mergeValue;

    mergeValue = get_timesec(item->data(4,Qt::DisplayRole).toString());
    mergeValue = mergeValue + get_timesec(mergeItem->data(4,Qt::DisplayRole).toString());
    mergeItem->setData(4,Qt::DisplayRole,set_time(mergeValue));

    mergeValue = item->data(8,Qt::DisplayRole).toInt();
    mergeValue = mergeValue + mergeItem->data(8,Qt::DisplayRole).toInt();
    mergeItem->setData(8,Qt::DisplayRole,mergeValue);

    parent->removeChild(item);

    this->updated_changedInterval(mergeItem);
    this->refresh_activityTree();
    ui->treeWidget_activity->setCurrentItem(mergeItem);
}

void MainWindow::on_toolButton_delete_clicked()
{
    ui->treeWidget_activity->takeTopLevelItem(selectedInt);
}

void MainWindow::on_toolButton_add_clicked()
{
    QTreeWidgetItem *item = new QTreeWidgetItem();
    ui->treeWidget_activity->insertTopLevelItem(selectedInt,item);
}

void MainWindow::on_toolButton_upInt_clicked()
{
    QTreeWidgetItem *item = ui->treeWidget_activity->itemAbove(ui->treeWidget_activity->currentItem());
    ui->treeWidget_activity->setCurrentItem(item);
    this->set_selecteditem(item,0);
}

void MainWindow::on_toolButton_downInt_clicked()
{
    QTreeWidgetItem *item = ui->treeWidget_activity->itemBelow(ui->treeWidget_activity->currentItem());
    ui->treeWidget_activity->setCurrentItem(item);
    this->set_selecteditem(item,0);
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
        this->freeMem();
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
            this->load_activity(filename,true);
        }
    }
}

void MainWindow::on_actionReset_triggered()
{
    this->clearActivtiy();
    avgSelect_del.sport = QString();
    ui->horizontalSlider_factor->setEnabled(false);
}

void MainWindow::on_toolButton_clearSelect_clicked()
{
    this->reset_avgSelection();
}

void MainWindow::on_actionIntervall_Editor_triggered()
{
    int dialog_code;
    Dialog_workCreator workCreator(this,stdWorkouts,workSchedule);
    workCreator.setModal(true);
    dialog_code = workCreator.exec();

    if(dialog_code == QDialog::Accepted)
    {
        if(workSchedule->get_isUpdated())
        {
            ui->actionSave->setEnabled(true);
            foodPlan->update_fromSchedule();
        }
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
    currActivity->set_workoutContent(ui->lineEdit_workContent->text());
    ui->toolButton_sync->setEnabled(false);
}

void MainWindow::on_toolButton_clearContent_clicked()
{
    ui->lineEdit_workContent->clear();
    ui->toolButton_clearContent->setEnabled(false);
    ui->toolButton_sync->setEnabled(false);
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
    ui->frame_pmcControl->setVisible(!checked);
    ui->stressPlot_Layout->itemAt(0)->widget()->setVisible(!checked);
    ui->widget_distribution->setVisible(!checked);

    ui->frame_saisonEstimate->setVisible(checked);
    ui->tableWidget_saison->setVisible(checked);
    ui->tableWidget_weekAvg->setVisible(checked);
    ui->stressPlot_Layout->itemAt(1)->widget()->setVisible(checked);

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
    ui->frame_phases->setVisible(checked);
}

void MainWindow::on_actionRefresh_Filelist_triggered()
{
    statusLabel->setText("Refresh Filelist");
    ui->treeWidget_activityfiles->clear();
    ui->treeWidget_activityfiles->setColumnCount(currActivity->gcActivtiesMap.last().count());
    set_progress(10);
    currActivity->check_activityFiles();
    this->activityList();
    set_progress(100);
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

void MainWindow::set_menuItemValues(QDate selectDay,QString section)
{
    QString foodName,listName;

    QStandardItem *dayItem = foodPlan->get_modelItem(foodPlan->foodPlanModel,selectDay.toString("yyyy-MM-dd"),0);
    QStandardItem *sectionItem = dayItem->child(foodPlan->mealsHeader.indexOf(section));

    ui->listWidget_menuEdit->clear();
    QListWidgetItem *listItem;
    QMap<int,QListWidgetItem *> itemMap;

    int colCount = foodTree->columnCount();

    for(int meal = 0; meal < sectionItem->rowCount(); ++meal)
    {
        foodName = sectionItem->child(meal,1)->data(Qt::DisplayRole).toString();

        listItem = new QListWidgetItem();


        listName =  foodName +" ("+
                      sectionItem->child(meal,2)->data(Qt::DisplayRole).toString()+" - "+
                      sectionItem->child(meal,3)->data(Qt::DisplayRole).toString()+")";

        listItem->setData(Qt::DisplayRole,listName);
        listItem->setData(Qt::AccessibleTextRole,sectionItem->child(meal,0)->data(Qt::DisplayRole));
        listItem->setData(Qt::AccessibleDescriptionRole,foodName);

        for(int value = 2; value <= colCount; ++value)
        {
            listItem->setData(Qt::UserRole+(value-1),sectionItem->child(meal,value)->data(Qt::DisplayRole).toDouble());
        }

        listItem->setData(Qt::UserRole,colCount);
        listItem->setData(Qt::UserRole+colCount,sectionItem->child(meal,10)->data(Qt::DisplayRole).toInt());

        itemMap.insert(sectionItem->child(meal,9)->data(Qt::DisplayRole).toInt(),listItem);
    }

    this->set_mealItems(itemMap);

    ui->toolButton_addMeal->setProperty("Editmode",DEL);

}

void MainWindow::set_mealItems(QMap<int,QListWidgetItem *> itemMap)
{
    QVector<int> menuSum(6,0);

    for(QMap<int,QListWidgetItem*>::const_iterator it = itemMap.cbegin(); it != itemMap.cend(); ++it)
    {
        for(int value = 2,pos=0; value < it.value()->data(Qt::UserRole).toInt(); ++value,++pos)
        {
            menuSum[pos] = menuSum.at(pos) + it.value()->data(Qt::UserRole+value).toInt();
        }

        ui->listWidget_menuEdit->addItem(it.value());
    }

    QStringList macroShort;
    macroShort << " - C:" << " - P:" << " - F:" << " - Fi:" << " - S:";
    QString foodName;

    for(int i = 0; i < macroShort.count(); ++i)
    {
        foodName = foodName + macroShort.at(i)+QString::number(menuSum.at(i+1));
    }

    ui->label_menuCal->setText("Summery: "+QString::number(menuSum[0]) +" KCal " + foodName);
}

void MainWindow::calc_mealItems()
{
    QMap<int,QListWidgetItem *> itemMap;

    for(int i = 0; i < ui->listWidget_menuEdit->count(); ++i)
    {
        itemMap.insert(i,ui->listWidget_menuEdit->item(i));
    }

    this->set_mealItems(itemMap);
}


void MainWindow::set_selectedMeals(QTreeWidgetItem *listItem)
{
    bool addnew = true;
    double portion = listItem->data(1,Qt::DisplayRole).toInt();

    if(portion <= 0)
    {
        ui->doubleSpinBox_portion->setRange(0.0,0.0);
        ui->doubleSpinBox_portion->setSingleStep(0.00);
    }
    if(portion >= 0.0 && portion < 10.0)
    {
        ui->doubleSpinBox_portion->setRange(0.0,50.0);
        ui->doubleSpinBox_portion->setSingleStep(0.05);
    }
    if(portion > 10.0)
    {
        ui->doubleSpinBox_portion->setRange(0.0,500.0);
        ui->doubleSpinBox_portion->setSingleStep(5.0);
    }

    for(int i = 0; i < ui->listWidget_menuEdit->count(); ++i)
    {
        if(listItem->data(0,Qt::AccessibleTextRole) == ui->listWidget_menuEdit->item(i)->data(Qt::AccessibleTextRole))
        {
            ui->listWidget_menuEdit->setCurrentItem(ui->listWidget_menuEdit->item(i));
            ui->toolButton_addMeal->setProperty("Editmode",EDIT);
            addnew = false;
            break;
        }
    }
    ui->doubleSpinBox_portion->setValue(0);
    ui->lineEdit_Mealname->setText(listItem->data(0,Qt::DisplayRole).toString());
    ui->spinBox_calories->setValue(listItem->data(2,Qt::DisplayRole).toInt());

    if(addnew)
    {
        ui->spinBox_portFactor->setValue(portion);
        ui->doubleSpinBox_portion->setValue(portion);
        ui->toolButton_addMeal->setProperty("Editmode",ADD);
    }
    else
    {
        ui->spinBox_portFactor->setValue(foodTree->currentItem()->data(1,Qt::DisplayRole).toInt());
        ui->doubleSpinBox_portion->setValue(ui->listWidget_menuEdit->currentItem()->data(Qt::UserRole+1).toDouble());
        ui->toolButton_addMeal->setProperty("Editmode",EDIT);
    }
}

void MainWindow::reset_menuEdit()
{
    ui->toolButton_foodUp->setEnabled(false);
    ui->toolButton_foodDown->setEnabled(false);
    ui->toolButton_menuEdit->setEnabled(false);
    ui->toolButton_menuClear->setEnabled(false);
    ui->spinBox_portFactor->clear();
    ui->doubleSpinBox_portion->clear();
    ui->lineEdit_calories->clear();
    ui->lineEdit_editSection->clear();
    ui->dateEdit_selectDay->clear();
    ui->listWidget_menuEdit->clear();
}

void MainWindow::on_listWidget_weekPlans_itemClicked(QListWidgetItem *item)
{
    if(foodPlan->copyQueue_hasData())
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

    foodPlan->set_currentWeek(item->data(Qt::UserRole).toDate());
    ui->comboBox_weightmode->setCurrentText(item->data(Qt::AccessibleDescriptionRole).toString());
    ui->dateEdit_selectWeek->setDate(item->data(Qt::UserRole).toDate());
    ui->lineEdit_selectWeek->setText(item->data(Qt::AccessibleTextRole).toString());
    ui->comboBox_weightmode->setEnabled(true);

    this->fill_foodPlanTable(item->data(Qt::UserRole).toDate());
    this->fill_foodSumTable(item->data(Qt::UserRole).toDate());
}


void MainWindow::on_listWidget_menuEdit_itemDoubleClicked(QListWidgetItem *item)
{
    if(item->isSelected())
    {
        ui->listWidget_menuEdit->takeItem(ui->listWidget_menuEdit->currentRow());
        ui->listWidget_menuEdit->clearSelection();
        this->calc_mealItems();
    }
}

void MainWindow::on_toolButton_menuEdit_clicked()
{
    int dataCount = 0;
    QListWidgetItem *item;

    QString foodName;
    QMap<int,QList<QStandardItem*>> itemMap;
    QList<QStandardItem*> itemList;
    QStandardItem *mealItem;

    for(int row = 0; row < ui->listWidget_menuEdit->count(); ++row)
    {
        itemList.clear();
        item = ui->listWidget_menuEdit->item(row);
        dataCount = item->data(Qt::UserRole).toInt();
        foodName = item->data(Qt::AccessibleDescriptionRole).toString();

        itemList.insert(0,new QStandardItem(item->data(Qt::AccessibleTextRole).toString()));
        itemList.insert(1,new QStandardItem(foodName));

        for(int i = 1; i < dataCount; ++i)
        {
            mealItem = new QStandardItem();
            mealItem->setData(item->data(Qt::UserRole+i).toDouble(),Qt::DisplayRole);
            itemList.insert(i+1,mealItem);
        }

        mealItem = new QStandardItem();
        mealItem->setData(row,Qt::DisplayRole);
        itemList.append(mealItem);

        mealItem = new QStandardItem();
        mealItem->setData(item->data(Qt::UserRole+dataCount).toInt(),Qt::DisplayRole);
        itemList.append(mealItem);

        itemMap.insert(row,itemList);
    }

    foodPlan->update_foodPlanModel(ui->dateEdit_selectDay->date(),ui->lineEdit_editSection->text(),itemMap);
    this->reset_menuEdit();
}

void MainWindow::on_calendarWidget_Food_clicked(const QDate &date)
{
    QDate firstDay = date.addDays(1 - date.dayOfWeek());

    for(int week = 0; week < ui->listWidget_weekPlans->count(); ++week)
    {
        if(firstDay == ui->listWidget_weekPlans->item(week)->data(Qt::UserRole).toDate())
        {
            ui->listWidget_weekPlans->setCurrentItem(ui->listWidget_weekPlans->item(week));
            emit ui->listWidget_weekPlans->itemClicked(ui->listWidget_weekPlans->item(week));
        }
        else
        {
            ui->listWidget_weekPlans->clearSelection();
            ui->lineEdit_selectWeek->setText("Add: " + calc_weekID(date));
            ui->dateEdit_selectWeek->setDate(date);
            ui->comboBox_weightmode->setCurrentIndex(0);
            ui->actionNew->setEnabled(true);
        }
    }
}

void MainWindow::on_doubleSpinBox_portion_valueChanged(double value)
{
    ui->lineEdit_calories->setText(QString::number(round(value * ui->spinBox_calories->value()/ui->spinBox_portFactor->value())));
}

void MainWindow::on_toolButton_addMeal_clicked()
{
    double foodFactor = ui->doubleSpinBox_portion->value() / ui->spinBox_portFactor->value();
    QString listName,foodName;

    QListWidgetItem *mealItem = nullptr;

    if(ui->toolButton_addMeal->property("Editmode") == EDIT)
    {
        mealItem = ui->listWidget_menuEdit->currentItem();
    }
    else if(ui->toolButton_addMeal->property("Editmode") == ADD)
    {
        mealItem = new QListWidgetItem();
    }
    else
    {
        mealItem = new QListWidgetItem();
    }

    foodName = foodTree->currentItem()->data(0,Qt::DisplayRole).toString();

    listName = foodName +" ("+
               QString::number(ui->doubleSpinBox_portion->value()) +" - "+
               QString::number(round(foodTree->currentItem()->data(2,Qt::DisplayRole).toDouble()*foodFactor))+")";

    mealItem->setData(Qt::DisplayRole,listName);
    mealItem->setData(Qt::AccessibleTextRole,foodTree->currentItem()->data(0,Qt::AccessibleTextRole).toString());
    mealItem->setData(Qt::AccessibleDescriptionRole,foodName);

    mealItem->setData(Qt::UserRole+1,set_doubleValue(ui->doubleSpinBox_portion->value(),false));

    int colCount = foodTree->columnCount();

    for(int col = 2; col < colCount; ++col)
    {
        mealItem->setData(Qt::UserRole+col,round(foodTree->currentItem()->data(col,Qt::DisplayRole).toDouble()*foodFactor));
    }
    mealItem->setData(Qt::UserRole,colCount);
    mealItem->setData(Qt::UserRole+colCount,foodTree->currentItem()->data(0,Qt::UserRole).toInt());

    if(ui->toolButton_addMeal->property("Editmode") == ADD)
    {
        ui->listWidget_menuEdit->addItem(mealItem);
    }

    this->calc_mealItems();
}

void MainWindow::change_foodOrder(int moveID)
{
    int currentindex = ui->listWidget_menuEdit->currentRow();
    QListWidgetItem *currentItem = ui->listWidget_menuEdit->takeItem(currentindex);

    ui->listWidget_menuEdit->insertItem(currentindex+moveID,currentItem);
    ui->listWidget_menuEdit->setCurrentRow(currentindex+moveID);
}

void MainWindow::fill_foodTrees()
{
    QStandardItemModel *model = nullptr;
    QStandardItem *item;
    QTreeWidgetItem *rootItem,*sectionItem,*treeItem;

    QList<QTreeWidget*> treeList;
    treeList.insert(0,ui->treeWidget_recipe);
    treeList.insert(1,ui->treeWidget_ingred);
    treeList.insert(2,ui->treeWidget_drink);

    QMap<int,QStandardItemModel*> modelMap;
    modelMap.insert(0,foodPlan->recipeModel);
    modelMap.insert(1,foodPlan->ingredModel);
    modelMap.insert(2,foodPlan->drinkModel);

    QString foodID;

    for(QMap<int,QStandardItemModel*>::const_iterator it = modelMap.cbegin(); it != modelMap.cend(); ++it)
    {
        treeList.at(it.key())->clear();
        model = it.value();
        rootItem = treeList.at(it.key())->invisibleRootItem();

        for(int row = 0; row < model->rowCount(); ++row)
        {
            item = model->item(row,0);

            sectionItem = new QTreeWidgetItem();
            sectionItem->setData(0,Qt::DisplayRole,item->data(Qt::DisplayRole).toString());

            if(item->hasChildren())
            {
                for(int child = 0; child < item->rowCount(); ++child)
                {
                    foodID = item->child(child,0)->data(Qt::DisplayRole).toString();

                    treeItem = new QTreeWidgetItem();
                    treeItem->setData(0,Qt::AccessibleTextRole,foodID);
                    treeItem->setData(0,Qt::UserRole,it.key());
                    treeItem->setData(0,Qt::DisplayRole,item->child(child,1)->data(Qt::DisplayRole).toString());
                    treeItem->setData(1,Qt::DisplayRole,item->child(child,2)->data(Qt::DisplayRole).toDouble());
                    treeItem->setData(2,Qt::DisplayRole,item->child(child,3)->data(Qt::DisplayRole).toDouble());
                    treeItem->setData(3,Qt::DisplayRole,item->child(child,4)->data(Qt::DisplayRole).toDouble());
                    treeItem->setData(4,Qt::DisplayRole,item->child(child,5)->data(Qt::DisplayRole).toDouble());
                    treeItem->setData(5,Qt::DisplayRole,item->child(child,6)->data(Qt::DisplayRole).toDouble());
                    treeItem->setData(6,Qt::DisplayRole,item->child(child,7)->data(Qt::DisplayRole).toDouble());
                    treeItem->setData(7,Qt::DisplayRole,item->child(child,8)->data(Qt::DisplayRole).toDouble());
                    sectionItem->addChild(treeItem);
                    foodTreeMap.insert(foodID,treeItem);
                }
            }

            rootItem->addChild(sectionItem);
        }
        treeList.at(it.key())->sortItems(0,Qt::AscendingOrder);
    }
    foodTree = ui->treeWidget_recipe;
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
    int dataCount = item->data(Qt::UserRole).toInt();

    ui->toolBox_foodSelect->setCurrentIndex(item->data(Qt::UserRole+dataCount).toInt());
    foodTree->setCurrentItem(foodTreeMap.value(item->data(Qt::AccessibleTextRole).toString()));

    this->set_selectedMeals(foodTree->currentItem());
}

void MainWindow::on_tableWidget_foodPlan_itemClicked(QTableWidgetItem *item)
{
    ui->toolBox_foodSelect->setCurrentIndex(0);

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

    this->set_menuItemValues(item->data(Qt::UserRole).toDate(),foodPlan->mealsHeader.at(item->row()));
    foodPlan->clear_dragDrop();
}

void MainWindow::on_tableWidget_foodPlan_itemPressed(QTableWidgetItem *item)
{
    foodPlan->set_dragDrop(item->data(Qt::UserRole).toDate(),foodPlan->mealsHeader.at(item->row()));
}

void MainWindow::refresh_foodTables()
{
    ui->tableWidget_foodPlan->blockSignals(true);
    this->fill_foodPlanTable(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate());
    ui->tableWidget_foodPlan->blockSignals(false);
    ui->actionSave->setEnabled(true);
}

void MainWindow::refresh_summeryTables()
{
    this->fill_foodSumTable(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate());
}

void MainWindow::on_tableWidget_foodPlan_itemChanged(QTableWidgetItem *item)
{
    if(foodPlan->dragDrop_hasData())
    { 
       foodPlan->set_dropMeal(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate().addDays(item->column()),foodPlan->mealsHeader.at(item->row()));
    }
}

void MainWindow::on_toolButton_mealreset_clicked()
{
    if(ui->toolBox_foodSelect->currentIndex() == 0)
    {
        ui->treeWidget_recipe->collapseAll();
    }
    else if(ui->toolBox_foodSelect->currentIndex() == 1)
    {
        ui->treeWidget_ingred->collapseAll();
    }
    else if(ui->toolBox_foodSelect->currentIndex() == 2)
    {
        ui->treeWidget_drink->collapseAll();
    }
}

void MainWindow::on_toolButton_menuClear_clicked()
{
    ui->listWidget_menuEdit->clear();
    this->calc_mealItems();
}

void MainWindow::on_toolButton_menuCopy_clicked()
{
    foodPlan->fill_copyMap(ui->tableWidget_foodPlan->currentItem()->data(Qt::UserRole).toDate(),foodPlan->mealsHeader.at(ui->tableWidget_foodPlan->currentItem()->row()),true);

    ui->dateEdit_copyDay->setDate(ui->dateEdit_selectDay->date());
    ui->toolButton_menuPaste->setEnabled(true);
}

void MainWindow::on_toolButton_menuPaste_clicked()
{
    ui->listWidget_menuEdit->clear();
    QPair<QDate,QString> mealCopy = foodPlan->get_copyMeal();
    this->set_menuItemValues(mealCopy.first,mealCopy.second);
    ui->toolButton_menuCopy->setEnabled(false);
    ui->toolButton_menuCopy->setChecked(false);
}

void MainWindow::on_comboBox_weightmode_currentIndexChanged(const QString &value)
{
    if(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate() == ui->dateEdit_selectWeek->date())
    {
        foodPlan->update_foodMode(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate(),value);
        ui->comboBox_weightmode->setEnabled(false);
        ui->actionSave->setEnabled(true);
        this->fill_foodPlanList(false,ui->listWidget_weekPlans->currentRow());
        this->fill_foodSumTable(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate());
    }
}

void MainWindow::selectFoodSection(int selectedSection)
{
    ui->tableWidget_selectedSection->clearContents();
    ui->label_CopyInfo->setText("Selected Meal:");
    ui->dateEdit_copyDay->setVisible(false);
    ui->dateEdit_copyDay->setDate(ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate());
    ui->dateEdit_copyDay->setProperty("Section",true);
    ui->lineEdit_copySection->setVisible(true);
    ui->lineEdit_copySection->setText(foodPlan->mealsHeader.at(selectedSection));

    foodPlan->check_copyQueue();

    QTableWidgetItem *item;

    for(int day = 0; day < weekDays; ++day)
    {
        item = ui->tableWidget_foodPlan->item(selectedSection,day);
        foodPlan->fill_copyMap(item->data(Qt::UserRole).toDate(),foodPlan->mealsHeader.at(selectedSection),false);
    }

    ui->frame_dayShow->setVisible(true);
    ui->frame_menuEdit->setVisible(false);
    ui->tableWidget_selectedSection->verticalHeader()->setVisible(false);
    ui->tableWidget_selectedSection->horizontalHeader()->setVisible(false);
}

void MainWindow::selectFoodDay(int selectedDay)
{
    ui->label_CopyInfo->setText("Selected "+ QLocale().dayName(selectedDay+1)+":");
    ui->lineEdit_copySection->setVisible(false);
    ui->dateEdit_copyDay->setVisible(true);
    ui->dateEdit_copyDay->setDate(ui->tableWidget_foodPlan->horizontalHeaderItem(selectedDay)->data(Qt::UserRole).toDate());
    ui->dateEdit_copyDay->setProperty("Section",false);

    foodPlan->check_copyQueue();

    QTableWidgetItem *item;

    for(int section = 0; section < foodPlan->mealsHeader.count(); ++section)
    {
        item = ui->tableWidget_foodPlan->item(section,selectedDay);
        foodPlan->fill_copyMap(item->data(Qt::UserRole).toDate(),foodPlan->mealsHeader.at(section),false);
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
    QVector<double> mealCalories(4,0);
    QVector<double> sumCalories(4,0);
    QMap<int,QVector<double>> sumMap;
    int fiberSum = 0;


    QStandardItem *dayItem = foodPlan->get_proxyItem(0)->child(selectedDay,0);
    QStandardItem *sectionItem;

    for(int section = 0; section < dayItem->rowCount(); ++section)
    {
        sectionItem = dayItem->child(section,0);
        for(int meal = 0; meal < sectionItem->rowCount();++meal)
        {
            mealCalories[0] = round(mealCalories.at(0) + sectionItem->child(meal,3)->data(Qt::DisplayRole).toDouble());
            mealCalories[1] = round(mealCalories.at(1) + (sectionItem->child(meal,4)->data(Qt::DisplayRole).toDouble()*4.1));
            mealCalories[2] = round(mealCalories.at(2) + (sectionItem->child(meal,5)->data(Qt::DisplayRole).toDouble()*4.1));
            mealCalories[3] = round(mealCalories.at(3) + (sectionItem->child(meal,6)->data(Qt::DisplayRole).toDouble()*9.3));
            fiberSum = fiberSum + sectionItem->child(meal,7)->data(Qt::DisplayRole).toDouble();
        }
        sumMap.insert(section,mealCalories);
        sumCalories[0] = sumCalories.at(0) + mealCalories.at(0);
        sumCalories[1] = sumCalories.at(1) + mealCalories.at(1);
        sumCalories[2] = sumCalories.at(2) + mealCalories.at(2);
        sumCalories[3] = sumCalories.at(3) + mealCalories.at(3);
        sumMap.insert(foodPlan->mealsHeader.count(),sumCalories);
        mealCalories.fill(0);
        fiberSum = 0;
    }

    double percent = 0;
    double sumCal = sumMap.last().at(0);

    for(QMap<int,QVector<double>>::const_iterator it = sumMap.cbegin(); it != sumMap.cend(); ++it)
    {
        for(int col = 0; col < it.value().count(); ++col)
        {
            if(col == 0)
            {
                percent = set_doubleValue((it.value().at(col) / sumCal)*100.0,false);
            }
            else
            {
                percent = set_doubleValue((it.value().at(col) / it.value().at(0))*100.0,false);
            }

            item = new QTableWidgetItem();
            item->setData(Qt::DisplayRole,QString::number(it.value().at(col))+" ("+QString::number(percent)+")");
            item->setData(Qt::UserRole,percent);
            ui->tableWidget_selectedSection->setItem(it.key(),col,item);
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
    QDate copyDate;
    if(ui->dateEdit_copyDay->property("Section").toBool())
    {
        copyDate = ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate();
    }
    else
    {
        copyDate = ui->listWidget_weekPlans->currentItem()->data(Qt::UserRole).toDate().addDays(ui->tableWidget_foodPlan->currentColumn());
    }

    foodPlan->execute_copy(copyDate,ui->dateEdit_copyDay->property("Section").toBool());
}

void MainWindow::on_tableWidget_schedule_itemClicked(QTableWidgetItem *item)
{
    int dialog_code;

    if(item->column() == 0)
    {
        Dialog_week_copy week_copy(this,item->data(Qt::UserRole).toString(),workSchedule,true);
        week_copy.setModal(true);
        dialog_code = week_copy.exec();

        if(dialog_code == QDialog::Accepted)
        {
           this->refresh_schedule();
           ui->actionSave->setEnabled(workSchedule->get_isUpdated());
           foodPlan->update_fromSchedule();
        }
    }
    else
    {
        QDate selectedDate = item->data(Qt::UserRole).toDate();
        day_popup day_pop(this,selectedDate,workSchedule,stdWorkouts);
        day_pop.setModal(true);
        dialog_code = day_pop.exec();

        if(dialog_code == QDialog::Rejected)
        {
            ui->actionSave->setEnabled(workSchedule->get_isUpdated());
            if(foodPlan->check_foodDay(selectedDate))
            {
                foodPlan->update_fromSchedule();
            }
        }
    }
}

void MainWindow::on_tableWidget_saison_itemClicked(QTableWidgetItem *item)
{
    int dialog_code;

    if(item->column() == 0)
    {
       Dialog_addweek addweek(this,item->data(Qt::UserRole).toString(),workSchedule);
       addweek.setModal(true);
       dialog_code = addweek.exec();

       if(dialog_code == QDialog::Accepted)
       {
          this->refresh_saison();
          ui->actionSave->setEnabled(workSchedule->get_isUpdated());
       }
    }
}

void MainWindow::on_treeWidget_activityfiles_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    ui->lineEdit_workContent->clear();

    if(this->clearActivtiy()) this->load_activity(item->data(0,Qt::UserRole).toString(),false);
}

void MainWindow::on_treeWidget_activity_itemClicked(QTreeWidgetItem *item, int column)
{
    this->set_selecteditem(item,column);
}

void MainWindow::on_timeEdit_intDuration_userTimeChanged(const QTime &time)
{
    this->recalc_selectedInt(time,ui->doubleSpinBox_intDistance->value());
}

void MainWindow::on_doubleSpinBox_intDistance_valueChanged(double dist)
{
    this->recalc_selectedInt(ui->timeEdit_intDuration->time(),dist);
}

void MainWindow::on_actionExpand_Activity_Tree_triggered()
{
    ui->treeWidget_activity->collapseAll();
}

void MainWindow::on_pushButton_stressPlot_clicked(bool checked)
{
    ui->frame_PMC->setVisible(checked);

    if(isWeekMode)
    {
        if(checked)
        {
            weekRange = weekRange / 2;
            ui->pushButton_stressPlot->setIcon(iconMap.value("PaneDown"));
        }
        else
        {
            weekRange = settings::get_intValue("weekrange");
            ui->pushButton_stressPlot->setIcon(iconMap.value("PaneUp"));
        }
        this->refresh_schedule();
    }
    else
    {
        if(checked)
        {
            weekRange = (weekRange / 2)+1;
            ui->pushButton_stressPlot->setIcon(iconMap.value("PaneDown"));
        }
        else
        {
            weekRange = settings::get_intValue("weekrange")+2;
            ui->pushButton_stressPlot->setIcon(iconMap.value("PaneUp"));
        }
        this->refresh_saison();
    }
}

void MainWindow::on_pushButton_currentWeek_clicked(bool checked)
{
    workSchedule->calc_pmcPlot(firstdayofweek.addDays(weekCounter*weekDays),checked,ui->spinBox_extWeeks->value());
    ui->spinBox_extWeeks->setEnabled(!checked);
}

void MainWindow::on_spinBox_extWeeks_valueChanged(int value)
{
    workSchedule->calc_pmcPlot(firstdayofweek.addDays(weekCounter*weekDays),ui->pushButton_currentWeek->isChecked(),value);
}

void MainWindow::on_toolButton_extReset_clicked()
{
    ui->spinBox_extWeeks->setValue(ui->spinBox_extWeeks->minimum());
}

void MainWindow::on_comboBox_saisonSport_currentIndexChanged(const QString &sport)
{
    Q_UNUSED(sport)

    //this->set_compData(ui->spinBox_phaseWeeks->value(),ui->dateEdit_phaseStart->date(),sport);
}

void MainWindow::on_toolButton_recipeEditor_clicked()
{
    Dialog_nutrition nutritionEditor(this,foodPlan);
    nutritionEditor.setModal(true);
    int dialog_code = nutritionEditor.exec();

    if(dialog_code == QDialog::Accepted) this->fill_foodTrees();
}
void MainWindow::on_treeWidget_recipe_itemClicked(QTreeWidgetItem *item, int column)
{
    if(item->childCount() > 0 && column == 0)
    {
        ui->treeWidget_recipe->expand(ui->treeWidget_recipe->currentIndex());
    }
    else
    {
        //this->set_selectedMeals(item,item->data(1,Qt::DisplayRole).toDouble());
        this->set_selectedMeals(item);
    }
}

void MainWindow::on_treeWidget_ingred_itemClicked(QTreeWidgetItem *item, int column)
{
    if(item->childCount() > 0 && column == 0)
    {
        ui->treeWidget_ingred->expand(ui->treeWidget_ingred->currentIndex());
    }
    else
    {
        //this->set_selectedMeals(item,item->data(1,Qt::DisplayRole).toDouble());
        this->set_selectedMeals(item);
    }
}

void MainWindow::on_treeWidget_drink_itemClicked(QTreeWidgetItem *item, int column)
{
    if(item->childCount() > 0 && column == 0)
    {
        ui->treeWidget_drink->expand(ui->treeWidget_drink->currentIndex());
    }
    else
    {
        //this->set_selectedMeals(item,item->data(1,Qt::DisplayRole).toDouble());
        this->set_selectedMeals(item);
    }
}

void MainWindow::on_toolBox_foodSelect_currentChanged(int index)
{
    if(index == 0)
    {
        foodTree = ui->treeWidget_recipe;
        ui->label_portInfo->setText("Portion");
    }
    if(index == 1)
    {
        foodTree = ui->treeWidget_ingred;
        ui->label_portInfo->setText("Gramm");
    }
    if(index == 2)
    {
        foodTree = ui->treeWidget_drink;
        ui->label_portInfo->setText("ml");
    }
}
