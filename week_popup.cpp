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

#include "week_popup.h"
#include "ui_week_popup.h"

week_popup::week_popup(QWidget *parent,QString weekid,schedule *p_sched,standardWorkouts *stdWork) :
    QDialog(parent),
    ui(new Ui::week_popup)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    isLoad = false;
    dayCount = 7;
    workSched = p_sched;
    stdWorkouts = stdWork;
    weekID = weekid;

    barSelection << "Duration" << "Distance" << "Work(kj)" << "Distribution";
    ui->comboBox_yValue->addItems(barSelection);
    xDates.resize(dayCount);
    xDateTick.resize(dayCount+1);
    yWorkouts.resize(dayCount);
    yStress.resize(dayCount+1);
    yDura.resize(dayCount);
    yDist.resize(dayCount);
    yLTS.resize(dayCount+1);
    yWorkCount.resize(dayCount);
    yWorkKj.resize(dayCount);
    yValues.resize(dayCount);
    maxValues.resize(4);
    maxValues.fill(0);

    levelList = settings::get_listValues("Level");

    for(int i = 0; i < levelList.count();++i)
    {
        zoneTime.insert(i,0);
    }

    this->set_plotValues();
}

enum {DURATION,DISTANCE,KJ,DISTRIBUTION};

week_popup::~week_popup()
{
    delete ui;
}

void week_popup::set_plotValues()
{
    QStringList weekMeta = workSched->get_weekMeta(weekID);
    QHash<QDate,QMap<QString,QVector<double> >> *compMap = workSched->get_compValues();
    QMap<QString,QVector<double>> workMap;
    QMap<QDate,QVector<double>> *stressMap = workSched->get_stressMap();
    QMap<int, QStringList> dayWorkouts;
    QMap<int,QString> weekWorkouts;

    QDate workoutDate = QDate::fromString(weekMeta.at(3),workSched->dateFormat);
    QDateTime weekStart;

    weekStart.setDate(QDate::fromString(weekMeta.at(3),workSched->dateFormat));
    weekStart.setTime(QTime::fromString("00:00:00","hh:mm:ss"));
    weekStart.setTimeSpec(Qt::LocalTime);
    firstDay = weekStart;
    int workoutCount = 0;

    for(int daytick = 0; daytick < dayCount+1; ++daytick)
    {
        xDateTick[daytick] = weekStart.addDays(daytick-1).toTime_t() + 3600;
        yStress[daytick] = stressMap->value(workoutDate.addDays(daytick-1)).at(0);
        yLTS[daytick] = round(stressMap->value(workoutDate.addDays(daytick-1)).at(2));
        if(maxValues[0] < yStress[daytick]) maxValues[0] = yStress[daytick];
    }

    for(int day = 0; day < dayCount; ++day)
    {
        xDates[day] = weekStart.addDays(day).toTime_t() + 3600;
        workMap = compMap->value(weekStart.addDays(day).date());

        dayWorkouts = workSched->get_workouts(true,workoutDate.addDays(day).toString(workSched->dateFormat));
        for(QMap<int, QStringList>::const_iterator it = dayWorkouts.cbegin(), end = dayWorkouts.cend(); it != end; ++it)
        {
            weekWorkouts.insert(workoutCount++,it.value().last());
        }

        for(QMap<QString,QVector<double>>::const_iterator it = workMap.cbegin(), end = workMap.cend(); it != end; ++it)
        {
            yWorkouts[day] = yWorkouts.at(day) + it.value().at(0);
            yDura[day] = yDura.at(day) + round((it.value().at(1)/60.0));
            yDist[day] = yDist.at(day) + it.value().at(3);
            yWorkKj[day] = yWorkKj.at(day) + it.value().at(5);
        }

        yWorkCount[day] = yWorkouts.at(day)*10.0;

        if(maxValues[1] < yDura[day]) maxValues[1] = yDura[day];
        if(maxValues[2] < yDist[day]) maxValues[2] = yDist[day];
        if(maxValues[3] < yWorkKj[day]) maxValues[3] = yWorkKj[day];
    }

    //Load Level Distribution
    QVector<double> workLoad(levelList.count(),0);
    double zoneValue = 0;
    double weekSum = 0;

    for( QMap<int,QString>::const_iterator it = weekWorkouts.cbegin(), end = weekWorkouts.cend(); it != end; ++it)
    {
        workLoad = stdWorkouts->get_workLevelLoad(it.value());
        if(workLoad.count() > 0)
        {
            for(int level = 0; level < levelList.count(); ++level)
            {
                weekSum = weekSum + workLoad.at(level);
                zoneValue = zoneTime.value(level) + workLoad.at(level);
                zoneTime.insert(level,zoneValue);
            }
        }
    }
    for( QMap<int,double>::const_iterator it = zoneTime.cbegin(), end = zoneTime.cend(); it != end; ++it)
    {
        if(it.value() > 0)
        {
            zoneTime.insert(it.key(),round((it.value()/weekSum)*100.0));
        }
    }

    ui->label_weekinfos->setText("Week: " + weekMeta.at(0) + " - Phase: " + weekMeta.at(2) + " - Workouts: " + QString::number(workoutCount));

    this->set_graph();
    this->set_weekPlot(0);
}


