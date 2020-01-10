#include "stress_popup.h"
#include "ui_stress_popup.h"
#include <cmath>

stress_popup::stress_popup(QWidget *parent,schedule *p_sched,const QDate startDate) :
    QDialog(parent),
    ui(new Ui::stress_popup)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    isLoad = false;
    workSched = p_sched;

    startDay = startDate.addDays(1-startDate.dayOfWeek());
    firstDay = QDate::currentDate().addDays(1 - QDate::currentDate().dayOfWeek());
    dateRange = 6;
    showNum = QIcon(":/images/icons/Comment-add.png");
    hideNum = QIcon(":/images/icons/Comment-delete.png");
    ui->dateEdit_start->setDateRange(firstDay,p_sched->get_stressMap()->lastKey().addDays(-dateRange));
    ui->dateEdit_start->setDate(startDay);
    ui->dateEdit_end->setDateRange(startDay.addDays(dateRange),p_sched->get_stressMap()->lastKey());
    ui->dateEdit_end->setDate(startDay.addDays(dateRange));
    ui->pushButton_values->setIcon(hideNum);

    connect(ui->widget_stressPlot,SIGNAL(selectionChangedByUser()),this,SLOT(selectionChanged()));
    this->set_graph();
}

stress_popup::~stress_popup()
{
    delete ui;
}

void stress_popup::on_toolButton_close_clicked()
{
    reject();
}

void stress_popup::set_graph()
{
    QFont plotFont,selectFont;
    plotFont.setBold(true);
    plotFont.setPointSize(8);
    selectFont.setPointSize(8);
    selectFont.setItalic(true);

    QList<QCPAxis*> xaxisList;
    xaxisList.append(ui->widget_stressPlot->xAxis);
    xaxisList.append(ui->widget_stressPlot->xAxis2);

    QList<QCPAxis*> yaxisList;
    yaxisList.append(ui->widget_stressPlot->yAxis);
    //yaxisList.append(ui->widget_stressPlot->yAxis2);

    ui->widget_stressPlot->setInteractions(QCP::iSelectLegend | QCP::iMultiSelect | QCP::iRangeDrag | QCP::iRangeZoom);

    ui->widget_stressPlot->xAxis->setLabel("Date");
    ui->widget_stressPlot->xAxis->setLabelFont(plotFont);

    ui->widget_stressPlot->xAxis2->setVisible(true);
    ui->widget_stressPlot->xAxis2->setLabelFont(plotFont);

    ui->widget_stressPlot->yAxis->setLabel("Stress");
    ui->widget_stressPlot->yAxis->setLabelFont(plotFont);

    ui->widget_stressPlot->yAxis2->setVisible(true);
    ui->widget_stressPlot->yAxis2->setLabel("TSB");
    ui->widget_stressPlot->yAxis2->setLabelFont(plotFont);

    ui->widget_stressPlot->legend->setVisible(true);
    ui->widget_stressPlot->legend->setFont(plotFont);
    ui->widget_stressPlot->legend->setSelectedFont(selectFont);
    ui->widget_stressPlot->legend->setSelectableParts(QCPLegend::spItems);

    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    ui->widget_stressPlot->plotLayout()->addElement(1,0,subLayout);
    subLayout->setMargins(QMargins(dateRange*10,0,dateRange*10,5));
    subLayout->addElement(0,0,ui->widget_stressPlot->legend);

    ui->widget_stressPlot->axisRect()->setRangeDragAxes(xaxisList,yaxisList);
    ui->widget_stressPlot->axisRect()->setRangeZoomAxes(xaxisList,yaxisList);
    ui->widget_stressPlot->addLayer("GRID",ui->widget_stressPlot->layer(0),QCustomPlot::limAbove);
    ui->widget_stressPlot->addLayer("TSB",ui->widget_stressPlot->layer(1),QCustomPlot::limAbove);
    ui->widget_stressPlot->addLayer("STS",ui->widget_stressPlot->layer(2),QCustomPlot::limAbove);
    ui->widget_stressPlot->addLayer("LTS",ui->widget_stressPlot->layer(3),QCustomPlot::limAbove);
    ui->widget_stressPlot->addLayer("StressScore",ui->widget_stressPlot->layer(4),QCustomPlot::limAbove);
    ui->widget_stressPlot->xAxis->grid()->setLayer("GRID");
    ui->widget_stressPlot->yAxis->grid()->setLayer("GRID");
    this->set_stressValues(ui->dateEdit_start->date(),ui->dateEdit_end->date());
}

