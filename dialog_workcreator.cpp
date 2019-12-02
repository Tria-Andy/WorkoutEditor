#include "dialog_workcreator.h"
#include "ui_dialog_workcreator.h"

Dialog_workCreator::Dialog_workCreator(QWidget *parent, standardWorkouts *pworkouts,schedule *psched) :
    QDialog(parent),
    ui(new Ui::Dialog_workCreator)
{
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    stdWorkouts = pworkouts;
    worksched = psched;
    workoutMap = stdWorkouts->get_workoutMap();
    xmlTagMap = stdWorkouts->get_xmlTagMap();
    isSwim = isBike = isRun = isStrength = isAlt = isOther = isTria = false;

    plotModel = new QStandardItemModel(this);
    valueModel = new QStandardItemModel(this);

    metaProxy = new QSortFilterProxyModel(this);
    metaProxy->setSourceModel(stdWorkouts->workouts_meta);
    stepProxy = new QSortFilterProxyModel(this);
    stepProxy->setSourceModel(stdWorkouts->workouts_steps);
    schedProxy = new QSortFilterProxyModel(this);
    schedProxy->setSourceModel(worksched->scheduleModel);
    proxyFilter = new QSortFilterProxyModel(this);
    proxyFilter->setSourceModel(schedProxy);

    editRow <<1<<1<<1<<0<<0<<1<<0<<0<<0<<0;

    isSeries = "Series";
    isGroup = "Group";
    ui->comboBox_partName->addItem(isSeries);
    ui->comboBox_partName->addItem(isGroup);
    ui->comboBox_stepName->addItems(settings::get_listValues("IntEditor"));
    ui->comboBox_levelName->addItems(settings::get_listValues("Level"));
    isBreak = generalValues->value("breakname");
    dateFormat = settings::get_format("dateformat");
    longTime = settings::get_format("dateformat");
    shortTime = settings::get_format("dateformat");
    groupList << isGroup << isSeries;
    levelList = settings::get_listValues("Level");
    ui->listWidget_group->addItems(groupList);
    ui->listWidget_group->setItemDelegate(&mousehover_del);
    ui->listWidget_phases->addItems(settings::get_listValues("IntEditor"));
    ui->listWidget_phases->setItemDelegate(&mousehover_del);
    ui->comboBox_sport->addItems(settings::get_listValues("Sport"));
    ui->comboBox_code->addItems(settings::get_listValues("WorkoutCode"));
    clearFlag = false;

    modelHeader << "Phase" << "Level" << "Threshold %" << "Pace" << "Power" << "Time" << "TSS" << "Work" << "Distance" << "Repeats";
    ui->treeWidget_workoutTree->setHeaderLabels(modelHeader);
    ui->treeWidget_workoutTree->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidget_workoutTree->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->treeWidget_workoutTree->setAcceptDrops(true);
    ui->treeWidget_workoutTree->setDragEnabled(true);
    ui->treeWidget_workoutTree->setDragDropMode(QAbstractItemView::DragDrop);
    ui->treeWidget_workoutTree->setEnabled(true);
    ui->treeWidget_workoutTree->setItemDelegate(&workTree_del);
    workTree_del.groupList = groupList;

    ui->listWidget_workouts->setItemDelegate(&mousehover_del);

    ui->frame_edit->setVisible(false);
    ui->label_head->setText("Add Phase");

    viewBackground = "background-color: #e6e6e6";

    ui->toolButton_copy->setEnabled(false);
    ui->label_picName->setVisible(false);
    this->set_controlButtons(false);

    ui->treeWidget_workoutTree->setStyleSheet(viewBackground);
    ui->listWidget_group->setStyleSheet(viewBackground);
    ui->listWidget_phases->setStyleSheet(viewBackground);
    ui->widget_plot->setStyleSheet(viewBackground);

    ui->widget_plot->xAxis->setTicks(true);
    ui->widget_plot->xAxis->setTickLabels(true);
    ui->widget_plot->xAxis->setLabel("Time - Minutes");
    ui->widget_plot->yAxis->setTicks(true);
    ui->widget_plot->yAxis->setTickLabels(true);
    ui->widget_plot->yAxis->setLabel("Threshold %");
    ui->widget_plot->xAxis2->setVisible(true);
    ui->widget_plot->xAxis2->setLabel("Distance - KM");
    ui->widget_plot->yAxis2->setVisible(true);


    //Update Dialog
    updateDialog = new QDialog(this);
    updateDialog->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    updateDialog->setFixedHeight(260);
    updateDialog->setFixedWidth(280);

    workoutModel = new QStandardItemModel(updateDialog);

    QVBoxLayout *diaLayout = new QVBoxLayout(updateDialog);
    diaLayout->setContentsMargins(3,3,3,3);
    diaLayout->setSpacing(5);

    QFrame *labelFrame = new QFrame(updateDialog);
    labelFrame->setMaximumWidth(updateDialog->width());
    labelFrame->setMaximumHeight(20);
    QHBoxLayout *hLabel = new QHBoxLayout(labelFrame);
    hLabel->setContentsMargins(5,2,5,2);
    QLabel *updateLabel = new QLabel(labelFrame);
    //updateLabel->setFont(QFont().setBold(true));
    updateLabel->setText("Update future connected Workouts in schedule?");
    hLabel->addWidget(updateLabel);
    labelFrame->setLayout(hLabel);
    diaLayout->addWidget(labelFrame);

    QGroupBox *selectGroup = new QGroupBox(updateDialog);
    selectGroup->setTitle("Selection");
    selectGroup->setMaximumHeight(70);
    selectGroup->setMaximumWidth(updateDialog->width());
    QVBoxLayout *selectBox = new QVBoxLayout(selectGroup);
    selectBox->setContentsMargins(5,2,5,2);
    selectBox->setSpacing(5);
    selectGroup->setLayout(selectBox);

    QFrame *radioFrame = new QFrame(selectGroup);
    radioFrame->setMaximumWidth(updateDialog->width());
    radioFrame->setMaximumHeight(30);
    QHBoxLayout *hRadio = new QHBoxLayout(radioFrame);
    hRadio->setContentsMargins(5,2,5,2);
    hRadio->setSpacing(5);
    updateAll = new QRadioButton("All Workouts",radioFrame);
    updateRange = new QRadioButton("Workouts in Range",radioFrame);
    updateAll->setChecked(true);
    hRadio->addWidget(updateAll);
    hRadio->addStretch();
    hRadio->addWidget(updateRange);

    QFrame *dateFrame = new QFrame(selectGroup);
    dateFrame->setMaximumWidth(updateDialog->width());
    dateFrame->setMaximumHeight(30);
    QHBoxLayout *hDate = new QHBoxLayout(dateFrame);
    hDate->setContentsMargins(5,2,5,2);
    hDate->setSpacing(5);
    QLabel *fromLabel = new QLabel(labelFrame);
    fromLabel->setText("From:");
    updateFrom = new QDateEdit(dateFrame);
    updateFrom->setEnabled(false);
    updateFrom->setCalendarPopup(true);
    QLabel *toLabel = new QLabel(labelFrame);
    toLabel->setText("To:");
    updateTo = new QDateEdit(dateFrame);
    updateTo->setEnabled(false);
    updateTo->setCalendarPopup(true);
    hDate->addWidget(fromLabel);
    hDate->addWidget(updateFrom);
    hDate->addStretch();
    hDate->addWidget(toLabel);
    hDate->addWidget(updateTo);
    dateFrame->setLayout(hDate);

    selectBox->addWidget(radioFrame);
    selectBox->addWidget(dateFrame);
    diaLayout->addWidget(selectGroup);

    QFrame *viewFrame = new QFrame(updateDialog);
    viewFrame->setFrameStyle(QFrame::Box | QFrame::Sunken);
    viewFrame->setMaximumWidth(updateDialog->width());
    viewFrame->setMaximumHeight(140);
    QVBoxLayout *viewBox = new QVBoxLayout(viewFrame);
    viewBox->setContentsMargins(1,1,1,1);
    workoutView = new QListView(viewFrame);
    workoutView->setModel(workoutModel);
    workoutView->setSelectionMode(QAbstractItemView::NoSelection);
    viewBox->addWidget(workoutView);

    QFrame *statusFrame = new QFrame(viewFrame);
    statusFrame->setMaximumWidth(updateDialog->width());
    statusFrame->setMaximumHeight(30);
    QHBoxLayout *statusBox = new QHBoxLayout(statusFrame);
    updateProgess = new QProgressBar(statusFrame);
    updateProgess->setTextVisible(true);
    updateProgess->setMinimum(0);
    updateProgess->setFixedHeight(15);
    updateProgess->setFormat("%v");
    statusBox->setContentsMargins(2,2,2,2);
    updateOk = new QToolButton(statusFrame);
    updateOk->setFixedSize(26,26);
    updateOk->setIcon(QIcon(":/images/icons/Sync.png"));
    updateOk->setIconSize(QSize(20,20));
    updateOk->setAutoRaise(true);
    statusBox->addWidget(updateProgess);
    statusBox->addWidget(updateOk);
    viewBox->addWidget(statusFrame);
    diaLayout->addWidget(viewFrame);

    QFrame *buttonFrame = new QFrame(updateDialog);
    buttonFrame->setMaximumWidth(updateDialog->width());
    buttonFrame->setMaximumHeight(25);
    QHBoxLayout *hButton = new QHBoxLayout(buttonFrame);
    hButton->setContentsMargins(5,2,5,2);
    hButton->setSpacing(5);
    updateClose = new QPushButton(buttonFrame);
    updateClose->setText("Close");
    updateClose->setFixedWidth(100);
    hButton->addWidget(updateClose);
    hDate->addStretch();
    hButton->addWidget(updateClose);
    diaLayout->addWidget(buttonFrame);

    connect(updateClose,SIGNAL(clicked(bool)),updateDialog,SLOT(reject()));
    connect(updateAll,SIGNAL(toggled(bool)),this,SLOT(set_updateDates(bool)));
    connect(updateFrom,SIGNAL(dateChanged(QDate)),this,SLOT(set_workoutModel(QDate)));
    connect(updateTo,SIGNAL(dateChanged(QDate)),this,SLOT(set_workoutModel(QDate)));
    connect(updateOk,SIGNAL(clicked(bool)),this,SLOT(update_workouts()));

    this->resetAxis();
}

