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

#include "year_popup.h"
#include "ui_year_popup.h"

year_popup::year_popup(QWidget *parent, QString pInfo,int position,schedule *p_sched,QString pPhase, int pIndex) :
    QDialog(parent),
    ui(new Ui::year_popup)
{
    ui->setupUi(this);
       setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
       isLoad = false;
       partInfo = pInfo.split("-");
       workSched = p_sched;
       phase = pPhase;
       phaseindex = pIndex;
       phaseList << "Year" << settings::get_listValues("Phase");
       selectList << "Duration" << "Distance" << "Workouts";
       ui->comboBox_select->addItems(selectList);
       sportUseList = settings::get_listValues("Sportuse");

       if(position == 0)
       {
           col = workSched->week_content->columnCount()-1;
       }
       else
       {
           col = position+1;
       }

       ui->label_info->setText(phaseList.at(phaseindex) +": " + partInfo.at(0) + " Workouts: " + partInfo.at(1) + " - Hours: " + partInfo.at(2) + " - Distance: " + partInfo.at(4));

       metaProxy = new QSortFilterProxyModel(this);
       metaProxy->setSourceModel(workSched->week_meta);
       proxyFilter = new QSortFilterProxyModel(this);
       proxyFilter->setSourceModel(metaProxy);
       contentProxy = new QSortFilterProxyModel(this);
       contentProxy->setSourceModel(workSched->week_content);

       this->set_plotValues();
}

enum {DURATION,DISTANCE,WORKOUTS};

year_popup::~year_popup()
{
    delete ui;
}

void year_popup::set_plotValues()
{
    QString selSaison = workSched->get_selSaison();

    metaProxy->invalidate();
    metaProxy->setFilterFixedString(selSaison);
    metaProxy->setFilterKeyColumn(0);


    QString weekID;

    if(phaseindex == 0)
    {
        weekcount = workSched->get_saisonInfo(selSaison,"weeks").toInt();
    }
    else
    {
        proxyFilter->invalidate();
        proxyFilter->setFilterFixedString(phase);
        proxyFilter->setFilterKeyColumn(3);
        weekcount = proxyFilter->rowCount();
    }

    if(weekcount > 40)
    {
        widthFactor = 25;
        heightFactor = 1;
    }
    else if(weekcount <= 40 && weekcount > 20)
    {
        widthFactor = 50;
        heightFactor = 1;
    }
    else if(weekcount <= 20 && weekcount > 10)
    {
        widthFactor = 70;
        heightFactor = 1;
    }
    else if(weekcount <= 10 && weekcount > 5)
    {
        widthFactor = 100;
        heightFactor = 1;
    }
    else if(weekcount <= 5 && weekcount > 3)
    {
        widthFactor = 150;
        heightFactor = 2;
    }
    else
    {
        widthFactor = 300;
        heightFactor = 3;
    }

    this->setFixedWidth(widthFactor*weekcount);
    maxValues.resize(3);
    maxValues.fill(0.0);
    xWeeks.resize(weekcount);
    yStress.resize(weekcount);
    yDura.resize(weekcount);
    yDist.resize(weekcount);
    yWorks.resize(weekcount);
    yValues.resize(weekcount);

    for(int i = 1; i <= weekcount; ++i)
    {
        xWeeks[i-1] = i;
    }

    QString sumValue,stress,duration,distance,workouts;
    QStringList sumValues;
    max_stress = 0.0;

    for(int week = 0; week < proxyFilter->rowCount(); ++week)
    {
        weekID = proxyFilter->data(proxyFilter->index(week,2)).toString();
        contentProxy->invalidate();
        contentProxy->setFilterRegExp("\\b"+weekID+"\\b");
        contentProxy->setFilterKeyColumn(1);

        sumValue = contentProxy->data(contentProxy->index(0,col)).toString();
        sumValues = sumValue.split("-");
        workouts = sumValues.at(0);
        distance = sumValues.at(1);
        duration = sumValues.at(2);
        stress = sumValues.at(3);

        yStress[week] = stress.toDouble();
        if(max_stress < yStress[week]) max_stress = yStress[week];

        yDura[week] = this->set_doubleValue(static_cast<double>(this->get_timesec(duration) / 60.0),false);
        if(maxValues[0] < yDura[week]) maxValues[0] = yDura[week];
        yDist[week] = this->set_doubleValue(distance.toDouble(),false);
        if(maxValues[1] < yDist[week]) maxValues[1] = yDist[week];
        yWorks[week] = workouts.toDouble();
        if(maxValues[2] < yWorks[week]) maxValues[2] = yWorks[week];
        if(phaseindex != 0) weekList << weekID;
    }

    this->set_graph();
}

