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
    isLoad = false;
    dayCount = 7;
    week_info << weekinfo.split("#");
    workSched = p_sched;
    filledWeek = true;
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    ui->comboBox_yValue->addItem("Duration");
    ui->comboBox_yValue->addItem("Distance");
    xStress.resize(dayCount);
    yStress.resize(dayCount);
    xBar.resize(dayCount);
    yDura.resize(dayCount);
    yDist.resize(dayCount);
    xWorks.resize(dayCount);
    yWorks.resize(dayCount);
    yValues.resize(dayCount);
    maxValues.resize(3);
    maxValues.fill(0);
    this->set_plotModel();
}

enum {DURATION,DISTANCE};

week_popup::~week_popup()
{
    delete ui;
}

void week_popup::freeMem()
{

}

void week_popup::set_plotModel()
{
    QModelIndex index;
    QDate tempDate;
    QList<QStandardItem*> list = workSched->workout_schedule->findItems(week_info.at(0),Qt::MatchExactly,0);
    index = workSched->workout_schedule->indexFromItem(list.at(0));
    QDateTime weekStart,workoutDate;
    QTime wTime;
    wTime.fromString("00:00:00","hh:mm:ss");
    tempDate = QDate::fromString(workSched->workout_schedule->item(index.row(),1)->text(),"dd.MM.yyyy");
    weekStart.setDate(tempDate.addDays(1 - tempDate.dayOfWeek()));
    weekStart.setTime(wTime);
    weekStart.setTimeSpec(Qt::UTC);
    firstDay = weekStart.date();

    for(int i = 0; i < 7; ++i)
    {
        weekDates.insert(i,weekStart.addDays(i));
    }

    plotmodel = new QStandardItemModel(list.count(),4);

    if(!list.isEmpty())
    {
        double stress,dura,dist;

        for(int i = 0; i < list.count(); ++i)
        {
            index = workSched->workout_schedule->indexFromItem(list.at(i));
            workoutDate.setDate(QDate::fromString(workSched->workout_schedule->item(index.row(),1)->text(),"dd.MM.yyyy"));
            workoutDate.setTime(wTime);
            workoutDate.setTimeSpec(Qt::UTC);
            plotmodel->setData(plotmodel->index(i,0,QModelIndex()),workoutDate); //Date
            plotmodel->setData(plotmodel->index(i,1,QModelIndex()),workSched->workout_schedule->item(index.row(),8)->text().toInt()); //Stress
            plotmodel->setData(plotmodel->index(i,2,QModelIndex()),settings::get_timesec(workSched->workout_schedule->item(index.row(),6)->text()) / 60.0); //Dura
            plotmodel->setData(plotmodel->index(i,3,QModelIndex()),workSched->workout_schedule->item(index.row(),7)->text().toDouble()); //Dist
        }
        plotmodel->sort(0);

        ui->label_weekinfos->setText("Week: " + week_info.at(0) + " - Phase: " + week_info.at(1) + " - Workouts: " + QString::number(list.count()));

        double dateValue = 0;

        for(int i = 0; i < dayCount; ++i)
        {
            dateValue = weekDates.at(i).toTime_t();
            xStress[i] = dateValue;
            xBar[i] = dateValue;
        }
        workoutDate.setTimeSpec(Qt::UTC);

        for(int i = 0,day = 0; i < plotmodel->rowCount(); ++i)
        {
            workoutDate = plotmodel->data(plotmodel->index(i,0,QModelIndex())).toDateTime();
            stress = plotmodel->data(plotmodel->index(i,1,QModelIndex())).toDouble();
            dura = plotmodel->data(plotmodel->index(i,2,QModelIndex())).toDouble() / 60;
            dist = plotmodel->data(plotmodel->index(i,3,QModelIndex())).toDouble();

            if(workoutDate == weekDates.at(day))
            {
                yStress[day] = yStress[day] + stress;
                yDura[day] = yDura[day] + settings::set_doubleValue(dura,false);
                yDist[day] = yDist[day] + dist;
            }
            else
            {
                ++day;
                yStress[day] = stress;
                yDura[day] = settings::set_doubleValue(dura,false);
                yDist[day] = dist;
            }
            if(maxValues[0] < yStress[day]) maxValues[0] = yStress[day];
            if(maxValues[1] < yDura[day]) maxValues[1] = yDura[day];
            if(maxValues[2] < yDist[day]) maxValues[2] = yDist[day];
        }

        delete plotmodel;
    }
    else
    {
        ui->label_weekinfos->setText("Week: " + week_info.at(0) + " - Phase: " + week_info.at(1) + " - Workouts: " + QString::number(list.count()));
    }
    this->set_graph();
}