Dialog_workCreator::~Dialog_workCreator()
{
    delete ui;
}

void Dialog_workCreator::set_controlButtons(bool setButton)
{
    ui->pushButton_clear->setEnabled(setButton);
    ui->toolButton_save->setEnabled(setButton);
    ui->toolButton_delete->setEnabled(setButton);
    ui->pushButton_sync->setEnabled(setButton);
    ui->comboBox_code->setEnabled(setButton);
}

void Dialog_workCreator::get_workouts(QString sport)
{
    ui->listWidget_workouts->clear();

    QHash<QString,QVector<QString>> sportMap = workoutMap->value(sport);

    for(QHash<QString,QVector<QString>>::const_iterator workstart = sportMap.cbegin(), workend = sportMap.cend(); workstart != workend; ++workstart)
    {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole,workstart.value().at(0)+" - "+workstart.value().at(1));
        item->setData(Qt::UserRole,workstart.key());
        item->setData(Qt::UserRole+1,workstart.value().at(6));
        item->setData(Qt::ToolTipRole,set_time(workstart.value().at(2).toInt())+" - "+workstart.value().at(3)+" - "+workstart.value().at(4)+" - "+workstart.value().at(5));

        ui->listWidget_workouts->addItem(item);
    }

    ui->label_workouts->setText("Workouts: "+QString::number(sportMap.count()));

    QColor sportColor = settings::get_itemColor(sport);
    QString sportBack = "rgb("+QString::number(sportColor.red())+","+QString::number(sportColor.green())+","+QString::number(sportColor.blue())+")";
    QString actBackground = "background: "+sportBack;
    ui->listWidget_workouts->setStyleSheet(actBackground);
    this->set_sportData(sport);
}

void Dialog_workCreator::on_listWidget_workouts_itemClicked(QListWidgetItem *item)
{
    this->load_selectedWorkout(item->data(Qt::UserRole).toString());
    ui->comboBox_code->setCurrentText(item->data(Qt::DisplayRole).toString().split(" - ").first());
    ui->lineEdit_workoutname->setText(item->data(Qt::DisplayRole).toString().split(" - ").at(1));
    ui->checkBox_timebased->setChecked(item->data(Qt::UserRole+1).toBool());

    /*
    ui->label_picName->setText(metaProxy->data(metaProxy->index(0,9)).toString());
    ui->pushButton_sync->setText(" - "+QString::number(proxyFilter->rowCount()));
    this->control_editPanel(false);
    */
}

void Dialog_workCreator::load_selectedWorkout(QString workID)
{
    selworkModel = stdWorkouts->get_selectedWorkout(workID);
    ui->treeWidget_workoutTree->clear();
    QTreeWidgetItem *rootItem = ui->treeWidget_workoutTree->invisibleRootItem();


    for(int row = 0; row < selworkModel->rowCount(); ++row)
    {
        if(selworkModel->item(row,0)->hasChildren())
        {
            this->read_selectedChild(selworkModel->item(row,0),this->set_selectedData(selworkModel->item(row,0),rootItem));
        }
        else
        {
            this->set_selectedData(selworkModel->item(row,0),rootItem);
        }
    }
    ui->treeWidget_workoutTree->expandAll();
}

void Dialog_workCreator::read_selectedChild(QStandardItem *parent,QTreeWidgetItem *treeItem)
{
    QStandardItem *childItem;

    if(parent->hasChildren())
    {
        for(int row = 0; row < parent->rowCount(); ++row)
        {
            childItem = parent->child(row,0);
            if(childItem->hasChildren())
            {
                this->read_selectedChild(childItem,this->set_selectedData(childItem,treeItem));
            }
            else
            {
                this->set_selectedData(childItem,treeItem);
            }
        }
    }
}

