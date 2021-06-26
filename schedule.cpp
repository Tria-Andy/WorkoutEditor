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

#include "schedule.h"

schedule::schedule(standardWorkouts *pworkouts)
{
    stdWorkouts = pworkouts;
    macroTags = settings::get_xmlMapping("macro");
    scheduleTags = settings::get_xmlMapping("schedule");
    saisonTags = settings::get_xmlMapping("saisons");
    sportTags = settings::get_listValues("Sportuse");
    firstdayofweek = settings::firstDayofWeek;
    dateFormat = settings::get_format("dateformat");
    longTime = settings::get_format("longtime");
    shortTime = settings::get_format("shorttime");
    doubleValues = settings::getdoubleMapPointer(settings::dMap::Double);
    gcValues = settings::getStringMapPointer(settings::stingMap::GC);
    fileMap = settings::getStringMapPointer(settings::stingMap::File);
    weekDays = settings::get_intValue("weekdays");
    levelList = settings::get_listValues("Level");
    compChanged = new QAction();

    scheduleModel = new QStandardItemModel();
    phaseModel = new QStandardItemModel();

    if(!gcValues->value("schedule").isEmpty())
    {
       //Schedule
       this->xml_toTreeModel(fileMap->value("schedulefile"),scheduleModel);
       //Saison - Phase
       this->xml_toTreeModel(fileMap->value("saisonfile"),phaseModel);
       //StressMap
       this->xml_toListMap(fileMap->value("stressfile"));
       this->set_stressMap();

       this->remove_WeekofPast(firstdayofweek.addDays(-14));
       this->set_saisonValues();
    }
    isUpdated = false;

    stressPlot = new QCustomPlot();
    levelPlot = new QCustomPlot();
    compPlot = new QCustomPlot();

    this->set_plotObj();
}

enum {ADD,EDIT,COPY,DEL};
enum {SAISON,SCHEDULE};

void schedule::save_workouts(bool saveModel)
{
    if(saveModel == SAISON)
    {
        this->treeModel_toXml(phaseModel,fileMap->value("saisonfile"));
    }

    if(saveModel == SCHEDULE)
    {
        this->treeModel_toXml(scheduleModel,fileMap->value("schedulefile"));
        this->save_ltsFile();
    }
    isUpdated = false;
}

void schedule::add_newSaison(QStringList saisonInfo)
{
    phaseModel->blockSignals(true);
    scheduleModel->blockSignals(true);
    QDate saisonStart = QDate::fromString(saisonInfo.at(1),dateFormat);
    QDate firstday;
    int weekCount = 0;
    QVector<double> phaseCount = settings::doubleVector.value("Phaseweeks");
    QStringList phaseList = settings::get_listValues("Phase");
    QStringList sportUseList = settings::get_listValues("Sportuse");
    sportUseList.removeLast();

    QList<QStandardItem*> itemList;
    for(int info = 0; info < saisonInfo.count(); ++info)
    {
        itemList << new QStandardItem(saisonInfo.at(info));
    }
    itemList.at(0)->setData(saisonTags->at(0),Qt::AccessibleTextRole);
    phaseModel->invisibleRootItem()->appendRow(itemList);

    QStandardItem *macroItem = itemList.at(0);
    itemList.clear();

    QStandardItem *weekItem,*dayItem;
    QStandardItem *scheduleRoot = scheduleModel->invisibleRootItem();

    for(int meso = 0; meso < phaseList.count(); ++meso)
    {
        itemList << new QStandardItem(phaseList.at(meso));
        itemList << new QStandardItem(QString::number(meso));

        itemList.at(0)->setData(saisonTags->at(1),Qt::AccessibleTextRole);
        macroItem->appendRow(itemList);
        QStandardItem *mesoItem = itemList.at(0);
        itemList.clear();

        for(int micro = 0; micro < static_cast<int>(phaseCount.at(meso)); ++micro)
        {
            firstday = saisonStart.addDays(7*weekCount++);
            itemList << new QStandardItem(this->calc_weekID(firstday));
            itemList << new QStandardItem(QString::number(weekCount));
            itemList << new QStandardItem(phaseList.at(meso)+"_X");
            itemList << new QStandardItem(firstday.toString(dateFormat));
            itemList << new QStandardItem("-");
            itemList << new QStandardItem("-");

            itemList.at(0)->setData(saisonTags->at(2),Qt::AccessibleTextRole);
            mesoItem->appendRow(itemList);
            QStandardItem *microItem = itemList.at(0);
            itemList.clear();
            weekItem = new QStandardItem(this->calc_weekID(firstday));
            weekItem->setData(scheduleTags->at(0),Qt::AccessibleTextRole);

            for(int day = 0; day < weekDays; ++day)
            {
                dayItem = new QStandardItem(firstday.addDays(day).toString(dateFormat));
                dayItem->setData(scheduleTags->at(1),Qt::AccessibleTextRole);
                update_stressMap(firstday.addDays(day),QVector<double>(5,0));
                weekItem->appendRow(dayItem);
            }
            scheduleRoot->appendRow(weekItem);

            for(int comp = 0; comp < sportUseList.count(); ++comp)
            {
                itemList << new QStandardItem(QString::number(comp));
                itemList << new QStandardItem(sportUseList.at(comp));
                itemList << new QStandardItem("0");
                itemList << new QStandardItem("0");
                itemList << new QStandardItem("0.0");
                itemList << new QStandardItem("0.0");
                itemList << new QStandardItem("0");

                itemList.at(0)->setData(saisonTags->at(3),Qt::AccessibleTextRole);
                microItem->appendRow(itemList);
                itemList.clear();
            }
        }
    }
    this->set_saisonValues();
    this->save_workouts(SAISON);
    this->save_workouts(SCHEDULE);
    this->newSaison = false;
    scheduleModel->blockSignals(false);
    phaseModel->blockSignals(false);    
}