void week_popup::set_graph()
{
    ui->widget_plot->yAxis->setLabel("Stress");
    ui->widget_plot->xAxis->setLabel("Day of Week");
    ui->widget_plot->xAxis2->setVisible(true);
    ui->widget_plot->xAxis2->setTickLabels(false);
    ui->widget_plot->xAxis2->setLabel("Workouts");
    ui->widget_plot->yAxis2->setVisible(true);
    this->set_weekPlot(0);
}

void week_popup::set_weekPlot(int yValue)
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();
    QCPRange xRange(QCPAxisTickerDateTime::dateTimeToKey(firstDay.addDays(-1)),QCPAxisTickerDateTime::dateTimeToKey(firstDay.addDays(dayCount)));

    QCPGraph *stressLine = ui->widget_plot->addGraph();
    QCPGraph *workLine = ui->widget_plot->addGraph();

    QFont lineFont,barFont;
    lineFont.setPointSize(10);
    barFont.setPointSize(10);

    stressLine->setLineStyle(QCPGraph::lsLine);
    stressLine->setAntialiased(true);
    stressLine->setPen(QPen(QColor(255,0,0),2));
    stressLine->setBrush(QBrush(QColor(255,0,0,70)));
    stressLine->setData(xStress,yStress);

    ui->widget_plot->yAxis->setRange(0,maxValues[0]+10);

    if(yValue == DURATION)
    {
        qCopy(yDura.begin(),yDura.end(),yValues.begin());

        QCPBars *bars = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
        bars->setWidth(dayCount*6000);
        bars->setData(xBar,yValues);
        bars->setBrush(QBrush(QColor(0,85,255,80)));
        bars->setPen(QPen(Qt::darkBlue));


        ui->widget_plot->yAxis2->setRange(0,maxValues[1]+0.5);
        ui->widget_plot->yAxis2->setLabel("Duration");
    }
    if(yValue == DISTANCE)
    {
        qCopy(yDist.begin(),yDist.end(),yValues.begin());

        QCPBars *barDist = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
        barDist->setWidth(dayCount*6000);
        barDist->setData(xBar,yValues);
        barDist->setBrush(QBrush(QColor(0,85,255,80)));
        barDist->setPen(QPen(Qt::darkBlue));

        ui->widget_plot->yAxis2->setRange(0,maxValues[2]+5);
        ui->widget_plot->yAxis2->setLabel("Distance");

    }

    for(int i = 0; i < dayCount; ++i)
    {
        QCPItemTracer *itemTracer = new QCPItemTracer(ui->widget_plot);
        itemTracer->setGraph(stressLine);
        itemTracer->setGraphKey(xStress[i]);
        itemTracer->setStyle(QCPItemTracer::tsCircle);
        itemTracer->setBrush(Qt::red);


            QCPItemText *lineText = new QCPItemText(ui->widget_plot);
            lineText->position->setType(QCPItemPosition::ptPlotCoords);
            lineText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
            lineText->position->setCoords(xStress[i],yStress[i]+1);
            lineText->setText(QString::number(yStress[i]));
            lineText->setTextAlignment(Qt::AlignCenter);
            lineText->setFont(lineFont);
            lineText->setPadding(QMargins(1, 1, 1, 1));

            QCPItemText *barText = new QCPItemText(ui->widget_plot);
            barText->position->setType(QCPItemPosition::ptPlotCoords);
            barText->position->setAxes(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
            barText->position->setCoords(xBar[i],yValues[i]/2);
            barText->setText(QString::number(yValues[i]));
            barText->setFont(barFont);
            barText->setColor(Qt::white);
    }


    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeSpec(Qt::UTC);
    dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dateTimeTicker->setDateTimeFormat("dd.MM");
    dateTimeTicker->setTickCount(dayCount);

    ui->widget_plot->xAxis->setRange(xRange);
    ui->widget_plot->xAxis->setTicker(dateTimeTicker);

    ui->widget_plot->replot();
    isLoad = true;
}

void week_popup::on_pushButton_close_clicked()
{

    this->freeMem();
    reject();
}

void week_popup::on_pushButton_edit_clicked()
{
    this->freeMem();
    accept();
}

void week_popup::on_comboBox_yValue_currentIndexChanged(int index)
{
    if(isLoad) this->set_weekPlot(index);
}