QTreeWidgetItem* Dialog_workCreator::set_selectedData(QStandardItem *item,QTreeWidgetItem *treeItem)
{
    QTreeWidgetItem *stepitem = new QTreeWidgetItem(treeItem);

    if(this->get_modelValue(item,"name") == isGroup || this->get_modelValue(item,"name") == isSeries)
    {
        stepitem->setData(0,Qt::DisplayRole,this->get_modelValue(item,"name"));
        stepitem->setData(9,Qt::DisplayRole,this->get_modelValue(item,"repeat"));
    }
    else
    {
        stepitem->setData(0,Qt::DisplayRole,this->get_modelValue(item,"name").toString());
        stepitem->setData(1,Qt::DisplayRole,this->get_modelValue(item,"level").toString());
        stepitem->setData(2,Qt::DisplayRole,this->get_modelValue(item,"threshold").toInt());
        stepitem->setData(3,Qt::DisplayRole,set_time(this->calc_thresPace(this->get_modelValue(item,"threshold").toDouble())));
        stepitem->setData(4,Qt::DisplayRole,this->calc_thresPower(this->get_modelValue(item,"threshold").toDouble()));
        stepitem->setData(5,Qt::DisplayRole,set_time(this->get_modelValue(item,"duration").toInt()));
        stepitem->setData(6,Qt::DisplayRole,this->calc_stressScore(this->get_modelValue(item,"threshold").toDouble(),this->get_modelValue(item,"duration").toInt()));
        stepitem->setData(7,Qt::DisplayRole,this->calc_totalWork(currentSport,thresValue,this->get_modelValue(item,"duration").toInt(),this->get_swimStyleID(this->get_modelValue(item,"name").toString())));
        stepitem->setData(8,Qt::DisplayRole,this->get_modelValue(item,"distance").toDouble());
    }
    return stepitem;
}

QVariant Dialog_workCreator::get_modelValue(QStandardItem *item, QString tag)
{
    return item->index().siblingAtColumn(xmlTagMap->value(item->data(Qt::UserRole).toString()).key(tag)).data(Qt::DisplayRole);
}

void Dialog_workCreator::set_sportData(QString sport)
{
    currentSport = sport;
    isSwim = isBike = isRun = isStrength = isAlt = isOther = isTria = false;

    if(currentSport == settings::isSwim) isSwim = true;
    if(currentSport == settings::isBike) isBike = true;
    if(currentSport == settings::isRun) isRun = true;
    if(currentSport == settings::isStrength) isStrength = true;
    if(currentSport == settings::isAlt) isAlt = true;
    if(currentSport == settings::isOther) isOther = true;
    if(currentSport == settings::isTria) isTria = true;

    if(isBike)
    {
        usePMData = static_cast<bool>(thresValues->value("bikepm"));
        thresPower = static_cast<int>(thresValues->value("bikepower"));
        thresPace = static_cast<int>(thresValues->value("bikepace"));
        thresSpeed = static_cast<int>(thresValues->value("bikespeed"));
        workFactor = thresValues->value("bikefactor");
        sportMark = " Watt";
    }
    else if(isRun)
    {
        usePMData = static_cast<bool>(thresValues->value("runpm"));
        thresPower = static_cast<int>(thresValues->value("runcp"));
        thresPace = static_cast<int>(thresValues->value("runpace"));
        workFactor = thresValues->value("bikefactor");
        sportMark = "/km";
    }
    else if(isSwim)
    {
        usePMData = static_cast<bool>(thresValues->value("swimpm"));
        thresPower = static_cast<int>(thresValues->value("swimpower"));
        thresPace = static_cast<int>(thresValues->value("swimpace"));
        workFactor = thresValues->value("bikefactor");
        sportMark = "/100m";
    }
    else if(isStrength)
    {
        usePMData = true;
        thresPower = static_cast<int>(thresValues->value("stgpower"));
        workFactor = 0;
        thresPace = 0;
        sportMark = " Watt";
    }
    else if(isAlt)
    {
        usePMData = true;
        thresPower = static_cast<int>(thresValues->value("runpower"));
        thresPace = 0;
        workFactor = 0;
        sportMark = " Watt";
    }
    else if(isTria)
    {
       usePMData = true;
       thresPower = static_cast<int>(thresValues->value("stgpower"));
       thresPace = 0;
       workFactor = 0;
       sportMark = " Watt";
    }
    else
    {
        usePMData = false;
        thresPower = 0;
        thresPace = 0;
        workFactor = 0;
        sportMark = "-";
    }

    if(usePMData)
    {
        thresValue = thresPower;
        ui->label_threshold->setText(QString::number(thresPower) + sportMark);
    }
    else
    {
        thresValue = thresPace;
        ui->label_threshold->setText(this->set_time(thresPace) + sportMark);
    }
}

int Dialog_workCreator::get_swimStyleID(QString partName)
{
    QStringList styles =  settings::get_listValues("SwimStyle");

    if(styles.indexOf(partName) == -1)
    {
        return styles.indexOf("Mixed");
    }
    else
    {
        return styles.indexOf(partName);
    }
}

QString Dialog_workCreator::get_workoutTime(double time)
{
    QString worktime;

    if(isAlt || isStrength)
    {
        worktime = this->set_time(static_cast<int>(time));
    }
    else
    {
        worktime = this->set_time(static_cast<int>(ceil(time/60.0)*60));
    }

    if(worktime.length() == 5)
    {
        worktime = "00:"+worktime;
    }

    return worktime;
}

