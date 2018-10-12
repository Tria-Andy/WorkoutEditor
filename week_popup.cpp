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

week_popup::week_popup(QWidget *parent,QString weekinfo,schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::week_popup)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    isLoad = false;
    dayCount = 7;
    workSched = p_sched;
    week_info << weekinfo.split("#");

    barSelection << "Duration" << "Distance" << "Work(kj)" << "Distribution";
    ui->comboBox_yValue->addItems(barSelection);
    xStress.resize(dayCount);
    xLTS.resize(dayCount+1);
    yLTS.resize(dayCount+1);
    yStress.resize(dayCount);
    xBar.resize(dayCount);
    yDura.resize(dayCount);
    yDist.resize(dayCount);
    yWorkKj.resize(dayCount);
    xWorks.resize(dayCount);
    yWorks.resize(dayCount);
    yWorkCount.resize(dayCount);
    yValues.resize(dayCount);
    maxValues.resize(4);
    maxValues.fill(0);

    levelList = settings::get_listValues("Level");
    levelCount = levelList.count();

    for(int i = 0; i < levelCount;++i)
    {
        zoneTime.insert(levelList.at(i),0);
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
    workSched->filter_schedule(week_info.at(0),0,false);
    workSched->scheduleProxy->sort(0);
    int proxyCount = workSched->scheduleProxy->rowCount();
    QDate tempDate;
    QDateTime workDateTime;
    QStringList workDetails;
    QString currZone,part,parent;
    QMap<QString,int> factorMap;
    int groupFactor = 1;
    int seriesFactor = 1;
    int factor = 1;
    double timeSum = 0;
    double currTime = 0;

    if(proxyCount > 0)
    {
        QDateTime weekStart,workoutDate;
        QTime wTime;
        wTime.fromString("00:00:00","hh:mm:ss");
        tempDate = QDate::fromString(workSched->scheduleProxy->data(workSched->scheduleProxy->index(0,1)).toString(),"dd.MM.yyyy");
        weekStart.setDate(tempDate.addDays(1 - tempDate.dayOfWeek()));
        weekStart.setTime(wTime);
        weekStart.setTimeSpec(Qt::LocalTime);
        firstDay = weekStart;

        for(int i = 0; i < dayCount; ++i)
        {
            weekDates.insert(i,weekStart.addDays(i));
        }

        double stress,dura,dist,workkj;

        ui->label_weekinfos->setText("Week: " + week_info.at(0) + " - Phase: " + week_info.at(1) + " - Workouts: " + QString::number(proxyCount));

        double dateValue = 0;
        double ltsDays = ltsValues->value("ltsdays");
        double lte = (double)exp(-1.0/ltsDays);
        int ltsStart = -ltsDays;
        double ltsStress = 0,currStress = 0,pastStress = 0,startLTS = 0;
        QMap<QDate,QPair<double,double> > *stressMap = workSched->get_StressMap();
        pastStress = ltsValues->value("lastlts");

        for(QMap<QDate,QPair<double,double> >::const_iterator it = stressMap->cbegin(), end = stressMap->find(weekDates.at(0).date().addDays(ltsStart)); it != end; ++it)
        {
            currStress = it.value().first;
            ltsStress = (currStress * (1.0 - lte)) + (pastStress * lte);
            pastStress = ltsStress;
        }
        startLTS = pastStress;
        xLTS[0] = weekDates.at(0).addDays(-1).toTime_t();

        for(int i = 0; i < dayCount; ++i)
        {
            pastStress = startLTS;
            dateValue = weekDates.at(i).toTime_t() + 3600;
            xStress[i] = dateValue;
            xBar[i] = dateValue;
            xWorks[i] = dateValue;
            xLTS[i+1] = dateValue;

            for(int x = ltsStart; x <= 0; ++x)
            {
                if(i == 0 && x == 0) yLTS[0] = round(pastStress);
                currStress = stressMap->value(weekDates.at(i).date().addDays(x)).first;
                ltsStress = (currStress * (1.0 - lte)) + (pastStress * lte);
                pastStress = ltsStress;
                if(x == ltsStart) startLTS = ltsStress;

            }
            yLTS[i+1] = round(ltsStress);
        }

        for(int i = 0,day = 0; i < proxyCount; ++i)
        {
            workoutDate = QDateTime::fromString(workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,1,QModelIndex())).toString(),"dd.MM.yyyy");
            workoutDate.setTime(wTime);
            workoutDate.setTimeSpec(Qt::LocalTime);

            stress = workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,8,QModelIndex())).toDouble();
            dura = static_cast<double>(this->get_timesec(workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,6,QModelIndex())).toString())) / 60;
            dist = workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,7,QModelIndex())).toDouble();
            workkj = workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,9,QModelIndex())).toDouble();

            for( ; day < weekDates.count(); ++day)
            {
                if(workoutDate == weekDates.at(day))
                {
                    yStress[day] = yStress[day] + stress;
                    yDura[day] = yDura[day] + this->set_doubleValue(dura,false);
                    yDist[day] = yDist[day] + dist;
                    yWorkKj[day] = yWorkKj[day] + workkj;
                    yWorkCount[day] = yWorkCount[day]+1;
                }
                if(maxValues[0] < yStress[day]) maxValues[0] = yStress[day];
                if(maxValues[1] < yDura[day]) maxValues[1] = yDura[day];
                if(maxValues[2] < yDist[day]) maxValues[2] = yDist[day];
                if(maxValues[3] < yWorkKj[day]) maxValues[3] = yWorkKj[day];
            }
            day = 0;
        }

        for(int i = 0; i < yWorkCount.count(); ++i)
        {
            yWorks[i] = yWorkCount[i]*10;
        }

        //Load Level Distribution
        workSched->filter_schedule(week_info.at(0),0,false);
        workSched->scheduleProxy->sort(1);

        for(int i = 0; i < workSched->scheduleProxy->rowCount(); ++i)
        {
            workDateTime.setTime(QTime());
            workDateTime.setDate(QDate::fromString(workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,1)).toString(),"dd.MM.yyyy"));
            workDateTime.setTime(QTime::fromString(workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,2)).toString(),"hh:mm"));

            workDetails.append(workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,3)).toString());
            workDetails.append(workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,6)).toString());
            workDetails.append(workSched->scheduleProxy->data(workSched->scheduleProxy->index(i,10)).toString());

            weekworkouts.insert(workDateTime,workDetails);
            workDetails.clear();
        }

        for(QMap<QDateTime,QStringList>::const_iterator it = weekworkouts.cbegin(), end = weekworkouts.cend(); it != end; ++it)
        {
            this->filter_steps(it.value().at(2),false);

            for(int x = 0; x < stepProxy->rowCount(); ++x)
            {
                currZone = stepProxy->data(stepProxy->index(x,3)).toString();
                part = stepProxy->data(stepProxy->index(x,2)).toString();
                parent = stepProxy->data(stepProxy->index(x,8)).toString();

                if(part.contains("Group") || part.contains("Series"))
                {
                    factorMap.insert(part,stepProxy->data(stepProxy->index(x,7)).toInt());
                }

                if(parent.contains("Group") && part.contains("Series"))
                {
                    groupFactor = factorMap.value(parent);
                    seriesFactor = factorMap.value(part);
                    factorMap.insert(part,groupFactor*seriesFactor);
                    factor = factorMap.value(parent);
                }
                else
                {
                    factor = 1;
                }

                currTime = this->get_timesec(stepProxy->data(stepProxy->index(x,5)).toString())*factor;
                zoneTime.insert(currZone,zoneTime.value(currZone) + currTime);
                timeSum = timeSum + currTime;
            }
            factorMap.clear();
        }

        for(QMap<QString,double>::const_iterator it = zoneTime.cbegin(), end = zoneTime.cend(); it != end; ++it)
        {
            zoneTime.insert(it.key(),round((it.value() / timeSum)*100.0));
        }

        this->set_graph();
        this->set_weekPlot(0);
    }
    else
    {
        ui->label_weekinfos->setText("Week: " + week_info.at(0) + " - Phase: " + week_info.at(1) + " - Workouts: " + QString::number(proxyCount));
        this->set_graph();
    }
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


