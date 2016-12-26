#include "stress_popup.h"
#include "ui_stress_popup.h"
#include <cmath>

stress_popup::stress_popup(QWidget *parent,schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::stress_popup)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    stressMap = p_sched->get_StressMap();
    isLoad = false;
    ltsDays = settings::get_ltsValue("ltsdays");
    stsDays = settings::get_ltsValue("stsdays");
    lastLTS = settings::get_ltsValue("lastlts");
    lastSTS = settings::get_ltsValue("laststs");
    firstDayofWeek = QDate::currentDate().addDays(1-QDate::currentDate().dayOfWeek());
    dateRange = 6;
    ui->dateEdit_start->setDateRange(firstDayofWeek,stressMap->lastKey().addDays(-dateRange));
    ui->dateEdit_start->setDate(firstDayofWeek);
    ui->dateEdit_end->setDateRange(firstDayofWeek.addDays(dateRange),stressMap->lastKey());
    ui->dateEdit_end->setDate(firstDayofWeek.addDays(dateRange));
    ui->pushButton_values->setIcon(QIcon(":/images/icons/No.png"));
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

    ui->widget_stressPlot->setInteractions(QCP::iSelectLegend | QCP::iMultiSelect);

    ui->widget_stressPlot->xAxis->setLabel("Date");
    ui->widget_stressPlot->xAxis->setLabelFont(plotFont);

    ui->widget_stressPlot->xAxis2->setVisible(true);
    ui->widget_stressPlot->xAxis2->setLabel("Weeks");
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
    startDate.setTimeSpec(Qt::UTC);

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

    this->set_stressplot(rangeStart,rangeEnd,true);
}

