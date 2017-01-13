#include "dialog_workcreator.h"
#include "ui_dialog_workcreator.h"

Dialog_workCreator::Dialog_workCreator(QWidget *parent,standardWorkouts *p_workouts) :
    QDialog(parent),
    ui(new Ui::Dialog_workCreator)
{
    ui->setupUi(this);
    stdWorkouts = p_workouts;
    listModel = new QStandardItemModel;
    plotModel = new QStandardItemModel;
    valueModel = new QStandardItemModel;

    metaProxy = new QSortFilterProxyModel;
    metaProxy->setSourceModel(stdWorkouts->workouts_meta);
    stepProxy = new QSortFilterProxyModel;
    stepProxy->setSourceModel(stdWorkouts->workouts_steps);

    editRow <<1<<1<<1<<0<<1<<0<<1<<0;

    isSeries = "Series";
    isGroup = "Group";
    groupList << isGroup << isSeries;
    levelList = settings::get_listValues("Level");
    ui->listWidget_group->addItems(groupList);
    ui->listWidget_phases->addItems(settings::get_listValues("IntEditor"));
    ui->comboBox_sport->addItems(settings::get_listValues("Sport"));
    ui->comboBox_code->addItems(settings::get_listValues("WorkoutCode"));
    clearFlag = false;

    modelHeader << "Phase" << "Level" << "Threshold %" << "Value" << "Time" << "TSS" << "Distance" << "Repeats";
    ui->treeWidget_intervall->setHeaderLabels(modelHeader);
    ui->treeWidget_intervall->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidget_intervall->setAcceptDrops(true);
    ui->treeWidget_intervall->setDragEnabled(true);
    ui->treeWidget_intervall->setDragDropMode(QAbstractItemView::DragDrop);
    ui->treeWidget_intervall->setEnabled(true);

    ui->frame_edit->setVisible(false);
    ui->listView_values->setModel(valueModel);
    ui->listView_values->setItemDelegate(&edit_del);
    ui->label_head->setText("Add Phase");
}

Dialog_workCreator::~Dialog_workCreator()
{
    delete metaProxy;
    delete stepProxy;
    delete plotModel;
    delete valueModel;
    delete listModel;
    delete ui;
}

void Dialog_workCreator::on_pushButton_close_clicked()
{
    reject();
}

