#include <QDebug>
#include "dialog_inteditor.h"
#include "ui_dialog_inteditor.h"

Dialog_inteditor::Dialog_inteditor(QWidget *parent, settings *p_setting,standardWorkouts *p_workouts) :
    QDialog(parent),
    ui(new Ui::Dialog_inteditor)
{
    ui->setupUi(this);
    ui->treeWidget_planer->setAcceptDrops(true);
    ui->treeWidget_planer->setDragEnabled(true);
    ui->treeWidget_planer->setDragDropMode(QAbstractItemView::InternalMove);
    pop_settings = p_setting;
    stdWorkouts = p_workouts;
    powerlist = pop_settings->get_powerList();
    model_header << "Phase" << "Level" << "Threshold %" << "Value" << "Time" << "TSS" << "Distance" << "Repeats";
    current_workID = QString();
    ui->treeWidget_planer->setColumnCount(7);
    ui->treeWidget_planer->setHeaderHidden(false);
    ui->treeWidget_planer->setHeaderLabels(model_header);
    ui->treeWidget_planer->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    isSeries = "Series";
    isGroup = "Group";
    clearFlag = false;
    powerfactor.resize(7);
    for(int i = 1; i <= 7; ++i)
    {
        powerfactor[i-1] = 2.5 * i;
    }

    this->init_model();
    this->set_plot_graphic(0);

    ui->comboBox_topitem->addItem(isGroup);
    ui->comboBox_topitem->addItem(isSeries);
    ui->comboBox_level->addItems(pop_settings->get_levelList());
    ui->comboBox_sport->addItems(pop_settings->get_sportList());
    ui->comboBox_part->addItems(pop_settings->get_intPlanerList());
    ui->comboBox_code->addItems(pop_settings->get_codeList());
    ui->comboBox_reps->addItem("-");
    for(int i = 2; i <= 20; ++i)
    {
        ui->comboBox_reps->addItem(QString::number(i));
    }

    ui->pushButton_down->setEnabled(false);
    ui->pushButton_up->setEnabled(false);
    ui->pushButton_save_std->setEnabled(false);
    ui->pushButton_copy_std->setEnabled(false);
    ui->pushButton_delete_std->setEnabled(false);
    this->set_pushbutton(false);
}

Dialog_inteditor::~Dialog_inteditor()
{
    delete ui;
}

void Dialog_inteditor::init_model()
{
    plot_model = new QStandardItemModel();
    plot_model->setColumnCount(4);
    this->get_workouts(ui->comboBox_sport->currentText());
}

void Dialog_inteditor::refresh_model()
{
    this->set_plot_model();
}

void Dialog_inteditor::reset_values()
{
    ui->comboBox_level->setCurrentIndex(0);
    ui->spinBox_threshold->setValue(0);
    ui->timeEdit_int_time->setTime(QTime::fromString("00:00","mm:ss"));
    ui->doubleSpinBox_int_dist->setValue(0.0);
    if(current_workID.isEmpty()) ui->comboBox_code->setCurrentIndex(0);
    this->set_pushbutton(false);
    this->set_plot_model();
    clearFlag = false;
}

void Dialog_inteditor::reset_workoutInfo()
{
    current_workID = QString();
    ui->pushButton_save_std->setEnabled(false);
    ui->pushButton_copy_std->setEnabled(false);
    ui->pushButton_delete_std->setEnabled(false);
    ui->lineEdit_workoutname->setText("");
}

void Dialog_inteditor::set_components(QString select)
{
    if(select.contains(isSeries) || select.contains(isGroup))
    {
        this->reset_values();
        ui->comboBox_level->setEnabled(false);
        ui->timeEdit_int_time->setEnabled(false);
        ui->spinBox_threshold->setEnabled(false);
        ui->doubleSpinBox_int_dist->setEnabled(false);
    }
    else
    {
        ui->comboBox_level->setEnabled(true);
        ui->timeEdit_int_time->setEnabled(true);
        ui->spinBox_threshold->setEnabled(true);
        ui->doubleSpinBox_int_dist->setEnabled(true);
    }
}

void Dialog_inteditor::set_pushbutton(bool boolValue)
{
    ui->pushButton_delete->setEnabled(boolValue);
    ui->pushButton_edit->setEnabled(boolValue);
}

