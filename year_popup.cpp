#include "year_popup.h"
#include "ui_year_popup.h"

year_popup::year_popup(QWidget *parent, QString pInfo,int position,schedule *p_sched,QString pPhase, int pIndex,settings *p_setting) :
    QDialog(parent),
    ui(new Ui::year_popup)
{
    ui->setupUi(this);
    pop_settings = p_setting;
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    partInfo = pInfo.split("-");
    workSched = p_sched;
    phase = pPhase;
    phaseindex = pIndex;
    phaseList << "Year";
    for(int i = 0; i < pop_settings->get_phaseList().count(); ++i)
    {
        phaseList << pop_settings->get_phaseList().at(i);
    }
    if(position == 0) col = 7;
    if(position == 1) col = 2;
    if(position == 2) col = 3;
    if(position == 3) col = 4;
    if(position == 4) col = 5;
    if(position == 5) col = 6;

    if(phaseindex == 0) widthFactor = 18;   //All
    if(phaseindex == 1) widthFactor = 100;  //OFF
    if(phaseindex == 2) widthFactor = 80;   //PREP
    if(phaseindex == 3) widthFactor = 70;   //BASE
    if(phaseindex == 4) widthFactor = 90;   //BUILD
    if(phaseindex == 5) widthFactor = 200;  //PEAK
    if(phaseindex == 6) widthFactor = 80;   //WK

    this->set_plot();
}

year_popup::~year_popup()
{
    delete ui;
}

void year_popup::on_pushButton_close_clicked()
{
    reject();
}

void year_popup::set_plot()
{
    int size;
    QList<QStandardItem*> list;
    QModelIndex index;
    QString weekID;
    if(phaseindex == 0)
    {
        size = pop_settings->get_saisonWeeks();
    }
    else
    {
        size = workSched->week_meta->findItems(phase,Qt::MatchContains,2).count();
        list = workSched->week_meta->findItems(phase,Qt::MatchContains,2);
    }
    QVector<double> ticks(size);
    for(int i = 0; i < size; ++i)
    {
        ticks[i] = i;
    }

    this->setFixedWidth(widthFactor*size);
    QVector<double> x(size),y_stress(size),y_dura(size);
    QVector<QString> x_labels(size);
    QString sumValue,stress,duration;
    QStringList sumValues;
    double max_stress = 0.0, max_dura = 0.0;
    workSched->week_content->sort(0);

    if(phaseindex == 0)
    {
        for(int week = 0; week < workSched->week_content->rowCount(); ++week)
        {
            sumValue = workSched->week_content->data(workSched->week_content->index(week,col,QModelIndex())).toString();
            sumValues = sumValue.split("-");
            duration = sumValues.at(2);
            stress = sumValues.at(3);

            x[week] = week;
            y_stress[week] = stress.toDouble();
            y_dura[week] = static_cast<double>(pop_settings->get_timesec(duration) / 60.0);

            if(max_stress < y_stress[week]) max_stress = y_stress[week];
            if(max_dura < y_dura[week]) max_dura = y_dura[week];
            x_labels[week] = QString::number(week+1);
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
                    duration = sumValues.at(2);
                    stress = sumValues.at(3);

                    x[week] = week;
                    y_stress[week] = stress.toDouble();
                    y_dura[week] = static_cast<double>(pop_settings->get_timesec(duration) / 60.0);

                    if(max_stress < y_stress[week]) max_stress = y_stress[week];
                    if(max_dura < y_dura[week]) max_dura = y_dura[week];
                }
            }
            x_labels[week] = QString::number(week+1);
        }
    }
    ui->label_info->setText(phaseList.at(phaseindex) +": " + partInfo.at(0) + " Workouts: " + partInfo.at(1) + " - Hours: " + partInfo.at(2) + " - Distance: " + partInfo.at(4));

    ui->widget_plot->xAxis->setAutoTicks(false);
    ui->widget_plot->xAxis->setAutoTickLabels(false);
    ui->widget_plot->xAxis->setTickVector(ticks);
    ui->widget_plot->xAxis->setTickVectorLabels(x_labels);
    ui->widget_plot->xAxis->setLabel("Weeks");
    ui->widget_plot->xAxis->setRange(-1,size);


    ui->widget_plot->yAxis->setLabel("Stress Score");
    ui->widget_plot->yAxis->setRange(0,max_stress + 100);

    QCPGraph *stressline = ui->widget_plot->addGraph();
    stressline->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssStar));
    stressline->setData(x,y_stress);
    stressline->setPen(QPen(Qt::red));
    stressline->setBrush(QColor(255,0,0,60));

    QCPBars *duraBar = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    ui->widget_plot->addPlottable(duraBar);
    ui->widget_plot->yAxis2->setLabel("Duration");
    ui->widget_plot->yAxis2->setRange(0,max_dura + 1);
    ui->widget_plot->yAxis2->setVisible(true);
    if(phaseindex == 0)
    {
        duraBar->setWidth(25/static_cast<double>(x.size()));
    }
    else
    {
        duraBar->setWidth((size/2)/static_cast<double>(x.size()));
    }
    duraBar->setData(x,y_dura);
    duraBar->setPen(QPen(Qt::blue));
    duraBar->setBrush(QColor(0, 0, 255, 120));
}