void year_popup::set_graph()
{
    QFont plotFont;
    plotFont.setBold(true);
    plotFont.setPointSize(8);

    ui->widget_plot->xAxis->setLabel("Weeks");
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
    subLayout->setMargins(QMargins(weekcount*10,0,weekcount*10,5));
    subLayout->addElement(0,0,ui->widget_plot->legend);

    this->set_plot(0);
}

void year_popup::set_plot(int yValue)
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();   
    ui->widget_plot->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->widget_plot->plotLayout()->setRowStretchFactor(1,0.0001);

    QSharedPointer<QCPAxisTickerText> weekTicker(new QCPAxisTickerText);
    weekTicker->addTick(0,"Start");
    if(phaseindex != 0)
    {
        for(int i = 1; i <= weekcount; ++i)
        {
            weekTicker->addTick(i,weekList.at(i-1));
        }
    }
    else
    {
        for(int i = 1; i <= weekcount; ++i)
        {
            weekTicker->addTick(i,QString::number(i));
        }
    }
    weekTicker->addTick(weekcount+1,"End");

    ui->widget_plot->xAxis->setRange(0,weekcount+1);
    ui->widget_plot->xAxis->setTicker(weekTicker);

    QFont lineFont,barFont;
    lineFont.setPointSize(10);
    barFont.setPointSize(10);

    QCPGraph *stressLine = ui->widget_plot->addGraph();
    stressLine->setName("StressScore");
    stressLine->setLineStyle(QCPGraph::lsLine);
    stressLine->setData(xWeeks,yStress);
    stressLine->setAntialiased(true);
    stressLine->setBrush(QBrush(QColor(255,0,0,50)));
    stressLine->setPen(QPen(QColor(255,0,0),2));

    QCPBars *bars = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    bars->setWidth(0.75);
    bars->setAntialiased(true);
    bars->setBrush(QBrush(QColor(0,85,255,70)));
    bars->setPen(QPen(Qt::darkBlue));

    if(yValue == DURATION)
    {
        qCopy(yDura.begin(),yDura.end(),yValues.begin());
        bars->setName(selectList.at(0));
        ui->widget_plot->yAxis2->setRange(0,maxValues[0]+(maxValues[0]*0.2));
        ui->widget_plot->yAxis2->setLabel(selectList.at(0));
    }
    if(yValue == DISTANCE)
    {
        qCopy(yDist.begin(),yDist.end(),yValues.begin());
        bars->setName(selectList.at(1));
        ui->widget_plot->yAxis2->setRange(0,maxValues[1]+(maxValues[1]*0.2));
        ui->widget_plot->yAxis2->setLabel(selectList.at(1));
    }
    if(yValue == WORKOUTS)
    {
        qCopy(yWorks.begin(),yWorks.end(),yValues.begin());
        bars->setName(selectList.at(2));
        ui->widget_plot->yAxis2->setRange(0,maxValues[2]+(maxValues[2]*0.3));
        ui->widget_plot->yAxis2->setLabel(selectList.at(2));
    }

    bars->setData(xWeeks,yValues);

    if(phaseindex != 0)
    {
        for(int i = 0; i < weekcount; ++i)
        {
            QCPItemTracer *itemTracer = new QCPItemTracer(ui->widget_plot);
            itemTracer->setGraph(stressLine);
            itemTracer->setGraphKey(xWeeks[i]);
            itemTracer->setStyle(QCPItemTracer::tsCircle);
            itemTracer->setBrush(Qt::red);

            QCPItemText *lineText = new QCPItemText(ui->widget_plot);
            lineText->position->setType(QCPItemPosition::ptPlotCoords);
            lineText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
            lineText->position->setCoords(xWeeks[i],yStress[i]+1);
            lineText->setText(QString::number(yStress[i]));
            lineText->setTextAlignment(Qt::AlignCenter);
            lineText->setFont(lineFont);
            lineText->setPadding(QMargins(1, 1, 1, 1));

            QCPItemText *barText = new QCPItemText(ui->widget_plot);
            barText->position->setType(QCPItemPosition::ptPlotCoords);
            barText->position->setAxes(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
            barText->position->setCoords(xWeeks[i],yValues[i]/2);
            barText->setText(QString::number(yValues[i]));
            barText->setFont(barFont);
            barText->setColor(Qt::white);
        }
        ui->widget_plot->yAxis->setRange(0,max_stress+(weekcount*(heightFactor*10)));
    }
    else
    {
        ui->widget_plot->yAxis->setRange(0,max_stress+(max_stress*0.2));
    }

    ui->widget_plot->replot();
    isLoad = true;
}

void year_popup::on_comboBox_select_currentIndexChanged(int index)
{
    if(isLoad)this->set_plot(index);
}

void year_popup::on_toolButton_close_clicked()
{
    reject();
}
