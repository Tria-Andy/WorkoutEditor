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
    generalValues = settings::getStringMapPointer(settings::stingMap::General);
    stdWorkTags = settings::get_xmlMapping("standardworkouts");
    workoutTags = settings::get_xmlMapping("standardworkout");
    partTags = settings::get_xmlMapping("part");
    stepTags = settings::get_xmlMapping("step");
    isSeries = "Series";
    isGroup = "Group";
    ui->comboBox_partName->addItem(isSeries);
    ui->comboBox_partName->addItem(isGroup);
    ui->comboBox_stepName->addItems(settings::get_listValues("IntEditor"));
    ui->comboBox_levelName->addItems(settings::get_listValues("Level"));
    isBreak = generalValues->value("breakname");
    dateFormat = settings::get_format("dateformat");
    longTime = settings::get_format("longtime");
    shortTime = settings::get_format("shorttime");
    groupList << isGroup << isSeries;
    levelList = settings::get_listValues("Level");
    ui->listWidget_group->addItems(groupList);
    ui->listWidget_group->setItemDelegate(&mousehover_del);
    ui->listWidget_phases->addItems(settings::get_listValues("IntEditor"));
    ui->listWidget_phases->setItemDelegate(&mousehover_del);
    ui->comboBox_sport->addItems(settings::get_listValues("Sport"));
    ui->comboBox_code->addItems(settings::get_listValues("WorkoutCode"));
    clearFlag = false;
    isWorkLoaded = false;

    workoutHeader = settings::getHeaderMap("workouteditor");
    QTreeWidgetItem *headerItem = new QTreeWidgetItem();

    for(int i = 0; i < workoutHeader->count(); ++i)
    {
        headerItem->setData(i,Qt::DisplayRole,workoutHeader->at(i));

    }
    ui->treeWidget_workoutTree->setHeaderItem(headerItem);

    ui->treeWidget_workoutTree->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidget_workoutTree->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->treeWidget_workoutTree->setAcceptDrops(true);
    ui->treeWidget_workoutTree->setDragEnabled(true);
    ui->treeWidget_workoutTree->setDragDropMode(QAbstractItemView::DragDrop);
    ui->treeWidget_workoutTree->setEnabled(true);
    ui->treeWidget_workoutTree->setItemDelegate(&workTree_del);

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
    workoutUpdateWidget = new QListWidget(viewFrame);
    workoutUpdateWidget->setSelectionMode(QAbstractItemView::NoSelection);
    viewBox->addWidget(workoutUpdateWidget);

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

    QHash<QString,QVector<QString>> sportMap = stdWorkouts->get_workoutMap()->value(sport);

    for(QHash<QString,QVector<QString>>::const_iterator workstart = sportMap.cbegin(), workend = sportMap.cend(); workstart != workend; ++workstart)
    {
        QListWidgetItem *item = new QListWidgetItem();
        item->setData(Qt::DisplayRole,workstart.value().at(0)+" - "+workstart.value().at(1));
        item->setData(Qt::AccessibleTextRole,workstart.key());
        item->setData(Qt::UserRole,workstart.value().at(8));
        item->setData(Qt::UserRole+1,workstart.value().at(6));
        item->setData(Qt::UserRole+2,workstart.value().at(7));
        item->setData(Qt::ToolTipRole,set_time(workstart.value().at(2).toInt())+" - "+workstart.value().at(3)+" - "+workstart.value().at(4)+" - "+workstart.value().at(5));
        workoutTitle.insert(workstart.value().at(1));
        ui->listWidget_workouts->addItem(item);
    }
    ui->listWidget_workouts->sortItems(Qt::AscendingOrder);
    ui->label_workouts->setText("Workouts: "+QString::number(sportMap.count()));

    QColor sportColor = settings::get_itemColor(sport);
    QString sportBack = "rgb("+QString::number(sportColor.red())+","+QString::number(sportColor.green())+","+QString::number(sportColor.blue())+")";
    QString actBackground = "background: "+sportBack;
    ui->listWidget_workouts->setStyleSheet(actBackground);
    this->set_sportData(sport);
}