void Dialog_inteditor::get_workouts(QString sport)
{
    QStandardItemModel *std_model;
    std_model = stdWorkouts->workouts_meta;

    QModelIndex model_index,index;
    QList<QStandardItem*> list = std_model->findItems(sport,Qt::MatchExactly,0);
    workout_model = new QStandardItemModel(list.count(),6,this);

    for(int i = 0; i < list.count(); ++i)
    {
        QString listString;
        model_index = std_model->indexFromItem(list.at(i));
        index = workout_model->index(i,0,QModelIndex());
        listString = std_model->item(model_index.row(),2)->text() + " - " + std_model->item(model_index.row(),3)->text();
        workout_model->setData(index,listString);

        workout_model->setData(workout_model->index(i,1,QModelIndex()),std_model->item(model_index.row(),0)->text());
        workout_model->setData(workout_model->index(i,2,QModelIndex()),std_model->item(model_index.row(),1)->text());
        workout_model->setData(workout_model->index(i,3,QModelIndex()),std_model->item(model_index.row(),4)->text());
        workout_model->setData(workout_model->index(i,4,QModelIndex()),std_model->item(model_index.row(),5)->text());
        workout_model->setData(workout_model->index(i,5,QModelIndex()),std_model->item(model_index.row(),6)->text());
    }
    workout_model->sort(0);
    ui->listView_workouts->setModel(workout_model);
}

void Dialog_inteditor::set_sport_threshold(QString sport)
{
    if(sport == pop_settings->isAlt || sport == pop_settings->isOther )
    {
        threshold_pace = 0;
        threshold_power = 0.0;
        ui->label_sportThreshold->setText("-");
    }
    if(sport == pop_settings->isSwim)
    {
       threshold_power = powerlist[0];
       threshold_pace = pop_settings->get_timesec(pop_settings->get_paceList().at(0));
       ui->label_sportThreshold->setText(pop_settings->set_time(threshold_pace) + " /100m");
    }
    if(sport == pop_settings->isBike)
    {
       threshold_power = powerlist[1];
       threshold_pace = pop_settings->get_timesec(pop_settings->get_paceList().at(1));
       ui->label_sportThreshold->setText(QString::number(threshold_power) + " Watt");
    }
    if(sport == pop_settings->isRun)
    {
       threshold_power = powerlist[2];
       threshold_pace = pop_settings->get_timesec(pop_settings->get_paceList().at(2));
       ui->label_sportThreshold->setText(pop_settings->set_time(threshold_pace) + " /km");
    }
    if(sport == pop_settings->isStrength)
    {
       threshold_power = powerlist[3];
       threshold_pace = 0;
       ui->label_sportThreshold->setText(QString::number(threshold_power) + " Watt");
    }

    this->calc_threshold(static_cast<double>(ui->spinBox_threshold->value()));
}

QString Dialog_inteditor::calc_threshold(double percent)
{
    QString thresValue;
    if(percent > 0)
    {
        if(current_sport == pop_settings->isAlt || current_sport == pop_settings->isOther )
        {
            ui->label_thresValue->setText("---");
        }
        if(current_sport == pop_settings->isSwim)
        {
            ui->label_thresValue->setText(pop_settings->set_time(static_cast<int>(round(threshold_pace / (percent/100.0)))));
            thresValue = pop_settings->set_time(static_cast<int>(round(threshold_pace / (percent/100.0))));
            current_power = threshold_power * (percent/100);
        }
        if(current_sport == pop_settings->isBike)
        {
            ui->label_thresValue->setText(QString::number(threshold_power * (percent/100)));
            thresValue = QString::number(threshold_power * (percent/100));
            current_pace = static_cast<int>(round(3600/(((3600/threshold_pace)*0.7) + (speedfactor * (percent/100.0)))));
        }
        if(current_sport == pop_settings->isRun)
        {
            ui->label_thresValue->setText(pop_settings->set_time(static_cast<int>(round(threshold_pace / (percent/100.0)))));
            thresValue = pop_settings->set_time(static_cast<int>(round(threshold_pace / (percent/100.0))));
            current_power = threshold_power * (percent/100);
        }
        if(current_sport == pop_settings->isStrength)
        {
            ui->label_thresValue->setText(QString::number(threshold_power * (percent/100)));
            thresValue = QString::number(threshold_power * (percent/100));
            current_pace = 0;
        }
    }
    else
    {
        ui->label_thresValue->setText("0");
    }
    return thresValue;
}

void Dialog_inteditor::calc_distance(QString sport)
{
    if(ui->timeEdit_int_time->time() > QTime::fromString("00:00"))
    {
        if(sport == pop_settings->isSwim)
        {
            if(ui->comboBox_level->currentIndex() == 0)
            {
                ui->doubleSpinBox_int_dist->setValue(0.0);
            }
            else
            {
                ui->doubleSpinBox_int_dist->setValue((static_cast<double>(pop_settings->get_timesec(ui->timeEdit_int_time->text())) / static_cast<double>(pop_settings->get_timesec(ui->label_thresValue->text()))) / 10.0);
            }
        }
        if(sport == pop_settings->isBike)
        {
            ui->doubleSpinBox_int_dist->setValue(static_cast<double>(pop_settings->get_timesec(ui->timeEdit_int_time->text()) / static_cast<double>(current_pace)));
        }
        if(sport == pop_settings->isRun)
        {
            ui->doubleSpinBox_int_dist->setValue(static_cast<double>(pop_settings->get_timesec(ui->timeEdit_int_time->text()) / static_cast<double>(pop_settings->get_timesec(ui->label_thresValue->text()))));
        }
    }
}