void Dialog_workCreator::save_workout()
{
    int counter = 1;
    int workcounter;
    QString workID,sport,phase,subphase,currWorkID;
    QStringList workoutValues,existWorkIDs,sportWorkIDs;
    sport = ui->comboBox_sport->currentText();
    QTreeWidgetItem *currentItem;
    QStandardItemModel *workModel;
    this->set_metaFilter(sport,0,true);

    existWorkIDs = stdWorkouts->get_workoutIds();

    for(int i = 0; i < existWorkIDs.count(); ++i)
    {
        currWorkID = existWorkIDs.at(i);
        if(currWorkID.contains(sport))
        {
            sportWorkIDs << currWorkID;
        }
    }

    //Update Workout -> delete first
    if(currentWorkID.isEmpty())
    {
        workcounter = metaProxy->rowCount();
        workID = sport + "_" + QString::number(workcounter+1);

        for(int i = 0; i < sportWorkIDs.count(); ++i)
        {
             if(workID == sportWorkIDs.at(i))
             {
                 workcounter++;
                 workID = sport + "_" + QString::number(workcounter+1);
                 i = 0;
             }
        }
        currentWorkID = workID;
    }
    else
    {
        stdWorkouts->delete_stdWorkout(currentWorkID,false);
        workID = currentWorkID;
    }

    //Metadaten

    int roundNum = static_cast<int>(round(workSum)) % 10;

    if(roundNum <= 2)
    {
        workSum = round(workSum - roundNum);
    }
    else if(roundNum > 2 && roundNum < 8)
    {
        workSum = static_cast<int>(round(workSum) + 5-roundNum);
    }
    else if(roundNum >= 8)
    {
        workSum = round(workSum + (10-roundNum));
    }
    workModel = stdWorkouts->workouts_meta;

    workoutValues << currentSport
                  << workID
                  << ui->comboBox_code->currentText()
                  << ui->lineEdit_workoutname->text()
                  << get_workoutTime(timeSum)
                  << QString::number(distSum)
                  << QString::number(round(stressSum))
                  << QString::number(workSum)
                  << QString::number(ui->checkBox_timebased->isChecked())
                  << ui->label_picName->text();

     this->save_workout_values(workoutValues,workModel);

    //Intervalldaten
     workModel = stdWorkouts->workouts_steps;
     for(int c_item = 0; c_item < ui->treeWidget_workoutTree->topLevelItemCount(); ++c_item,++counter)
     {
         currentItem = ui->treeWidget_workoutTree->topLevelItem(c_item);

         phase = get_treeValue(c_item,0,0,0,0);

         workoutValues.clear();

         workoutValues << workID
                  << QString::number(counter)
                  << get_treeValue(c_item,0,0,0,0)   //part
                  << get_treeValue(c_item,0,0,1,0)   //level
                  << get_treeValue(c_item,0,0,2,0)   //threshold
                  << get_treeValue(c_item,0,0,5,0)   //time
                  << get_treeValue(c_item,0,0,8,0)   //dist
                  << get_treeValue(c_item,0,0,9,0)   //repeats
                  << "-";
         this->save_workout_values(workoutValues,workModel);

         if(currentItem->childCount() > 0)
         {
             for(int c_child = 0; c_child < currentItem->childCount(); ++c_child)
             {
                 ++counter;
                 workoutValues.clear();
                 workoutValues << workID
                          << QString::number(counter)
                          << get_treeValue(c_item,c_child,0,0,1)
                          << get_treeValue(c_item,c_child,0,1,1)
                          << get_treeValue(c_item,c_child,0,2,1)
                          << get_treeValue(c_item,c_child,0,5,1)
                          << get_treeValue(c_item,c_child,0,8,1)
                          << get_treeValue(c_item,c_child,0,9,1)
                          << phase;

                 this->save_workout_values(workoutValues,workModel);

                 if(currentItem->child(c_child)->childCount() > 0)
                 {
                     for(int subchild = 0; subchild < currentItem->child(c_child)->childCount(); ++subchild)
                     {
                         ++counter;
                         subphase = get_treeValue(c_item,c_child,0,0,1);
                         workoutValues.clear();
                         workoutValues << workID
                                  << QString::number(counter)
                                  << get_treeValue(c_item,c_child,subchild,0,2)
                                  << get_treeValue(c_item,c_child,subchild,1,2)
                                  << get_treeValue(c_item,c_child,subchild,2,2)
                                  << get_treeValue(c_item,c_child,subchild,5,2)
                                  << get_treeValue(c_item,c_child,subchild,8,2)
                                  << get_treeValue(c_item,c_child,subchild,9,2)
                                  << subphase;

                         this->save_workout_values(workoutValues,workModel);
                     }
                 }
             }
         }
     }
     this->get_workouts(currentSport);
     stdWorkouts->write_standard_workouts();
}

void Dialog_workCreator::save_workout_values(QStringList values, QStandardItemModel *model)
{
    int row = model->rowCount();
    model->insertRows(row,1,QModelIndex());

    for(int i = 0; i < values.count();++i)
    {
        model->setData(model->index(row,i,QModelIndex()),values.at(i));
    }
}

void Dialog_workCreator::on_treeWidget_workoutTree_itemChanged(QTreeWidgetItem *item, int column)
{
    /*
    Q_UNUSED(column)
    if(currentItem != item)
    {
        if(item->data(0,Qt::DisplayRole) == isGroup || item->data(0,Qt::DisplayRole) == isSeries)
        {
            this->set_defaultData(item,false);
        }
        else
        {
            this->set_defaultData(item,true);
        }
    }
    this->set_controlButtons(true);
    */
}

void Dialog_workCreator::set_defaultData(QTreeWidgetItem *item, bool hasValues)
{
    /*
    currentItem = item;
    QString defaultTime = "05:00";
    int usePM = 0;
    int level = 1;
    int tempID = 0;
    double pValue = 0;
    double defaultDist = 0.0;
    double percent = this->get_thresPercent(currentSport,levelList.at(level),false);
    QString thresValue = this->calc_thresPace(thresPace,percent);

    if(isSwim)
    {
        usePM = thresValues->value("swimpm");
        if(item->data(0,Qt::DisplayRole).toString() == isBreak)
        {
            defaultDist = percent = 0.0;
            thresValue = "00:00";
            defaultTime = "00:30";
            level = 0;
            tempID = 0;
            pValue = 0;
        }
        else
        {
            defaultDist = 0.1;
            defaultTime = this->calc_duration(currentSport,defaultDist,thresValue);
            tempID = 6;
            pValue = this->get_timesec(thresValue);
        }
    }
    else if (isBike)
    {
        usePM = thresValues->value("bikepm");
        pValue = calc_thresPower(thresPower,percent);
        defaultDist = this->calc_distance(defaultTime,3600.0/this->wattToSpeed(thresPower,pValue));
    }
    else if(isRun)
    {
        usePM = thresValues->value("runpm");
        defaultDist = this->calc_distance(defaultTime,static_cast<double>(this->get_timesec(threstopace(thresPace,percent))));
        pValue = get_speed(QTime::fromString(calc_thresPace(thresPace,percent),"mm:ss"),0,currentSport,true);
    }
    else if(isStrength || isAlt)
    {
        usePM = 1;
        pValue = percent / 10.0;
    }
    else if(isTria)
    {
        usePM = 1;
        pValue = percent / 10.0;
    }


    if(hasValues)
    {
        item->setData(1,Qt::EditRole,levelList.at(level));
        item->setData(2,Qt::EditRole,percent);
        item->setData(3,Qt::EditRole,thresValue);
        item->setData(4,Qt::EditRole,QString::number(round(thresPower* (percent/100.0))));
        item->setData(5,Qt::EditRole,defaultTime);
        item->setData(6,Qt::EditRole,this->estimate_stress(currentSport,thresValue,,usePM));
        item->setData(7,Qt::EditRole,this->set_doubleValue(this->calc_totalWork(currentSport,pValue,this->get_timesec(defaultTime),tempID),false));
        item->setData(8,Qt::EditRole,defaultDist);
        item->setData(9,Qt::EditRole,"");
    }
    else
    {
        QString itemName = item->data(0,Qt::DisplayRole).toString();
        QList<QTreeWidgetItem*> groupCount = ui->treeWidget_workoutTree->findItems(itemName,Qt::MatchRecursive | Qt::MatchContains,0);
        itemName = itemName +"-"+QString::number(groupCount.count());
        item->setData(0,Qt::EditRole,itemName);
        item->setData(9,Qt::EditRole,2);
    }
    ui->treeWidget_workoutTree->expandAll();
    ui->treeWidget_workoutTree->setTreePosition(-1);
    this->set_plotModel();
    */
}