void stress_popup::set_stressplot(QDate rangeStart,QDate rangeEnd,bool showValues)
{
    ui->widget_stressPlot->clearPlottables();
    ui->widget_stressPlot->clearItems();
    ui->widget_stressPlot->legend->setFillOrder(QCPLegend::foColumnsFirst);
    ui->widget_stressPlot->plotLayout()->setRowStretchFactor(1,0.0001);

    QCPRange xRange(QCPAxisTickerDateTime::dateTimeToKey(rangeStart.addDays(-1)),QCPAxisTickerDateTime::dateTimeToKey(rangeEnd.addDays(1)));

    QFont lineFont;
    lineFont.setPointSize(8);
    int xTickCount = dateRange;

    QCPGraph *ltsLine = ui->widget_stressPlot->addGraph();
    ltsLine->setName("LTS");
    ltsLine->setLineStyle(QCPGraph::lsLine);
    ltsLine->setData(xDate,yLTS);
    ltsLine->setAntialiased(true);
    ltsLine->setPen(QPen(QColor(0,255,0),1));

    QCPGraph *stsLine = ui->widget_stressPlot->addGraph();
    stsLine->setName("STS");
    stsLine->setLineStyle(QCPGraph::lsLine);
    stsLine->setData(xDate,ySTS);
    stsLine->setAntialiased(true);
    stsLine->setPen(QPen(QColor(255,0,0),1));

    QCPGraph *tsbLine = ui->widget_stressPlot->addGraph(ui->widget_stressPlot->xAxis,ui->widget_stressPlot->yAxis2);
    tsbLine->setName("TSB");
    tsbLine->setLineStyle(QCPGraph::lsLine);
    tsbLine->setData(xDate,yTSB);
    tsbLine->setAntialiased(true);
    tsbLine->setBrush(QBrush(QColor(255,170,0,50)));
    tsbLine->setPen(QPen(QColor(255,170,0),1));

    for(int i = 0; i < xDate.count(); ++i)
    {
        QCPItemTracer *ltsTracer = new QCPItemTracer(ui->widget_stressPlot);
        ltsTracer->setGraph(ltsLine);
        ltsTracer->setGraphKey(xDate[i]);
        ltsTracer->setStyle(QCPItemTracer::tsCircle);
        ltsTracer->setBrush(Qt::green);

        QCPItemTracer *stsTracer = new QCPItemTracer(ui->widget_stressPlot);
        stsTracer->setGraph(stsLine);
        stsTracer->setGraphKey(xDate[i]);
        stsTracer->setStyle(QCPItemTracer::tsCircle);
        stsTracer->setBrush(Qt::red);

        QCPItemTracer *tsbTracer = new QCPItemTracer(ui->widget_stressPlot);
        tsbTracer->setGraph(tsbLine);
        tsbTracer->setGraphKey(xDate[i]);
        tsbTracer->setStyle(QCPItemTracer::tsCircle);
        tsbTracer->setBrush(QBrush(QColor(255,170,0)));

        if(showValues)
        {
            QCPItemText *ltsText = new QCPItemText(ui->widget_stressPlot);
            ltsText->position->setType(QCPItemPosition::ptPlotCoords);
            ltsText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
            ltsText->position->setCoords(xDate[i],yLTS[i]+1);
            ltsText->setText(QString::number(yLTS[i]));
            ltsText->setTextAlignment(Qt::AlignCenter);
            ltsText->setFont(lineFont);
            ltsText->setPadding(QMargins(1, 1, 1, 1));

            QCPItemText *stsText = new QCPItemText(ui->widget_stressPlot);
            stsText->position->setType(QCPItemPosition::ptPlotCoords);
            stsText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
            stsText->position->setCoords(xDate[i],ySTS[i]+1);
            stsText->setText(QString::number(ySTS[i]));
            stsText->setTextAlignment(Qt::AlignCenter);
            stsText->setFont(lineFont);
            stsText->setPadding(QMargins(1, 1, 1, 1));

            QCPItemText *tsbText = new QCPItemText(ui->widget_stressPlot);
            tsbText->position->setType(QCPItemPosition::ptPlotCoords);
            tsbText->setPositionAlignment(Qt::AlignHCenter|Qt::AlignBottom);
            tsbText->position->setCoords(xDate[i],yTSB[i]+1);
            tsbText->position->setAxes(ui->widget_stressPlot->xAxis,ui->widget_stressPlot->yAxis2);
            tsbText->setText(QString::number(yTSB[i]));
            tsbText->setTextAlignment(Qt::AlignCenter);
            tsbText->setFont(lineFont);
            tsbText->setPadding(QMargins(1, 1, 1, 1));
        }

        if(tsbMinMax[0] > yTSB[i]) tsbMinMax[0] = yTSB[i];
        if(tsbMinMax[1] < yTSB[i]) tsbMinMax[1] = yTSB[i];
    }

    QSharedPointer<QCPAxisTickerDateTime> dateTimeTicker(new QCPAxisTickerDateTime);
    dateTimeTicker->setDateTimeSpec(Qt::UTC);
    dateTimeTicker->setTickStepStrategy(QCPAxisTicker::tssMeetTickCount);
    dateTimeTicker->setDateTimeFormat("dd.MM");

    if(dateRange > 20 && dateRange < 42)
    {
        xTickCount = (dateRange+1)/3;
    }
    else if(dateRange >= 42)
    {
        xTickCount = (dateRange+1)/5;
    }
    dateTimeTicker->setTickCount(xTickCount);

    ui->widget_stressPlot->yAxis->setRange(0,stressMax+10);
    ui->widget_stressPlot->xAxis2->setRange(0,(ui->dateEdit_start->date().daysTo(ui->dateEdit_end->date().addDays(1)))/7);
    ui->widget_stressPlot->yAxis2->setRange(tsbMinMax[0]-5,tsbMinMax[1]+5);
    ui->widget_stressPlot->xAxis->setRange(xRange);
    ui->widget_stressPlot->xAxis->setTicker(dateTimeTicker);

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
        ui->pushButton_values->setIcon(QIcon(":/images/icons/No.png"));
        this->set_stressplot(ui->dateEdit_start->date(),ui->dateEdit_end->date(),checked);
    }
    else
    {
        ui->pushButton_values->setIcon(QIcon(":/images/icons/Yes.png"));
        this->set_stressplot(ui->dateEdit_start->date(),ui->dateEdit_end->date(),checked);
    }
}