void Dialog_workCreator::get_workouts(QString sport)
{
    QString workID,workTitle,listString;
    workoutMap.clear();
    listModel->clear();
    metaProxy->invalidate();
    metaProxy->setFilterFixedString(sport);
    metaProxy->setFilterKeyColumn(0);

    listModel->setColumnCount(2);
    listModel->setRowCount(metaProxy->rowCount());

    for(int i = 0; i < metaProxy->rowCount(); ++i)
    {
        workID = metaProxy->data(metaProxy->index(i,1)).toString();
        workTitle = metaProxy->data(metaProxy->index(i,3)).toString();
        listString = metaProxy->data(metaProxy->index(i,2)).toString() + " - " + workTitle;
        listModel->setData(listModel->index(i,0,QModelIndex()),listString);
        listModel->setData(listModel->index(i,1,QModelIndex()),workID);
        workoutMap.insert(workID,workTitle);
    }
    listModel->sort(0);

    ui->listView_workouts->setModel(listModel);
    ui->listView_workouts->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void Dialog_workCreator::open_stdWorkout(QString workID)
{
    clearFlag = true;
    ui->treeWidget_intervall->clearFocus();
    if(ui->treeWidget_intervall->topLevelItemCount() > 0) ui->treeWidget_intervall->clear();

    QStringList valueList;
    QString parentItem,partName,thresValue,stepTime,empty = "";
    double percent;
    double currDist;
    bool isBike = false;
    bool isSwim = false;
    stepProxy->setFilterRegExp("\\b"+workID+"\\b");
    stepProxy->setFilterKeyColumn(0);

    if(current_sport == settings::isSwim) isSwim = true;
    if(current_sport == settings::isBike) isBike = true;

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
                      << stepProxy->data(stepProxy->index(i,6)).toString()
                      << stepProxy->data(stepProxy->index(i,7)).toString();
        }
        else
        {
            percent = stepProxy->data(stepProxy->index(i,4)).toDouble();
            thresValue = this->calc_threshold(current_sport,currThres,percent);
            stepTime = stepProxy->data(stepProxy->index(i,5)).toString();

            if(isBike)
            {
                currDist = this->calc_distance(stepTime,this->get_timesec(this->threstopace(threshold_pace,percent)));
            }
            else
            {
                currDist = this->calc_distance(stepTime,this->get_timesec(thresValue));
                if(isSwim) currDist = currDist/10.0;
            }

            valueList << stepProxy->data(stepProxy->index(i,2)).toString()
                      << stepProxy->data(stepProxy->index(i,3)).toString()
                      << QString::number(percent)
                      << thresValue
                      << stepTime
                      << QString::number(this->estimate_stress(ui->comboBox_sport->currentText(),thresValue,this->get_timesec(stepTime)))
                      << QString::number(this->set_doubleValue(currDist,true))
                      << stepProxy->data(stepProxy->index(i,7)).toString();
        }
        QTreeWidgetItem *item = new QTreeWidgetItem(valueList);
        this->set_backColor(item);
        parentItem = stepProxy->data(stepProxy->index(i,8)).toString();

        if(parentItem.contains(isGroup) || parentItem.contains(isSeries))
        {
            QList<QTreeWidgetItem*>   pItem = ui->treeWidget_intervall->findItems(parentItem,Qt::MatchExactly | Qt::MatchRecursive,0);
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
}
void Dialog_workCreator::save_workout()
{
    int counter = 1;
    int workcounter;
    QString workID,sport,phase,subphase,worktime,currWorkID;
    QStringList workoutValues,existWorkIDs,sportWorkIDs;
    sport = ui->comboBox_sport->currentText();
    QTreeWidgetItem *currentItem;
    QStandardItemModel *workModel;
    metaProxy->setFilterFixedString(sport);
    metaProxy->setFilterKeyColumn(0);

    existWorkIDs = stdWorkouts->get_workoutIds();

    for(int i = 0; i < existWorkIDs.count(); ++i)
    {
        currWorkID = existWorkIDs.at(i);
        if(currWorkID.contains(sport))
        {
            sportWorkIDs << currWorkID;
        }
    }

    worktime = this->set_time(static_cast<int>(ceil(time_sum)*60));

    //Update Workout -> delete first
    if(current_workID.isEmpty())
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
        current_workID = workID;
    }
    else
    {
        stdWorkouts->delete_stdWorkout(current_workID,false);
        workID = current_workID;
    }

    if(worktime.length() == 5)
    {
        worktime = "00:"+worktime;
    }

    //Metadaten
    workoutValues << current_sport
                  << workID
                  << ui->comboBox_code->currentText()
                  << ui->lineEdit_workoutname->text()
                  << worktime
                  << QString::number(dist_sum)
                  << QString::number(round(stress_sum));

     workModel = stdWorkouts->workouts_meta;
     this->save_workout_values(workoutValues,workModel);

    //Intervalldaten
     workModel = stdWorkouts->workouts_steps;
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
                  << get_treeValue(c_item,0,0,4,0)   //time
                  << get_treeValue(c_item,0,0,6,0)   //dist
                  << get_treeValue(c_item,0,0,7,0)   //repeats
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
                          << get_treeValue(c_item,c_child,0,4,1)
                          << get_treeValue(c_item,c_child,0,6,1)
                          << get_treeValue(c_item,c_child,0,7,1)
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
                                  << get_treeValue(c_item,c_child,subchild,4,2)
                                  << get_treeValue(c_item,c_child,subchild,6,2)
                                  << get_treeValue(c_item,c_child,subchild,7,2)
                                  << subphase;

                         this->save_workout_values(workoutValues,workModel);
                     }
                 }

             }
         }
     }
     this->get_workouts(current_sport);
     stdWorkouts->save_stdWorkouts();
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
}