int Dialog_inteditor::get_group_count()
{
    QList<QTreeWidgetItem*> groupList = ui->treeWidget_planer->findItems(isGroup,Qt::MatchContains,0);

    return groupList.count()+1;
}

int Dialog_inteditor::get_series_count()
{
    QList<QTreeWidgetItem*> seriesList = ui->treeWidget_planer->findItems(isSeries,Qt::MatchRecursive | Qt::MatchContains,0);

    return seriesList.count()+1;
}

void Dialog_inteditor::load_item(QTreeWidgetItem *item)
{
    if(item->data(0,Qt::DisplayRole).toString().contains(isGroup) || item->data(0,Qt::DisplayRole).toString().contains(isSeries))
    {
        this->reset_values();
        ui->comboBox_topitem->setCurrentText(item->data(0,Qt::DisplayRole).toString().split("-").first());
        ui->comboBox_reps->setCurrentText(item->data(7,Qt::DisplayRole).toString());
        this->set_pushbutton(true);
    }
    else
    {
        ui->comboBox_part->setCurrentText(item->data(0,Qt::DisplayRole).toString());
        ui->comboBox_level->setCurrentText(item->data(1,Qt::DisplayRole).toString());
        ui->spinBox_threshold->setValue(item->data(2,Qt::DisplayRole).toInt());
        ui->timeEdit_int_time->setTime(QTime::fromString(item->data(4,Qt::DisplayRole).toString(),"mm:ss"));
        ui->doubleSpinBox_int_dist->setValue(item->data(6,Qt::DisplayRole).toDouble());
    }
}

void Dialog_inteditor::edit_item(QTreeWidgetItem *item)
{
    if(item->data(0,Qt::DisplayRole).toString().contains(isGroup) || item->data(0,Qt::DisplayRole).toString().contains(isSeries))
    {
         item->setData(7,Qt::DisplayRole,ui->comboBox_reps->currentText());
    }
    else
    {
        item->setData(0,Qt::DisplayRole,ui->comboBox_part->currentText());
        item->setData(1,Qt::DisplayRole,ui->comboBox_level->currentText());
        item->setData(2,Qt::DisplayRole,QString::number(ui->spinBox_threshold->text().toDouble()));
        item->setData(3,Qt::DisplayRole,ui->label_thresValue->text());
        item->setData(4,Qt::DisplayRole,ui->timeEdit_int_time->time().toString("mm:ss"));
        item->setData(5,Qt::DisplayRole,QString::number(pop_settings->estimate_stress(ui->comboBox_sport->currentText(),ui->label_thresValue->text(),ui->timeEdit_int_time->time())));
        item->setData(6,Qt::DisplayRole,QString::number(ui->doubleSpinBox_int_dist->value()));
    }
    this->set_plot_model();
}

void Dialog_inteditor::add_topItem(QString label)
{
    int counter = 0;
    if(label == isSeries)
    {
        counter = this->get_series_count();
    }
    else
    {
        counter = this->get_group_count();
    }

    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0,label+"-"+QString::number(counter));
    item->setText(7,ui->comboBox_reps->currentText());
    ui->treeWidget_planer->addTopLevelItem(item);
}

QStringList Dialog_inteditor::add_int_values()
{
    QStringList items;

    items << ui->comboBox_part->currentText()
          << ui->comboBox_level->currentText()
          << QString::number(ui->spinBox_threshold->text().toDouble())
          << ui->label_thresValue->text()
          << ui->timeEdit_int_time->time().toString("mm:ss")
          << QString::number(pop_settings->estimate_stress(ui->comboBox_sport->currentText(),ui->label_thresValue->text(),ui->timeEdit_int_time->time()))
          << QString::number(ui->doubleSpinBox_int_dist->value())
          << ui->comboBox_reps->currentText();

    return items;
}

void Dialog_inteditor::add_interval()
{
    QTreeWidgetItem *item = new QTreeWidgetItem(this->add_int_values());
    ui->treeWidget_planer->insertTopLevelItem(ui->treeWidget_planer->topLevelItemCount(),item);
    ui->treeWidget_planer->expandAll();
    this->set_plot_model();
}