void Dialog_workCreator::edit_selectedStep(QTreeWidgetItem *item)
{
    QList<QListWidgetItem*> itemList;

    currentItem = item;
    QString stepName = item->data(0,Qt::DisplayRole).toString();

    if(stepName == isSeries || stepName == isGroup)
    {
        ui->frame_partEdit->setVisible(true);
        ui->frame_stepEdit->setVisible(false);
        ui->comboBox_partName->setCurrentText(item->data(0,Qt::DisplayRole).toString());
        ui->spinBox_repeats->setValue(item->data(9,Qt::DisplayRole).toInt());
    }
    else
    {
        ui->frame_partEdit->setVisible(false);
        ui->frame_stepEdit->setVisible(true);
        ui->comboBox_stepName->setCurrentText(item->data(0,Qt::DisplayRole).toString());
        ui->comboBox_levelName->setCurrentText(item->data(1,Qt::DisplayRole).toString());
        ui->spinBox_level->setValue(item->data(2,Qt::DisplayRole).toInt());
        ui->timeEdit_pace->setTime(QTime::fromString(item->data(3,Qt::DisplayRole).toString(),"mm:ss"));
        ui->lineEdit_power->setText(item->data(4,Qt::DisplayRole).toString());
        ui->timeEdit_lapTime->setTime(QTime::fromString(item->data(5,Qt::DisplayRole).toString(),"mm:ss"));
        ui->doubleSpinBox_stressScore->setValue(item->data(6,Qt::DisplayRole).toDouble());
        ui->doubleSpinBox_work->setValue(item->data(7,Qt::DisplayRole).toDouble());
        ui->doubleSpinBox_distance->setValue(item->data(8,Qt::DisplayRole).toDouble());
        ui->lineEdit_speed->setText("-");
    }

    this->control_editPanel(true);
}

void Dialog_workCreator::refresh_editStep()
{
    ui->doubleSpinBox_stressScore->setValue(this->calc_stressScore(ui->spinBox_level->value(),this->get_timesec(ui->timeEdit_lapTime->time().toString("mm:ss"))));
    ui->doubleSpinBox_work->setValue(this->calc_totalWork(currentSport,thresValue,this->get_timesec(ui->timeEdit_lapTime->time().toString("mm:ss")),this->get_swimStyleID(ui->comboBox_stepName->currentText())));
    ui->lineEdit_speed->setText(QString::number(this->get_speed(ui->timeEdit_pace->time(),ui->doubleSpinBox_distance->value(),currentSport,true))+" km/h");
    //ui->timeEdit_lapTime->setTime(this->calc_duration())
}

void Dialog_workCreator::set_itemData(QTreeWidgetItem *item)
{
    QString itemIdent = item->data(0,Qt::DisplayRole).toString();

    if(itemIdent.contains(isGroup) || itemIdent.contains(isSeries))
    {
        item->setData(0,Qt::EditRole,valueModel->data(valueModel->index(0,0)));
        item->setData(9,Qt::EditRole,valueModel->data(valueModel->index(1,0)));
    }
    else
    {
        for(int i = 0; i < valueModel->rowCount()-1;++i)
        {
            item->setData(i,Qt::EditRole,valueModel->data(valueModel->index(i,0)));
        }
        item->setData(9,Qt::EditRole,"");
    }
}

void Dialog_workCreator::clearIntTree()
{
    clearFlag = true;
    QTreeWidgetItem *treeItem;
    while(ui->treeWidget_workoutTree->topLevelItemCount() > 0)
    {
        for(int c_item = 0; c_item < ui->treeWidget_workoutTree->topLevelItemCount(); ++c_item)
        {
            treeItem = ui->treeWidget_workoutTree->topLevelItem(c_item);

            if(treeItem->childCount() > 0)
            {
                for(int c_child = 0; c_child < treeItem->childCount(); ++c_child)
                {
                    if(treeItem->child(c_child)->childCount() > 0)
                    {
                        for(int subchild = 0; subchild < treeItem->child(c_child)->childCount(); ++subchild)
                        {
                            delete treeItem->child(c_child)->child(subchild);
                        }
                    }
                    delete treeItem->child(c_child);
                }
            }
            delete treeItem;
        }
    }
    plotModel->clear();
    this->set_plotGraphic(0);

    currentWorkID = QString();
    ui->lineEdit_workoutname->clear();
    ui->comboBox_code->setCurrentIndex(0);
    ui->checkBox_timebased->setChecked(false);
    this->set_controlButtons(false);
    this->control_editPanel(false);
    ui->listWidget_workouts->clearSelection();
    ui->listWidget_group->clearSelection();
    ui->listWidget_phases->clearSelection();
    ui->pushButton_sync->setText(" - 0");
    this->resetAxis();
}

void Dialog_workCreator::set_plotModel()
{
    plotModel->clear();
    plotModel->setColumnCount(7);
    QTreeWidget *intTree = ui->treeWidget_workoutTree;
    int groupReps = 0;
    int seriesReps = 0;
    int childCount = 0;
    int subchildCount = 0;
    int currIndex = 0;
    bool isload = false;
    QString phase,subphase;
    QTreeWidgetItem *topItem,*childItem,*subItem;

    for(int c_item = 0; c_item < ui->treeWidget_workoutTree->topLevelItemCount(); ++c_item)
    {
        //TopItems
        topItem = intTree->topLevelItem(c_item);
        phase = topItem->data(0,Qt::DisplayRole).toString();
        childCount = topItem->childCount();
        intTree->setCurrentItem(topItem);
        currIndex = intTree->currentIndex().row();

        if(childCount > 0)
        {
            if(phase.contains(isGroup))
            {
                groupReps = topItem->data(9,Qt::DisplayRole).toInt();

                for(int repeat = 0; repeat < groupReps; ++repeat)
                {
                    for(int groupChild = 0; groupChild < childCount ; ++groupChild)
                    {
                        childItem = topItem->child(groupChild);
                        subchildCount = childItem->childCount();
                        subphase = childItem->data(0,Qt::DisplayRole).toString();
                        intTree->setCurrentItem(childItem);
                        currIndex = intTree->currentIndex().row();

                        if(subchildCount > 0)
                        {
                            if(subphase.contains(isSeries))
                            {
                                seriesReps = childItem->data(9,Qt::DisplayRole).toInt();
                                for(int repeat = 0; repeat < seriesReps; ++repeat)
                                {
                                    for(int subChild = 0; subChild < subchildCount; ++subChild)
                                    {
                                        subItem = childItem->child(subChild);
                                        intTree->setCurrentItem(subItem);
                                        currIndex = intTree->currentIndex().row();
                                        this->add_to_plot(subItem,currIndex);
                                    }
                                }
                            }
                        }
                        else
                        {
                            this->add_to_plot(childItem,currIndex);
                        }
                    }
                }
            }
            else if(phase.contains(isSeries))
            {
                seriesReps = topItem->data(9,Qt::DisplayRole).toInt();

                for(int repeat = 0; repeat < seriesReps; ++repeat)
                {
                    for(int seriesChild = 0; seriesChild < childCount ; ++seriesChild)
                    {
                        childItem = topItem->child(seriesChild);
                        intTree->setCurrentItem(childItem);
                        currIndex = intTree->currentIndex().row();
                        this->add_to_plot(childItem,currIndex);
                    }
                }
            }
        }
        else
        {
            //Items without connection
            if(!phase.contains(isSeries) && !phase.contains(isGroup))
            {
                this->add_to_plot(topItem,currIndex);
            }
        }
        if(c_item == intTree->topLevelItemCount()-1) isload = true;
    }
    if(isload) this->set_plotGraphic(plotModel->rowCount());
}