bool schedule::delete_Saison(QString saisonName)
{
   QModelIndex saisonIndex =  get_modelIndex(phaseModel,saisonName,0);
   QStandardItem *saisonItem = phaseModel->itemFromIndex(saisonIndex);
   phaseModel->removeRows(0,saisonItem->rowCount(),saisonIndex);
   phaseModel->removeRow(saisonIndex.row(),phaseModel->invisibleRootItem()->index());
   saisonValues.remove(saisonName);
   this->save_workouts(SAISON);

   return true;
}

void schedule::update_stressMap(QDate stressDate, QVector<double> stressValues)
{
    stressMap.insert(stressDate,stressValues);
    this->recalc_stressValues();
}

void schedule::calc_levelPlot(QDate startDate)
{
    levelPlot->clearItems();

    QMap<int, QStringList> dayWorkouts;
    QMap<int,QString> weekWorkouts;
    QMap<int,double> zoneTime;
    QStringList levelList = settings::get_listValues("Level");
    int workoutCount = 0;
    QFont barFont;
    barFont.setPointSize(10);

    for(int day = 0; day < weekDays; ++day)
    {
        dayWorkouts = get_workouts(true,startDate.addDays(day).toString(dateFormat));
        for(QMap<int, QStringList>::const_iterator it = dayWorkouts.cbegin(), end = dayWorkouts.cend(); it != end; ++it)
        {
            weekWorkouts.insert(workoutCount++,it.value().last());
        }

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
    }

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

    levelPlot->xAxis->setTicker(zoneTicker);
    levelPlot->xAxis->setRange(-1,7);
    levelPlot->yAxis->setRange(0,100);
    levelPlot->xAxis->setLabel("Level");
    levelPlot->yAxis->setLabel("Percent");

    levelBar->setWidthType(QCPBars::wtPlotCoords);
    levelBar->setData(tickCount,zoneValues);

    double yoffSet = levelPlot->yAxis->range().size()/10.0;

    for(int i = 0; i < zoneValues.count(); ++i)
    {
        this->create_itemBarText(levelPlot,"main",barFont,Qt::black,tickCount.at(i),zoneValues.at(i),0,yoffSet,levelPlot->yAxis);
    }

    levelPlot->replot();
}

void schedule::calc_pmcPlot(QDate rangeStart,bool currWeek, int extWeek)
{
    stressPlot->clearItems();

    int plotRange = currWeek ? weekDays : weekDays*extWeek;
    int dayCount = rangeStart.addDays(-1).daysTo(rangeStart.addDays(plotRange));

    QVector<double> xDate(dayCount,0),yLTS(dayCount,0),ySTS(dayCount,0),yTSB(dayCount,0),yStress(dayCount,0),yDura(dayCount,0),yDist(dayCount,0);
    QPair<double,double> tsbMinMax;
    double stressMax = 0;

    QDateTime startDate;
    QTime wTime;
    wTime.fromString("00:00:00","hh:mm:ss");
    startDate.setDate(rangeStart);
    startDate.setTime(wTime);
    startDate.setTimeSpec(Qt::LocalTime);

    int day = 0;

    for(QMap<QDate,QVector<double>>::iterator stressStart = stressMap.find(rangeStart.addDays(-1)), end = stressMap.find(rangeStart.addDays(plotRange)); stressStart != end; ++stressStart)
    {
        startDate.setDate(stressStart.key());
        xDate[day] = startDate.toTime_t() + 3600;
        yStress[day] = stressStart.value().at(0);
        ySTS[day] = round(stressStart.value().at(1));
        yLTS[day] = round(stressStart.value().at(2));
        yDura[day] = round(stressStart.value().at(3)/60.0);
        yDist[day] = round(stressStart.value().at(4));

        if(stressMax < yStress[day]) stressMax = yStress[day];

        if(day > 0)
        {
            yTSB[day] = yLTS[day-1] - ySTS[day-1];
        }
        else
        {
            yTSB[day] = round(stressMap.value(rangeStart.addDays(-2)).at(2) - stressMap.value(rangeStart.addDays(-2)).at(1));
        }

        if(tsbMinMax.first > yTSB.at(day)) tsbMinMax.first = yTSB.at(day);
        if(tsbMinMax.second < yTSB.at(day)) tsbMinMax.second = yTSB.at(day);

        ++day;
    }

    QCPRange xRange(QCPAxisTickerDateTime::dateTimeToKey(rangeStart.addDays(-1)),QCPAxisTickerDateTime::dateTimeToKey(rangeStart.addDays(plotRange)));

    QSharedPointer<QCPAxisTickerDateTime> dateTicker(new QCPAxisTickerDateTime);
    dateTicker->setDateTimeSpec(Qt::LocalTime);
    dateTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dateTicker->setDateTimeFormat("dd.MM");

    QSharedPointer<QCPAxisTickerFixed> dayTicker(new QCPAxisTickerFixed);
    dayTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dayTicker->setTickStep(1.0);

    int xTickCount = dayCount;

    if(plotRange > 21 && plotRange < 42)
    {
        xTickCount = (plotRange+1)/3;
    }
    else if(plotRange >= 42)
    {
        xTickCount = (plotRange+1)/5;
    }
    if(dayCount >= 22)
    {
        dayCount = dayCount / 7;
    }

    dateTicker->setTickCount(xTickCount);
    dayTicker->setTickCount(dayCount);

    stressPlot->xAxis->setRange(xRange);
    stressPlot->xAxis->setTicker(dateTicker);

    double barWidth = (stressPlot->xAxis->range().size()/plotRange)/extWeek;

    ltsLine->setData(xDate,yLTS);
    stsLine->setData(xDate,ySTS);
    stressLine->setData(xDate,yStress);
    tsbLine->setData(xDate,yTSB);
    duraBars->setData(xDate,yDura);
    duraBars->setWidthType(QCPBars::wtPlotCoords);
    duraBars->setWidth(barWidth);
    distBars->setData(xDate,yDist);
    distBars->setWidthType(QCPBars::wtPlotCoords);
    distBars->setWidth(barWidth);

    stressPlot->yAxis->setRange(0,stressMax+stressMax*0.15);
    stressPlot->yAxis2->setRange(tsbMinMax.first-5,tsbMinMax.second+5);
    stressPlot->xAxis2->setRange(0,dayCount);
    stressPlot->xAxis2->setTicker(dayTicker);

    double xoffSet = barWidth/2;
    double yoffSet = stressPlot->yAxis->range().size()/20.0;

    QFont lineFont;
    lineFont.setPointSize(8);

    for(int i = 0; i < xDate.count(); ++i)
    {
        this->create_itemTracer(stressPlot,"LTS",ltsLine,xDate.at(i),yLTS.at(i),Qt::darkGreen,true);
        this->create_itemTracer(stressPlot,"STS",stsLine,xDate.at(i),ySTS.at(i),Qt::darkRed,true);
        this->create_itemTracer(stressPlot,"StressScore",stressLine,xDate.at(i),yStress.at(i),QColor(225,150,0),true);
        this->create_itemTracer(stressPlot,"TSB",tsbLine,xDate.at(i),yTSB.at(i),QColor(255,170,0),true);
        this->create_itemBarText(stressPlot,"CompBars",lineFont,Qt::black,xDate.at(i),yDura.at(i),-xoffSet,yoffSet,stressPlot->yAxis);
        this->create_itemBarText(stressPlot,"CompBars",lineFont,Qt::black,xDate.at(i),yDist.at(i),xoffSet,yoffSet,stressPlot->yAxis);
    }

    stressPlot->plotLayout()->elementAt(1)->setMargins(QMargins(50,1,50,5));
    stressPlot->plotLayout()->setRowStretchFactor(1,0.0001);
    stressPlot->replot();
}