void Dialog_workCreator::on_listWidget_workouts_itemClicked(QListWidgetItem *item)
{
    isWorkLoaded = true;
    this->load_selectedWorkout(item->data(Qt::AccessibleTextRole).toString());
    QString workoutId = item->data(Qt::AccessibleTextRole).toString();

    stdWorkoutMapping = worksched->get_linkStdWorkouts(workoutId);
    workoutUpdateWidget->clear();

    if(stdWorkoutMapping.count() > 0)
    {
        int counter = 1;
        for(QMap<QDate,int>::const_iterator it = stdWorkoutMapping.cbegin(), end = stdWorkoutMapping.cend(); it != end; ++it)
        {
            QListWidgetItem *item = new QListWidgetItem(workoutUpdateWidget);
            item->setData(Qt::DisplayRole,QString::number(counter++)+" - "+it.key().toString(dateFormat)+" - Workout: "+QString::number(it.value()+1));
        }
        ui->pushButton_sync->setEnabled(true);
        ui->pushButton_sync->setAccessibleName(workoutId);
    }
    ui->pushButton_sync->setText(" - "+QString::number(stdWorkoutMapping.count()));

    ui->comboBox_code->setCurrentText(item->data(Qt::DisplayRole).toString().split(" - ").first());
    ui->lineEdit_workoutname->setText(item->data(Qt::DisplayRole).toString().split(" - ").at(1));
    ui->checkBox_timebased->setChecked(item->data(Qt::UserRole+1).toBool());
    ui->timeEdit_lapTime->setEnabled(item->data(Qt::UserRole+1).toBool());
    ui->doubleSpinBox_distance->setEnabled(!item->data(Qt::UserRole+1).toBool());
    ui->toolButton_map->setProperty("Image",item->data(Qt::UserRole+2).toString());
    ui->pushButton_clear->setEnabled(isWorkLoaded);
    this->control_editPanel(false);
}

void Dialog_workCreator::load_selectedWorkout(QString workID)
{
    ui->treeWidget_workoutTree->blockSignals(true);
    ui->treeWidget_workoutTree->clear();
    currentWorkID = workID;
    QStandardItem* workoutItem = stdWorkouts->get_selectedWorkout(workID);
    QTreeWidgetItem *rootItem = ui->treeWidget_workoutTree->invisibleRootItem();

    for(int row = 0; row < workoutItem->rowCount(); ++row)
    {
        if(workoutItem->child(row,0)->hasChildren())
        {
            this->read_selectedChild(workoutItem->child(row,0),this->set_itemToWidget(workoutItem->child(row,0),rootItem));
        }
        else
        {
            this->set_itemToWidget(workoutItem->child(row,0),rootItem);
        }
    }
    ui->treeWidget_workoutTree->expandAll();
    this->read_currentWorkTree();
    ui->treeWidget_workoutTree->blockSignals(false);
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
                this->read_selectedChild(childItem,this->set_itemToWidget(childItem,treeItem));
            }
            else
            {
                this->set_itemToWidget(childItem,treeItem);
            }
        }
    }
}

QTreeWidgetItem* Dialog_workCreator::set_itemToWidget(QStandardItem *item,QTreeWidgetItem *treeItem)
{
    QTreeWidgetItem *stepitem = new QTreeWidgetItem(treeItem);

    if(this->get_itemValue(item,partTags,partTags->at(1)) == isGroup || this->get_itemValue(item,partTags,partTags->at(1)) == isSeries)
    {
        stepitem->setData(0,Qt::AccessibleTextRole,stdWorkTags->at(2));
        stepitem->setData(0,Qt::DisplayRole,this->get_itemValue(item,partTags,partTags->at(1)));
        stepitem->setData(9,Qt::DisplayRole,this->get_itemValue(item,partTags,partTags->at(2)));
    }
    else
    {
        stepitem->setData(0,Qt::AccessibleTextRole,stdWorkTags->at(3));
        stepitem->setData(0,Qt::DisplayRole,this->get_itemValue(item,stepTags,stepTags->at(1)));
        stepitem->setData(1,Qt::DisplayRole,this->get_itemValue(item,stepTags,stepTags->at(4)));
        stepitem->setData(2,Qt::DisplayRole,this->get_itemValue(item,stepTags,stepTags->at(2)).toInt());
        stepitem->setData(3,Qt::DisplayRole,set_time(this->calc_thresPace(this->get_itemValue(item,stepTags,stepTags->at(2)).toDouble())));
        stepitem->setData(4,Qt::DisplayRole,this->calc_thresPower(this->get_itemValue(item,stepTags,stepTags->at(2)).toDouble()));
        stepitem->setData(5,Qt::DisplayRole,set_time(this->get_itemValue(item,stepTags,stepTags->at(3)).toInt()));
        stepitem->setData(6,Qt::DisplayRole,this->calc_stressScore(this->get_itemValue(item,stepTags,stepTags->at(2)).toDouble(),this->get_itemValue(item,stepTags,stepTags->at(3)).toInt()));
        stepitem->setData(7,Qt::DisplayRole,this->set_doubleValue(this->calc_totalWork(this->get_baseValue(this->get_itemValue(item,stepTags,stepTags->at(2)).toDouble()),this->get_itemValue(item,stepTags,stepTags->at(3)).toInt(),this->get_swimStyleID(this->get_itemValue(item,stepTags,stepTags->at(1)).toString()))*workFactor,false));
        stepitem->setData(8,Qt::DisplayRole,this->get_itemValue(item,stepTags,stepTags->at(5)).toDouble());
    }
    return stepitem;
}

