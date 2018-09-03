#include "foodmacro_popup.h"
#include "ui_foodmacro_popup.h"

foodmacro_popup::foodmacro_popup(QWidget *parent,foodplanner *pFood,const QDate startDate) :
    QDialog(parent),
    ui(new Ui::foodmacro_popup)
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    foodplan = pFood;
    startDay = startDate.addDays(1-startDate.dayOfWeek());
    dayCount = 7;
    carbValues.resize(dayCount);
    proteinValues.resize(dayCount);
    fatValues.resize(dayCount);
    fiberValues.resize(dayCount);
    sugarValues.resize(dayCount);

    carbMarco.resize(dayCount);
    proteinMacro.resize(dayCount);
    fatMacro.resize(dayCount);
    fiberMacro.resize(dayCount);
    sugarMacro.resize(dayCount);
    yMax = 0;
    xValues.resize(dayCount);

    macroHeader = settings::get_listValues("MacroHeader");

    ui->tableWidget_macros->setRowCount(macroHeader.count());
    ui->tableWidget_macros->setColumnCount(dayCount);
    ui->tableWidget_macros->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_macros->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableWidget_macros->setItemDelegate(&del_foodMacro);

    for(int i = 0; i < macroHeader.count(); ++i)
    {
        macroColors.insert(macroHeader.at(i),settings::get_colorMap().value(macroHeader.at(i)));
    }

    set_plotValues(startDay);
}

foodmacro_popup::~foodmacro_popup()
{
    delete ui;
}

void foodmacro_popup::on_toolButton_close_clicked()
{
    reject();
}

void foodmacro_popup::set_plotValues(QDate startDate)
{   
    ui->widget_plot->xAxis->setLabel("Day");
    ui->widget_plot->yAxis->setLabel("Gramms");

    ui->widget_plot->legend->setVisible(true);

    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    ui->widget_plot->plotLayout()->addElement(1,0,subLayout);
    subLayout->setMargins(QMargins(10,1,10,1));
    subLayout->addElement(0,0,ui->widget_plot->legend);

    QString itemValue;
    QDateTime weekStart;
    QTime wTime;
    wTime.fromString("00:00:00","hh:mm:ss");
    weekStart.setDate(startDate);
    weekStart.setTime(wTime);
    weekStart.setTimeSpec(Qt::LocalTime);

    for(int i = 0; i < dayCount; ++i)
    {
        weekDates.insert(i,weekStart.addDays(i));
        carbValues[i] = foodplan->dayTarget.value(startDate.addDays(i)).at(0);
        carbMarco[i] = foodplan->dayMacros.value(startDate.addDays(i)).at(0);
        proteinValues[i] = foodplan->dayTarget.value(startDate.addDays(i)).at(1);
        proteinMacro[i] = foodplan->dayMacros.value(startDate.addDays(i)).at(1);
        fatValues[i] = foodplan->dayTarget.value(startDate.addDays(i)).at(2);
        fatMacro[i] = foodplan->dayMacros.value(startDate.addDays(i)).at(2);
        fiberValues[i] = foodplan->dayTarget.value(startDate.addDays(i)).at(3);
        fiberMacro[i] = foodplan->dayMacros.value(startDate.addDays(i)).at(3);
        sugarValues[i] = foodplan->dayTarget.value(startDate.addDays(i)).at(4);
        sugarMacro[i] = foodplan->dayMacros.value(startDate.addDays(i)).at(4);
        xValues[i] = weekDates.at(i).toTime_t() + 3600;
    }

    for(QMap<QDate,QVector<double>>::const_iterator it = foodplan->dayTarget.cbegin(), end = foodplan->dayTarget.cend(); it != end; ++it )
    {
        for(int i = 0; i < 5; ++i)
        {
            if(it.value().at(i) > yMax)
            {
                yMax = it.value().at(i);
            }
        }
    }

    for(QMap<QDate,QVector<double>>::const_iterator it = foodplan->dayMacros.cbegin(), end = foodplan->dayMacros.cend(); it != end; ++it )
    {
        for(int i = 0; i < 5; ++i)
        {
            if(it.value().at(i) > yMax)
            {
                yMax = it.value().at(i);
            }
        }
    }

    for(int day = 0,pos = 0; day < dayCount; ++day)
    {
        dateHeader << weekDates.at(day).toString("dd.MM");

        for(int mac = 0; mac < macroHeader.count(); ++mac)
        {
            QTableWidgetItem *item = new QTableWidgetItem();

            if(mac %2 == 0)
            {
                itemValue = QString::number(foodplan->dayTarget.value(weekDates.at(day).date()).at(pos));
            }
            else
            {
                itemValue = QString::number(foodplan->dayMacros.value(weekDates.at(day).date()).at(pos));
            }

            if(mac %2 == 1) ++pos;

            item->setText(itemValue);
            ui->tableWidget_macros->setItem(mac,day,item);
        }
        pos = 0;

    }

    ui->tableWidget_macros->setHorizontalHeaderLabels(dateHeader);
    ui->tableWidget_macros->setVerticalHeaderLabels(macroHeader);

    set_graph(startDate);
}