void stress_popup::set_stressValues(QDate rangeStart, QDate rangeEnd)
{
    QMap<QDate,QVector<double>> *stressMap =  workSched->get_stressMap();
    int dayCount = rangeStart.addDays(-1).daysTo(rangeEnd.addDays(1));
    xDate.resize(dayCount);
    yLTS.resize(dayCount);
    ySTS.resize(dayCount);
    yTSB.resize(dayCount);
    yStress.resize(dayCount);
    yDura.resize(dayCount);

    stressMax = 0;
    tsbMinMax.resize(2);
    tsbMinMax.fill(0);

    QDateTime startDate;
    QTime wTime;
    wTime.fromString("00:00:00","hh:mm:ss");
    startDate.setDate(rangeStart);
    startDate.setTime(wTime);
    startDate.setTimeSpec(Qt::LocalTime);
    int day = 0;

    for(QMap<QDate,QVector<double>>::const_iterator stressStart = stressMap->find(rangeStart.addDays(-1)), end = stressMap->find(rangeEnd.addDays(1)); stressStart != end; ++stressStart)
    {
        startDate.setDate(stressStart.key());
        xDate[day] = startDate.toTime_t() + 3600;
        yStress[day] = stressStart.value().at(0);
        ySTS[day] = round(stressStart.value().at(1));
        yLTS[day] = round(stressStart.value().at(2));
        yDura[day] = round(stressStart.value().at(3)/60.0);

        if(stressMax < yStress[day]) stressMax = yStress[day];
        if(day > 0) yTSB[day] = yLTS[day-1] - ySTS[day-1];
        ++day;
    }

    this->set_stressplot(rangeStart,rangeEnd,ui->pushButton_values->isChecked());
}

QCPGraph *stress_popup::get_QCPLine(QString name,QColor gColor,QVector<double> &ydata, bool secondAxis)
{
    QCPGraph *graph = ui->widget_stressPlot->addGraph();
    if(secondAxis)
    {
        graph->setValueAxis(ui->widget_stressPlot->yAxis2);
    }
    graph->setName(name);
    graph->setLineStyle(QCPGraph::lsLine);
    graph->setData(xDate,ydata);
    graph->setAntialiased(true);
    graph->setPen(QPen(gColor,1));
    graph->setLayer(name);

    return graph;
}

void stress_popup::set_itemTracer(QString layer,QCPGraph *graphline, QColor tColor,int pos)
{
    QCPItemTracer *tracer = new QCPItemTracer(ui->widget_stressPlot);
    tracer->setGraph(graphline);
    tracer->setGraphKey(xDate[pos]);
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setBrush(QBrush(tColor));
    tracer->setLayer(layer);
}

void stress_popup::set_itemText(QString layer,QFont lineFont, QVector<double> &ydata,int pos,bool secondAxis)
{
    QCPItemText *itemText = new QCPItemText(ui->widget_stressPlot);
    if(secondAxis)
    {
        itemText->position->setAxes(ui->widget_stressPlot->xAxis,ui->widget_stressPlot->yAxis2);
    }
    itemText->position->setType(QCPItemPosition::ptPlotCoords);
    itemText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
    itemText->position->setCoords(xDate[pos],ydata[pos]+1);
    itemText->setText(QString::number(ydata[pos]));
    itemText->setTextAlignment(Qt::AlignCenter);
    itemText->setFont(lineFont);
    itemText->setPadding(QMargins(1, 1, 1, 1));
    itemText->setLayer(layer);
}

