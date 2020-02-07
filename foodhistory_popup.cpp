#include "foodhistory_popup.h"
#include "ui_foodhistory_popup.h"

foodhistory_popup::foodhistory_popup(QWidget *parent,foodplanner *pFood) :
    QDialog(parent),
    ui(new Ui::foodhistory_popup)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    foodplan = pFood;
    weekCount = foodplan->historyModel->rowCount();
    historyHeader = settings::getHeaderMap("foodhistheader");
    athleteWeight = this->athleteValues->value("weight");
    updateHistory = false;

    ui->comboBox_weekCount->blockSignals(true);

    for(int i = 0; i < 15 && i < weekCount; ++i)
    {
        ui->comboBox_weekCount->addItem(QString::number(i+1));
    }
    ui->comboBox_weekCount->blockSignals(false);

    ui->widget_plot->legend->setVisible(true);
    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    ui->widget_plot->plotLayout()->addElement(1,0,subLayout);
    subLayout->setMargins(QMargins(10,1,10,1));
    subLayout->addElement(0,0,ui->widget_plot->legend);

    foodHistory = foodplan->get_foodHistoryMap();
    this->set_foodHistoryTree();

    if(weekCount > 5)
    {
        ui->comboBox_weekCount->setCurrentIndex(4);
    }
    else
    {
        ui->comboBox_weekCount->setCurrentIndex(ui->comboBox_weekCount->count()-1);
    }

}

foodhistory_popup::~foodhistory_popup()
{
    delete ui;
}

void foodhistory_popup::on_toolButton_close_clicked()
{
    if(updateHistory)
    {
        done(QDialog::Accepted);
    }
    else
    {
        done(QDialog::Rejected);
    }
}

void foodhistory_popup::set_foodHistoryTree()
{
    QTreeWidgetItem *rootItem = ui->treeWidget_foodhistory->invisibleRootItem();
    QTreeWidgetItem *weekItem, *dayItem;
    ui->treeWidget_foodhistory->blockSignals(true);
    ui->treeWidget_foodhistory->setEditTriggers(QAbstractItemView::AllEditTriggers);
    ui->treeWidget_foodhistory->setColumnCount(historyHeader->count());
    QModelIndex weekIndex;
    QString weekID;

    QStringList headerLabel;
    for(int col = 0; col < historyHeader->count(); ++col)
    {
        headerLabel << historyHeader->at(col);
    }
    ui->treeWidget_foodhistory->setHeaderLabels(headerLabel);

    for(QMap<QPair<QDate,int>,QMap<QDate,QList<QVariant>>>::const_iterator it = foodHistory->cbegin(); it != foodHistory->cend(); ++it)
    {
        weekIndex = foodplan->get_modelIndex(foodplan->historyModel,it.key().first.toString("dd.MM.yyyy"),2);

        weekItem = new QTreeWidgetItem(rootItem);
        weekItem->setData(0,Qt::DisplayRole,it.key().first);
        weekItem->setData(1,Qt::DisplayRole,it.key().second);
        weekItem->setData(2,Qt::DisplayRole,weekIndex.siblingAtColumn(3).data());
        weekItem->setData(3,Qt::DisplayRole,weekIndex.siblingAtColumn(4).data());
        weekItem->setData(7,Qt::DisplayRole,weekIndex.siblingAtColumn(5).data());

        for(QMap<QDate,QList<QVariant>>::const_iterator day = it.value().cbegin(); day != it.value().cend(); ++day)
        {
            dayItem = new QTreeWidgetItem();
            dayItem->setData(0,Qt::DisplayRole,day.key());
            dayItem->setData(3,Qt::DisplayRole,weekIndex.siblingAtColumn(4).data());
            dayItem->setData(4,Qt::DisplayRole,day.value().at(2));
            dayItem->setData(5,Qt::DisplayRole,day.value().at(1));
            dayItem->setData(6,Qt::DisplayRole,day.value().at(4));
            dayItem->setData(7,Qt::UserRole,day.value().at(3));
            weekItem->addChild(dayItem);
        }
    }
    ui->treeWidget_foodhistory->blockSignals(false);
}

void foodhistory_popup::reset_editValues()
{
    ui->spinBox_base->setValue(0);
    ui->dateEdit_day->setDate(QDate::currentDate());
    ui->spinBox_sport->setValue(0);
    ui->spinBox_food->setValue(0);
    ui->spinBox_diff->setValue(0);
    ui->spinBox_summery->setValue(0);
}