QVariant Dialog_workCreator::get_itemValue(QStandardItem *item,QStringList *list,QString tagName)
{
    return item->index().siblingAtColumn(list->indexOf(tagName)).data(Qt::DisplayRole);
}

void Dialog_workCreator::set_sportData(QString sport)
{
    this->set_currentSport(sport);

    if(isSwim)
    {
        ui->doubleSpinBox_distance->setSingleStep(0.05);
    }
    else
    {
        ui->doubleSpinBox_distance->setSingleStep(0.1);
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

    /*
    if(isSwim)
    {
        usePMData = static_cast<bool>(thresValues->value("swimpm"));
        thresPower = static_cast<int>(thresValues->value("swimpower"));
        thresPace = static_cast<int>(thresValues->value("swimpace"));
        workFactor = thresValues->value("swimfactor");
        sportMark = "/100m";
        ui->doubleSpinBox_distance->setSingleStep(0.05);
    }
    else if(isBike)
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
        workFactor = thresValues->value("runfactor");
        if(usePMData)
        {
            sportMark = " Watt";
        }
        else
        {
            sportMark = "/km";
        }

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
        thresPower = static_cast<int>(thresValues->value("runcp"));
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
    */
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

void Dialog_workCreator::on_treeWidget_workoutTree_itemChanged(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    if(currentItem != item)
    {
        if(item->data(0,Qt::DisplayRole) == isGroup || item->data(0,Qt::DisplayRole) == isSeries)
        {
            this->set_defaultData(item,true);
        }
        else
        {
            this->set_defaultData(item,false);
        }
    }
    this->set_controlButtons(true);
}

void Dialog_workCreator::set_defaultData(QTreeWidgetItem *item, bool hasChilds)
{
    currentItem = item;
    QTime defaultTime;
    defaultTime.setHMS(0,5,0);
    int level = 1;
    int tempID = 0;
    double pValue = 0;
    double defaultDist = 0.0;
    double percent = this->get_thresPercent(levelList.at(level),false);

    if(isSwim)
    {
        if(item->data(0,Qt::DisplayRole).toString() == isBreak)
        {
            defaultDist = percent = 0.0;
            defaultTime.setHMS(0,0,30);
            level = 0;
            tempID = 0;
            pValue = 0;
        }
        else
        {
            defaultDist = 0.1;
            defaultTime = this->calc_duration(defaultDist,this->calc_thresPace(percent));
            tempID = 6;
            pValue = this->get_baseValue(percent);
        }
    }
    else if (isBike)
    {
        pValue = this->get_baseValue(percent);
        defaultDist = this->calc_distance(get_secFromTime(defaultTime),3600.0/this->wattToSpeed(thresPower,pValue));
    }
    else if(isRun)
    {
        defaultDist = this->calc_distance(get_secFromTime(defaultTime),this->calc_thresPace(percent));
        pValue = get_speed(defaultTime,defaultDist,false);
    }
    else if(isStrength || isAlt)
    {
        pValue = percent / 10.0;
    }
    else if(isTria)
    {
        pValue = percent / 10.0;
    }

    if(hasChilds)
    {
        item->setData(0,Qt::AccessibleTextRole,stdWorkTags->at(2));
        item->setData(0,Qt::DisplayRole,item->data(0,Qt::DisplayRole).toString());
        item->setData(9,Qt::DisplayRole,2);
    }
    else
    {
        item->setData(0,Qt::AccessibleTextRole,stdWorkTags->at(3));
        item->setData(1,Qt::DisplayRole,levelList.at(level));
        item->setData(2,Qt::DisplayRole,percent);
        item->setData(3,Qt::DisplayRole,set_time(this->calc_thresPace(percent)));
        item->setData(4,Qt::DisplayRole,QString::number(this->calc_thresPower(percent)));
        item->setData(5,Qt::DisplayRole,defaultTime.toString(shortTime));
        item->setData(6,Qt::DisplayRole,this->calc_stressScore(percent,this->get_secFromTime(defaultTime)));
        item->setData(7,Qt::DisplayRole,this->set_doubleValue(this->calc_totalWork(pValue,this->get_secFromTime(defaultTime),tempID)*workFactor,false));
        item->setData(8,Qt::DisplayRole,defaultDist);
    }
    ui->treeWidget_workoutTree->expandAll();
    ui->treeWidget_workoutTree->setTreePosition(-1);
    this->read_currentWorkTree();
}

void Dialog_workCreator::edit_selectedStep(QTreeWidgetItem *item)
{
    QList<QListWidgetItem*> itemList;

    if(item->data(0,Qt::DisplayRole).toString() == isSeries || item->data(0,Qt::DisplayRole).toString() == isGroup)
    {
        ui->frame_partEdit->setVisible(true);
        ui->frame_stepEdit->setVisible(false);
        ui->comboBox_partName->setCurrentText(item->data(0,Qt::DisplayRole).toString());
        ui->spinBox_repeats->setValue(item->data(9,Qt::DisplayRole).toInt());
        currentItem->setData(0,Qt::UserRole+1,true);
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
        currentItem->setData(0,Qt::UserRole+1,false);
    }

    this->control_editPanel(true);
}

void Dialog_workCreator::update_selectedStep()
{
    if(currentItem->data(0,Qt::UserRole+1).toBool())
    {
        currentItem->setData(0,Qt::DisplayRole,ui->comboBox_partName->currentText());
        currentItem->setData(9,Qt::DisplayRole,ui->spinBox_repeats->value());
    }
    else
    {
        currentItem->setData(0,Qt::DisplayRole,ui->comboBox_stepName->currentText());
        currentItem->setData(1,Qt::DisplayRole,ui->comboBox_levelName->currentText());
        currentItem->setData(2,Qt::DisplayRole,ui->spinBox_level->value());
        currentItem->setData(3,Qt::DisplayRole,ui->timeEdit_pace->time().toString(shortTime));
        currentItem->setData(4,Qt::DisplayRole,ui->lineEdit_power->text());
        currentItem->setData(5,Qt::DisplayRole,ui->timeEdit_lapTime->time().toString(shortTime));
        currentItem->setData(6,Qt::DisplayRole,ui->doubleSpinBox_stressScore->value());
        currentItem->setData(7,Qt::DisplayRole,ui->doubleSpinBox_work->value());
        currentItem->setData(8,Qt::DisplayRole,QString::number(ui->doubleSpinBox_distance->value()));
    }
    this->read_currentWorkTree();
}

void Dialog_workCreator::refresh_editStep()
{
    if(ui->checkBox_timebased->isChecked())
    {
        ui->doubleSpinBox_distance->setValue(this->calc_distance(this->get_secFromTime(ui->timeEdit_lapTime->time()),this->get_secFromTime(ui->timeEdit_pace->time())));
    }
    else
    {
        ui->timeEdit_lapTime->setTime(this->calc_duration(ui->doubleSpinBox_distance->value(),this->get_secFromTime(ui->timeEdit_pace->time())));
    }

    ui->doubleSpinBox_stressScore->setValue(this->calc_stressScore(ui->spinBox_level->value(),this->get_secFromTime(ui->timeEdit_lapTime->time())));
    ui->doubleSpinBox_work->setValue(this->calc_totalWork(this->get_baseValue(ui->spinBox_level->value()),this->get_secFromTime(ui->timeEdit_lapTime->time()),this->get_swimStyleID(ui->comboBox_stepName->currentText()))*workFactor);
    ui->lineEdit_speed->setText(QString::number(this->get_speed(ui->timeEdit_pace->time(),ui->doubleSpinBox_distance->value(),true))+" km/h");
}

void Dialog_workCreator::read_currentWorkTree()
{
    plotMap.clear();
    plotRange.clear();
    lastPlotValues.resize(5);
    lastPlotValues.fill(0);

    ui->treeWidget_workoutTree->blockSignals(true);
    QTreeWidgetItem *rootItem = ui->treeWidget_workoutTree->invisibleRootItem();

    for(int row = 0; row < rootItem->childCount(); ++row)
    {
        if(rootItem->child(row)->childCount() > 0)
        {
            this->read_currentChild(rootItem->child(row),rootItem->child(row)->data(0,Qt::DisplayRole).toString(),row);
        }
        else
        {
            rootItem->child(row)->setData(0,Qt::UserRole,rootItem->child(row)->data(0,Qt::DisplayRole).toString()+":"+QString::number(row));
            this->read_currentData(rootItem->child(row));

        }
    }
    this->draw_plotGraphic(plotMap.count());
    ui->treeWidget_workoutTree->blockSignals(false);
}

void Dialog_workCreator::read_currentChild(QTreeWidgetItem *item,QString parentStep,int stepID)
{
    QString stepIdent = parentStep+":"+QString::number(stepID);

    if(item->childCount() > 0)
    {
        item->setData(0,Qt::UserRole,stepIdent);
        int childCount = item->data(9,Qt::DisplayRole).toInt();

        for(int repeat = 0; repeat < childCount; ++repeat)
        {
            for(int child = 0; child < item->childCount(); ++child)
            {
                item->child(child)->setData(0,Qt::UserRole,stepIdent);
                this->read_currentChild(item->child(child),stepIdent,child);
            }
        }
    }
    else
    {
        item->setData(0,Qt::UserRole,stepIdent);
        this->read_currentData(item);
    }
}

void Dialog_workCreator::read_currentData(QTreeWidgetItem *item)
{
    double timeSum = 0;
    double distSum = 0;
    double stressSum = 0.0;
    double workSum = 0;
    int row = plotMap.count();
    QVector<double> stepValues(5,0);

    if(!plotMap.isEmpty())
    {
        timeSum = lastPlotValues.at(1);
        distSum = lastPlotValues.at(2);
        stressSum = lastPlotValues.at(3);
        workSum = lastPlotValues.at(4);
    }
    stepValues[0] = item->data(2,Qt::DisplayRole).toDouble();
    stepValues[1] = timeSum;
    stepValues[2] = distSum;
    stepValues[3] = stressSum;
    stepValues[4] = workSum;
    plotMap.insert(row,qMakePair(item->data(0,Qt::UserRole).toString(),stepValues));

    lastPlotValues[0] = item->data(2,Qt::DisplayRole).toDouble();
    lastPlotValues[1] = timeSum + this->get_timesec(item->data(5,Qt::DisplayRole).toString());
    lastPlotValues[2] = distSum + item->data(8,Qt::DisplayRole).toDouble();
    lastPlotValues[3] = stressSum + item->data(6,Qt::DisplayRole).toDouble();
    lastPlotValues[4] = workSum + item->data(7,Qt::DisplayRole).toDouble();
    plotMap.insert(row+1,qMakePair(item->data(0,Qt::UserRole).toString(),lastPlotValues));
    plotRange.insertMulti(item->data(0,Qt::UserRole).toString(),qMakePair(row,row+1));
}

void Dialog_workCreator::save_selectedWorkout()
{
    QString currentSport = ui->comboBox_sport->currentText();
    QTreeWidgetItem *rootItem = ui->treeWidget_workoutTree->invisibleRootItem();
    QString workCount;

    if(currentWorkID.isEmpty())
    {
        QPair<int,QString> newWorkID = stdWorkouts->create_newWorkout(currentSport);
        workCount = QString::number(newWorkID.first);
        currentWorkID = newWorkID.second;
    }
    else
    {
        workCount = stdWorkouts->get_workoutCount(currentWorkID);
    }

    QList<QStandardItem*> metaList;

    metaList.insert(0, new QStandardItem(workCount));
    metaList.insert(1, new QStandardItem(currentWorkID));
    metaList.insert(2, new QStandardItem(ui->comboBox_code->currentText()));
    metaList.insert(3, new QStandardItem(ui->lineEdit_workoutname->text()));
    metaList.insert(4, new QStandardItem(QString::number(ceil(plotMap.last().second.at(1)/10.0)*10.0)));
    metaList.insert(5, new QStandardItem(QString::number(round(plotMap.last().second.at(2)*10.0)/10.0)));
    metaList.insert(6, new QStandardItem(QString::number(round(plotMap.last().second.at(3)))));
    metaList.insert(7, new QStandardItem(QString::number(round_workValue(static_cast<int>(plotMap.last().second.at(4))))));
    metaList.insert(8, new QStandardItem(QString::number(ui->checkBox_timebased->isChecked())));
    metaList.insert(9, new QStandardItem(ui->toolButton_map->property("Image").toString()));
    metaList.at(0)->setData(stdWorkTags->at(1),Qt::AccessibleTextRole);

    stdWorkouts->update_selectedWorkout(currentWorkID,metaList);

    QStandardItem *workItem = stdWorkouts->get_selectedWorkout(currentWorkID);

    for(int row = 0; row < rootItem->childCount(); ++row)
    {
        if(rootItem->child(row)->childCount() > 0)
        {
             this->read_childToWorkout(rootItem->child(row),add_itemToWorkout(rootItem->child(row),workItem,row),row);
        }
        else
        {
            this->add_itemToWorkout(rootItem->child(row),workItem,row);
        }
    }

    this->get_workouts(currentSport);
}

void Dialog_workCreator::read_childToWorkout(QTreeWidgetItem *item,QStandardItem *targetItem,int pos)
{
    if(item->childCount() > 0)
    {
        for(int child = 0; child < item->childCount(); ++child)
        {
            if(item->child(child)->childCount() > 0)
            {
                this->read_childToWorkout(item->child(child),this->add_itemToWorkout(item->child(child),targetItem,child),child);
            }
            else
            {
                this->add_itemToWorkout(item->child(child),targetItem,pos);
            }
        }
    }
}

QStandardItem* Dialog_workCreator::add_itemToWorkout(QTreeWidgetItem *item,QStandardItem *targetItem,int pos)
{
    QList<QStandardItem*> itemList;

    if(item->data(0,Qt::DisplayRole).toString() == isGroup || item->data(0,Qt::DisplayRole).toString() == isSeries)
    {
        itemList << new QStandardItem(QString::number(pos));
        itemList << new QStandardItem(item->data(0,Qt::DisplayRole).toString());
        itemList << new QStandardItem(item->data(9,Qt::DisplayRole).toString());
        itemList.at(0)->setData(stdWorkTags->at(2),Qt::AccessibleTextRole);
    }
    else
    {
        itemList << new QStandardItem(QString::number(pos));
        itemList << new QStandardItem(item->data(0,Qt::DisplayRole).toString());
        itemList << new QStandardItem(item->data(2,Qt::DisplayRole).toString());
        itemList << new QStandardItem(QString::number(this->get_timesec(item->data(5,Qt::DisplayRole).toString())));
        itemList << new QStandardItem(item->data(1,Qt::DisplayRole).toString());
        itemList << new QStandardItem(item->data(8,Qt::DisplayRole).toString());
        itemList.at(0)->setData(stdWorkTags->at(3),Qt::AccessibleTextRole);
    }

    targetItem->appendRow(itemList);
    return itemList.at(0);
}

void Dialog_workCreator::draw_plotGraphic(int dataPoints)
{
    ui->widget_plot->clearPlottables();
    QVector<double> x_time(dataPoints),x_dist(dataPoints),y_thres(dataPoints);
    double timeSum = 0.0;
    double distSum = 0.0;
    double thres_high = 0.0;
    double timeRange = 0;
    int data = 0;
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

        for(QMap<int,QPair<QString,QVector<double>>>::const_iterator it = plotMap.cbegin(), end = plotMap.cend(); it != end; ++it)
        {
            y_thres[data] = it.value().second.at(0);
            x_time[data] = it.value().second.at(1)/60;
            x_dist[data] = it.value().second.at(2);
            if(y_thres[data] > thres_high)
            {
                thres_high = y_thres[data];
            }
            ++data;
        }

        timeSum = plotMap.last().second.at(1);
        distSum = plotMap.last().second.at(2);

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
    this->set_sumLabel(false);
}

void Dialog_workCreator::set_sumLabel(bool reset)
{
    if(reset)
    {
          ui->label_duration->setText("Time: 00:00:00 - Distance: 0.0 - Stress: 0 - Work: 0");
    }
    else
    {
        ui->label_duration->setText("Time: " + this->set_time(static_cast<int>(ceil(plotMap.last().second.at(1)/10.0)*10.0)) + " - " + "Distance: " + QString::number(set_doubleValue(round(plotMap.last().second.at(2)*10.0)/10.0,false)) + " - " + "Stress: " + QString::number(round(plotMap.last().second.at(3)))+ " - " + "Work: "+QString::number(round_workValue(static_cast<int>(plotMap.last().second.at(4)))));
    }

}

void Dialog_workCreator::clearIntTree()
{
    clearFlag = true;
    ui->treeWidget_workoutTree->clear();
    currentWorkID.clear();

    ui->lineEdit_workoutname->clear();
    ui->comboBox_code->setCurrentIndex(0);
    ui->checkBox_timebased->setChecked(false);
    ui->listWidget_workouts->clearSelection();
    ui->listWidget_group->clearSelection();
    ui->listWidget_phases->clearSelection();
    ui->pushButton_sync->setText(" - 0");

    this->set_sumLabel(true);
    this->control_editPanel(false);
    this->resetAxis();
}

void Dialog_workCreator::set_itemDataRange(QTreeWidgetItem *item)
{
    QCPGraph *graph = ui->widget_plot->graph(0);
    graph->setSelectable(QCP::stMultipleDataRanges);
    QCPDataSelection selection;

    for(QMap<QString,QPair<int,int>>::const_iterator it = plotRange.cbegin(), end = plotRange.cend(); it != end; ++it)
    {
        if(it.key().startsWith(item->data(0,Qt::UserRole).toString()))
        {
            selection.addDataRange(QCPDataRange(it.value().first,it.value().second+1));
        }
    }
    graph->setSelection(selection);
    ui->widget_plot->replot();
}

void Dialog_workCreator::resetAxis()
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->xAxis->setRange(0,10.0);
    ui->widget_plot->xAxis->rescale(true);
    ui->widget_plot->yAxis->setRange(0,50.0);
    ui->widget_plot->xAxis2->setRange(0,5.0);
    ui->widget_plot->yAxis2->setRange(0,50.0);
    ui->widget_plot->replot();
}

void Dialog_workCreator::on_treeWidget_workoutTree_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    currentItem = item;
    ui->widget_plot->graph(0)->setSelection(QCPDataSelection(QCPDataRange(0,0)));
    this->edit_selectedStep(item);
    this->set_itemDataRange(item);
}