void schedule::calc_compPlot(int plotCount, QDate startDate, QString sport)
{
    QVector<double> maxValues(2,0);
    QVector<double> xWeeks(plotCount,0);
    QVector<double> yStress(plotCount,0);
    QVector<double> yDura(plotCount,0);
    QVector<double> yDist(plotCount,0);
    QVector<double> yWorks(plotCount,0);

    QString weekID;
    QMap<QString,QVector<double>> compValues;
    QMap<double,QString> tickerList;
    tickerList.insert(0,"Start");

    QSharedPointer<QCPAxisTickerText> weekTicker(new QCPAxisTickerText);
    int vectorPos = 0;
    for(int i = 1; i <= plotCount; ++i,++vectorPos)
    {
        weekID = weekPhaseMap.value(startDate.addDays(weekDays*(i-1))).first;
        xWeeks[vectorPos] = i;
        tickerList.insert(i,weekID);

        if(sport == settings::getStringMapPointer(settings::stingMap::General)->value("sum"))
        {
            compValues = compWeekMap.value(weekID);
            for(QMap<QString,QVector<double>>::const_iterator week = compValues.cbegin(); week != compValues.cend(); ++week)
            {
                yWorks[vectorPos] += week.value().at(0);
                yDura[vectorPos] += week.value().at(1)/60.0;
                yDist[vectorPos] += week.value().at(3);
                yStress[vectorPos] += week.value().at(4);
            }
        }
        else
        {
            yWorks[vectorPos] = compWeekMap.value(weekID).value(sport).at(0);
            yDura[vectorPos] = compWeekMap.value(weekID).value(sport).at(1)/60.0;
            yDist[vectorPos] = compWeekMap.value(weekID).value(sport).at(3);
            yStress[vectorPos] = compWeekMap.value(weekID).value(sport).at(4);
        }
        if(maxValues.at(0) < yDist.at(vectorPos)) maxValues[0] = yDist.at(vectorPos);
        if(maxValues.at(1) < yDura.at(vectorPos)) maxValues[1] = yDura.at(vectorPos);
    }

    tickerList.insert(tickerList.count(),"End");
    weekTicker->addTicks(tickerList);
    compPlot->xAxis->setRange(0,plotCount+1);
    compPlot->xAxis->setTicker(weekTicker);

    double barWidth = compPlot->xAxis->range().size()/ pow(tickerList.count(),2) > 50 ? 50 : (compPlot->xAxis->range().size()/tickerList.count())/compBars->size();

    workCompBar->setData(xWeeks,yWorks);
    workCompBar->setWidthType(QCPBars::wtPlotCoords);
    workCompBar->setWidth(barWidth);

    distCompBar->setData(xWeeks,yDist);
    distCompBar->setWidthType(QCPBars::wtPlotCoords);
    distCompBar->setWidth(barWidth);

    duraCompBar->setData(xWeeks,yDura);
    duraCompBar->setWidthType(QCPBars::wtPlotCoords);
    duraCompBar->setWidth(barWidth);

    stressCompBar->setData(xWeeks,yStress);
    stressCompBar->setWidthType(QCPBars::wtPlotCoords);
    stressCompBar->setWidth(barWidth);

    compPlot->yAxis->setRange(0,maxValues.at(0)+(maxValues.at(0)*0.1));
    compPlot->yAxis2->setRange(0,maxValues.at(1)+(maxValues.at(1)*0.1));

    compPlot->replot();
}