void week_popup::set_graph()
{
    QFont plotFont;
    plotFont.setBold(true);
    plotFont.setPointSize(8);

    ui->widget_plot->xAxis->setLabel("Day of Week");
    ui->widget_plot->xAxis->setLabelFont(plotFont);
    ui->widget_plot->xAxis2->setVisible(true);
    ui->widget_plot->xAxis2->setLabelFont(plotFont);
    ui->widget_plot->xAxis2->setTickLabels(false);
    ui->widget_plot->yAxis->setLabel("Stress");
    ui->widget_plot->yAxis->setLabelFont(plotFont);
    ui->widget_plot->yAxis2->setVisible(true);
    ui->widget_plot->yAxis2->setLabelFont(plotFont);
    ui->widget_plot->legend->setVisible(true);
    ui->widget_plot->legend->setFont(plotFont);

    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    ui->widget_plot->plotLayout()->addElement(1,0,subLayout);
    subLayout->setMargins(QMargins(dayCount*10,0,dayCount*10,5));
    subLayout->addElement(0,0,ui->widget_plot->legend);

    ui->widget_plot->addLayer("abovemain", ui->widget_plot->layer("main"), QCustomPlot::limAbove);
    ui->widget_plot->addLayer("belowmain", ui->widget_plot->layer("main"), QCustomPlot::limBelow);
}


QCPGraph *week_popup::get_QCPLine(QString name,QColor gColor,QVector<double> &xdata, QVector<double> &ydata, bool secondAxis)
{
    QCPGraph *graph = ui->widget_plot->addGraph();
    if(secondAxis)
    {
        graph->setValueAxis(ui->widget_plot->yAxis2);
    }
    graph->setName(name);
    graph->setLineStyle(QCPGraph::lsLine);
    graph->setData(xdata,ydata);
    graph->setAntialiased(true);
    graph->setPen(QPen(gColor,2));

    return graph;
}

QCPBars *week_popup::get_QCPBar(QColor gColor, int width, int factor,bool secondAxis)
{
    QCPBars *bars;
    if(secondAxis)
    {
        bars = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    }
    else
    {
        bars = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    }

    bars->setWidth(factor*width);
    bars->setAntialiased(true);
    bars->setPen(QPen(gColor));
    gColor.setAlpha(80);
    bars->setBrush(QBrush(gColor));

    return bars;
}

void week_popup::set_itemTracer(QCPGraph *graphline, QVector<double> &xdata,QColor tColor,int pos)
{
    QCPItemTracer *tracer = new QCPItemTracer(ui->widget_plot);
    tracer->setGraph(graphline);
    tracer->setGraphKey(xdata[pos]);
    tracer->setStyle(QCPItemTracer::tsCircle);
    tracer->setBrush(QBrush(tColor));
}

void week_popup::set_itemLineText(QFont lineFont, QVector<double> &xdata,QVector<double> &ydata,int pos)
{
    QCPItemText *itemText = new QCPItemText(ui->widget_plot);
    itemText->position->setType(QCPItemPosition::ptPlotCoords);
    itemText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
    itemText->position->setCoords(xdata[pos],ydata[pos]+1);
    itemText->setText(QString::number(ydata[pos]));
    itemText->setTextAlignment(Qt::AlignCenter);
    itemText->setFont(lineFont);
    itemText->setPadding(QMargins(1, 1, 1, 1));
}