void Dialog_inteditor::open_stdWorkout(QString workID)
{
    clearFlag = true;
    ui->treeWidget_planer->clearFocus();
    if(ui->treeWidget_planer->topLevelItemCount() > 0) ui->treeWidget_planer->clear();

    QStandardItemModel *step_model = stdWorkouts->workouts_steps;
    QModelIndex index;
    QStringList valueList;
    QString parentItem,thresValue;
    QList<QStandardItem*> workout = step_model->findItems(workID,Qt::MatchExactly,0);

    for(int i = 0; i < workout.count();++i)
    {
        valueList = QStringList();
        index = step_model->indexFromItem(workout.at(i));

        thresValue = this->calc_threshold(step_model->item(index.row(),4)->text().toDouble());

        valueList << step_model->item(index.row(),2)->text()
                  << step_model->item(index.row(),3)->text()
                  << step_model->item(index.row(),4)->text()
                  << thresValue
                  << step_model->item(index.row(),5)->text()
                  << QString::number(pop_settings->estimate_stress(ui->comboBox_sport->currentText(),thresValue,QTime::fromString(step_model->item(index.row(),5)->text(),"mm:ss")))
                  << step_model->item(index.row(),6)->text()
                  << step_model->item(index.row(),7)->text();

        QTreeWidgetItem *item = new QTreeWidgetItem(valueList);

        parentItem = step_model->item(index.row(),8)->text();

        if(parentItem.contains("Group") || parentItem.contains("Series"))
        { 
            QList<QTreeWidgetItem*>   pItem = ui->treeWidget_planer->findItems(parentItem,Qt::MatchExactly | Qt::MatchRecursive,0);
            pItem.at(0)->addChild(item);
        }
        else
        {
            ui->treeWidget_planer->insertTopLevelItem(ui->treeWidget_planer->topLevelItemCount(),item);
        }

        ui->treeWidget_planer->expandAll();       
    }
    this->set_plot_model();
    ui->pushButton_save_std->setEnabled(true);
    ui->pushButton_copy_std->setEnabled(true);
    ui->pushButton_delete_std->setEnabled(true);
    clearFlag = false;
}


void Dialog_inteditor::set_plot_model()
{
    if(plot_model->rowCount() > 0) plot_model->removeRows(0,plot_model->rowCount());
    int parentReps = 0;
    int childReps = 0;
    int childCount = 0;
    int subchildCount = 0;

    QString phase,subphase;

    for(int c_item = 0; c_item < ui->treeWidget_planer->topLevelItemCount(); ++c_item)
    {
        //TopItems
        phase = ui->treeWidget_planer->topLevelItem(c_item)->data(0,Qt::DisplayRole).toString();
        childCount = ui->treeWidget_planer->topLevelItem(c_item)->childCount();

        if(childCount > 0)
        {
            if(phase.contains(isGroup))
            {
                parentReps = ui->treeWidget_planer->topLevelItem(c_item)->data(7,Qt::DisplayRole).toInt();

                for(int c_parent = 0; c_parent < parentReps; ++c_parent)
                {
                    for(int c_childs = 0; c_childs < childCount ; ++c_childs)
                    {
                        subchildCount = ui->treeWidget_planer->topLevelItem(c_item)->child(c_childs)->childCount();
                        subphase = ui->treeWidget_planer->topLevelItem(c_item)->child(c_childs)->data(0,Qt::DisplayRole).toString();

                        if(subchildCount > 0)
                        {
                            if(subphase.contains(isSeries))
                            {
                                childReps = ui->treeWidget_planer->topLevelItem(c_item)->child(c_childs)->data(7,Qt::DisplayRole).toInt();
                                for(int c_child = 0; c_child < childReps; ++c_child)
                                {
                                    for(int c_subchilds = 0; c_subchilds < subchildCount; ++c_subchilds)
                                    {
                                        this->add_to_plot(ui->treeWidget_planer->topLevelItem(c_item)->child(c_childs)->child(c_subchilds));
                                    }
                                }
                            }
                        }
                        else
                        {
                            this->add_to_plot(ui->treeWidget_planer->topLevelItem(c_item)->child(c_childs));
                        }
                    }
                }

            }
            else if(phase.contains(isSeries))
            {
                parentReps = ui->treeWidget_planer->topLevelItem(c_item)->data(7,Qt::DisplayRole).toInt();

                for(int c_parent = 0; c_parent < parentReps; ++c_parent)
                {
                    for(int c_childs = 0; c_childs < childCount ; ++c_childs)
                    {
                        this->add_to_plot(ui->treeWidget_planer->topLevelItem(c_item)->child(c_childs));
                    }
                }
            }
        }
        else
        {
            //Items without connection
            if(!phase.contains(isSeries) && !phase.contains(isGroup))
            {
            this->add_to_plot(ui->treeWidget_planer->topLevelItem(c_item));
            }

        }
    }

    this->set_plot_graphic(plot_model->rowCount());
}

void Dialog_inteditor::add_to_plot(QTreeWidgetItem *item)
{
    double time_sum = 0;
    double dist_sum = 0;
    double stress_sum = 0.0;
    int row = plot_model->rowCount();
    if(row != 0)
    {
        time_sum = plot_model->data(plot_model->index(row-1,1,QModelIndex())).toDouble();
        dist_sum = plot_model->data(plot_model->index(row-1,2,QModelIndex())).toDouble();
        stress_sum = plot_model->data(plot_model->index(row-1,3,QModelIndex())).toDouble();
    }

    plot_model->insertRows(row,1,QModelIndex());
    plot_model->setData(plot_model->index(row,0,QModelIndex()),item->data(2,Qt::DisplayRole).toDouble());
    plot_model->setData(plot_model->index(row,1,QModelIndex()),time_sum + (pop_settings->get_timesec(item->data(4,Qt::DisplayRole).toString()) / 60.0));
    plot_model->setData(plot_model->index(row,2,QModelIndex()),dist_sum + item->data(6,Qt::DisplayRole).toDouble());
    plot_model->setData(plot_model->index(row,3,QModelIndex()),stress_sum + item->data(5,Qt::DisplayRole).toDouble());
    this->set_plot_graphic(plot_model->rowCount());
}