void Dialog_workCreator::add_to_plot(QTreeWidgetItem *item,int currIndex)
{
    double timeSum = 0;
    double distSum = 0;
    double stressSum = 0.0;
    double workSum = 0;
    int row = plotModel->rowCount();

    if(row != 0)
    {
        timeSum = plotModel->data(plotModel->index(row-1,1,QModelIndex())).toDouble();
        distSum = plotModel->data(plotModel->index(row-1,2,QModelIndex())).toDouble();
        stressSum = plotModel->data(plotModel->index(row-1,3,QModelIndex())).toDouble();
        workSum = plotModel->data(plotModel->index(row-1,4,QModelIndex())).toDouble();
    }

    plotModel->insertRows(row,2,QModelIndex());
    plotModel->setData(plotModel->index(row,0,QModelIndex()),item->data(2,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row,1,QModelIndex()),timeSum);
    plotModel->setData(plotModel->index(row,2,QModelIndex()),distSum);
    plotModel->setData(plotModel->index(row,3,QModelIndex()),0);
    plotModel->setData(plotModel->index(row,4,QModelIndex()),workSum);

    plotModel->setData(plotModel->index(row+1,0,QModelIndex()),item->data(2,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row+1,1,QModelIndex()),timeSum + (this->get_timesec(item->data(5,Qt::DisplayRole).toString())));
    plotModel->setData(plotModel->index(row+1,2,QModelIndex()),distSum + item->data(8,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row+1,3,QModelIndex()),stressSum + item->data(6,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row+1,4,QModelIndex()),workSum + item->data(7,Qt::DisplayRole).toDouble());

    if(item->parent() == nullptr)
    {
        plotModel->setData(plotModel->index(row,5,QModelIndex()),item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
        plotModel->setData(plotModel->index(row+1,5,QModelIndex()),item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
    }
    else
    {
        plotModel->setData(plotModel->index(row,5,QModelIndex()),item->parent()->data(0,Qt::DisplayRole).toString()+"#"+item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
        plotModel->setData(plotModel->index(row+1,5,QModelIndex()),item->parent()->data(0,Qt::DisplayRole).toString()+"#"+item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex));
    }
    plotModel->setData(plotModel->index(row,6,QModelIndex()),row);
    plotModel->setData(plotModel->index(row+1,6,QModelIndex()),row+1);
}

void Dialog_workCreator::set_plotGraphic(int dataPoints)
{
    ui->widget_plot->clearPlottables();
    QVector<double> x_time(dataPoints),x_dist(dataPoints),y_thres(dataPoints);
    timeSum = 0.0;
    distSum = 0.0;
    stressSum = 0.0;
    workSum = 0.0;
    double thres_high = 0.0;
    double timeRange = 0;

    ui->widget_plot->setInteractions(QCP::iSelectPlottables | QCP::iMultiSelect);
    QCPSelectionDecorator *lineDec = new QCPSelectionDecorator();
    lineDec->setPen(QPen(QColor(255,0,0)));
    lineDec->setBrush(QColor(255,0,0,50));
    QCPGraph *workout_line = ui->widget_plot->addGraph();
    workout_line->setSelectionDecorator(lineDec);

    if(dataPoints != 0)
    {
        workout_line->setPen(QPen(Qt::blue));
        workout_line->setBrush(QColor(0,170,255,60));
        workout_line->setLineStyle((QCPGraph::lsLine));
        workout_line->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QPen(Qt::red),QBrush(Qt::white), 3));

        for (int data=0 ;  data < dataPoints; ++data)
        {
             y_thres[data] = plotModel->data(plotModel->index(data,0,QModelIndex())).toDouble();
             x_time[data] = plotModel->data(plotModel->index(data,1,QModelIndex())).toDouble()/60;
             x_dist[data] = plotModel->data(plotModel->index(data,2,QModelIndex())).toDouble();
             if(y_thres[data] > thres_high)
             {
                 thres_high = y_thres[data];
             }
        }

        timeSum = plotModel->data(plotModel->index(dataPoints-1,1,QModelIndex())).toDouble();
        distSum = plotModel->data(plotModel->index(dataPoints-1,2,QModelIndex())).toDouble();
        stressSum = plotModel->data(plotModel->index(dataPoints-1,3,QModelIndex())).toDouble();
        workSum = plotModel->data(plotModel->index(dataPoints-1,4,QModelIndex())).toDouble();

        workout_line->setData(x_time,y_thres,true);
        timeRange = timeSum/60;
        ui->widget_plot->xAxis->setRange(0,timeRange+(timeRange*0.015));
        ui->widget_plot->xAxis2->setRange(0,distSum+(distSum*0.015));
        ui->widget_plot->yAxis->setRange(0,thres_high+20.0);
        ui->widget_plot->yAxis2->setRange(0,thres_high+20.0);
    }

    if(!isSwim)
    {
        distSum = this->set_doubleValue(ceil(distSum*10.0)/10,false);
    }

    ui->widget_plot->replot();
    ui->label_duration->setText("Time: " + this->set_time(static_cast<int>(timeSum)) + " - " + "Distance: " + QString::number(distSum) + " - " + "Stress: " + QString::number(round(stressSum))+ " - " + "Work: "+QString::number(round(workSum)));
}

void Dialog_workCreator::set_selectData(QTreeWidgetItem *item)
{
    QCPGraph *graph = ui->widget_plot->graph(0);
    int currIndex = ui->treeWidget_workoutTree->currentIndex().row();
    QString itemIdent;
    int dataCount = 0;
    bool groupRange = false;

    QSortFilterProxyModel *plotProxy = new QSortFilterProxyModel;
    plotProxy->setSourceModel(plotModel);

    if(item->parent() == nullptr)
    {
        graph->setSelectable(QCP::stDataRange);
        itemIdent = item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex);

        if(itemIdent.contains(isSeries))
        {
            itemIdent = itemIdent.split(":").first();
        }
        else if(itemIdent.contains(isGroup))
        {
            groupRange = true;
            int childCount,groupCount;
            dataCount = groupCount = item->data(9,Qt::DisplayRole).toInt()*2;
            childCount = item->childCount();
            if(childCount > 0)
            {
                itemIdent = item->child(0)->data(0,Qt::DisplayRole).toString();
                for(int i = 0; i < childCount; ++i)
                {
                    if(item->child(i)->childCount() > 0)
                    {
                        int subCount = item->child(i)->childCount();
                        dataCount = dataCount*(subCount*item->child(i)->data(9,Qt::DisplayRole).toInt());
                    }
                    else
                    {
                        dataCount += groupCount;
                    }
                }
            }
        }
    }
    else
    {
        graph->setSelectable(QCP::stMultipleDataRanges);
        itemIdent = item->parent()->data(0,Qt::DisplayRole).toString()+"#"+item->data(0,Qt::DisplayRole).toString()+":"+QString::number(currIndex);
        if(item->data(0,Qt::DisplayRole).toString().contains(isSeries))
        {
            itemIdent = itemIdent.split("#").last();
            itemIdent = itemIdent.split(":").first();
        }     
    }
    plotProxy->setFilterRegExp("^"+itemIdent);
    plotProxy->setFilterKeyColumn(5);
    plotProxy->sort(6);

    QCPDataSelection selection;

    if(graph->selectable() == QCP::stDataRange)
    {
        if(groupRange)
        {
            int dataStart = plotProxy->data(plotProxy->index(0,6)).toInt();
            selection.addDataRange(QCPDataRange(dataStart,dataStart+dataCount));
        }
        else
        {
            selection.addDataRange(QCPDataRange(plotProxy->data(plotProxy->index(0,6)).toInt(),plotProxy->data(plotProxy->index(plotProxy->rowCount()-1,6)).toInt()+1));
        }
    }

    if(graph->selectable() == QCP::stMultipleDataRanges)
    {
        for(int i = 0; i < plotProxy->rowCount(); ++i)
        {
            selection.addDataRange(QCPDataRange(plotProxy->data(plotProxy->index(i,6)).toInt(),plotProxy->data(plotProxy->index(i,6)).toInt()+1),false);
        }
    }

    delete plotProxy;
    graph->setSelection(selection);
    ui->widget_plot->replot();
}