void Dialog_workCreator::set_defaultData(QTreeWidgetItem *item, bool hasValues)
{
    currentItem = item;
    QString defaultTime = "05:00";
    int level = 1;
    double defaultDist;
    double percent = this->get_thresPercent(current_sport,levelList.at(level),false);
    QString threshold = this->calc_threshold(current_sport,currThres,percent);

    if(current_sport == settings::isSwim)
    {
        if(item->data(0,Qt::DisplayRole).toString() == settings::get_generalValue("breakname"))
        {
            defaultDist = percent = 0.0;
            threshold = "00:00";
            defaultTime = "00:30";
            level = 0;
        }
        else
        {
            defaultDist = 0.1;
            defaultTime = this->calc_duration(current_sport,defaultDist,threshold);
        }
    }
    else
    {
        defaultDist = this->calc_distance(defaultTime,static_cast<double>(this->get_timesec(threstopace(threshold_pace,percent))));
    }

    if(hasValues)
    {
        item->setData(1,Qt::EditRole,levelList.at(level));
        item->setData(2,Qt::EditRole,percent);
        item->setData(3,Qt::EditRole,threshold);
        item->setData(4,Qt::EditRole,defaultTime);
        item->setData(5,Qt::EditRole,this->estimate_stress(current_sport,threshold,this->get_timesec(defaultTime)));
        item->setData(6,Qt::EditRole,defaultDist);
        item->setData(7,Qt::EditRole,"");
    }
    else
    {
        QString itemName = item->data(0,Qt::DisplayRole).toString();
        QList<QTreeWidgetItem*> groupCount = ui->treeWidget_intervall->findItems(itemName,Qt::MatchContains,0);
        itemName = itemName +"-"+QString::number(groupCount.count());
        item->setData(0,Qt::EditRole,itemName);
        item->setData(7,Qt::EditRole,2);
    }
    this->set_backColor(item);
    ui->treeWidget_intervall->expandAll();
    this->set_plotModel();
}

void Dialog_workCreator::set_backColor(QTreeWidgetItem *item)
{
    QColor itemColor;
    QString itemName = item->data(0,Qt::DisplayRole).toString();

    itemColor = settings::get_itemColor(item->data(1,Qt::DisplayRole).toString());
    if(itemName.contains(isGroup)) itemColor = QColor(Qt::darkGray);
    if(itemName.contains(isSeries)) itemColor = QColor(Qt::lightGray);

    for(int i = 0; i < item->columnCount(); ++i)
    {
        item->setBackground(i,QBrush(itemColor));
    }
}

void Dialog_workCreator::show_editItem(QTreeWidgetItem *item)
{
    int rowCount = 8;
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
        valueModel->setData(valueModel->index(1,0,QModelIndex()),item->data(7,Qt::DisplayRole));
        valueModel->setData(valueModel->index(1,2,QModelIndex()),true);
    }
    else
    {
        valueModel->setRowCount(rowCount);
        for(int i = 0; i < item->columnCount();++i)
        {
            valueModel->setData(valueModel->index(i,0,QModelIndex()),item->data(i,Qt::DisplayRole));
            valueModel->setData(valueModel->index(i,1,QModelIndex()),0);
            valueModel->setData(valueModel->index(i,2,QModelIndex()),editRow[i]);
        }
        valueModel->setData(valueModel->index(2,1,QModelIndex()),levelList.indexOf(valueModel->data(valueModel->index(1,0)).toString()));
        if(current_sport == settings::isBike)
        {
            valueModel->setData(valueModel->index(7,0,QModelIndex()),this->calc_lapSpeed(current_sport,get_timesec(threstopace(threshold_pace,item->data(2,Qt::DisplayRole).toDouble()))));
        }
        else
        {
            valueModel->setData(valueModel->index(7,0,QModelIndex()),this->calc_lapSpeed(current_sport,static_cast<double>(this->get_timesec(item->data(3,Qt::DisplayRole).toString()))));
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
        item->setData(7,Qt::EditRole,valueModel->data(valueModel->index(1,0)));
    }
    else
    {
        for(int i = 0; i < valueModel->rowCount()-1;++i)
        {
            item->setData(i,Qt::EditRole,valueModel->data(valueModel->index(i,0)));
        }
    }
    this->set_backColor(item);
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

    ui->lineEdit_workoutname->clear();
    ui->comboBox_code->setCurrentIndex(0);
}

