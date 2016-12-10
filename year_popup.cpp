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
       partInfo = pInfo.split("-");
       workSched = p_sched;
       phase = pPhase;
       phaseindex = pIndex;
       phaseList << "Year";

       phasechart = new QChart();
       chartview = new QChartView(phasechart);
       chartview->setRenderHint(QPainter::Antialiasing);

       stressLine = workSched->get_qLineSeries(true);
       selectBar = new QBarSet("Duration");
       selectBars = new QBarSeries();      
       selectBars->append(selectBar);

       axisX = new QBarCategoryAxis();
       axisX->setTitleText("Week");
       axisX->setTitleVisible(true);

       phasechart->addSeries(selectBars);
       phasechart->addSeries(stressLine);

       yStress = workSched->get_qValueAxis("Stress",true,10,8);
       phasechart->addAxis(yStress,Qt::AlignLeft);
       stressLine->attachAxis(yStress);
       phasechart->setAxisX(axisX,stressLine);

       yBars = workSched->get_qValueAxis("Duration",true,10,8);
       phasechart->addAxis(yBars,Qt::AlignRight);
       selectBars->attachAxis(yBars);
       phasechart->setAxisX(axisX,selectBars);

       for(int i = 0; i < settings::get_phaseList().count(); ++i)
       {
           phaseList << settings::get_phaseList().at(i);
       }

       if(position == 0) col = 7;
       if(position == 1) col = 2;
       if(position == 2) col = 3;
       if(position == 3) col = 4;
       if(position == 4) col = 5;
       if(position == 5) col = 6;

       if(phaseindex == 0) widthFactor = 25;   //All
       if(phaseindex == 1) widthFactor = 100;  //OFF
       if(phaseindex == 2) widthFactor = 80;   //PREP
       if(phaseindex == 3) widthFactor = 70;   //BASE
       if(phaseindex == 4) widthFactor = 90;   //BUILD
       if(phaseindex == 5) widthFactor = 200;  //PEAK
       if(phaseindex == 6) widthFactor = 80;   //WK

       ui->verticalLayout_plot->addWidget(chartview);
       ui->label_info->setText(phaseList.at(phaseindex) +": " + partInfo.at(0) + " Workouts: " + partInfo.at(1) + " - Hours: " + partInfo.at(2) + " - Distance: " + partInfo.at(4));

       this->fill_values();

       selectList << "Duration" << "Distance" << "Workouts";
       ui->comboBox_select->addItems(selectList);
}

year_popup::~year_popup()
{
    delete ui;
}

void year_popup::on_pushButton_close_clicked()
{
    reject();
}