void Dialog_inteditor::set_plot_graphic(int c_ints)
{
    ui->widget_planerplot->clearPlottables();
    int int_loops = c_ints;
    int offset = 1;
    QVector<double> x_time(int_loops+offset),x_dist(int_loops+offset),y_thres(int_loops+offset);
    time_sum = 0.0;
    dist_sum = 0.0;
    stress_sum = 0.0;
    double thres_high = 0.0;

    QCPGraph *workout_line = ui->widget_planerplot->addGraph();

    if(c_ints != 0)
    {
        workout_line->setPen(QPen(Qt::blue));
        workout_line->setBrush(QColor(0,170,255,60));
        workout_line->setLineStyle((QCPGraph::lsStepLeft));
        workout_line->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc, QPen(Qt::red),QBrush(Qt::white), 5));

        x_time[0] = 0.01;
        y_thres[0] = plot_model->data(plot_model->index(0,0,QModelIndex())).toDouble();

        for (int i=0,counter=1 ; i < int_loops; ++i,++counter)
        {
             y_thres[counter-1] = plot_model->data(plot_model->index(i,0,QModelIndex())).toDouble();
             y_thres[counter] = y_thres[counter-1];
             x_time[counter] = plot_model->data(plot_model->index(i,1,QModelIndex())).toDouble();
             x_dist[counter] = plot_model->data(plot_model->index(i,2,QModelIndex())).toDouble();
             if(y_thres[counter] > thres_high)
             {
                 thres_high = y_thres[counter];
             }

        }
        time_sum = plot_model->data(plot_model->index(plot_model->rowCount()-1,1,QModelIndex())).toDouble();
        dist_sum = plot_model->data(plot_model->index(plot_model->rowCount()-1,2,QModelIndex())).toDouble();
        stress_sum = plot_model->data(plot_model->index(plot_model->rowCount()-1,3,QModelIndex())).toDouble();
        workout_line->setData(x_time,y_thres);
        ui->widget_planerplot->graph()->rescaleAxes(true);
        ui->widget_planerplot->xAxis->setRange(0,time_sum+((time_sum/2)*0.1));
        ui->widget_planerplot->xAxis2->setRange(0,dist_sum+((dist_sum/2)*0.1));
        ui->widget_planerplot->xAxis2->setVisible(true);
        ui->widget_planerplot->yAxis->setRange(this->get_yaxis_values(false),thres_high+20.0);
    }
    else
    {
        ui->widget_planerplot->clearGraphs();
        ui->widget_planerplot->xAxis->setRange(0,this->get_xaxis_values());
        ui->widget_planerplot->xAxis2->setRange(0,this->get_x2axis_values());
        ui->widget_planerplot->xAxis2->setVisible(true);
        ui->widget_planerplot->yAxis->setRange(this->get_yaxis_values(false),this->get_yaxis_values(true));
    }

    ui->widget_planerplot->xAxis->setTicks(true);
    ui->widget_planerplot->yAxis->setTicks(true);
    ui->widget_planerplot->xAxis->setLabel("Time - Minutes");
    ui->widget_planerplot->xAxis2->setLabel("Distance - KM");
    ui->widget_planerplot->yAxis->setLabel("Threshold %");
    ui->widget_planerplot->xAxis->setTickLabels(true);
    ui->widget_planerplot->yAxis->setTickLabels(true);
    ui->widget_planerplot->replot(QCustomPlot::rpImmediate);

    ui->label__duration->setText("Time:" + pop_settings->set_time((int)time_sum) + " - " + "Distance:" + QString::number(dist_sum) + " - " + "Stress:" + QString::number(round(stress_sum)));
}

int Dialog_inteditor::get_x2axis_values()
{
    //KM
    if(current_sport == pop_settings->isAlt) return 1;
    if(current_sport == pop_settings->isSwim) return 1;
    if(current_sport == pop_settings->isBike) return 10;
    if(current_sport == pop_settings->isRun) return 5;
    if(current_sport == pop_settings->isStrength) return 1;
    if(current_sport == pop_settings->isOther) return 1;
    return 1;
}


int Dialog_inteditor::get_xaxis_values()
{
    //Minutes
    if(current_sport == pop_settings->isAlt) return 10;
    if(current_sport == pop_settings->isSwim) return 20;
    if(current_sport == pop_settings->isBike) return 30;
    if(current_sport == pop_settings->isRun) return 15;
    if(current_sport == pop_settings->isStrength) return 10;
    if(current_sport == pop_settings->isOther) return 10;
    return 5;
}