void Dialog_workCreator::set_plotModel()
{
    plotModel->clear();
    plotModel->setColumnCount(4);
    //plotModel->setHorizontalHeaderLabels(modelHeader);
    int parentReps = 0;
    int childReps = 0;
    int childCount = 0;
    int subchildCount = 0;
    bool isload = false;
    QString phase,subphase;
    QTreeWidgetItem *topItem,*childItem;

    for(int c_item = 0; c_item < ui->treeWidget_intervall->topLevelItemCount(); ++c_item)
    {
        //TopItems
        topItem = ui->treeWidget_intervall->topLevelItem(c_item);
        phase = topItem->data(0,Qt::DisplayRole).toString();
        childCount = topItem->childCount();

        qDebug() << phase+"-Top";

        if(childCount > 0)
        {
            if(phase.contains(isGroup))
            {
                parentReps = topItem->data(7,Qt::DisplayRole).toInt();

                for(int c_parent = 0; c_parent < parentReps; ++c_parent)
                {
                    for(int c_childs = 0; c_childs < childCount ; ++c_childs)
                    {
                        childItem = ui->treeWidget_intervall->topLevelItem(c_item)->child(c_childs);
                        subchildCount = childItem->childCount();
                        subphase = childItem->data(0,Qt::DisplayRole).toString();

                        if(subchildCount > 0)
                        {
                            if(subphase.contains(isSeries))
                            {
                                childReps = childItem->data(7,Qt::DisplayRole).toInt();
                                for(int c_child = 0; c_child < childReps; ++c_child)
                                {
                                    for(int c_subchilds = 0; c_subchilds < subchildCount; ++c_subchilds)
                                    {
                                        this->add_to_plot(ui->treeWidget_intervall->topLevelItem(c_item)->child(c_childs)->child(c_subchilds));
                                    }
                                }
                                for(int i = 0; i < childCount; ++i)
                                {
                                    qDebug() << childItem->data(0,Qt::DisplayRole).toString()+"-"+QString::number(i);
                                }
                            }
                        }
                        else
                        {
                            this->add_to_plot(childItem);
                        }
                    }
                }
                for(int i = 0; i < childCount; ++i)
                {
                    qDebug() << topItem->data(0,Qt::DisplayRole).toString()+"-"+QString::number(i);
                }
            }
            else if(phase.contains(isSeries))
            {
                parentReps = topItem->data(7,Qt::DisplayRole).toInt();

                for(int c_parent = 0; c_parent < parentReps; ++c_parent)
                {
                    for(int c_childs = 0; c_childs < childCount ; ++c_childs)
                    {
                        this->add_to_plot(ui->treeWidget_intervall->topLevelItem(c_item)->child(c_childs));
                    }
                }
                for(int i = 0; i < childCount; ++i)
                {
                    qDebug() << topItem->data(0,Qt::DisplayRole).toString()+"-"+QString::number(i);
                }
            }
        }
        else
        {
            //Items without connection
            if(!phase.contains(isSeries) && !phase.contains(isGroup))
            {
                this->add_to_plot(topItem);
            }
        }
        if(c_item == ui->treeWidget_intervall->topLevelItemCount()-1) isload = true;
    }
    if(isload) this->set_plotGraphic(plotModel->rowCount());
}

