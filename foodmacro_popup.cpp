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

    xValues.resize(dayCount);

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
        fatValues[i] = foodplan->dayTarget.value(startDate.addDays(i)).at(2);
        fiberValues[i] = foodplan->dayTarget.value(startDate.addDays(i)).at(3);
        sugarValues[i] = foodplan->dayTarget.value(startDate.addDays(i)).at(4);
        xValues[i] = weekDates.at(i).toTime_t() + 3600;
    }

    set_graph(startDate);
}

void foodmacro_popup::set_graph(QDate startDate)
{

    QCPBarsGroup *barGroup = new QCPBarsGroup(ui->widget_plot);

    QCPBars *carbBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    carbBar->setWidth(0.15);
    carbBar->setAntialiased(true);
    carbBar->setBrush(QBrush(QColor(0,85,255,70)));
    carbBar->setPen(QPen(Qt::darkBlue));
    carbBar->setData(xValues,carbValues);
    carbBar->setBarsGroup(barGroup);

    QCPBars *carbMBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    carbMBar->setWidth(0.15);
    carbMBar->setAntialiased(true);
    carbMBar->setBrush(QBrush(QColor(0,85,255,70)));
    carbMBar->setPen(QPen(Qt::darkBlue));
    carbMBar->setData(xValues,carbMarco);
    carbMBar->setBarsGroup(barGroup);

    QCPBars *proteinBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    proteinBar->setWidth(0.5);
    proteinBar->setAntialiased(true);
    proteinBar->setBrush(QBrush(QColor(85,170,0,70)));
    proteinBar->setPen(QPen(Qt::green));
    proteinBar->setData(xValues,proteinValues);

    QCPBars *fatBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    fatBar->setWidth(0.5);
    fatBar->setAntialiased(true);
    fatBar->setBrush(QBrush(QColor(255,0,0,70)));
    fatBar->setPen(QPen(Qt::red));
    fatBar->setData(xValues,fatValues);

    QCPBars *fiberBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    fiberBar->setWidth(0.5);
    fiberBar->setAntialiased(true);
    fiberBar->setBrush(QBrush(QColor(0,170,255,70)));
    fiberBar->setPen(QPen(Qt::blue));
    fiberBar->setData(xValues,fiberValues);

    QCPBars *sugarBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis);
    sugarBar->setWidth(0.5);
    sugarBar->setAntialiased(true);
    sugarBar->setBrush(QBrush(QColor(255,255,125,70)));
    sugarBar->setPen(QPen(Qt::yellow));
    sugarBar->setStackingGap(1);
    sugarBar->setData(xValues,sugarValues);

    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeSpec(Qt::LocalTime);
    dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dateTimeTicker->setDateTimeFormat("dd.MM");
    dateTimeTicker->setTickCount(dayCount);

    ui->widget_plot->xAxis->setRange(QCPAxisTickerDateTime::dateTimeToKey(startDate),QCPAxisTickerDateTime::dateTimeToKey(startDate.addDays(dayCount)));
    ui->widget_plot->yAxis->setRange(0,1000);
    ui->widget_plot->xAxis->setTicker(dateTimeTicker);
}