int Dialog_inteditor::get_yaxis_values(bool max_value)
{
    if(current_sport == pop_settings->isAlt && max_value)
    {
        return 50;
    }
    if(current_sport == pop_settings->isAlt && !max_value)
    {
        return 0;
    }
    if(current_sport == pop_settings->isSwim && max_value)
    {
        return 140;
    }
    if(current_sport == pop_settings->isSwim && !max_value)
    {
        return 0;
    }
    if(current_sport == pop_settings->isBike && max_value)
    {
        return 150;
    }
    if(current_sport == pop_settings->isBike && !max_value)
    {
        return 40;
    }
    if(current_sport == pop_settings->isRun && max_value)
    {
        return 150;
    }
    if(current_sport == pop_settings->isRun && !max_value)
    {
        return 50;
    }
    if(current_sport == pop_settings->isStrength && max_value)
    {
        return 70;
    }
    if(current_sport == pop_settings->isStrength && !max_value)
    {
        return 0;
    }
    if(current_sport == pop_settings->isOther && max_value)
    {
        return 50;
    }
    if(current_sport == pop_settings->isOther && !max_value)
    {
        return 0;
    }
    return 0;
}

void Dialog_inteditor::set_min_max(int index, QString sport)
{
    QString range,min,max;

    if(sport == pop_settings->isSwim)
    {
        range = pop_settings->get_swimRange().at(index);
    }
    if(sport == pop_settings->isBike)
    {
        range = pop_settings->get_bikeRange().at(index);
        speedfactor = powerfactor[index];
    }
    if(sport == pop_settings->isRun)
    {
        range = pop_settings->get_runRange().at(index);
    }
    if(sport == pop_settings->isStrength)
    {
        range = pop_settings->get_stgRange().at(index);
    }
    min = range.split("-").first();
    max = range.split("-").last();
    ui->spinBox_threshold->setMinimum(min.toInt());
    ui->spinBox_threshold->setMaximum(max.toInt());

    ui->spinBox_threshold->setValue(ui->spinBox_threshold->minimum());
}

void Dialog_inteditor::clearIntTree()
{
    clearFlag = true;
    QTreeWidgetItem *currentItem;
    while(ui->treeWidget_planer->topLevelItemCount() > 0)
    {
        for(int c_item = 0; c_item < ui->treeWidget_planer->topLevelItemCount(); ++c_item)
        {
        currentItem = ui->treeWidget_planer->topLevelItem(c_item);

        if(currentItem->childCount() > 0)
        {
            for(int c_child = 0; c_child < currentItem->childCount(); ++c_child)
            {
                if(currentItem->child(c_child)->childCount() > 0)
                {
                    for(int subchild = 0; subchild < currentItem->child(c_child)->childCount(); ++subchild)
                    {
                        delete currentItem->child(c_child)->child(subchild);
                    }
                }
                delete currentItem->child(c_child);
            }
        }
        delete currentItem;
        }
    }
}

void Dialog_inteditor::on_pushButton_close_clicked()
{
    this->clearIntTree();
    delete plot_model;
    reject();
}

void Dialog_inteditor::on_pushButton_add_clicked()
{
    this->add_interval();
    stdWorkouts->set_saveFlag(true);
}

void Dialog_inteditor::on_pushButton_clear_clicked()
{
    current_workID = QString();
    this->clearIntTree();
    this->reset_values();
    this->reset_workoutInfo();
}

void Dialog_inteditor::on_comboBox_sport_currentIndexChanged(const QString &sel_sport)
{
    current_sport = sel_sport;
    this->get_workouts(current_sport);
    this->set_sport_threshold(current_sport);
    this->set_min_max(ui->comboBox_level->currentIndex(),sel_sport);
    ui->label_est_speed->setText(pop_settings->get_workout_pace(ui->doubleSpinBox_int_dist->value(),ui->timeEdit_int_time->time(),sel_sport,false));
    this->reset_values();
    this->reset_workoutInfo();
    this->set_plot_graphic(plot_model->rowCount());
}

void Dialog_inteditor::on_timeEdit_int_time_timeChanged(const QTime &time)
{
    ui->label_est_speed->setText(pop_settings->get_workout_pace(ui->doubleSpinBox_int_dist->value(),time,ui->comboBox_sport->currentText(),false));
    this->calc_distance(ui->comboBox_sport->currentText());
}

void Dialog_inteditor::on_doubleSpinBox_int_dist_valueChanged(double dist)
{
    ui->label_est_speed->setText(pop_settings->get_workout_pace(dist,ui->timeEdit_int_time->time(),ui->comboBox_sport->currentText(),false));
}

