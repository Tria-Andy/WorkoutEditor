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

year_popup::year_popup(QWidget *parent, QString pInfo,schedule *p_sched,QString pPhase) :
    QDialog(parent),
    ui(new Ui::year_popup)
{
    ui->setupUi(this);
       setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
       isLoad = false;
       phaseInfo = pInfo.split("-");
       workSched = p_sched;
       phaseName = pPhase;
       phaseList << "Year" << settings::get_listValues("Phase");
       selectList << "Duration" << "Distance" << "Workouts";
       ui->comboBox_select->addItems(selectList);
       sportUseList = settings::get_listValues("Sportuse");

       ui->label_info->setText(phaseName +": " + phaseInfo.at(0) + " Workouts: " + phaseInfo.at(1) + " - Hours: " + set_time(phaseInfo.at(2).toInt()) + " - Distance: " + phaseInfo.at(4));

       this->set_plotValues();
}

enum {DURATION,DISTANCE,WORKOUTS};

year_popup::~year_popup()
{
    delete ui;
}

void year_popup::set_comValues(const QVector<double> values,int pos)
{
    yWorks[pos] = yWorks.at(pos) + values.at(0);
    yDura[pos] = yDura.at(pos) + (values.at(1)/60.0/60.0);
    yDist[pos] = yDist.at(pos) + values.at(3);
    yStress[pos] = yStress.at(pos) + values.at(4);

    if(maxValues[0] < yDura[pos]) maxValues[0] = yDura.at(pos);
    if(maxValues[1] < yDist[pos]) maxValues[1] = yDist.at(pos);
    if(maxValues[2] < yWorks[pos]) maxValues[2] = yWorks.at(pos);
    if(maxValues[3] < yStress[pos]) maxValues[3] = yStress.at(pos);
}


void year_popup::read_compValues(QStandardItem *phaseItem,QString sport,int pos)
{
    QHash<QString,QMap<QString,QVector<double> >> *compWeekMap = workSched->get_compWeekValues();
    QMap<QString,QVector<double>> compValues;

    if(phaseItem->hasChildren())
    {
        for(int week = 0; week < phaseItem->rowCount(); ++week)
        {
            compValues = compWeekMap->value(phaseItem->child(week,0)->data(Qt::DisplayRole).toString());
            weekList << phaseItem->child(week,0)->data(Qt::DisplayRole).toString();

            if(sport == generalValues->value("sum"))
            {
                for(QMap<QString,QVector<double>>::const_iterator it = compValues.cbegin(), end = compValues.cend(); it != end; ++it)
                {
                    this->set_comValues(it.value(),week+pos);
                }
            }
            else
            {
                this->set_comValues(compValues.value(sport),week+pos);
            }
        }
    }
}

void year_popup::set_plotValues()
{
    QStandardItem *phaseItem = nullptr;

    if(phaseName == "All")
    {
        QString selSaison = workSched->get_selSaison();
        phaseItem = workSched->get_phaseItem(selSaison);
        weekcount = workSched->get_saisonValues()->value(selSaison).at(2).toInt();
    }
    else
    {
        phaseItem = workSched->get_phaseItem(phaseName);
        weekcount = phaseItem->rowCount();
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
    maxValues.resize(4);
    maxValues.fill(0.0);
    xWeeks.resize(weekcount);
    yStress.resize(weekcount);
    yStress.fill(0);
    yDura.resize(weekcount);
    yDura.fill(0);
    yDist.resize(weekcount);
    yDist.fill(0);
    yWorks.resize(weekcount);
    yWorks.fill(0);
    yValues.resize(weekcount);

    for(int i = 1; i <= weekcount; ++i)
    {
        xWeeks[i-1] = i;
    }

    if(phaseName == "All")
    {
        int phasePos = 0;

        for(int phase = 0; phase < phaseItem->rowCount(); ++phase)
        {
            this->read_compValues(phaseItem->child(phase,0),phaseInfo.at(0),phasePos);
            phasePos = phasePos + phaseItem->child(phase,0)->rowCount();
        }
    }
    else
    {
        this->read_compValues(phaseItem,phaseInfo.at(0),0);
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

    QMap<double,QString> tickerList;
    tickerList.insert(0,"Start");

    QSharedPointer<QCPAxisTickerText> weekTicker(new QCPAxisTickerText);
    if(phaseName != "All")
    {
        for(int i = 1; i <= weekcount; ++i)
        {
            tickerList.insert(i,weekList.at(i-1));
        }
    }
    else
    {
        for(int i = 1; i <= weekcount; ++i)
        {
            tickerList.insert(i,QString::number(i));
        }
    }
    tickerList.insert(tickerList.count(),"End");
    weekTicker->addTicks(tickerList);

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

        std::copy(yDura.begin(),yDura.end(),yValues.begin());
        bars->setName(selectList.at(0));
        ui->widget_plot->yAxis2->setRange(0,maxValues[0]+(maxValues[0]*0.2));
        ui->widget_plot->yAxis2->setLabel(selectList.at(0));
    }
    if(yValue == DISTANCE)
    {
        std::copy(yDist.begin(),yDist.end(),yValues.begin());
        bars->setName(selectList.at(1));
        ui->widget_plot->yAxis2->setRange(0,maxValues[1]+(maxValues[1]*0.2));
        ui->widget_plot->yAxis2->setLabel(selectList.at(1));
    }
    if(yValue == WORKOUTS)
    {
        std::copy(yWorks.begin(),yWorks.end(),yValues.begin());
        bars->setName(selectList.at(2));
        ui->widget_plot->yAxis2->setRange(0,maxValues[2]+(maxValues[2]*0.3));
        ui->widget_plot->yAxis2->setLabel(selectList.at(2));
    }

    bars->setData(xWeeks,yValues);

    if(phaseName != "All")
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
        ui->widget_plot->yAxis->setRange(0,maxValues.at(3)+(weekcount*(heightFactor*10)));
    }
    else
    {
        ui->widget_plot->yAxis->setRange(0,maxValues.at(3)+(maxValues.at(3)*0.2));
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