void Dialog_workCreator::add_to_plot(QTreeWidgetItem *item)
{
    double time_sum = 0;
    double dist_sum = 0;
    double stress_sum = 0.0;
    int row = plotModel->rowCount();

    if(row != 0)
    {
        time_sum = plotModel->data(plotModel->index(row-1,1,QModelIndex())).toDouble();
        dist_sum = plotModel->data(plotModel->index(row-1,2,QModelIndex())).toDouble();
        stress_sum = plotModel->data(plotModel->index(row-1,3,QModelIndex())).toDouble();
    }

    plotModel->insertRows(row,1,QModelIndex());
    plotModel->setData(plotModel->index(row,0,QModelIndex()),item->data(2,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row,1,QModelIndex()),time_sum + (this->get_timesec(item->data(4,Qt::DisplayRole).toString()) / 60.0));
    plotModel->setData(plotModel->index(row,2,QModelIndex()),dist_sum + item->data(6,Qt::DisplayRole).toDouble());
    plotModel->setData(plotModel->index(row,3,QModelIndex()),stress_sum + item->data(5,Qt::DisplayRole).toDouble());
}

void Dialog_workCreator::set_plotGraphic(int c_ints)
{
    ui->widget_plot->clearPlottables();
    int int_loops = c_ints;
    int offset = 1;
    QVector<double> x_time(int_loops+offset),x_dist(int_loops+offset),y_thres(int_loops+offset);
    time_sum = 0.0;
    dist_sum = 0.0;
    stress_sum = 0.0;
    double thres_high = 0.0;

    ui->widget_plot->setInteraction(QCP::iSelectPlottables);
    QCPSelectionDecorator *lineDec = new QCPSelectionDecorator;
    lineDec->setPen(QPen(QColor(255,0,0)));
    lineDec->setBrush(QColor(255,0,0,50));
    QCPGraph *workout_line = ui->widget_plot->addGraph();
    workout_line->setSelectionDecorator(lineDec);

    if(c_ints != 0)
    {
        workout_line->setPen(QPen(Qt::blue));
        workout_line->setBrush(QColor(0,170,255,60));
        workout_line->setLineStyle((QCPGraph::lsStepLeft));
        workout_line->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QPen(Qt::red),QBrush(Qt::white), 5));

        x_time[0] = 0.01;
        y_thres[0] = plotModel->data(plotModel->index(0,0,QModelIndex())).toDouble();

        for (int i=0,counter=1 ; i < int_loops; ++i,++counter)
        {
             y_thres[counter-1] = plotModel->data(plotModel->index(i,0,QModelIndex())).toDouble();
             y_thres[counter] = y_thres[counter-1];
             x_time[counter] = plotModel->data(plotModel->index(i,1,QModelIndex())).toDouble();
             x_dist[counter] = plotModel->data(plotModel->index(i,2,QModelIndex())).toDouble();
             if(y_thres[counter] > thres_high)
             {
                 thres_high = y_thres[counter];
             }
        }
        time_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,1,QModelIndex())).toDouble();
        dist_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,2,QModelIndex())).toDouble();
        stress_sum = plotModel->data(plotModel->index(plotModel->rowCount()-1,3,QModelIndex())).toDouble();
        workout_line->setData(x_time,y_thres);
        ui->widget_plot->graph()->rescaleAxes(true);
        ui->widget_plot->xAxis->setRange(0,time_sum+((time_sum/2)*0.1));
        ui->widget_plot->xAxis2->setRange(0,dist_sum+((dist_sum/2)*0.1));
        ui->widget_plot->xAxis2->setVisible(true);
        ui->widget_plot->yAxis->setRange(0,thres_high+20.0);
    }

    ui->widget_plot->xAxis->setTicks(true);
    ui->widget_plot->yAxis->setTicks(true);
    ui->widget_plot->xAxis->setLabel("Time - Minutes");
    ui->widget_plot->xAxis2->setLabel("Distance - KM");
    ui->widget_plot->yAxis->setLabel("Threshold %");
    ui->widget_plot->xAxis->setTickLabels(true);
    ui->widget_plot->yAxis->setTickLabels(true);
    ui->widget_plot->replot();

    ui->label_duration->setText("Time:" + this->set_time(static_cast<int>(ceil(time_sum))) + " - " + "Distance:" + QString::number(dist_sum) + " - " + "Stress:" + QString::number(round(stress_sum)));
}