QCPGraph *week_popup::get_QCPLine(QString name,QColor gColor,QVector<double> &xdata,QVector<double> &ydata, bool secondAxis)
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

void week_popup::set_itemBarText(QFont barFont, QColor gColor, QVector<double> &xdata, QVector<double> &ydata,QVector<double> &ytext, int pos, bool secondAxis)
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

    barText->setText(QString::number(ytext[pos]));
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
            zoneValues << zoneTime.value(levelList.at(i));
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
            this->set_itemBarText(barFont,Qt::black,tickCount,zoneValues,zoneValues,i,false);
        }

        ui->widget_plot->xAxis->setRange(-1,7);
        ui->widget_plot->yAxis->setRange(0,100);
        ui->widget_plot->xAxis->setLabel("Level");
        ui->widget_plot->yAxis->setLabel("Percent");
    }
    else
    {
        QCPRange xRange(QCPAxisTickerDateTime::dateTimeToKey(firstDay.addDays(-1)),QCPAxisTickerDateTime::dateTimeToKey(firstDay.addDays(dayCount)));

        QCPGraph *stressLine = this->get_QCPLine("StessScore",QColor(255,0,0),xStress,yStress,false);
        stressLine->setBrush(QBrush(QColor(255,0,0,50)));

        QCPGraph *ltsLine = this->get_QCPLine("LTS",QColor(0,255,0),xLTS,yLTS,false);

        QCPBars *workBars = this->get_QCPBar(QColor(225,225,100),3000,dayCount,false);
        workBars->setName("Workouts");
        workBars->setData(xWorks,yWorks);

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
            qCopy(yDura.begin(),yDura.end(),yValues.begin());
            scaleBars->setName(barSelection.at(0)+"(Min)");
            scaleBars->setData(xBar,yValues);
            ui->widget_plot->yAxis2->setRange(0,maxValues[1]+(maxValues[1]*0.1));
            ui->widget_plot->yAxis2->setLabel(barSelection.at(0)+"(Min)");
        }
        if(yValue == DISTANCE)
        {
            qCopy(yDist.begin(),yDist.end(),yValues.begin());
            scaleBars->setName(barSelection.at(1)+"(Km)");
            scaleBars->setData(xBar,yValues);
            ui->widget_plot->yAxis2->setRange(0,maxValues[2]+(maxValues[2]*0.1));
            ui->widget_plot->yAxis2->setLabel(barSelection.at(1)+"(Km)");
        }
        if(yValue == KJ)
        {
            qCopy(yWorkKj.begin(),yWorkKj.end(),yValues.begin());
            scaleBars->setName(barSelection.at(2));
            scaleBars->setData(xBar,yValues);
            ui->widget_plot->yAxis2->setRange(0,maxValues[3]+(maxValues[3]*0.1));
            ui->widget_plot->yAxis2->setLabel(barSelection.at(2));
        }

        for(int i = 0; i < dayCount; ++i)
        {
            this->set_itemTracer(stressLine,xStress,Qt::red,i);
            this->set_itemLineText(lineFont,xStress,yStress,i);
            this->set_itemBarText(barFont,Qt::white,xBar,yValues,yValues,i,true);
            this->set_itemBarText(barFont,Qt::red,xWorks,yWorks,yWorkCount,i,false);
        }

        for(int i = 0; i < xLTS.count(); ++i)
        {
            this->set_itemTracer(ltsLine,xLTS,Qt::green,i);
            this->set_itemLineText(lineFont,xLTS,yLTS,i);
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