void schedule::set_weekMeta(QStringList metaData)
{
    QModelIndex weekIndex = this->get_modelIndex(phaseModel,metaData.at(0),0);
    QStandardItem *weekItem = phaseModel->itemFromIndex(weekIndex);

    phaseModel->blockSignals(true);

    phaseModel->removeRows(0,weekItem->rowCount(),weekIndex);

    for(int meta = 0; meta < metaData.count(); ++meta)
    {
       phaseModel->setData(weekIndex.siblingAtColumn(meta),metaData.at(meta));
    }

    this->save_workouts(SAISON);

    phaseModel->blockSignals(false);
}

void schedule::set_weekScheduleMeta(QStringList metaData)
{
    QModelIndex weekIndex = this->get_modelIndex(scheduleModel,metaData.at(0),0);

    scheduleModel->blockSignals(true);

    for(int meta = 0; meta < metaData.count(); ++meta)
    {
       scheduleModel->setData(weekIndex.siblingAtColumn(meta),metaData.at(meta));
    }

    this->save_workouts(SCHEDULE);

    scheduleModel->blockSignals(false);
}

QModelIndex schedule::get_modelIndex(QStandardItemModel *model,QString searchString, int col)
{
    QList<QStandardItem*> list;

    list = model->findItems(searchString,Qt::MatchExactly | Qt::MatchRecursive,col);

    if(list.count() > 0)
    {
        return model->indexFromItem(list.at(0));
    }
    else
    {
        return QModelIndex();
    }
}

void schedule::set_plotObj()
{
    QFont plotFont,selectFont;
    plotFont.setBold(true);
    plotFont.setPointSize(8);
    selectFont.setPointSize(8);
    selectFont.setItalic(true);

    //StressPlot
    QList<QCPAxis*> xaxisList;
    xaxisList.append(stressPlot->xAxis);

    QList<QCPAxis*> yaxisList;
    yaxisList.append(stressPlot->yAxis);

    stressPlot->legend->setFillOrder(QCPLegend::foColumnsFirst);

    stressPlot->setInteractions(QCP::iSelectLegend | QCP::iMultiSelect | QCP::iRangeDrag | QCP::iRangeZoom);

    stressPlot->yAxis->setLabel("Stress");
    stressPlot->yAxis->setLabelFont(plotFont);

    stressPlot->yAxis2->setVisible(true);
    stressPlot->yAxis2->setLabel("TSB");
    stressPlot->yAxis2->setLabelFont(plotFont);

    stressPlot->legend->setVisible(true);
    stressPlot->legend->setFont(plotFont);
    stressPlot->legend->setSelectedFont(selectFont);
    stressPlot->legend->setSelectableParts(QCPLegend::spItems);

    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    stressPlot->plotLayout()->addElement(1,0,subLayout);
    subLayout->addElement(0,0,stressPlot->legend);

    stressPlot->axisRect()->setRangeDragAxes(xaxisList,yaxisList);
    stressPlot->axisRect()->setRangeZoomAxes(xaxisList,yaxisList);
    stressPlot->addLayer("GRID",stressPlot->layer(0),QCustomPlot::limAbove);
    stressPlot->addLayer("TSB",stressPlot->layer(1),QCustomPlot::limAbove);
    stressPlot->addLayer("STS",stressPlot->layer(2),QCustomPlot::limAbove);
    stressPlot->addLayer("LTS",stressPlot->layer(3),QCustomPlot::limAbove);
    stressPlot->addLayer("StressScore",stressPlot->layer(4),QCustomPlot::limAbove);
    stressPlot->addLayer("CompBars",stressPlot->layer(5),QCustomPlot::limAbove);
    stressPlot->xAxis->grid()->setLayer("GRID");
    stressPlot->yAxis->grid()->setLayer("GRID");

    ltsLine = this->create_QCPLine(stressPlot,"LTS",QColor(0,255,0),false);
    stsLine = this->create_QCPLine(stressPlot,"STS",QColor(255,0,0),false);
    stressLine = this->create_QCPLine(stressPlot,"StressScore",QColor(225,150,0),false);
    tsbLine = this->create_QCPLine(stressPlot,"TSB",QColor(255,170,0),true);
    tsbLine->setBrush(QBrush(QColor(255,170,0,50)));
    stressBars = new QCPBarsGroup(stressPlot);
    duraBars = this->create_QCPBar(stressPlot,"Duration",QColor(0,85,255),false);
    duraBars->setBarsGroup(stressBars);
    duraBars->setLayer("CompBars");
    distBars = this->create_QCPBar(stressPlot,"Distance",QColor(0,200,255),false);
    distBars->setBarsGroup(stressBars);
    distBars->setLayer("CompBars");

    //Level Plot
    levelPlot->xAxis->setLabel("Level");
    levelPlot->xAxis->setLabelFont(plotFont);
    levelPlot->xAxis2->setVisible(true);
    levelPlot->xAxis2->setLabelFont(plotFont);
    levelPlot->xAxis2->setTickLabels(false);
    levelPlot->yAxis->setLabel("Percent");
    levelPlot->yAxis->setLabelFont(plotFont);
    levelPlot->yAxis2->setVisible(true);
    levelPlot->yAxis2->setLabelFont(plotFont);

    levelBar = this->create_QCPBar(levelPlot,"Level Distribution",QColor(0,85,255),false);

    //Comp Stress Plot
    workCompBar = this->create_QCPBar(compPlot,"Workouts",Qt::gray,false);
    distCompBar = this->create_QCPBar(compPlot,"Distance",Qt::green,false);
    duraCompBar = this->create_QCPBar(compPlot,"Duration",Qt::blue,true);
    stressCompBar = this->create_QCPBar(compPlot,"StressScore",Qt::red,true);

    compBars = new QCPBarsGroup(compPlot);
    workCompBar->setBarsGroup(compBars);
    distCompBar->setBarsGroup(compBars);
    duraCompBar->setBarsGroup(compBars);
    stressCompBar->setBarsGroup(compBars);

    compPlot->legend->setVisible(true);
    compPlot->xAxis->setLabelFont(plotFont);
    compPlot->yAxis->setLabel("Workouts / Distance");
    compPlot->yAxis->setLabelFont(plotFont);
    compPlot->yAxis2->setVisible(true);
    compPlot->yAxis2->setLabel("Duration / StressScore");
    compPlot->yAxis2->setLabelFont(plotFont);
}