void foodmacro_popup::set_graph(QDate startDate)
{
    ui->widget_plot->clearPlottables();
    ui->widget_plot->clearItems();
    ui->widget_plot->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->widget_plot->plotLayout()->setRowStretchFactor(1,0.0001);

    double barWidth = 6.0;
    QCPBarsGroup *barGroup = new QCPBarsGroup(ui->widget_plot);

    QCPBars *carbBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    carbBar->setName(macroHeader.at(0));
    carbBar->setWidthType(QCPBars::wtAbsolute);
    carbBar->setWidth(barWidth);
    carbBar->setAntialiased(true);
    carbBar->setBrush(QBrush(macroColors.value(macroHeader.at(0))));
    carbBar->setPen(QPen(Qt::darkGray));
    carbBar->setData(xValues,carbValues);
    carbBar->setBarsGroup(barGroup);

    QCPBars *carbMBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    carbMBar->setName(macroHeader.at(1));
    carbMBar->setWidthType(QCPBars::wtAbsolute);
    carbMBar->setWidth(barWidth);
    carbMBar->setAntialiased(true);
    carbMBar->setBrush(QBrush(macroColors.value(macroHeader.at(1))));
    carbMBar->setPen(QPen(Qt::darkGray));
    carbMBar->setData(xValues,carbMarco);
    carbMBar->setBarsGroup(barGroup);

    QCPBars *proteinBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    proteinBar->setName(macroHeader.at(2));
    proteinBar->setWidthType(QCPBars::wtAbsolute);
    proteinBar->setWidth(barWidth);
    proteinBar->setAntialiased(true);
    proteinBar->setBrush(QBrush(macroColors.value(macroHeader.at(2))));
    proteinBar->setPen(QPen(Qt::darkGray));
    proteinBar->setData(xValues,proteinValues);
    proteinBar->setBarsGroup(barGroup);

    QCPBars *proteinMBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    proteinMBar->setName(macroHeader.at(3));
    proteinMBar->setWidthType(QCPBars::wtAbsolute);
    proteinMBar->setWidth(barWidth);
    proteinBar->setAntialiased(true);
    proteinMBar->setBrush(QBrush(macroColors.value(macroHeader.at(3))));
    proteinMBar->setPen(QPen(Qt::darkGray));
    proteinMBar->setData(xValues,proteinMacro);
    proteinMBar->setBarsGroup(barGroup);

    QCPBars *fatBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    fatBar->setName(macroHeader.at(4));
    fatBar->setWidthType(QCPBars::wtAbsolute);
    fatBar->setWidth(barWidth);
    fatBar->setAntialiased(true);
    fatBar->setBrush(QBrush(macroColors.value(macroHeader.at(4))));
    fatBar->setPen(QPen(Qt::darkGray));
    fatBar->setData(xValues,fatValues);
    fatBar->setBarsGroup(barGroup);

    QCPBars *fatMBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    fatMBar->setName(macroHeader.at(5));
    fatMBar->setWidthType(QCPBars::wtAbsolute);
    fatMBar->setWidth(barWidth);
    fatMBar->setAntialiased(true);
    fatMBar->setBrush(QBrush(macroColors.value(macroHeader.at(5))));
    fatMBar->setPen(QPen(Qt::darkGray));
    fatMBar->setData(xValues,fatMacro);
    fatMBar->setBarsGroup(barGroup);

    QCPBars *fiberBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    fiberBar->setName(macroHeader.at(6));
    fiberBar->setWidthType(QCPBars::wtAbsolute);
    fiberBar->setWidth(barWidth);
    fiberBar->setAntialiased(true);
    fiberBar->setBrush(QBrush(macroColors.value(macroHeader.at(6))));
    fiberBar->setPen(QPen(Qt::darkGray));
    fiberBar->setData(xValues,fiberValues);
    fiberBar->setBarsGroup(barGroup);

    QCPBars *fiberMBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    fiberMBar->setName(macroHeader.at(7));
    fiberMBar->setWidthType(QCPBars::wtAbsolute);
    fiberMBar->setWidth(barWidth);
    fiberMBar->setAntialiased(true);
    fiberMBar->setBrush(QBrush(macroColors.value(macroHeader.at(7))));
    fiberMBar->setPen(QPen(Qt::darkGray));
    fiberMBar->setData(xValues,fiberMacro);
    fiberMBar->setBarsGroup(barGroup);

    QCPBars *sugarBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    sugarBar->setName(macroHeader.at(8));
    sugarBar->setWidthType(QCPBars::wtAbsolute);
    sugarBar->setWidth(barWidth);
    sugarBar->setAntialiased(true);
    sugarBar->setBrush(QBrush(macroColors.value(macroHeader.at(8))));
    sugarBar->setPen(QPen(Qt::darkGray));
    sugarBar->setData(xValues,sugarValues);
    sugarBar->setBarsGroup(barGroup);

    QCPBars *sugarMBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    sugarMBar->setName(macroHeader.at(9));
    sugarMBar->setWidthType(QCPBars::wtAbsolute);
    sugarMBar->setWidth(barWidth);
    sugarMBar->setAntialiased(true);
    sugarMBar->setBrush(QBrush(macroColors.value(macroHeader.at(9))));
    sugarMBar->setPen(QPen(Qt::darkGray));
    sugarMBar->setData(xValues,sugarMacro);
    sugarMBar->setBarsGroup(barGroup);

    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeSpec(Qt::LocalTime);
    dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dateTimeTicker->setDateTimeFormat("dd.MM");
    dateTimeTicker->setTickCount(dayCount+1);

    ui->widget_plot->xAxis->setRange(QCPAxisTickerDateTime::dateTimeToKey(startDate.addDays(-1)),QCPAxisTickerDateTime::dateTimeToKey(startDate.addDays(dayCount)));
    ui->widget_plot->yAxis->setRange(0,yMax+50.0);
    ui->widget_plot->xAxis->setTicker(dateTimeTicker);
}
