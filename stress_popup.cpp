#include "stress_popup.h"
#include "ui_stress_popup.h"
#include <cmath>

stress_popup::stress_popup(QWidget *parent,schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::stress_popup)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    stressMap = p_sched->get_StressMap();
    isLoad = false;
    ltsDays = settings::get_ltsValue("ltsdays");
    stsDays = settings::get_ltsValue("stsdays");
    lastLTS = settings::get_ltsValue("lastlts");
    lastSTS = settings::get_ltsValue("laststs");
    firstDayofWeek = QDate::currentDate().addDays(1-QDate::currentDate().dayOfWeek());
    dateRange = 6;
    showNum = QIcon(":/images/icons/Comment-add.png");
    hideNum = QIcon(":/images/icons/Comment-delete.png");
    ui->dateEdit_start->setDateRange(firstDayofWeek,stressMap->lastKey().addDays(-dateRange));
    ui->dateEdit_start->setDate(firstDayofWeek);
    ui->dateEdit_end->setDateRange(firstDayofWeek.addDays(dateRange),stressMap->lastKey());
    ui->dateEdit_end->setDate(firstDayofWeek.addDays(dateRange));
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
    ui->widget_stressPlot->xAxis->grid()->setLayer("GRID");
    ui->widget_stressPlot->yAxis->grid()->setLayer("GRID");
    this->set_stressValues(ui->dateEdit_start->date(),ui->dateEdit_end->date());
}

double stress_popup::calc_stress(double pastStress, double currStress,double pDays)
{
    double stress = 0;
    double factor = (double)exp(-1.0/pDays);

    stress = (currStress * (1.0 - factor)) + (pastStress * factor);

    return stress;
}

void stress_popup::set_stressValues(QDate rangeStart, QDate rangeEnd)
{
<<<<<<< HEAD
    //ui->pushButton_values->setIcon(hideNum);
=======
>>>>>>> refs/remotes/origin/develop
    double pastStress,currStress,startStress,calcStress = 0;
    int ltsStart = -ltsDays;
    int stsStart = -stsDays;
    pastStress = lastLTS;
    int dayCount = dateRange+1;
    xDate.resize(dayCount);
    yLTS.resize(dayCount);
    ySTS.resize(dayCount);
    yTSB.resize(dayCount);

    stressMax = 0;
    tsbMinMax.resize(2);
    tsbMinMax.fill(0);

    for(QMap<QDate,double>::const_iterator it = stressMap->cbegin(), end = stressMap->find(rangeStart.addDays(ltsStart)); it != end; ++it)
    {
        calcStress = calc_stress(pastStress,it.value(),ltsDays);
        pastStress = calcStress;
    }
    startStress = pastStress;

    double dateValue = 0;
    QDateTime startDate;
    QDate dayDate;
    QTime wTime;
    wTime.fromString("00:00:00","hh:mm:ss");
    startDate.setDate(rangeStart);
    startDate.setTime(wTime);
    startDate.setTimeSpec(Qt::LocalTime);

    for(int i = 0; i < dayCount; ++i)
    {
        pastStress = startStress;
        dateValue = startDate.addDays(i).toTime_t();
        xDate[i] = dateValue;
        dayDate = startDate.date().addDays(i);

        for(int x = ltsStart; x <= 0; ++x)
        {
            if(i == 0 && x == 0) yTSB[0] = round(pastStress);
            currStress = stressMap->value(dayDate.addDays(x));
            calcStress = calc_stress(pastStress,currStress,ltsDays);
            pastStress = calcStress;
            if(x == ltsStart) startStress = calcStress;
        }
        yLTS[i] = round(calcStress);
        if(stressMax < yLTS[i]) stressMax = yLTS[i];
    }
    pastStress = lastSTS;

    for(QMap<QDate,double>::const_iterator it = stressMap->cbegin(), end = stressMap->find(rangeStart.addDays(stsStart)); it != end; ++it)
    {
        calcStress = calc_stress(pastStress,it.value(),stsDays);
        pastStress = calcStress;
    }
    startStress = pastStress;

    for(int i = 0; i < dayCount; ++i)
    {
        pastStress = startStress;
        dayDate = startDate.date().addDays(i);

        for(int x = ltsStart; x <= 0; ++x)
        {
            if(i == 0 && x == 0) yTSB[0] = round(yTSB[0]-pastStress);
            currStress = stressMap->value(dayDate.addDays(x));
            calcStress = calc_stress(pastStress,currStress,stsDays);
            pastStress = calcStress;
            if(x == ltsStart) startStress = calcStress;
        }
        ySTS[i] = round(calcStress);
        if(stressMax < ySTS[i]) stressMax = ySTS[i];
    }

    for(int i = 1; i < dayCount;++i)
    {
        yTSB[i] = yLTS[i-1] - ySTS[i-1];
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

    QCPRange xRange(QCPAxisTickerDateTime::dateTimeToKey(rangeStart.addDays(-1)),QCPAxisTickerDateTime::dateTimeToKey(rangeEnd.addDays(1)));

    QFont lineFont;
    lineFont.setPointSize(8);
    int xTickCount = dateRange;
    double dayCount = rangeStart.daysTo(rangeEnd)+2;
    QCPGraph *ltsLine = this->get_QCPLine("LTS",QColor(0,255,0),yLTS,false);
    QCPGraph *stsLine = this->get_QCPLine("STS",QColor(255,0,0),ySTS,false);
    QCPGraph *tsbLine = this->get_QCPLine("TSB",QColor(255,170,0),yTSB,true);
    tsbLine->setBrush(QBrush(QColor(255,170,0,50)));

    for(int i = 0; i < xDate.count(); ++i)
    {
        this->set_itemTracer("LTS",ltsLine,Qt::green,i);
        this->set_itemTracer("STS",stsLine,Qt::red,i);
        this->set_itemTracer("TSB",tsbLine,QColor(255,170,0),i);

        if(!showValues)
        {
            this->set_itemText("LTS",lineFont,yLTS,i,false);
            this->set_itemText("STS",lineFont,ySTS,i,false);
            this->set_itemText("TSB",lineFont,yTSB,i,true);
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
    dayTicker->setTickCount(dayCount);

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
            dateRange = ui->dateEdit_start->date().daysTo(ui->dateEdit_end->date());
            this->set_stressValues(ui->dateEdit_start->date(),ui->dateEdit_end->date());
        }
        else
        {
            dateRange = ui->dateEdit_start->date().daysTo(ui->dateEdit_end->date());
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
            dateRange = ui->dateEdit_start->date().daysTo(date);
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
<<<<<<< HEAD
=======
        ui->pushButton_values->setToolTip("Show Values");
>>>>>>> refs/remotes/origin/develop
    }
    else
    {
        ui->pushButton_values->setIcon(hideNum);
<<<<<<< HEAD
=======
        ui->pushButton_values->setToolTip("Hide Values");
>>>>>>> refs/remotes/origin/develop
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