void Dialog_workCreator::on_comboBox_sport_currentTextChanged(const QString &sport)
{
    this->get_workouts(sport);
    this->clearIntTree();
}

void Dialog_workCreator::on_pushButton_clear_clicked()
{
    this->clearIntTree();
    this->set_controlButtons(false);
}

void Dialog_workCreator::on_toolButton_update_clicked()
{
    this->control_editPanel(false);
    this->update_selectedStep();
    ui->treeWidget_workoutTree->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_toolButton_cancel_clicked()
{
    this->control_editPanel(false);

    ui->treeWidget_workoutTree->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_toolButton_remove_clicked()
{
    QTreeWidgetItem *deleteItem = currentItem;

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
    this->read_currentWorkTree();
    currentItem = ui->treeWidget_workoutTree->currentItem();
    ui->treeWidget_workoutTree->setCurrentItem(currentItem);
    this->edit_selectedStep(currentItem);
}

QTreeWidgetItem* Dialog_workCreator::move_item(int movement, QTreeWidgetItem *currentItem)
{
    int currentindex = ui->treeWidget_workoutTree->currentIndex().row();

    if(currentItem->parent())
    {
        QTreeWidgetItem *parent = currentItem->parent();
        int index = parent->indexOfChild(currentItem);
        QTreeWidgetItem *childItem = parent->takeChild(index);
        parent->insertChild(index+movement,childItem);
        return childItem;
    }
    else
    {
        QTreeWidgetItem *topItem = ui->treeWidget_workoutTree->takeTopLevelItem(currentindex);
        ui->treeWidget_workoutTree->insertTopLevelItem(currentindex+movement,topItem);

        return topItem;
    }
}

int Dialog_workCreator::round_workValue(int calcWork)
{
    int roundNum = calcWork % 10;

    if(roundNum <= 2)
    {
        calcWork = calcWork - roundNum;
    }
    else if(roundNum > 2 && roundNum < 8)
    {
        calcWork = calcWork + (5-roundNum);
    }
    else if(roundNum >= 8)
    {
        calcWork = calcWork + (10-roundNum);
    }

    return calcWork;
}

void Dialog_workCreator::on_toolButton_up_clicked()
{
    QTreeWidgetItem *currentItem = this->move_item(-1,ui->treeWidget_workoutTree->currentItem());
    ui->treeWidget_workoutTree->setCurrentItem(currentItem);
}

void Dialog_workCreator::on_toolButton_down_clicked()
{
    QTreeWidgetItem *currentItem = this->move_item(1,ui->treeWidget_workoutTree->currentItem());
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
        this->read_currentWorkTree();
    }
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
    this->save_selectedWorkout();
    ui->toolButton_delete->setEnabled(true);
    ui->toolButton_save->setEnabled(false);
}

void Dialog_workCreator::on_toolButton_copy_clicked()
{
    ui->toolButton_copy->setEnabled(false);
}

void Dialog_workCreator::on_toolButton_delete_clicked()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::critical(this,"Delete Std Workout","Delete selected Std Workout?",QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        stdWorkouts->delete_stdWorkout(currentWorkID,true);
        this->clearIntTree();
        this->get_workouts(currentSport);
    }
}

