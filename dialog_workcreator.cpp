#include "dialog_workcreator.h"
#include "ui_dialog_workcreator.h"

Dialog_workCreator::Dialog_workCreator(QWidget *parent, schedule *psched) :
    QDialog(parent),
    ui(new Ui::Dialog_workCreator)
{
    ui->setupUi(this);
    this->setStyleSheet(parent->styleSheet());
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    listModel = new QStandardItemModel(this);
    plotModel = new QStandardItemModel(this);
    valueModel = new QStandardItemModel(this);
    worksched = psched;
    metaProxy = new QSortFilterProxyModel(this);
    metaProxy->setSourceModel(this->workouts_meta);
    stepProxy = new QSortFilterProxyModel(this);
    stepProxy->setSourceModel(this->workouts_steps);
    schedProxy = new QSortFilterProxyModel(this);
    schedProxy->setSourceModel(worksched->workout_schedule);
    proxyFilter = new QSortFilterProxyModel(this);
    proxyFilter->setSourceModel(schedProxy);

    editRow <<1<<1<<1<<0<<0<<1<<0<<0<<0<<0;

    isSeries = "Series";
    isGroup = "Group";
    isBreak = generalValues->value("breakname");
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
    ui->treeWidget_intervall->setHeaderLabels(modelHeader);
    ui->treeWidget_intervall->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidget_intervall->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->treeWidget_intervall->setAcceptDrops(true);
    ui->treeWidget_intervall->setDragEnabled(true);
    ui->treeWidget_intervall->setDragDropMode(QAbstractItemView::DragDrop);
    ui->treeWidget_intervall->setEnabled(true);
    ui->treeWidget_intervall->setItemDelegate(&workTree_del);
    workTree_del.groupList = groupList;

    ui->frame_edit->setVisible(false);
    ui->listView_values->setModel(valueModel);
    ui->listView_values->setItemDelegate(&edit_del);
    ui->label_head->setText("Add Phase");

    viewBackground = "background-color: #e6e6e6";

    ui->toolButton_copy->setEnabled(false);
    this->set_controlButtons(false);

    ui->treeWidget_intervall->setStyleSheet(viewBackground);
    ui->listWidget_group->setStyleSheet(viewBackground);
    ui->listWidget_phases->setStyleSheet(viewBackground);
    ui->listView_values->setStyleSheet(viewBackground);
    ui->widget_plot->setStyleSheet(viewBackground);

    ui->listView_workouts->setModel(listModel);
    ui->listView_workouts->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->listView_workouts->setItemDelegate(&mousehover_del);

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
    delete workouts_meta;
    delete workouts_steps;
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
    QString workID,workTitle,listString;
    bool timeBased;
    workoutMap.clear();
    listModel->clear();
    metaProxy->invalidate();
    metaProxy->setFilterFixedString(sport);
    metaProxy->setFilterKeyColumn(0);

    schedProxy->invalidate();
    schedProxy->setFilterFixedString(sport);
    schedProxy->setFilterKeyColumn(3);

    listModel->setColumnCount(3);
    listModel->setRowCount(metaProxy->rowCount());

    for(int i = 0; i < metaProxy->rowCount(); ++i)
    {
        workID = metaProxy->data(metaProxy->index(i,1)).toString();
        workTitle = metaProxy->data(metaProxy->index(i,3)).toString();
        listString = metaProxy->data(metaProxy->index(i,2)).toString() + " - " + workTitle;
        timeBased = metaProxy->data(metaProxy->index(i,8)).toBool();
        listModel->setData(listModel->index(i,0,QModelIndex()),listString);
        listModel->setData(listModel->index(i,1,QModelIndex()),workID);
        listModel->setData(listModel->index(i,2,QModelIndex()),timeBased);
        workoutMap.insert(workID,workTitle);
    }
    listModel->sort(0);

    ui->label_workouts->setText("Workouts: "+QString::number(metaProxy->rowCount()));

    QColor sportColor = settings::get_itemColor(sport);
    QString sportBack = "rgb("+QString::number(sportColor.red())+","+QString::number(sportColor.green())+","+QString::number(sportColor.blue())+",35%)";
    QString actBackground = "background: "+sportBack;
    ui->listView_workouts->setStyleSheet(actBackground);
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

void Dialog_workCreator::open_stdWorkout(QString workID)
{
    clearFlag = true;
    ui->treeWidget_intervall->clearFocus();
    if(ui->treeWidget_intervall->topLevelItemCount() > 0) ui->treeWidget_intervall->clear();

    QStringList valueList;
    QString parentItem,partName,thresValue,stepTime,empty = "";
    int tempID = 0;
    double percent = 0;
    double currDist = 0;
    double pValue = 0;
    double workFactor = 1.0;

    int usePM = 0;

    bool timeBase = ui->checkBox_timebased->isChecked();
    edit_del.timeBased = timeBase;
    stepProxy->setFilterRegExp("\\b"+workID+"\\b");
    stepProxy->setFilterKeyColumn(0);

    for(int i = 0; i < stepProxy->rowCount();++i)
    {
        valueList.clear();
        partName = stepProxy->data(stepProxy->index(i,2)).toString();

        if(partName.contains(isGroup) || partName.contains(isSeries))
        {
            valueList << stepProxy->data(stepProxy->index(i,2)).toString()
                      << empty
                      << empty
                      << empty
                      << empty
                      << empty
                      << empty
                      << empty
                      << stepProxy->data(stepProxy->index(i,6)).toString()
                      << stepProxy->data(stepProxy->index(i,7)).toString();
        }
        else
        {
            percent = stepProxy->data(stepProxy->index(i,4)).toDouble();         
            stepTime = stepProxy->data(stepProxy->index(i,5)).toString();

            if(isBike)
            {
                usePM = thresValues->value("bikepm");
                pValue = this->calc_thresPower(thresPower,percent);
                thresValue = QString::number(this->calc_thresPower(thresPower,percent));

                if(timeBase)
                {
                    currDist = this->calc_distance(stepTime,3600.0/this->wattToSpeed(thresPower,thresSpeed,pValue));
                }
                else
                {
                    currDist = stepProxy->data(stepProxy->index(i,6)).toDouble();
                    stepTime = this->calc_duration(currentSport,currDist,this->calc_thresPace(thresPace,percent));
                }
            }
            else if(isRun)
            {
                usePM = thresValues->value("runpm");
                thresValue = this->calc_thresPace(thresPace,percent);
                pValue = 3600.0 / this->get_timesec(thresValue);
                currDist = this->calc_distance(stepTime,this->get_timesec(thresValue));
            }
            else if(isSwim)
            {
                usePM = thresValues->value("swimpm");
                thresValue = this->calc_thresPace(thresPace,percent);
                currDist = stepProxy->data(stepProxy->index(i,6)).toDouble();
                if(!partName.contains(isBreak))
                {
                    tempID = 6;
                    if(!timeBase) stepTime = this->calc_duration(currentSport,currDist,thresValue);
                    pValue = this->get_timesec(thresValue);
                }
                else
                {
                    tempID = 0;
                    pValue = 0.0;
                }
                if(timeBase)
                {
                    editRow[4] = 1;
                    workFactor = generalValues->value("workfactor").toDouble();
                }
            }
            else if(isStrength || isAlt)
            {
                thresValue = "0";
                pValue = percent / 10.0;
            }
            else
            {
                thresValue = this->calc_thresPower(thresPower,percent);
                pValue = 1.0;
            }

            valueList << stepProxy->data(stepProxy->index(i,2)).toString()
                      << stepProxy->data(stepProxy->index(i,3)).toString()
                      << QString::number(percent)
                      << this->calc_thresPace(thresPace,percent)
                      << QString::number(round(thresPower* (percent/100.0)))
                      << stepTime
                      << QString::number(this->estimate_stress(ui->comboBox_sport->currentText(),thresValue,this->get_timesec(stepTime),usePM))
                      << QString::number(this->set_doubleValue(this->calc_totalWork(currentSport,pValue,this->get_timesec(stepTime),tempID)*workFactor,false))
                      << QString::number(this->set_doubleValue(currDist,true))
                      << stepProxy->data(stepProxy->index(i,7)).toString();
        }

        QTreeWidgetItem *item = new QTreeWidgetItem(valueList);

        parentItem = stepProxy->data(stepProxy->index(i,8)).toString();

        if(parentItem.contains(isGroup) || parentItem.contains(isSeries))
        {
            QList<QTreeWidgetItem*> pItem = ui->treeWidget_intervall->findItems(parentItem,Qt::MatchExactly | Qt::MatchRecursive,0);
            pItem.at(0)->addChild(item);
        }
        else
        {
            ui->treeWidget_intervall->insertTopLevelItem(ui->treeWidget_intervall->topLevelItemCount(),item);
        }
    }

    ui->treeWidget_intervall->expandAll();
    this->set_plotModel();
    clearFlag = false;
    this->set_controlButtons(true);
    ui->toolButton_copy->setEnabled(false);
    if(proxyFilter->rowCount() == 0) ui->pushButton_sync->setEnabled(false);
    ui->treeWidget_intervall->setCurrentItem(ui->treeWidget_intervall->topLevelItem(0));
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
    metaProxy->setFilterFixedString(sport);
    metaProxy->setFilterKeyColumn(0);

    existWorkIDs = this->get_workoutIds();

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
        this->delete_stdWorkout(currentWorkID,false);
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

    workoutValues << currentSport
                  << workID
                  << ui->comboBox_code->currentText()
                  << ui->lineEdit_workoutname->text()
                  << get_workoutTime(timeSum)
                  << QString::number(distSum)
                  << QString::number(round(stressSum))
                  << QString::number(workSum)
                  << QString::number(ui->checkBox_timebased->isChecked());

     workModel = this->workouts_meta;
     this->save_workout_values(workoutValues,workModel);

    //Intervalldaten
     workModel = this->workouts_steps;
     for(int c_item = 0; c_item < ui->treeWidget_intervall->topLevelItemCount(); ++c_item,++counter)
     {
         currentItem = ui->treeWidget_intervall->topLevelItem(c_item);

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
     this->write_standard_workouts();
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

void Dialog_workCreator::on_treeWidget_intervall_itemChanged(QTreeWidgetItem *item, int column)
{
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
}

void Dialog_workCreator::set_defaultData(QTreeWidgetItem *item, bool hasValues)
{
    currentItem = item;
    QString defaultTime = "05:00";
    int level = 1;
    int tempID = 0;
    double pValue = 0;
    double defaultDist = 0.0;
    double percent = this->get_thresPercent(currentSport,levelList.at(level),false);
    QString thresValue = this->calc_thresPace(thresPace,percent);

    if(isSwim)
    {
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
        pValue = calc_thresPower(thresPower,percent);
        defaultDist = this->calc_distance(defaultTime,3600.0/this->wattToSpeed(thresPower,thresSpeed,pValue));
    }
    else if(isRun)
    {
        defaultDist = this->calc_distance(defaultTime,static_cast<double>(this->get_timesec(threstopace(thresPace,percent))));
        pValue = get_speed(QTime::fromString(calc_thresPace(thresPace,percent),"mm:ss"),0,currentSport,true);
    }
    else if(isStrength || isAlt)
    {
        pValue = percent / 10.0;
    }

    if(hasValues)
    {
        item->setData(1,Qt::EditRole,levelList.at(level));
        item->setData(2,Qt::EditRole,percent);
        item->setData(3,Qt::EditRole,thresValue);
        item->setData(4,Qt::EditRole,pValue);
        item->setData(5,Qt::EditRole,defaultTime);
        item->setData(6,Qt::EditRole,this->estimate_stress(currentSport,thresValue,this->get_timesec(defaultTime),0));
        item->setData(7,Qt::EditRole,this->set_doubleValue(this->calc_totalWork(currentSport,pValue,this->get_timesec(defaultTime),tempID),false));
        item->setData(8,Qt::EditRole,defaultDist);
        item->setData(9,Qt::EditRole,"");
    }
    else
    {
        QString itemName = item->data(0,Qt::DisplayRole).toString();
        QList<QTreeWidgetItem*> groupCount = ui->treeWidget_intervall->findItems(itemName,Qt::MatchRecursive | Qt::MatchContains,0);
        itemName = itemName +"-"+QString::number(groupCount.count());
        item->setData(0,Qt::EditRole,itemName);
        item->setData(8,Qt::EditRole,2);
    }
    ui->treeWidget_intervall->expandAll();
    ui->treeWidget_intervall->setTreePosition(-1);
    this->set_plotModel();
}

void Dialog_workCreator::show_editItem(QTreeWidgetItem *item)
{
    currentItem = item;
    QString itemIdent = item->data(0,Qt::DisplayRole).toString();
    edit_del.groupName = itemIdent;
    valueModel->clear();
    valueModel->setColumnCount(3);

    if(itemIdent.contains(isGroup) || itemIdent.contains(isSeries))
    {
        valueModel->setRowCount(2);
        edit_del.hasValue = false;
        valueModel->setData(valueModel->index(0,0,QModelIndex()),item->data(0,Qt::DisplayRole));
        valueModel->setData(valueModel->index(0,2,QModelIndex()),true);
        valueModel->setData(valueModel->index(1,0,QModelIndex()),item->data(9,Qt::DisplayRole));
        valueModel->setData(valueModel->index(1,2,QModelIndex()),true);
    }
    else
    {
        valueModel->setRowCount(editRow.count());
        for(int i = 0; i < item->columnCount();++i)
        {
            valueModel->setData(valueModel->index(i,0,QModelIndex()),item->data(i,Qt::DisplayRole));
            valueModel->setData(valueModel->index(i,1,QModelIndex()),0);
            valueModel->setData(valueModel->index(i,2,QModelIndex()),editRow[i]);
        }

        valueModel->setData(valueModel->index(2,1,QModelIndex()),levelList.indexOf(valueModel->data(valueModel->index(1,0)).toString()));

        if(isBike)
        {
            valueModel->setData(valueModel->index(9,0,QModelIndex()),set_doubleValue(this->wattToSpeed(thresPower,thresSpeed,item->data(4,Qt::DisplayRole).toDouble()),true));
        }
        else
        {
            if(item->data(2,Qt::DisplayRole).toInt() > 0)
            {
                valueModel->setData(valueModel->index(9,0,QModelIndex()),this->calc_lapSpeed(currentSport,static_cast<double>(this->get_timesec(item->data(3,Qt::DisplayRole).toString()))));
            }
            else
            {
                if(isSwim && item->data(0,Qt::DisplayRole) == generalValues->value("breakname"))
                {
                    valueModel->setData(valueModel->index(5,2),true);
                }
                valueModel->setData(valueModel->index(9,0,QModelIndex()),"0 km/h");
            }
        }
        edit_del.hasValue = true;
    }
    this->control_editPanel(true);
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
    while(ui->treeWidget_intervall->topLevelItemCount() > 0)
    {
        for(int c_item = 0; c_item < ui->treeWidget_intervall->topLevelItemCount(); ++c_item)
        {
            treeItem = ui->treeWidget_intervall->topLevelItem(c_item);

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
    ui->listView_workouts->clearSelection();
    ui->listWidget_group->clearSelection();
    ui->listWidget_phases->clearSelection();
    ui->pushButton_sync->setText(" - 0");
    this->resetAxis();
}

void Dialog_workCreator::set_plotModel()
{
    plotModel->clear();
    plotModel->setColumnCount(7);
    QTreeWidget *intTree = ui->treeWidget_intervall;
    int groupReps = 0;
    int seriesReps = 0;
    int childCount = 0;
    int subchildCount = 0;
    int currIndex = 0;
    bool isload = false;
    QString phase,subphase;
    QTreeWidgetItem *topItem,*childItem,*subItem;

    for(int c_item = 0; c_item < ui->treeWidget_intervall->topLevelItemCount(); ++c_item)
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
    int currIndex = ui->treeWidget_intervall->currentIndex().row();
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

void Dialog_workCreator::on_treeWidget_intervall_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    ui->widget_plot->graph(0)->setSelection(QCPDataSelection(QCPDataRange(0,0)));
    this->show_editItem(item);
    this->set_selectData(item);
}

void Dialog_workCreator::on_comboBox_sport_currentTextChanged(const QString &sport)
{
    currentSport = sport;
    isSwim = isBike = isRun = isStrength = isAlt = isOther = false;

    if(currentSport == settings::isSwim) isSwim = true;
    if(currentSport == settings::isBike) isBike = true;
    if(currentSport == settings::isRun) isRun = true;
    if(currentSport == settings::isStrength) isStrength = true;
    if(currentSport == settings::isAlt) isAlt = true;
    if(currentSport == settings::isOther) isOther = true;

    QString postFix;
    int usePM = 0;

    if(isOther)
    {
        thresPace = 0;
        thresPower = 0.0;
        ui->label_threshold->setText("-");
        currThres = thresPower;
        editRow[2] = 0;
        editRow[5] = 1;
        editRow[8] = 1;
    }
    if(isSwim)
    {
       usePM = thresValues->value("swimpm");
       thresPower = thresValues->value("swimpower");
       thresPace = thresValues->value("swimpace");
       currThres = thresPace;
       postFix = "/100m";
       editRow[2] = 1;
       editRow[5] = 0;
       editRow[8] = 1;
    }
    if(isBike)
    {
       usePM = thresValues->value("bikepm");
       thresPower = thresValues->value("bikepower");
       thresPace = thresValues->value("bikepace");
       thresSpeed = thresValues->value("bikespeed");
       currThres = thresPower;
       postFix = " Watt";
       editRow[2] = 1;
       editRow[5] = 1;
       editRow[8] = 1;
    }
    if(isRun)
    {
       postFix = "/km";
       thresPace = thresValues->value("runpace");

       if(thresValues->value("runpm") == 0)
       {
           thresPower = thresValues->value("runpower");
       }
       else
       {
           thresPower = thresValues->value("runcp");
       }
       usePM = 0;
       currThres = thresPace;
       editRow[2] = 1;
       editRow[5] = 1;
       editRow[8] = 1;
    }
    if(isStrength)
    {
       usePM = 1;
       thresPower = thresValues->value("stgpower");
       thresPace = 0;
       currThres = thresPower;
       postFix = " Watt";
       editRow[2] = 1;
       editRow[5] = 1;
       editRow[8] = 1;
    }
    if(isAlt)
    {
       usePM = 1;
       thresPower = thresValues->value("runpower");
       thresPace = 0;
       currThres = thresPower;
       postFix = " Watt";
       editRow[2] = 1;
       editRow[5] = 1;
       editRow[8] = 0;
    }

    if(usePM == 0)
    {
        ui->label_threshold->setText(this->set_time(thresPace) + postFix);
    }
    else
    {
        ui->label_threshold->setText(QString::number(thresPower) + postFix);
    }

    edit_del.thresPower = thresPower;
    edit_del.thresPace = thresPace;
    edit_del.thresSpeed = thresSpeed;

    edit_del.sport = sport;
    this->clearIntTree();
    this->get_workouts(sport);
}

void Dialog_workCreator::on_listView_workouts_clicked(const QModelIndex &index)
{
    QStringList workoutTitle = listModel->data(listModel->index(index.row(),0)).toString().split("-");
    QString workoutID = listModel->data(listModel->index(index.row(),1)).toString();
    QString workCode = workoutTitle.first();
    QString workTitle = workoutTitle.last();
    currentWorkID = workoutID;
    proxyFilter->invalidate();
    proxyFilter->setFilterRegExp("\\b"+workoutID+"\\b");
    proxyFilter->setFilterKeyColumn(10);

    ui->pushButton_sync->setText(" - "+QString::number(proxyFilter->rowCount()));
    ui->comboBox_code->setCurrentText(workCode.replace(" ",""));
    ui->lineEdit_workoutname->setText(workTitle.replace(" ",""));
    ui->checkBox_timebased->setChecked(listModel->data(listModel->index(index.row(),2)).toBool());
    this->control_editPanel(false);
    this->open_stdWorkout(workoutID);
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
    ui->treeWidget_intervall->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_toolButton_cancel_clicked()
{
    this->control_editPanel(false);
    this->set_plotModel();
    ui->treeWidget_intervall->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_toolButton_remove_clicked()
{
    QTreeWidgetItem *deleteItem = currentItem;

    if(ui->treeWidget_intervall->topLevelItemCount() == 1)
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
        currentItem = ui->treeWidget_intervall->currentItem();
        this->show_editItem(currentItem);
    }
    this->set_plotModel();
    ui->treeWidget_intervall->setCurrentItem(currentItem);
}

QTreeWidgetItem* Dialog_workCreator::move_item(bool up, QTreeWidgetItem *currentItem)
{
    int currentindex = ui->treeWidget_intervall->currentIndex().row();

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
        QTreeWidgetItem *topItem = ui->treeWidget_intervall->takeTopLevelItem(currentindex);
        if(up)
        {
            ui->treeWidget_intervall->insertTopLevelItem(currentindex-1,topItem);
        }
        else
        {
            ui->treeWidget_intervall->insertTopLevelItem(currentindex+1,topItem);
        }
        return topItem;
    }
    return 0;
}

void Dialog_workCreator::on_toolButton_up_clicked()
{
    QTreeWidgetItem *currentItem = this->move_item(true,ui->treeWidget_intervall->currentItem());
    this->set_plotModel();
    ui->treeWidget_intervall->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_toolButton_down_clicked()
{
    QTreeWidgetItem *currentItem = this->move_item(false,ui->treeWidget_intervall->currentItem());
    this->set_plotModel();
    ui->treeWidget_intervall->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_treeWidget_intervall_itemSelectionChanged()
{
    if(!clearFlag)
    {
        QTreeWidgetItem *current = ui->treeWidget_intervall->currentItem();
        int row = ui->treeWidget_intervall->currentIndex().row();
        int rowCount = 0;

        if(current->parent())
        {
            rowCount = ui->treeWidget_intervall->currentItem()->parent()->childCount();
        }
        else
        {
            rowCount = ui->treeWidget_intervall->topLevelItemCount();
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
        return ui->treeWidget_intervall->topLevelItem(item)->data(pos,Qt::DisplayRole).toString();
    }
    if(level == 1)
    {
        return ui->treeWidget_intervall->topLevelItem(item)->child(i_child)->data(pos,Qt::DisplayRole).toString();
    }
    if(level == 2)
    {
        return ui->treeWidget_intervall->topLevelItem(item)->child(i_child)->child(c_sub)->data(pos,Qt::DisplayRole).toString();
    }

    return 0;
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
        this->delete_stdWorkout(currentWorkID,true);
        currentWorkID = QString();
        this->clearIntTree();
        this->get_workouts(currentSport);
    }
}

void Dialog_workCreator::on_lineEdit_workoutname_textChanged(const QString &value)
{
    if(!value.isEmpty() && ui->treeWidget_intervall->topLevelItemCount() > 0)
    {
        ui->toolButton_save->setEnabled(true);
    }
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
}

void Dialog_workCreator::on_toolButton_close_clicked()
{
    if(worksched->get_isUpdated())
    {
        accept();
    }
    else
    {
        reject();
    }
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
        workDate = QDate::fromString(proxyFilter->data(proxyFilter->index(i,1)).toString(),"dd.MM.yyyy");

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
        wDate = QDate::fromString(proxyFilter->data(proxyFilter->index(i,1)).toString(),"dd.MM.yyyy");
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
    edit_del.timeBased = checked;
}