void schedule::update_linkedWorkouts(QDate workDate, QString workID,int pos,bool add)
{
    QMap<QDate,int> linkMap = linkedWorkouts.value(workID);

    if(add)
    {
        linkMap.insert(workDate,pos);
    }
    else
    {
        linkMap.remove(workDate);
    }

    linkedWorkouts.insert(workID,linkMap);
}

QStandardItem* schedule::get_phaseItem(QString phase)
{
    return phaseModel->itemFromIndex(get_modelIndex(phaseModel,phase,0));
}

QMap<int, QStringList> schedule::get_workouts(int dataSource,QString indexString)
{
    QMap<int,QStringList> workouts;
    QStringList workItems;

    QStandardItemModel *model = nullptr;
    if(dataSource == SAISON) model = phaseModel;
    else if (dataSource == SCHEDULE) model = scheduleModel;

    QModelIndex modelIndex = this->get_modelIndex(model,indexString,0);

    if(modelIndex.isValid())
    {
        QStandardItem *parent = model->itemFromIndex(modelIndex);

        if(parent->hasChildren())
        {
            for(int work = 0; work < parent->rowCount(); ++work)
            {
                for(int x = 1; x < model->columnCount(); ++x)
                {
                   workItems << model->data(model->index(work,x,modelIndex)).toString();
                }
                workouts.insert(work,workItems);
                workItems.clear();
            }
        }
    }
    return workouts;
}

QStringList schedule::get_weekMeta(QString weekID)
{
    QStringList weekMeta;
    QModelIndex weekIndex = this->get_modelIndex(phaseModel,weekID,0);

    if(weekIndex.isValid())
    {
        for(int meta = 0; meta < phaseModel->columnCount(); ++meta)
        {
            weekMeta << phaseModel->data(weekIndex.siblingAtColumn(meta)).toString();
        }
    }
    return weekMeta;
}

QStringList schedule::get_weekScheduleMeta(QString weekID)
{
    QStringList weekMeta;
    QModelIndex weekIndex = this->get_modelIndex(scheduleModel,weekID,0);
    QString value;

    if(weekIndex.isValid())
    {
        for(int meta = 0; meta < scheduleModel->columnCount(); ++meta)
        {
            value = scheduleModel->data(weekIndex.siblingAtColumn(meta)).toString();
            if(value.isEmpty())
            {
                if(meta == 1) weekMeta << settings::get_listValues("Phase").at(0);
                else if(meta == 2) weekMeta << "Strength - Cardio";
                else if(meta == 3) weekMeta << "Goal Weight";
                else
                {
                    weekMeta << "-";
                }
            }
            else
            {
                weekMeta << value;
            }
        }
    }
    return weekMeta;
}

QStringList schedule::get_remainWeeks()
{
    QStringList weekList;

    for(int row = 0; row < scheduleModel->rowCount(); ++row)
    {
        weekList.append(scheduleModel->data(scheduleModel->index(row,0)).toString());
    }

    return weekList;
}

void schedule::save_ltsFile()
{
    QStringList listValues;
    int counter = 0;
    int ltsDays = settings::get_intValue("ltsdays")+1;
    mapList.clear();

    for(QMap<QDate,QVector<double>>::iterator it = stressMap.find(firstdayofweek.addDays(-ltsDays)), end = stressMap.find(firstdayofweek.addDays(7)); it != end; ++it)
    {
        listValues << it.key().toString(dateFormat);
        for(int value = 0; value < it.value().count(); ++value)
        {
            listValues << QString::number(set_doubleValue(it.value().at(value),false));
        }
        mapList.insert(counter++,listValues);
        listValues.clear();
    }
    this->listMap_toXml(fileMap->value("stressfile"));
}

void schedule::add_contest(QString saison,QDate contestDate, QStringList contestValues)
{
    QMap<QDate,QStringList> contest = contestMap.value(saison);
    contest.insert(contestDate,contestValues);
    contestMap.insert(saison,contest);
}