void foodhistory_popup::set_plotValues(int weekCount,bool singleWeek)
{
    int xCount = singleWeek ? 7 : weekCount;

    weekLabels.clear();
    weekList.resize(xCount);
    metaRate.resize(xCount);
    calConversion.resize(xCount);
    calSport.resize(xCount);
    calFood.resize(xCount);
    calDiff.resize(xCount);
    weight.resize(xCount);
    double maxConvers = 0;

    QTreeWidgetItem *weekItem;

    if(singleWeek)
    {
        weekItem = ui->treeWidget_foodhistory->topLevelItem(weekCount);

        for(int day = 0; day < weekItem->childCount();++day)
        {
            weekLabels << weekItem->child(day)->data(0,Qt::DisplayRole).toString();
            weekList[day] = day+1;
            weight[day] = 0;
            metaRate[day] = weekItem->child(day)->data(3,Qt::DisplayRole).toDouble();
            calSport[day] = weekItem->child(day)->data(4,Qt::DisplayRole).toDouble();
            calFood[day] = weekItem->child(day)->data(5,Qt::DisplayRole).toDouble();
            calDiff[day] = weekItem->child(day)->data(6,Qt::DisplayRole).toDouble()*-1;

            calConversion[day] = metaRate[day] + calSport[day];
            if(calConversion[day] > maxConvers) maxConvers = calConversion[day];
        }
    }
    else
    {
        int startWeek = ui->treeWidget_foodhistory->topLevelItemCount()-weekCount;
        int weekCounter = 0;

        for( ;startWeek < ui->treeWidget_foodhistory->topLevelItemCount(); ++startWeek,++weekCounter)
        {
            weekItem = ui->treeWidget_foodhistory->topLevelItem(startWeek);
            weekLabels << weekItem->data(1,Qt::DisplayRole).toString();
            weekList[weekCounter] = weekCounter+1;
            weight[weekCounter] = weekItem->data(2,Qt::DisplayRole).toDouble();

            for(int day = 0; day < weekItem->childCount();++day)
            {
                metaRate[weekCounter] += weekItem->child(day)->data(3,Qt::DisplayRole).toDouble();
                calSport[weekCounter] += weekItem->child(day)->data(4,Qt::DisplayRole).toDouble();
                calFood[weekCounter] += weekItem->child(day)->data(5,Qt::DisplayRole).toDouble();
                calDiff[weekCounter] += weekItem->child(day)->data(6,Qt::DisplayRole).toDouble()*-1;

                calConversion[weekCounter] = metaRate[weekCounter] + calSport[weekCounter];
                if(calConversion[weekCounter] > maxConvers) maxConvers = calConversion[weekCounter];
            }
        }
    }

    this->set_graph(xCount,maxConvers);

}

void foodhistory_popup::set_graph(int weekcount,double maxCon)
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();
    ui->widget_plot->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->widget_plot->plotLayout()->setRowStretchFactor(1,0.0001);

    QMap<double,QString> tickerList;
    tickerList.insert(0,"Start");

    QSharedPointer<QCPAxisTickerText> weekTicker(new QCPAxisTickerText);
    for(int i = 1; i <= weekcount; ++i)
    {
        tickerList.insert(i,weekLabels.at(i-1));
    }

    tickerList.insert(tickerList.count(),"End");
    weekTicker->addTicks(tickerList);

    ui->widget_plot->xAxis->setRange(0,weekcount+1);
    ui->widget_plot->xAxis->setTicker(weekTicker);
    ui->widget_plot->xAxis->setLabel("Week");
    ui->widget_plot->yAxis->setLabel("Calories");
    ui->widget_plot->yAxis2->setLabel("Weight");
    ui->widget_plot->yAxis2->setVisible(true);

    double barWidth = 6.0;
    QCPBarsGroup *barGroup = new QCPBarsGroup(ui->widget_plot);

    QCPBars *conversBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    conversBar->setName("Conversion Total");
    conversBar->setWidthType(QCPBars::wtAbsolute);
    conversBar->setWidth(barWidth);
    conversBar->setAntialiased(true);
    conversBar->setBrush(Qt::darkRed);
    conversBar->setPen(QPen(Qt::darkGray));
    conversBar->setData(weekList,calConversion);
    conversBar->setBarsGroup(barGroup);


    QCPBars *metaBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    metaBar->setName("Metabolic Rate");
    metaBar->setWidthType(QCPBars::wtAbsolute);
    metaBar->setWidth(barWidth);
    metaBar->setAntialiased(true);
    metaBar->setBrush(Qt::blue);
    metaBar->setPen(QPen(Qt::darkGray));
    metaBar->setData(weekList,metaRate);
    metaBar->setBarsGroup(barGroup);

    QCPBars *sportBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    sportBar->setName("Sport");
    sportBar->setWidthType(QCPBars::wtAbsolute);
    sportBar->setWidth(barWidth);
    sportBar->setAntialiased(true);
    sportBar->setBrush(Qt::green);
    sportBar->setPen(QPen(Qt::darkGray));
    sportBar->setData(weekList,calSport);
    sportBar->setBarsGroup(barGroup);

    QCPBars *foodBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    foodBar->setName("Food");
    foodBar->setWidthType(QCPBars::wtAbsolute);
    foodBar->setWidth(barWidth);
    foodBar->setAntialiased(true);
    foodBar->setBrush(Qt::gray);
    foodBar->setPen(QPen(Qt::darkGray));
    foodBar->setData(weekList,calFood);
    foodBar->setBarsGroup(barGroup);

    QCPBars *diffBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    diffBar->setName("Diff");
    diffBar->setWidthType(QCPBars::wtAbsolute);
    diffBar->setWidth(barWidth);
    diffBar->setAntialiased(true);
    diffBar->setBrush(Qt::red);
    diffBar->setPen(QPen(Qt::darkGray));
    diffBar->setData(weekList,calDiff);
    diffBar->setBarsGroup(barGroup);

    QCPGraph *weightLine = ui->widget_plot->addGraph(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    weightLine->setName("Weight");
    weightLine->setLineStyle(QCPGraph::lsLine);
    weightLine->setData(weekList,weight);
    weightLine->setAntialiased(true);
    weightLine->setBrush(QBrush(QColor(255,0,0,50)));
    weightLine->setPen(QPen(QColor(255,0,0),2));

    for(int i = 0; i < weekcount; ++i)
    {
        QCPItemTracer *itemTracer = new QCPItemTracer(ui->widget_plot);
        itemTracer->setGraph(weightLine);
        itemTracer->setGraphKey(weekList[i]);
        itemTracer->setStyle(QCPItemTracer::tsCircle);
        itemTracer->setBrush(Qt::red);
    }

    ui->widget_plot->yAxis->setRange(0,maxCon+1000);
    ui->widget_plot->yAxis2->setRange(athleteWeight-(athleteWeight*0.10),athleteWeight+(athleteWeight*0.10));

    ui->widget_plot->replot();
}