void Dialog_workCreator::set_selectData(QTreeWidgetItem *item)
{
    QCPGraph *graph = ui->widget_plot->graph(0);
    QString itemIdent = item->data(0,Qt::DisplayRole).toString();

    if(item->parent() == nullptr)
    {
        qDebug() << item->data(0,Qt::DisplayRole).toString()+"-Top" << item->data(7,Qt::DisplayRole).toInt();
    }
    else
    {
        qDebug() << item->parent()->data(0,Qt::DisplayRole).toString()+"-"+QString::number(ui->treeWidget_intervall->currentIndex().row());
    }

    /*
    QCPDataSelection selection;

    if(itemIdent.contains(isGroup) || itemIdent.contains(isSeries))
    {
        graph->setSelectable(QCP::stDataRange);
        QCPDataRange dataRange;
        dataRange.setBegin(pos);
        dataRange.setEnd(pos*repeat);
        selection.addDataRange(dataRange);
    }
    else
    {
        graph->setSelectable(QCP::stMultipleDataRanges);
        for(int i = 0; i < repeat; ++i)
        {
            QCPDataRange *dataRange = new QCPDataRange();
            //selection.addDataRange(dataRange);
        }
    }

    graph->selectionChanged(selection);
    ui->widget_plot->replot();
    */
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
    current_sport = sport;
    edit_del.sport = sport;
    this->clearIntTree();
    this->get_workouts(sport);
    this->set_sport_threshold(sport);
}

void Dialog_workCreator::set_editRow(QString sport)
{
    if(sport == settings::isSwim)
    {
        editRow[2] = 1;
        editRow[4] = 0;
        editRow[6] = 1;
    }
    else if(sport == settings::isBike || sport == settings::isRun)
    {
        editRow[2] = 1;
        editRow[4] = 1;
        editRow[6] = 1;
    }
    else if(sport == settings::isStrength)
    {
        editRow[2] = 1;
        editRow[4] = 1;
        editRow[6] = 0;
    }
    else
    {
        editRow[2] = 0;
        editRow[4] = 1;
        editRow[6] = 1;
    }
}

void Dialog_workCreator::set_sport_threshold(QString sport)
{
    if(sport == settings::isAlt || sport == settings::isOther)
    {
        threshold_pace = 0;
        threshold_power = 0.0;
        ui->label_threshold->setText("-");
        currThres = threshold_power;

    }
    if(sport == settings::isSwim)
    {
       threshold_power = settings::get_thresValue("swimpower");
       threshold_pace = settings::get_thresValue("swimpace");
       ui->label_threshold->setText(this->set_time(threshold_pace) + " /100m");
       currThres = threshold_pace;
    }
    if(sport == settings::isBike)
    {
       threshold_power = settings::get_thresValue("bikepower");
       threshold_pace = settings::get_thresValue("bikepace");
       ui->label_threshold->setText(QString::number(threshold_power) + " Watt");
       currThres = threshold_power;
    }
    if(sport == settings::isRun)
    {
       threshold_power = settings::get_thresValue("runpower");
       threshold_pace = settings::get_thresValue("runpace");
       ui->label_threshold->setText(this->set_time(threshold_pace) + " /km");
       currThres = threshold_pace;
    }
    if(sport == settings::isStrength)
    {
       threshold_power = settings::get_thresValue("stgpower");
       threshold_pace = 0;
       ui->label_threshold->setText(QString::number(threshold_power) + " Watt");
       currThres = threshold_power;
    }
    edit_del.currThres = currThres;
    edit_del.thresPace = threshold_pace;
    this->set_editRow(sport);
}