void stress_popup::set_stressplot(QDate rangeStart,QDate rangeEnd,bool showValues)
{
    ui->widget_stressPlot->clearPlottables();
    ui->widget_stressPlot->clearItems();
    ui->widget_stressPlot->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->widget_stressPlot->plotLayout()->setRowStretchFactor(1,0.0001);

    QTime time(0,0,0);
    QDateTime rStart(rangeStart);
    rStart.setTime(time);
    rStart.setTimeSpec(Qt::LocalTime);
    QDateTime rStop(rangeEnd);
    rStop.setTime(time);
    rStop.setTimeSpec(Qt::LocalTime);
    QColor duraColor(0,85,255);
    QCPRange xRange(QCPAxisTickerDateTime::dateTimeToKey(rangeStart.addDays(-1)),QCPAxisTickerDateTime::dateTimeToKey(rangeEnd.addDays(1)));

    QFont lineFont;
    lineFont.setPointSize(8);
    int xTickCount = dateRange;
    double dayCount = rangeStart.daysTo(rangeEnd)+2;
    QCPGraph *ltsLine = this->get_QCPLine("LTS",QColor(0,255,0),yLTS,false);
    QCPGraph *stsLine = this->get_QCPLine("STS",QColor(255,0,0),ySTS,false);
    QCPGraph *stressLine = this->get_QCPLine("StressScore",QColor(225,150,0),yStress,false);
    QCPGraph *tsbLine = this->get_QCPLine("TSB",QColor(255,170,0),yTSB,true);
    tsbLine->setBrush(QBrush(QColor(255,170,0,50)));
    QCPBars *duraBars = new QCPBars(ui->widget_stressPlot->xAxis,ui->widget_stressPlot->yAxis);
    duraBars->setName("Duration");
    duraBars->setWidth(250000.0/dayCount);
    duraBars->setAntialiased(true);
    duraBars->setPen(QPen(duraColor));
    duraColor.setAlpha(80);
    duraBars->setBrush(QBrush(duraColor));
    duraBars->setLayer("StressScore");
    duraBars->setData(xDate,yDura);

    for(int i = 0; i < xDate.count(); ++i)
    {
        this->set_itemTracer("LTS",ltsLine,Qt::green,i);
        this->set_itemTracer("STS",stsLine,Qt::red,i);
        this->set_itemTracer("StressScore",stressLine,QColor(225,150,0),i);
        this->set_itemTracer("TSB",tsbLine,QColor(255,170,0),i);

        if(!showValues)
        {
            this->set_itemText("LTS",lineFont,yLTS,i,false);
            this->set_itemText("STS",lineFont,ySTS,i,false);
            this->set_itemText("StressScore",lineFont,yStress,i,false);
            this->set_itemText("TSB",lineFont,yTSB,i,true);

            QCPItemText *barText = new QCPItemText(ui->widget_stressPlot);
            barText->position->setType(QCPItemPosition::ptPlotCoords);
            barText->position->setCoords(xDate[i],20.0);
            barText->setText(QString::number(yDura[i]));
            barText->setFont(lineFont);
            barText->setLayer("StressScore");
        }

        if(tsbMinMax[0] > yTSB[i]) tsbMinMax[0] = yTSB[i];
        if(tsbMinMax[1] < yTSB[i]) tsbMinMax[1] = yTSB[i];
    }

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeSpec(Qt::LocalTime);
    dateTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dateTicker->setDateTimeFormat("dd.MM");

    QSharedPointer<QCPAxisTickerFixed> dayTicker(new QCPAxisTickerFixed);
    dayTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dayTicker->setTickStep(1.0);

    if(dateRange > 20 && dateRange < 42)
    {
        xTickCount = (dateRange+1)/3;
    }
    else if(dateRange >= 42)
    {
        xTickCount = (dateRange+1)/5;
    }
    if(dayCount >= 21)
    {
        dayCount = dayCount / 7;
        ui->widget_stressPlot->xAxis2->setLabel("Weeks");
    }
    else
    {
        ui->widget_stressPlot->xAxis2->setLabel("Days");
    }

    dateTicker->setTickCount(xTickCount);
    dayTicker->setTickCount(static_cast<int>(dayCount));

    ui->widget_stressPlot->yAxis->setRange(0,stressMax+10);
    ui->widget_stressPlot->yAxis2->setRange(tsbMinMax[0]-5,tsbMinMax[1]+5);
    ui->widget_stressPlot->xAxis->setRange(xRange);
    ui->widget_stressPlot->xAxis->setTicker(dateTicker);
    ui->widget_stressPlot->xAxis2->setRange(0,dayCount);
    ui->widget_stressPlot->xAxis2->setTicker(dayTicker);

    ui->widget_stressPlot->replot();

    isLoad = true;
}

void stress_popup::on_dateEdit_start_dateChanged(const QDate &date)
{
    if(isLoad)
    {
        if(date > ui->dateEdit_end->date())
        {
            ui->dateEdit_end->setDate(date.addDays(dateRange));
            dateRange = static_cast<int>(ui->dateEdit_start->date().daysTo(ui->dateEdit_end->date()));
            this->set_stressValues(ui->dateEdit_start->date(),ui->dateEdit_end->date());
        }
        else
        {
            dateRange = static_cast<int>(ui->dateEdit_start->date().daysTo(ui->dateEdit_end->date()));
            this->set_stressValues(date,ui->dateEdit_end->date());
        }
    }
}

void stress_popup::on_dateEdit_end_dateChanged(const QDate &date)
{
    if(isLoad)
    {
        if(ui->dateEdit_start->date().daysTo(date) > 0)
        {
            dateRange = static_cast<int>(ui->dateEdit_start->date().daysTo(date));
            this->set_stressValues(ui->dateEdit_start->date(),date);
        }
        else
        {
            ui->dateEdit_end->setDate(ui->dateEdit_start->date().addDays(6));
            this->set_stressValues(ui->dateEdit_start->date(),ui->dateEdit_end->date());
        }
    }
}

void stress_popup::on_pushButton_values_toggled(bool checked)
{
    if(checked)
    {
        ui->pushButton_values->setIcon(showNum);
        ui->pushButton_values->setToolTip("Show Values");
    }
    else
    {
        ui->pushButton_values->setIcon(hideNum);
        ui->pushButton_values->setToolTip("Hide Values");
    }
    this->set_stressplot(ui->dateEdit_start->date(),ui->dateEdit_end->date(),checked);
}

void stress_popup::on_pushButton_reset_clicked()
{
    this->set_stressplot(ui->dateEdit_start->date(),ui->dateEdit_end->date(),ui->pushButton_values->isChecked());
}

void stress_popup::selectionChanged()
{
    for(int i = 0; i < ui->widget_stressPlot->graphCount(); ++i)
    {
        QCPGraph *graph = ui->widget_stressPlot->graph(i);
        QCPPlottableLegendItem *item = ui->widget_stressPlot->legend->itemWithPlottable(graph);
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