void Dialog_workCreator::on_lineEdit_workoutname_textChanged(const QString &value)
{
    if(!value.isEmpty() && ui->treeWidget_workoutTree->invisibleRootItem()->childCount() > 0)
    {
        ui->toolButton_save->setEnabled(true);
    }

    if(workoutTitle.contains(value))
    {
        ui->toolButton_copy->setEnabled(false);
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

void Dialog_workCreator::on_toolButton_close_clicked()
{
    stdWorkouts->save_workouts();
    done(worksched->get_isUpdated());
}

void Dialog_workCreator::on_pushButton_sync_clicked()
{
    metaValues.resize(9);
    metaValues = stdWorkouts->get_workoutMap()->value(currentSport).value(ui->pushButton_sync->accessibleName());
    updateFrom->setDate(stdWorkoutMapping.firstKey());
    updateFrom->setDateRange(stdWorkoutMapping.firstKey(),stdWorkoutMapping.lastKey());
    updateTo->setDate(stdWorkoutMapping.lastKey());
    updateTo->setDateRange(stdWorkoutMapping.firstKey(),stdWorkoutMapping.lastKey());
    updateProgess->setValue(0);
    updateDialog->exec();
}

void Dialog_workCreator::update_workouts()
{
    QMap<int, QStringList> dayWorkouts;
    QStringList updateList;
    int progress = 100/stdWorkoutMapping.count();
    int updateStep = 1;

    for(QMap<QDate,int>::const_iterator it = stdWorkoutMapping.lowerBound(updateFrom->date()), end = stdWorkoutMapping.upperBound(updateTo->date()); it != end; ++it)
    {
        if(it.key() > QDate::currentDate())
        {
            updateProgess->setValue(progress*updateStep++);

            dayWorkouts = worksched->get_workouts(true,it.key().toString(dateFormat));

            updateList = dayWorkouts.value(it.value());
            updateList.replace(2,metaValues.at(0));
            updateList.replace(3,metaValues.at(1));
            updateList.replace(5,metaValues.at(2));
            updateList.replace(6,metaValues.at(3));
            updateList.replace(7,metaValues.at(4));
            updateList.replace(8,metaValues.at(5));

            dayWorkouts.insert(it.value(),updateList);
            worksched->workoutUpdates.insert(it.key(),dayWorkouts);
        }
    }
    worksched->set_workoutData();
    worksched->set_isUpdated(true);
    QTimer::singleShot(2000,updateProgess,SLOT(reset()));
}

void Dialog_workCreator::set_updateDates(bool pAll)
{
    updateTo->setEnabled(!pAll);
    updateFrom->setEnabled(!pAll);
}

void Dialog_workCreator::on_checkBox_timebased_clicked(bool checked)
{
    ui->timeEdit_lapTime->setEnabled(checked);
    ui->doubleSpinBox_distance->setEnabled(!checked);
}

void Dialog_workCreator::on_toolButton_map_clicked()
{
    int dialog_code;
    Dialog_map dialogMap(this,stdWorkouts,ui->toolButton_map->property("Image").toString());
    dialogMap.setModal(true);
    dialog_code = dialogMap.exec();

    if(dialog_code == QDialog::Accepted)
    {
        ui->toolButton_map->setProperty("Image",stdWorkouts->get_workoutImage());
    }

}

void Dialog_workCreator::on_spinBox_level_valueChanged(int value)
{
    ui->timeEdit_pace->setTime(QTime::fromString(set_time(this->calc_thresPace(value)),shortTime));
    ui->lineEdit_power->setText(QString::number(this->calc_thresPower(value)));
    this->refresh_editStep();
}

void Dialog_workCreator::on_comboBox_levelName_currentTextChanged(const QString &level)
{
    ui->spinBox_level->setMinimum(this->get_thresPercent(level,false));
    ui->spinBox_level->setMaximum(this->get_thresPercent(level,true));
    this->refresh_editStep();
}

void Dialog_workCreator::on_doubleSpinBox_distance_valueChanged(double value)
{
    Q_UNUSED(value)
    this->refresh_editStep();
}

void Dialog_workCreator::on_checkBox_timebased_stateChanged(int checked)
{
    if(checked)
    {
        ui->doubleSpinBox_distance->setEnabled(false);
        ui->timeEdit_lapTime->setEnabled(true);
    }
    else
    {
        ui->doubleSpinBox_distance->setEnabled(true);
        ui->timeEdit_lapTime->setEnabled(false);
    }
}

void Dialog_workCreator::on_timeEdit_lapTime_userTimeChanged(const QTime &time)
{
    Q_UNUSED(time)
    this->refresh_editStep();
}