void week_popup::set_itemBarText(QFont barFont, QColor gColor, QVector<double> &xdata, QVector<double> &ydata, int pos, bool secondAxis)
{
    double yCords = 0;

    QCPItemText *barText = new QCPItemText(ui->widget_plot);
    if(secondAxis)
    {
        barText->position->setAxes(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    }

    if(ydata[pos] < ui->widget_plot->yAxis->range().maxRange / 10.0 && !secondAxis)
    {
        yCords = ydata[pos] + 10.0;
    }
    else
    {
        yCords = ydata[pos]/2;
    }

    barText->position->setType(QCPItemPosition::ptPlotCoords);
    barText->position->setCoords(xdata[pos],yCords);

    barText->setText(QString::number(ydata[pos]));
    barText->setFont(barFont);
    barText->setColor(gColor);
}

void week_popup::set_weekPlot(int yValue)
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();
    ui->widget_plot->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->widget_plot->plotLayout()->setRowStretchFactor(1,0.0001);
    QFont lineFont,barFont;
    lineFont.setPointSize(10);
    barFont.setPointSize(10);

    if(yValue == DISTRIBUTION)
    {
        QVector<double> tickCount,zoneValues;
        QVector<QString> zoneLabel;

        for(int i = 0; i < levelList.count(); ++i)
        {
            tickCount << i;
            zoneLabel << levelList.at(i);
            zoneValues << zoneTime.value(i);
        }

        QSharedPointer<QCPAxisTickerText> zoneTicker(new QCPAxisTickerText);
        zoneTicker->addTicks(tickCount, zoneLabel);
        ui->widget_plot->xAxis->setTicker(zoneTicker);

        QCPBars *distbars = this->get_QCPBar(QColor(0,85,255),50,1,false);
        distbars->setWidthType(QCPBars::wtAbsolute);
        distbars->setName("Distribution");
        distbars->setData(tickCount,zoneValues);

        for(int i = 0; i < zoneValues.count(); ++i)
        {
            this->set_itemBarText(barFont,Qt::black,tickCount,zoneValues,i,false);
        }

        ui->widget_plot->xAxis->setRange(-1,7);
        ui->widget_plot->yAxis->setRange(0,100);
        ui->widget_plot->xAxis->setLabel("Level");
        ui->widget_plot->yAxis->setLabel("Percent");
    }
    else
    {
        QCPRange xRange(QCPAxisTickerDateTime::dateTimeToKey(firstDay.addDays(-1)),QCPAxisTickerDateTime::dateTimeToKey(firstDay.addDays(dayCount)));

        QCPGraph *stressLine = this->get_QCPLine("StessScore",QColor(255,0,0),xDateTick,yStress,false);
        stressLine->setBrush(QBrush(QColor(255,0,0,50)));

        QCPGraph *ltsLine = this->get_QCPLine("LTS",QColor(0,255,0),xDateTick,yLTS,false);

        QCPBars *workBars = this->get_QCPBar(QColor(225,225,100),3000,dayCount,false);
        workBars->setName("Workouts");
        workBars->setData(xDates,yWorkCount);

        QCPBars *scaleBars =  this->get_QCPBar(QColor(0,85,255),6000,dayCount,true);
        scaleBars->setPen(QPen(Qt::darkBlue));

        stressLine->setLayer("abovemain");
        workBars->setLayer("abovemain");
        scaleBars->setLayer("belowmain");
        ui->widget_plot->xAxis->grid()->setLayer("belowmain");
        ui->widget_plot->yAxis->grid()->setLayer("belowmain");

        ui->widget_plot->yAxis->setRange(0,maxValues[0]+20);

        if(yValue == DURATION)
        {
            std::copy(yDura.begin(),yDura.end(),yValues.begin());
            scaleBars->setName(barSelection.at(0)+"(Min)");
            scaleBars->setData(xDates,yValues);
            ui->widget_plot->yAxis2->setRange(0,maxValues[1]+(maxValues[1]*0.1));
            ui->widget_plot->yAxis2->setLabel(barSelection.at(0)+"(Min)");
        }
        if(yValue == DISTANCE)
        {
            std::copy(yDist.begin(),yDist.end(),yValues.begin());
            scaleBars->setName(barSelection.at(1)+"(Km)");
            scaleBars->setData(xDates,yValues);
            ui->widget_plot->yAxis2->setRange(0,maxValues[2]+(maxValues[2]*0.1));
            ui->widget_plot->yAxis2->setLabel(barSelection.at(1)+"(Km)");
        }
        if(yValue == KJ)
        {
            std::copy(yWorkKj.begin(),yWorkKj.end(),yValues.begin());
            scaleBars->setName(barSelection.at(2));
            scaleBars->setData(xDates,yValues);
            ui->widget_plot->yAxis2->setRange(0,maxValues[3]+(maxValues[3]*0.1));
            ui->widget_plot->yAxis2->setLabel(barSelection.at(2));
        }

        for(int i = 0; i < dayCount; ++i)
        {
            this->set_itemBarText(barFont,Qt::white,xDates,yValues,i,true);
            this->set_itemBarText(barFont,Qt::red,xDates,yWorkouts,i,false);
        }

        for(int x = 0; x < dayCount+1; ++x)
        {
            this->set_itemTracer(stressLine,xDateTick,Qt::red,x);
            this->set_itemLineText(lineFont,xDateTick,yStress,x);
            this->set_itemTracer(ltsLine,xDateTick,Qt::green,x);
            this->set_itemLineText(lineFont,xDateTick,yLTS,x);
        }

        QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
        dateTimeTicker->setDateTimeSpec(Qt::LocalTime);
        dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
        dateTimeTicker->setDateTimeFormat("dd.MM");
        dateTimeTicker->setTickCount(dayCount);

        ui->widget_plot->xAxis->setRange(xRange);
        ui->widget_plot->xAxis->setTicker(dateTimeTicker);
    }

    ui->widget_plot->replot();
    isLoad = true;
}

void week_popup::on_comboBox_yValue_currentIndexChanged(int index)
{
    if(isLoad) this->set_weekPlot(index);
}

void week_popup::on_toolButton_close_clicked()
{
    reject();
}

void week_popup::on_toolButton_edit_clicked()
{
    accept();
}