void foodhistory_popup::on_comboBox_weekCount_currentIndexChanged(int index)
{
    this->set_plotValues(index+1,false);
}

void foodhistory_popup::on_toolButton_reset_clicked()
{
    this->set_plotValues(ui->comboBox_weekCount->currentIndex()+1,false);
    ui->comboBox_weekCount->setEnabled(true);
}

void foodhistory_popup::on_toolButton_edit_clicked()
{
    QVector<double> dayValues(5,0);

    QTreeWidgetItem *item = ui->treeWidget_foodhistory->currentItem();

    dayValues[0] = ui->spinBox_base->value();
    dayValues[1] = ui->spinBox_food->value();
    dayValues[2] = ui->spinBox_sport->value();
    dayValues[3] = ui->spinBox_summery->value();
    dayValues[4] = ui->spinBox_diff->value();

    item->setData(4,Qt::DisplayRole,dayValues.at(2));
    item->setData(5,Qt::DisplayRole,dayValues.at(1));
    item->setData(6,Qt::DisplayRole,dayValues.at(4));
    item->setData(7,Qt::UserRole,dayValues.at(3));

    foodplan->update_foodHistory(ui->dateEdit_day->date(),dayValues);
    updateHistory = true;

    this->reset_editValues();
}

void foodhistory_popup::on_treeWidget_foodhistory_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);
    if(item->childCount() == 0)
    {
        ui->spinBox_base->setValue(item->data(3,Qt::DisplayRole).toInt());
        ui->dateEdit_day->setDate(item->data(0,Qt::DisplayRole).toDate());
        ui->spinBox_sport->setValue(item->data(4,Qt::DisplayRole).toInt());
        ui->spinBox_food->setValue(item->data(5,Qt::DisplayRole).toInt());
        ui->spinBox_diff->setValue(item->data(6,Qt::DisplayRole).toInt());
        ui->spinBox_summery->setValue(item->data(7,Qt::UserRole).toInt());
    }
    else
    {
        if(item->isExpanded())
        {
            ui->treeWidget_foodhistory->collapseItem(item);
            this->reset_editValues();
        }
        else
        {
            ui->treeWidget_foodhistory->expandItem(item);
        }
        ui->comboBox_weekCount->setEnabled(false);
        this->set_plotValues(ui->treeWidget_foodhistory->indexOfTopLevelItem(item),true);
    }
}

void foodhistory_popup::on_spinBox_sport_valueChanged(int sportValue)
{
    ui->spinBox_summery->setValue(ui->spinBox_base->value()+sportValue);
    ui->spinBox_diff->setValue(ui->spinBox_food->value()-ui->spinBox_summery->value());
}

void foodhistory_popup::on_spinBox_food_valueChanged(int foodValue)
{
    ui->spinBox_diff->setValue(foodValue-ui->spinBox_summery->value());
}