void Dialog_workCreator::on_listView_workouts_clicked(const QModelIndex &index)
{
    QStringList workoutTitle = listModel->data(listModel->index(index.row(),0)).toString().split("-");
    QString workoutID = listModel->data(listModel->index(index.row(),1)).toString();
    QString workCode = workoutTitle.first();
    QString workTitle = workoutTitle.last();
    current_workID = workoutID;
    ui->comboBox_code->setCurrentText(workCode.replace(" ",""));
    ui->lineEdit_workoutname->setText(workTitle.replace(" ",""));
    this->open_stdWorkout(workoutID);
}

void Dialog_workCreator::on_pushButton_clear_clicked()
{
    this->clearIntTree();
    this->control_editPanel(false);
}

void Dialog_workCreator::on_toolButton_update_clicked()
{
    this->control_editPanel(false);
    this->set_itemData(currentItem);
    this->set_plotModel();
}

void Dialog_workCreator::on_toolButton_remove_clicked()
{
    if(ui->treeWidget_intervall->topLevelItemCount() == 1)
    {
        //ui->treeWidget_intervall->clear();
        this->clearIntTree();
        this->control_editPanel(false);
    }
    else
    {
        if(currentItem->childCount() > 0)
        {
            while(currentItem->childCount() > 0)
            {
                for(int i = 0; i < currentItem->childCount(); ++i)
                {
                    delete currentItem->child(i);
                }
            }
        }
        delete currentItem;
        this->show_editItem(ui->treeWidget_intervall->currentItem());
    }
    this->set_plotModel();
}

void Dialog_workCreator::move_item(bool up)
{
    int currentindex = ui->treeWidget_intervall->currentIndex().row();

    QTreeWidgetItem *currentItem = ui->treeWidget_intervall->currentItem();

    if(currentItem->parent())
    {
        QTreeWidgetItem *parent = currentItem->parent();
        int index = parent->indexOfChild(currentItem);
        QTreeWidgetItem *child = parent->takeChild(index);
        if(up)
        {
            parent->insertChild(index-1,child);
            parent->setExpanded(true);
        }
        else
        {
            parent->insertChild(index+1,child);
            parent->setExpanded(true);
        }
    }
    else
    {
        ui->treeWidget_intervall->takeTopLevelItem(currentindex);
        if(up)
        {
            ui->treeWidget_intervall->insertTopLevelItem(currentindex-1,currentItem);
        }
        else
        {
            ui->treeWidget_intervall->insertTopLevelItem(currentindex+1,currentItem);
        }
        ui->treeWidget_intervall->setCurrentItem(currentItem);
    }
}

void Dialog_workCreator::on_toolButton_up_clicked()
{
    this->move_item(true);
    this->set_plotModel();
}

void Dialog_workCreator::on_toolButton_down_clicked()
{
    this->move_item(false);
    this->set_plotModel();
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
    ui->toolButton_copy->setEnabled(true);
    ui->toolButton_delete->setEnabled(true);
}

void Dialog_workCreator::on_toolButton_copy_clicked()
{
    current_workID = QString();
    this->save_workout();
}

void Dialog_workCreator::on_toolButton_delete_clicked()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::critical(this,"Delete Std Workout","Delete selected Std Workout?",QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        stdWorkouts->delete_stdWorkout(current_workID,true);
        current_workID = QString();
        this->clearIntTree();
        this->get_workouts(current_sport);
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