void schedule::remove_contest(QString saison, QDate contestDate)
{
    QMap<QDate,QStringList> contest = contestMap.value(saison);
    contest.remove(contestDate);
    contestMap.insert(saison,contest);

    QStandardItem *dayItem = scheduleModel->itemFromIndex(get_modelIndex(scheduleModel,contestDate.toString(dateFormat),0));
    scheduleModel->removeRows(0,dayItem->rowCount(),dayItem->index());
}

void schedule::compValuesUpdate()
{
    qDebug() << "Refresh CompValues";
}


void schedule::check_workouts(QDate date)
{
    QList<QStandardItem*> itemList;
    QString dateString = date.toString(dateFormat);
    QModelIndex index = this->get_modelIndex(scheduleModel,calc_weekID(date),0);

    //Check if Week and Date available
    if(!index.isValid())
    {
        itemList << new QStandardItem(calc_weekID(date));
        scheduleModel->invisibleRootItem()->appendRow(itemList);
        index = this->get_modelIndex(scheduleModel,calc_weekID(date),0);
    }
    itemList.clear();
    QStandardItem *item = scheduleModel->itemFromIndex(index);

    index = this->get_modelIndex(scheduleModel,dateString,0);

    if(!index.isValid())
    {
        itemList << new QStandardItem(dateString);
        item->appendRow(itemList);
    }

    //Read Contest/Races
    index = this->get_modelIndex(scheduleModel,dateString,0);
    QStandardItem *dayItem = scheduleModel->itemFromIndex(index);

    if(dayItem->hasChildren())
    {
        for(int work = 0; work < dayItem->rowCount(); ++work)
        {
            if(dayItem->child(work,3)->data(Qt::DisplayRole).toString() == "Race")
            {
                QStringList contestValues;
                index = get_modelIndex(phaseModel,calc_weekID(date),0);
                item = get_phaseItem(index.parent().parent().data(Qt::DisplayRole).toString());

                for(int value = 0; value < scheduleModel->columnCount(); ++value)
                {
                    contestValues << dayItem->child(work,value)->data(Qt::DisplayRole).toString();
                }
                add_contest(item->data(Qt::DisplayRole).toString(),date,contestValues);
            }

            this->update_linkedWorkouts(date,dayItem->child(work,11)->data(Qt::DisplayRole).toString(),dayItem->child(work,0)->data(Qt::DisplayRole).toInt(),true);
        }
    }
}

QString schedule::get_weekPhase(QDate currDate,bool selFlag)
{
    QStringList weekMeta;

    if(selFlag)
    {
        weekMeta = this->get_weekScheduleMeta(this->calc_weekID(currDate));
        return weekMeta.at(1);
    }
    else
    {
        weekMeta = this->get_weekMeta(this->calc_weekID(currDate));
        return weekMeta.at(2);
    }

    return nullptr;
}

QString schedule::get_saisonDate(QDate currDate)
{
    for(QMap<QString,QStringList>::const_iterator it = saisonValues.cbegin(); it != saisonValues.cend(); ++it)
    {
        if(currDate >= QDate::fromString(it.value().at(0),dateFormat) && currDate <= QDate::fromString(it.value().at(1),dateFormat) )
        {
            return it.key();
        }
    }

    return nullptr;
}

QStringList schedule::get_weekList()
{    
    QStringList weekList = compWeekMap.keys();
    std::sort(weekList.begin(),weekList.end());
    return weekList;
}

void schedule::remove_WeekofPast(QDate dayOfWeek)
{
    QModelIndex weekIndex = get_modelIndex(scheduleModel,calc_weekID(dayOfWeek),0);

    if(weekIndex.isValid())
    {
        QStandardItem *weekItem = scheduleModel->itemFromIndex(weekIndex);
        scheduleModel->removeRows(0,weekItem->rowCount(),weekIndex);
        scheduleModel->removeRow(weekIndex.row(),weekIndex.parent());
    }
}

void schedule::set_workoutData()
{
    QModelIndex dayIndex;
    QStandardItem *dayItem = nullptr;

    for(QHash<QDate,QMap<int,QStringList>>::const_iterator it =  workoutUpdates.cbegin(), end = workoutUpdates.cend(); it != end; ++it)
    {
        dayIndex = this->get_modelIndex(scheduleModel,it.key().toString(dateFormat),0);
        dayItem = scheduleModel->itemFromIndex(dayIndex);
        scheduleModel->removeRows(0,dayItem->rowCount(),dayIndex);

        for(QMap<int,QStringList>::const_iterator vit = it.value().cbegin(), vend = it.value().cend(); vit != vend; ++vit)
        { 
            QList<QStandardItem*> itemList;
            itemList << new QStandardItem(QString::number(vit.key()));
            for(int itemValue = 0; itemValue < vit.value().count(); ++itemValue)
            {
                itemList << new QStandardItem(vit.value().at(itemValue));
            }
            itemList.at(0)->setData(scheduleTags->at(2),Qt::AccessibleTextRole);
            this->update_linkedWorkouts(it.key(),vit.value().last(),vit.key(),true);
            dayItem->appendRow(itemList);
        }
        changedDays.enqueue(it.key());
        this->set_compValues(true,it.key(),it.value());
    }
    workoutUpdates.clear();
    isUpdated = true;
}