void Dialog_inteditor::on_pushButton_delete_clicked()
{
    if(select_item->childCount() > 0)
    {
        while(select_item->childCount() > 0)
        {
            for(int i = 0; i < select_item->childCount(); ++i)
            {
                delete select_item->child(i);
            }
        }
    }
    delete select_item;
    this->set_pushbutton(false);
    this->set_plot_model();
}

void Dialog_inteditor::on_pushButton_edit_clicked()
{
    this->edit_item(select_item);
}

void Dialog_inteditor::on_comboBox_part_currentIndexChanged(const QString &part)
{
    ui->comboBox_reps->setCurrentIndex(0);
    this->set_components(part);
}

void Dialog_inteditor::on_treeWidget_planer_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)
    select_item = item;
    this->set_pushbutton(true);
    this->load_item(item);
    this->refresh_model();
}

void Dialog_inteditor::on_pushButton_addtop_clicked()
{
    this->add_topItem(ui->comboBox_topitem->currentText());
    ui->comboBox_reps->setCurrentIndex(0);
}

void Dialog_inteditor::on_pushButton_save_std_clicked()
{
    this->save_workout();
    ui->pushButton_copy_std->setEnabled(true);
    ui->pushButton_delete_std->setEnabled(true);
}

void Dialog_inteditor::on_pushButton_copy_std_clicked()
{
    current_workID = QString();
    this->save_workout();
}

void Dialog_inteditor::on_pushButton_delete_std_clicked()
{
    QMessageBox::StandardButton reply;

    reply = QMessageBox::critical(this,"Delete Std Workout","Delete selected Std Workout?",QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        stdWorkouts->delete_stdWorkout(current_workID,true);
        current_workID = QString();
        this->clearIntTree();
        this->reset_values();
        this->reset_workoutInfo();
        this->get_workouts(current_sport);
    }
}

QString Dialog_inteditor::get_treeValue(int item,int i_child, int c_sub,int pos,int level)
{
    if(level == 0)
    {
        return ui->treeWidget_planer->topLevelItem(item)->data(pos,Qt::DisplayRole).toString();
    }
    if(level == 1)
    {
        return ui->treeWidget_planer->topLevelItem(item)->child(i_child)->data(pos,Qt::DisplayRole).toString();
    }
    if(level == 2)
    {
        return ui->treeWidget_planer->topLevelItem(item)->child(i_child)->child(c_sub)->data(pos,Qt::DisplayRole).toString();
    }

    return 0;
}

void Dialog_inteditor::save_workout_values(QStringList values, QStandardItemModel *workmodel)
{
    int row =  workmodel->rowCount();
    workmodel->insertRows(row,1,QModelIndex());

    for(int i = 0; i < values.count();++i)
    {
        workmodel->setData(workmodel->index(row,i,QModelIndex()),values.at(i));
    }
}