void year_popup::fill_values()
{
    QList<QStandardItem*> list;
    QModelIndex index;
    QString weekID;

    if(phaseindex == 0)
    {
        weekcount = settings::get_saisonInfo("weeks").toInt();
    }
    else
    {
        weekcount = workSched->week_meta->findItems(phase,Qt::MatchContains,2).count();
        list = workSched->week_meta->findItems(phase,Qt::MatchContains,2);
    }

    this->setFixedWidth(widthFactor*weekcount);
    maxValues.resize(3);
    maxValues.fill(0.0);
    y_stress.resize(weekcount);
    y_dura.resize(weekcount);
    y_dist.resize(weekcount);
    y_work.resize(weekcount);

    QString sumValue,stress,duration,distance,workouts;
    QStringList sumValues;
    workSched->week_content->sort(0);
    max_stress = 0.0;

    if(phaseindex == 0)
    {
        for(int week = 0; week < workSched->week_content->rowCount(); ++week)
        {
            sumValue = workSched->week_content->data(workSched->week_content->index(week,col,QModelIndex())).toString();
            sumValues = sumValue.split("-");
            workouts = sumValues.at(0);
            distance = sumValues.at(1);
            duration = sumValues.at(2);
            stress = sumValues.at(3);

            y_stress[week] = stress.toDouble();
            if(max_stress < y_stress[week]) max_stress = y_stress[week];

            y_dura[week] = settings::set_doubleValue(static_cast<double>(settings::get_timesec(duration) / 60.0),false);
            if(maxValues[0] < y_dura[week]) maxValues[0] = y_dura[week];
            y_dist[week] = settings::set_doubleValue(distance.toDouble(),false);
            if(maxValues[1] < y_dist[week]) maxValues[1] = y_dist[week];
            y_work[week] = workouts.toDouble();
            if(maxValues[2] < y_work[week]) maxValues[2] = y_work[week];
        }
    }
    else
    {
        for(int week = 0; week < list.count(); ++week)
        {
            index = workSched->week_meta->indexFromItem(list.at(week));
            weekID = workSched->week_meta->item(index.row(),1)->text();

            for(int i = 0; i < workSched->week_content->rowCount(); ++i)
            {
                if(weekID == workSched->week_content->data(workSched->week_content->index(i,1,QModelIndex())).toString())
                {
                    sumValue = workSched->week_content->data(workSched->week_content->index(i,col,QModelIndex())).toString();
                    sumValues = sumValue.split("-");
                    workouts = sumValues.at(0);
                    distance = sumValues.at(1);
                    duration = sumValues.at(2);
                    stress = sumValues.at(3);

                    y_stress[week] = stress.toDouble();
                    if(max_stress < y_stress[week]) max_stress = y_stress[week];

                    y_dura[week] = settings::set_doubleValue(static_cast<double>(settings::get_timesec(duration) / 60.0),false);
                    if(maxValues[0] < y_dura[week]) maxValues[0] = y_dura[week];
                    y_dist[week] = settings::set_doubleValue(distance.toDouble(),false);
                    if(maxValues[1] < y_dist[week]) maxValues[1] = y_dist[week];
                    y_work[week] = workouts.toDouble();
                    if(maxValues[2] < y_work[week]) maxValues[2] = y_work[week];
                }
            }
        }
    }

    selectBar->setPen(QPen(Qt::blue));
    selectBar->setBrush(QColor(0, 0, 255, 120));

    selectBars->setLabelsFormat("@value");
    selectBars->setLabelsVisible(true);

    phasechart->setMargins(QMargins(QMargins(5,5,5,5)));
    phasechart->setBackgroundRoundness(5);
    phasechart->setDropShadowEnabled(true);
    phasechart->legend()->hide();
}

void year_popup::set_plot(int index)
{
    QStringList axisValues;

    for(int i = 1; i <= weekcount; ++i)
    {
        axisValues << QString::number(i);
    }

    if(stressLine->count() > 0)
    {
        stressLine->clear();
        selectBar->remove(0,weekcount);
    }

    for(int i = 0; i < weekcount; ++i)
    {
        stressLine->append(i,y_stress[i]);
        if(index == 0)selectBar->append(y_dura[i]);
        if(index == 1)selectBar->append(y_dist[i]);
        if(index == 2)selectBar->append(y_work[i]);
    }

    axisX->append(axisValues);

    if(phaseindex == 0)
    {
        stressLine->setPointLabelsVisible(false);
        stressLine->setPointsVisible(false);
        selectBars->setLabelsVisible(false);
        selectBars->setBarWidth((25/static_cast<double>(weekcount)));
    }
    else
    {
        selectBars->setBarWidth((weekcount/2)/static_cast<double>(weekcount));
    }

    yStress->setMax(max_stress+20);
    yStress->setTickCount(8);
    yStress->applyNiceNumbers();
    yBars->setMax(maxValues[index]+(maxValues[index]*0.05));
    yBars->setTickCount(8);
    yBars->applyNiceNumbers();
    yBars->setTitleText(selectList.at(index));

}

void year_popup::on_comboBox_select_currentIndexChanged(int index)
{
    this->set_plot(index);
}