void Dialog_workCreator::resetAxis()
{
    ui->widget_plot->xAxis->setRange(0,10.0);
    ui->widget_plot->xAxis->rescale(true);
    ui->widget_plot->yAxis->setRange(0,50.0);
    ui->widget_plot->xAxis2->setRange(0,5.0);
    ui->widget_plot->yAxis2->setRange(0,50.0);
    //ui->widget_plot->graph()->rescaleAxes(true);
}

void Dialog_workCreator::on_treeWidget_workoutTree_itemClicked(QTreeWidgetItem *item, int column)
{
    //ui->widget_plot->graph(0)->setSelection(QCPDataSelection(QCPDataRange(0,0)));
    this->edit_selectedStep(item);
    //this->set_selectData(item);

}

void Dialog_workCreator::on_comboBox_sport_currentTextChanged(const QString &sport)
{
    this->get_workouts(sport);
}

void Dialog_workCreator::on_pushButton_clear_clicked()
{
    this->clearIntTree();
    this->set_controlButtons(false);
}

void Dialog_workCreator::on_toolButton_update_clicked()
{
    this->control_editPanel(false);
    this->set_itemData(currentItem);
    this->set_plotModel();
    ui->treeWidget_workoutTree->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_toolButton_cancel_clicked()
{
    this->control_editPanel(false);
    this->set_plotModel();
    ui->treeWidget_workoutTree->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_toolButton_remove_clicked()
{
    QTreeWidgetItem *deleteItem = currentItem;

    if(ui->treeWidget_workoutTree->topLevelItemCount() == 1)
    {
        this->clearIntTree();
    }
    else
    {
        if(deleteItem->childCount() > 0)
        {
            while(deleteItem->childCount() > 0)
            {
                for(int i = 0; i < deleteItem->childCount(); ++i)
                {
                    delete deleteItem->child(i);
                }
            }
        }
        delete deleteItem;
        currentItem = ui->treeWidget_workoutTree->currentItem();
        ui->treeWidget_workoutTree->setCurrentItem(currentItem);
    }
    this->set_plotModel();
}

QTreeWidgetItem* Dialog_workCreator::move_item(bool up, QTreeWidgetItem *currentItem)
{
    int currentindex = ui->treeWidget_workoutTree->currentIndex().row();

    if(currentItem->parent())
    {
        QTreeWidgetItem *parent = currentItem->parent();
        int index = parent->indexOfChild(currentItem);
        QTreeWidgetItem *childItem = parent->takeChild(index);
        if(up)
        {
            parent->insertChild(index-1,childItem);
            parent->setExpanded(true);
        }
        else
        {
            parent->insertChild(index+1,childItem);
            parent->setExpanded(true);
        }
        return childItem;
    }
    else
    {
        QTreeWidgetItem *topItem = ui->treeWidget_workoutTree->takeTopLevelItem(currentindex);
        if(up)
        {
            ui->treeWidget_workoutTree->insertTopLevelItem(currentindex-1,topItem);
        }
        else
        {
            ui->treeWidget_workoutTree->insertTopLevelItem(currentindex+1,topItem);
        }
        return topItem;
    }
}

void Dialog_workCreator::on_toolButton_up_clicked()
{
    QTreeWidgetItem *currentItem = this->move_item(true,ui->treeWidget_workoutTree->currentItem());
    this->set_plotModel();
    ui->treeWidget_workoutTree->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_toolButton_down_clicked()
{
    QTreeWidgetItem *currentItem = this->move_item(false,ui->treeWidget_workoutTree->currentItem());
    this->set_plotModel();
    ui->treeWidget_workoutTree->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_treeWidget_workoutTree_itemSelectionChanged()
{
    if(!clearFlag)
    {
        QTreeWidgetItem *current = ui->treeWidget_workoutTree->currentItem();
        int row = ui->treeWidget_workoutTree->currentIndex().row();
        int rowCount = 0;

        if(current->parent())
        {
            rowCount = ui->treeWidget_workoutTree->currentItem()->parent()->childCount();
        }
        else
        {
            rowCount = ui->treeWidget_workoutTree->topLevelItemCount();
        }

        if(row == 0 && rowCount > 1)
        {
            ui->toolButton_down->setEnabled(true);
            ui->toolButton_up->setEnabled(false);
        }
        else if(row > 0 && row < rowCount && row != rowCount-1)
        {
            ui->toolButton_down->setEnabled(true);
            ui->toolButton_up->setEnabled(true);
        }
        else if(row > 0 && row == rowCount-1)
        {
            ui->toolButton_down->setEnabled(false);
            ui->toolButton_up->setEnabled(true);
        }
        else if(row == 0 && row == rowCount-1)
        {
            ui->toolButton_down->setEnabled(false);
            ui->toolButton_up->setEnabled(false);
        }
    }
}

QString Dialog_workCreator::get_treeValue(int item,int i_child, int c_sub,int pos,int level)
{
    if(level == 0)
    {
        return ui->treeWidget_workoutTree->topLevelItem(item)->data(pos,Qt::DisplayRole).toString();
    }
    if(level == 1)
    {
        return ui->treeWidget_workoutTree->topLevelItem(item)->child(i_child)->data(pos,Qt::DisplayRole).toString();
    }
    if(level == 2)
    {
        return ui->treeWidget_workoutTree->topLevelItem(item)->child(i_child)->child(c_sub)->data(pos,Qt::DisplayRole).toString();
    }

    return nullptr;
}

void Dialog_workCreator::control_editPanel(bool setedit)
{
    if(setedit)
    {
        ui->frame_edit->setVisible(true);
        ui->frame_pick->setVisible(false);
        ui->label_head->setText("Edit Phase");
    }
    else
    {
        ui->frame_edit->setVisible(false);
        ui->frame_pick->setVisible(true);
        ui->label_head->setText("Add Phase");
    }
}

void Dialog_workCreator::on_toolButton_save_clicked()
{

    this->save_workout();

    ui->toolButton_delete->setEnabled(true);
    ui->toolButton_save->setEnabled(false);
}

void Dialog_workCreator::on_toolButton_copy_clicked()
{
    currentWorkID = QString();
    this->save_workout();
    ui->toolButton_copy->setEnabled(false);
}

void Dialog_workCreator::on_toolButton_delete_clicked()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::critical(this,"Delete Std Workout","Delete selected Std Workout?",QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        stdWorkouts->delete_stdWorkout(currentWorkID,true);
        currentWorkID = QString();
        this->clearIntTree();
        this->get_workouts(currentSport);
    }
}

void Dialog_workCreator::on_lineEdit_workoutname_textChanged(const QString &value)
{
    if(!value.isEmpty() && ui->treeWidget_workoutTree->topLevelItemCount() > 0)
    {
        ui->toolButton_save->setEnabled(true);
    }
    /*
    for(QMap<QString,QString>::const_iterator it =  workoutMap.cbegin(), end = workoutMap.cend(); it != end; ++it)
    {
        if(it.value() == value)
        {
            ui->toolButton_copy->setEnabled(false);
            break;
        }
        else
        {
            if(value.isEmpty())
            {
                ui->toolButton_copy->setEnabled(false);
            }
            else
            {
                ui->toolButton_copy->setEnabled(true);
            }
        }
    }
    */
}

void Dialog_workCreator::on_toolButton_close_clicked()
{
    done(worksched->get_isUpdated());
}

void Dialog_workCreator::set_workoutModel(QDate cDate)
{
    Q_UNUSED(cDate)
    QDate workDate;
    workoutModel->clear();
    workoutModel->setColumnCount(2);
    bool dateRange = updateRange->isChecked();
    int x = 0;

    for(int i = 0; i < proxyFilter->rowCount(); ++i)
    {
        workDate = QDate::fromString(proxyFilter->data(proxyFilter->index(i,1)).toString(),dateFormat);

        if(workDate.operator >(QDate::currentDate()))
        {
            if(dateRange)
            {
                if(workDate.operator >=(updateFrom->date()) && workDate.operator <=(updateTo->date()))
                {
                    workoutModel->insertRow(workoutModel->rowCount());
                    workoutModel->setData(workoutModel->index(x,0),proxyFilter->data(proxyFilter->index(i,1)).toString()+"-"+
                                                                   proxyFilter->data(proxyFilter->index(i,4)).toString());
                    workoutModel->setData(workoutModel->index(x,1),workDate);
                    ++x;
                }
            }
            else
            {
                workoutModel->insertRow(x);
                workoutModel->setData(workoutModel->index(x,0),proxyFilter->data(proxyFilter->index(i,1)).toString()+"-"+
                                                               proxyFilter->data(proxyFilter->index(i,4)).toString());
                workoutModel->setData(workoutModel->index(x,1),workDate);
                ++x;
            }
        }
    }
    workoutModel->sort(1);
    updateProgess->setMaximum(x);
}

void Dialog_workCreator::on_pushButton_sync_clicked()
{
    this->set_workoutModel(QDate());
    QDate timeRange = workoutModel->data(workoutModel->index(workoutModel->rowCount()-1,1)).toDate();
    updateFrom->setDate(QDate::currentDate());
    updateFrom->setDateRange(QDate::currentDate(),timeRange);
    updateTo->setDate(timeRange);
    updateTo->setDateRange(QDate::currentDate(),timeRange);

    updateDialog->exec();
}

void Dialog_workCreator::update_workouts()
{
    QPair<double,double> stressMap;

    QDate wDate;
    double stressValue = 0;
    double duraValue = 0;
    int progress = 0;

    for(int i = 0; i < proxyFilter->rowCount(); ++i)
    {
        wDate = QDate::fromString(proxyFilter->data(proxyFilter->index(i,1)).toString(),dateFormat);
        if(wDate.operator >(QDate::currentDate()))
        {
            stressValue = proxyFilter->data(proxyFilter->index(i,8)).toDouble();
            duraValue = proxyFilter->data(proxyFilter->index(i,7)).toDouble();
            stressValue = (worksched->stressValues.value(wDate).first - stressValue)+round(stressSum);
            duraValue = (worksched->stressValues.value(wDate).second - duraValue)+distSum;
            stressMap.first = stressValue;
            stressMap.second = duraValue;

            if(updateAll->isChecked())
            {
                this->update_workoutsSchedule(i,wDate,stressMap,++progress);
            }
            else
            {
                if(wDate.operator >=(updateFrom->date()) && wDate.operator <=(updateTo->date()))
                {
                    this->update_workoutsSchedule(i,wDate,stressMap,++progress);
                }
            }

        }
    }
    worksched->set_isUpdated(true);
    QTimer::singleShot(2000,updateProgess,SLOT(reset()));
}


void Dialog_workCreator::update_workoutsSchedule(int index,QDate wDate, QPair<double, double> stressMap,int progress)
{
    worksched->stressValues.insert(wDate,qMakePair(stressMap.first,stressMap.second));
    proxyFilter->setData(proxyFilter->index(index,4),ui->comboBox_code->currentText());        //code
    proxyFilter->setData(proxyFilter->index(index,5),ui->lineEdit_workoutname->text());        //title
    proxyFilter->setData(proxyFilter->index(index,6),get_workoutTime(timeSum));                //duration
    proxyFilter->setData(proxyFilter->index(index,7),QString::number(distSum));                //distance
    proxyFilter->setData(proxyFilter->index(index,8),QString::number(round(stressSum)));       //stress
    proxyFilter->setData(proxyFilter->index(index,9),QString::number(round(workSum)));                 //kj
    updateProgess->setValue(progress);
}

void Dialog_workCreator::set_metaFilter(QString filterTo, int col, bool fixString)
{
    metaProxy->invalidate();

    if(fixString)
    {
        metaProxy->setFilterFixedString(filterTo);
    }
    else
    {
        metaProxy->setFilterRegExp("\\b"+filterTo+"\\b");
    }

    metaProxy->setFilterKeyColumn(col);
}


void Dialog_workCreator::set_updateDates(bool pAll)
{
    updateTo->setEnabled(!pAll);
    updateFrom->setEnabled(!pAll);
    this->set_workoutModel(QDate());
}

void Dialog_workCreator::on_checkBox_timebased_clicked(bool checked)
{
    if(checked)
    {
        editRow[4] = 1;
    }
}

void Dialog_workCreator::on_toolButton_map_clicked()
{
    this->set_metaFilter(currentWorkID,1,false);
    QString image = metaProxy->data(metaProxy->index(0,9)).toString();

    Dialog_map dialogMap(this,metaProxy,currentWorkID,image);
    dialogMap.setModal(true);
    dialogMap.exec();
    ui->label_picName->setText(metaProxy->data(metaProxy->index(0,9)).toString());
}

void Dialog_workCreator::on_spinBox_level_valueChanged(int value)
{
    ui->timeEdit_pace->setTime(QTime::fromString(set_time(this->calc_thresPace(value)),"mm:ss"));
    this->refresh_editStep();
}

void Dialog_workCreator::on_comboBox_levelName_currentTextChanged(const QString &level)
{
    ui->spinBox_level->setMinimum(this->get_thresPercent(currentSport,level,false));
    ui->spinBox_level->setMaximum(this->get_thresPercent(currentSport,level,true));
    this->refresh_editStep();
}