void Dialog_inteditor::save_workout()
{
   int counter = 1;
   int workcounter;
   QString workID,sport,phase,subphase,worktime,currWorkID;
   QStringList workoutValues,existWorkIDs,sportWorkIDs;
   sport = ui->comboBox_sport->currentText();
   QTreeWidgetItem *currentItem;
   QStandardItemModel *workmodel;
   workmodel = stdWorkouts->workouts_meta;
   QList<QStandardItem*> workList = workmodel->findItems(sport,Qt::MatchExactly,0);

   existWorkIDs = stdWorkouts->get_workoutIds();

   for(int i = 0; i < existWorkIDs.count(); ++i)
   {
       currWorkID = existWorkIDs.at(i);
       if(currWorkID.contains(sport))
       {
           sportWorkIDs << currWorkID;
       }
   }

   worktime = pop_settings->set_time((int)time_sum*60);

   //Update Workout -> delete first
   if(current_workID.isEmpty())
   {
       workcounter = workList.count();
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

    this->save_workout_values(workoutValues,workmodel);

   //Intervalldaten
    for(int c_item = 0; c_item < ui->treeWidget_planer->topLevelItemCount(); ++c_item,++counter)
    {      
        currentItem = ui->treeWidget_planer->topLevelItem(c_item);

        phase = get_treeValue(c_item,0,0,0,0);

        workmodel = stdWorkouts->workouts_steps;
        workoutValues = QStringList();

        workoutValues << workID
                 << QString::number(counter)
                 << get_treeValue(c_item,0,0,0,0)   //part
                 << get_treeValue(c_item,0,0,1,0)   //level
                 << get_treeValue(c_item,0,0,2,0)   //threshold
                 << get_treeValue(c_item,0,0,4,0)   //time
                 << get_treeValue(c_item,0,0,6,0)   //dist
                 << get_treeValue(c_item,0,0,7,0)   //repeats
                 << "-";
        this->save_workout_values(workoutValues,workmodel);

        if(currentItem->childCount() > 0)
        {
            for(int c_child = 0; c_child < currentItem->childCount(); ++c_child)
            {
                ++counter;
                workoutValues = QStringList();
                workoutValues << workID
                         << QString::number(counter)
                         << get_treeValue(c_item,c_child,0,0,1)
                         << get_treeValue(c_item,c_child,0,1,1)
                         << get_treeValue(c_item,c_child,0,2,1)
                         << get_treeValue(c_item,c_child,0,4,1)
                         << get_treeValue(c_item,c_child,0,6,1)
                         << get_treeValue(c_item,c_child,0,7,1)
                         << phase;

                this->save_workout_values(workoutValues,workmodel);

                if(currentItem->child(c_child)->childCount() > 0)
                {
                    for(int subchild = 0; subchild < currentItem->child(c_child)->childCount(); ++subchild)
                    {
                        ++counter;
                        subphase = get_treeValue(c_item,c_child,0,0,1);
                        workoutValues = QStringList();
                        workoutValues << workID
                                 << QString::number(counter)
                                 << get_treeValue(c_item,c_child,subchild,0,2)
                                 << get_treeValue(c_item,c_child,subchild,1,2)
                                 << get_treeValue(c_item,c_child,subchild,2,2)
                                 << get_treeValue(c_item,c_child,subchild,4,2)
                                 << get_treeValue(c_item,c_child,subchild,6,2)
                                 << get_treeValue(c_item,c_child,subchild,7,2)
                                 << subphase;

                        this->save_workout_values(workoutValues,workmodel);
                    }
                }

            }
        }
    }
    this->get_workouts(current_sport);
    stdWorkouts->save_stdWorkouts();
}

void Dialog_inteditor::on_spinBox_threshold_valueChanged(int value)
{
    this->calc_threshold((double)value);
    this->calc_distance(ui->comboBox_sport->currentText());
}

void Dialog_inteditor::on_comboBox_level_currentIndexChanged(int index)
{
    this->set_min_max(index,ui->comboBox_sport->currentText());
    if(index == 0 && ui->comboBox_sport->currentText() == pop_settings->isSwim)
    {
        ui->doubleSpinBox_int_dist->setValue(0.0);
    }
}

void Dialog_inteditor::move_item(bool up)
{
    int currentindex = ui->treeWidget_planer->currentIndex().row();

    QTreeWidgetItem *currentItem = ui->treeWidget_planer->currentItem();

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
        ui->treeWidget_planer->takeTopLevelItem(currentindex);
        if(up)
        {
            ui->treeWidget_planer->insertTopLevelItem(currentindex-1,currentItem);
        }
        else
        {
            ui->treeWidget_planer->insertTopLevelItem(currentindex+1,currentItem);
        }
        ui->treeWidget_planer->setCurrentItem(currentItem);
    }
}

void Dialog_inteditor::on_pushButton_up_clicked()
{
    this->move_item(true);
    this->refresh_model();
}

void Dialog_inteditor::on_pushButton_down_clicked()
{
    this->move_item(false);
    this->refresh_model();
}

void Dialog_inteditor::on_listView_workouts_clicked(const QModelIndex &index)
{
    QStringList workoutTitle = workout_model->item(index.row(),0)->text().split("-");
    QString workCode = workoutTitle.first();
    QString workTitle = workoutTitle.last();

    QString workoutID = workout_model->item(index.row(),2)->text();
    current_workID = workoutID;
    ui->comboBox_code->setCurrentText(workCode.replace(" ",""));
    ui->lineEdit_workoutname->setText(workTitle.replace(" ",""));
    this->open_stdWorkout(workoutID);
}

void Dialog_inteditor::on_lineEdit_workoutname_textChanged(const QString &value)
{
    if(!value.isEmpty() && ui->treeWidget_planer->topLevelItemCount() > 0)
    {
        ui->pushButton_save_std->setEnabled(true);
    }
}

void Dialog_inteditor::on_treeWidget_planer_itemSelectionChanged()
{
    if(!clearFlag)
    {
        QTreeWidgetItem *current = ui->treeWidget_planer->currentItem();
        int row = ui->treeWidget_planer->currentIndex().row();
        int rowCount = 0;

        if(current->parent())
        {
            rowCount = ui->treeWidget_planer->currentItem()->parent()->childCount();
        }
        else
        {
            rowCount = ui->treeWidget_planer->topLevelItemCount();
        }

        if(row == 0 && rowCount > 1)
        {
            ui->pushButton_down->setEnabled(true);
            ui->pushButton_up->setEnabled(false);
        }
        else if(row > 0 && row < rowCount && row != rowCount-1)
        {
            ui->pushButton_down->setEnabled(true);
            ui->pushButton_up->setEnabled(true);
        }
        else if(row > 0 && row == rowCount-1)
        {
            ui->pushButton_down->setEnabled(false);
            ui->pushButton_up->setEnabled(true);
        }
        else if(row == 0 && row == rowCount-1)
        {
            ui->pushButton_down->setEnabled(false);
            ui->pushButton_up->setEnabled(false);
        }
    }
}

