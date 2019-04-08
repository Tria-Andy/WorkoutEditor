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
    ui->treeView_foodhistory->setModel(foodplan->historyModel);
    dialogResult = QDialog::Rejected;
    athleteWeight = this->athleteValues->value("weight");

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
    done(dialogResult);
}

void foodhistory_popup::set_plotValues(int currWeek,int count,bool week)
{
    int valueStart = 0;
    int labelCol = 0;
    QModelIndex sectionIndex;

    if(week)
    {
        sectionIndex = foodplan->historyModel->indexFromItem(foodplan->historyModel->item(currWeek,0));
        labelCol = 1;
    }
    else
    {
        valueStart = currWeek - count;
    }

    weekLabels.clear();
    weekList.resize(count);
    metaRate.resize(count);
    calConversion.resize(count);
    calSport.resize(count);
    calFood.resize(count);
    calDiff.resize(count);
    weight.resize(count);
    double maxConvers = 0;

    for(int i = 0; i < count; ++i,++valueStart)
    {
       weekLabels << foodplan->historyModel->data(foodplan->historyModel->index(valueStart,labelCol,sectionIndex)).toString();
       weight[i] = foodplan->historyModel->data(foodplan->historyModel->index(valueStart,2,sectionIndex)).toDouble();
       metaRate[i] = foodplan->historyModel->data(foodplan->historyModel->index(valueStart,3,sectionIndex)).toDouble();
       calSport[i] = foodplan->historyModel->data(foodplan->historyModel->index(valueStart,4,sectionIndex)).toDouble();
       calFood[i] = foodplan->historyModel->data(foodplan->historyModel->index(valueStart,5,sectionIndex)).toDouble();
       calDiff[i] = foodplan->historyModel->data(foodplan->historyModel->index(valueStart,6,sectionIndex)).toDouble();

       calConversion[i] = metaRate[i] + calSport[i];
       if(calConversion[i] > maxConvers) maxConvers = calConversion[i];

    }

    for(int i = 1; i <= count; ++i)
    {
        weekList[i-1] = i;
    }

    this->set_graph(count,maxConvers);
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
    this->set_plotValues(weekCount,index+1,false);
}

void foodhistory_popup::on_treeView_foodhistory_clicked(const QModelIndex &index)
{
    if(index.column() == 0)
    {
        ui->treeView_foodhistory->collapseAll();

        if(foodplan->historyModel->item(index.row(),0)->hasChildren())
        {
            this->set_plotValues(index.row(),foodplan->historyModel->item(index.row(),0)->rowCount(),true);
            ui->comboBox_weekCount->setEnabled(false);
            ui->treeView_foodhistory->expand(index);
        }
    }
    else
    {
        dialogResult = QDialog::Accepted;
    }
}

void foodhistory_popup::on_toolButton_reset_clicked()
{
    this->set_plotValues(weekCount,ui->comboBox_weekCount->currentIndex()+1,false);
    ui->treeView_foodhistory->collapseAll();
    ui->comboBox_weekCount->setEnabled(true);
}