void schedule::copyWeek(QString copyFrom,QString copyTo)
{
    QStandardItem *fromItem = scheduleModel->itemFromIndex(get_modelIndex(scheduleModel,copyFrom,0));

    QDate fromDate = QDate::fromString(this->get_weekMeta(copyFrom).at(3),dateFormat);
    QDate toDate = QDate::fromString(this->get_weekMeta(copyTo).at(3),dateFormat);

    for(int day = 0; day < fromItem->rowCount();++day)
    {
        workoutUpdates.insert(toDate.addDays(day),get_workouts(true,fromDate.addDays(day).toString(dateFormat)));
    }
    this->set_workoutData();
}

void schedule::clearWeek(QString weekID)
{
    QStandardItem *clearItem = scheduleModel->itemFromIndex(get_modelIndex(scheduleModel,weekID,0));

    QDate date = QDate::fromString(this->get_weekMeta(weekID).at(3),dateFormat);

    for(int day = 0; day < clearItem->rowCount();++day)
    {
        workoutUpdates.insert(date.addDays(day),QMap<int,QStringList>());
    }
    this->set_workoutData();
}

void schedule::set_weekCompValues(QStringList weekMeta,QMap<QString, QVector<double> > compValues)
{
    QModelIndex weekIndex = this->get_modelIndex(phaseModel,weekMeta.at(0),0);
    QStandardItem *weekItem = phaseModel->itemFromIndex(weekIndex);

    compWeekMap.insert(weekMeta.at(0),compValues);

    phaseModel->removeRows(0,weekItem->rowCount(),weekIndex);

    for(int meta = 0; meta < weekMeta.count(); ++meta)
    {
       phaseModel->setData(weekIndex.siblingAtColumn(meta),weekMeta.at(meta));
    }

    int sportID = 0;
    for(QMap<QString, QVector<double>>::const_iterator it = compValues.cbegin(), vend = compValues.cend(); it != vend; ++it)
    {
        QList<QStandardItem*> itemList;
        itemList << new QStandardItem(QString::number(sportID++));
        itemList << new QStandardItem(it.key());
        for(int itemValue = 0; itemValue < it.value().count(); ++itemValue)
        {
            itemList << new QStandardItem(QString::number(it.value().at(itemValue)));
        }
        itemList.at(0)->setData(saisonTags->at(3),Qt::AccessibleTextRole);
        weekItem->appendRow(itemList);
    }
    isUpdated = true;
}

void schedule::set_compValues(bool update,QDate workDate,QMap<int,QStringList> valueList)
{
    QMap<QString,QVector<double>> compSum;
    QMap<int,QStringList> compValues;
    QVector<double> stressValue(5);
    stressValue.fill(0);

    if(update)
    {
        this->update_compValues(&compSum,&valueList);
        compMap.insert(workDate,compSum);
        compSum.clear();

        stressValue[1] = stressMap.value(workDate).at(1);
        stressValue[2] = stressMap.value(workDate).at(2);
        compSum = compMap.value(workDate);

        for(QMap<QString,QVector<double>>::const_iterator it = compSum.cbegin(), end = compSum.cend(); it != end; ++it)
        {
            stressValue[0] += it.value().at(4);
            stressValue[3] += it.value().at(1);
            stressValue[4] += it.value().at(3);
        }
        stressMap.insert(workDate,stressValue);
    }
    else
    {
        QStringList saisons = saisonValues.keys();
        QDate compDate;
        qint64 daysLeft;
        for(int saison = 0; saison < saisons.count(); ++saison)
        {
            daysLeft = firstdayofweek.daysTo(QDate().fromString(saisonValues.value(saisons.at(saison)).at(1),dateFormat));
            for(int day = -7; day <= daysLeft; ++day)
            {
                compDate = firstdayofweek.addDays(day);

                this->check_workouts(compDate);
                compValues = this->get_workouts(true,compDate.toString(dateFormat));
                this->update_compValues(&compSum,&compValues);
                compMap.insert(compDate,compSum);
                compSum.clear();

                stressValue.fill(0);
                compSum = compMap.value(compDate);
                for(QMap<QString,QVector<double>>::const_iterator it = compSum.cbegin(), end = compSum.cend(); it != end; ++it)
                {
                    stressValue[0] += it.value().at(4);
                    stressValue[3] += it.value().at(1);
                    stressValue[4] += it.value().at(3);
                }
                compSum.clear();
                stressMap.insert(compDate,stressValue);
            }
        }
    }
    compChanged->setData(workDate.addDays(1 - workDate.dayOfWeek()));
    compChanged->triggered();
    this->recalc_stressValues();
}

void schedule::update_compValues(QMap<QString,QVector<double>> *compSum,QMap<int,QStringList> *compValues)
{
    QVector<double> comp(settings::getHeaderMap("summery")->count());
    comp.fill(0);
    QString compSport;
    int amount = 1;

    for(QMap<int,QStringList>::const_iterator it = compValues->cbegin(), end = compValues->cend(); it != end; ++it)
    {
        compSport = it.value().at(1);

        if(compSum->contains(compSport))
        {
            comp[0] = compSum->value(compSport).at(0) + amount;                                 //Workouts
            comp[1] = compSum->value(compSport).at(1) + it.value().at(5).toDouble();            //Duration
            comp[3] = compSum->value(compSport).at(3) + it.value().at(6).toDouble();            //Distance
            comp[4] = compSum->value(compSport).at(4) + it.value().at(7).toDouble();            //Stress
            comp[5] = compSum->value(compSport).at(5) + it.value().at(8).toDouble();            //Work KJ
        }
        else
        {
            comp[0] = amount;
            comp[1] = it.value().at(5).toDouble();
            comp[3] = it.value().at(6).toDouble();
            comp[4] = it.value().at(7).toDouble();
            comp[5] = it.value().at(8).toDouble();
        }

        compSum->insert(compSport,comp);
        comp.fill(0);
    }
}

void schedule::recalc_stressValues()
{
    int ltsDays = settings::get_intValue("ltsdays");
    int stsDays = settings::get_intValue("stsdays");

    double lte = static_cast<double>(exp(-1.0/ltsDays));
    double ste = static_cast<double>(exp(-1.0/stsDays));
    double ltsStress = 0,stsStress = 0;
    QVector<double> stressValue(5);

    stsStress = stressMap.value(firstdayofweek.addDays(-stsDays-1)).at(1);
    ltsStress = stressMap.value(firstdayofweek.addDays(-ltsDays-1)).at(2);

    for(QMap<QDate,QVector<double>>::const_iterator it = stressMap.cbegin(), end = stressMap.cend(); it != end; ++it)
    {

        if(it.key() != firstdayofweek.addDays(-ltsDays-1))
        {
            stressValue[0] = it.value().at(0);
            stressValue[1] = (it.value().at(0) * (1.0 - ste)) + (stsStress * ste);
            stressValue[2] = (it.value().at(0) * (1.0 - lte)) + (ltsStress * lte);
            stressValue[3] = it.value().at(3);
            stressValue[4] = it.value().at(4);
            stsStress = stressValue[1];
            ltsStress = stressValue[2];
            stressMap.insert(it.key(),stressValue);
        }
    }
}

void schedule::set_stressMap()
{
    QVector<double> stressValue(5,0);

    for(QMap<int,QStringList>::const_iterator it = mapList.cbegin(), end = mapList.cend(); it != end; ++it)
    {
        stressValue[0] = it.value().at(1).toDouble();
        stressValue[1] = it.value().at(2).toDouble();
        stressValue[2] = it.value().at(3).toDouble();
        stressValue[3] = it.value().at(4).toDouble();
        stressValue[4] = it.value().at(5).toDouble();
        stressMap.insert(QDate::fromString(it.value().at(0),dateFormat),stressValue);
        stressValue.fill(0);
    }
}

void schedule::set_saisonValues()
{
    QString saisonName,mapKey;
    QStringList mapValues;

    for(int saisons = 0; saisons < phaseModel->rowCount(); ++saisons)
    {
        for(int col = 0; col < macroTags->count(); ++col)
        {
            if(col == 0)
            {
                saisonName = phaseModel->data(phaseModel->index(saisons,col)).toString();
            }
            else
            {
                mapValues << phaseModel->data(phaseModel->index(saisons,col)).toString();
            }
        }
        saisonValues.insert(saisonName,mapValues);

        mapValues.clear();

        //Fill compWeekMap
        QStandardItem *saisonItem;
        saisonItem = phaseModel->item(saisons,0);
        QVector<double> sportValues(5,0);
        QString sportKey,phaseName;
        QDate startWeekDate;
        QMap<QString,QVector<double>> compValues;
        QMap<QDate,QPair<QString,QString>> weekMapping;

        QStringList sportUseList = settings::get_listValues("Sportuse");

        if(saisonItem->hasChildren())
        {
            for(int phase = 0; phase < saisonItem->rowCount(); ++phase) //Phases
            {
                phaseName = saisonItem->child(phase,0)->data(Qt::DisplayRole).toString();

                if(saisonItem->child(phase,0)->hasChildren())   //Check Weeks within Phases
                {
                    for(int week = 0; week < saisonItem->child(phase,0)->rowCount(); ++week) //Weeks
                    {
                        startWeekDate = QDate::fromString(saisonItem->child(phase,0)->child(week,3)->data(Qt::DisplayRole).toString(),dateFormat);
                        mapKey = saisonItem->child(phase,0)->child(week,0)->data(Qt::DisplayRole).toString();
                        weekMapping.insert(startWeekDate,qMakePair(mapKey,phaseName));

                        if(saisonItem->child(phase,0)->child(week,0)->hasChildren())    //Check Entries within Weeks
                        {
                            for(int comp = 0; comp < saisonItem->child(phase,0)->child(week,0)->rowCount(); ++comp)
                            {
                                for(int values = 1; values < phaseModel->columnCount(); ++values)
                                {
                                    if(values == 1)
                                    {
                                        sportKey = saisonItem->child(phase,0)->child(week,0)->child(comp,values)->data(Qt::DisplayRole).toString();
                                    }
                                    else
                                    {
                                        sportValues[values-2] = saisonItem->child(phase,0)->child(week,0)->child(comp,values)->data(Qt::DisplayRole).toDouble();
                                    }
                                }
                                compValues.insert(sportKey,sportValues);
                            }

                            for(int i = 0; i < sportUseList.count(); ++i)
                            {
                                if(!compValues.contains(sportUseList.at(i)))
                                {
                                   compValues.insert(sportUseList.at(i),QVector<double>(5,0));
                                }
                            }
                        }
                        compWeekMap.insert(mapKey,compValues);
                        weekPhaseMap.insert(startWeekDate,qMakePair(mapKey,phaseName));
                    }
                    saisonWeekMap.insert(saisonName,weekMapping);
                }
            }
        }
    }
    this->set_compValues(false,QDate(),QMap<int,QStringList>());
}
