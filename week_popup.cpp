#include "week_popup.h"
#include "ui_week_popup.h"

week_popup::week_popup(QWidget *parent,QString weekinfo,schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::week_popup)
{
    ui->setupUi(this);
    week_info << weekinfo.split("#");
    workSched = p_sched;
    filledWeek = true;
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->set_plotModel();
}

week_popup::~week_popup()
{
    delete ui;
}

void week_popup::freeMem()
{
    delete weekchart;
    delete yStress;
    delete yDura;
    delete axisX;
    delete stressLine;
    delete duraBar;
    delete duraBars;
    delete chartview;
    delete plotmodel;
}

void week_popup::set_plotModel()
{
    QModelIndex index;
    QList<QStandardItem*> list = workSched->workout_schedule->findItems(week_info.at(0),Qt::MatchExactly,0);
    QDateTime workoutDate;
    plotmodel = new QStandardItemModel(list.count(),3);

    if(!list.isEmpty())
    {
    for(int i = 0; i < list.count(); ++i)
    {
        index = workSched->workout_schedule->indexFromItem(list.at(i));
        workoutDate = QDateTime::fromString(workSched->workout_schedule->item(index.row(),1)->text(),"dd.MM.yyyy");
        plotmodel->setData(plotmodel->index(i,0,QModelIndex()),workoutDate);
        plotmodel->setData(plotmodel->index(i,1,QModelIndex()),workSched->workout_schedule->item(index.row(),8)->text().toInt());
        plotmodel->setData(plotmodel->index(i,2,QModelIndex()),settings::get_timesec(workSched->workout_schedule->item(index.row(),6)->text()) / 60.0);
    }
    plotmodel->sort(0);

    ui->label_weekinfos->setText("Week: " + week_info.at(0) + " - Phase: " + week_info.at(1) + " - Workouts: " + QString::number(list.count()));
    this->set_weekInfos();
    }
    else
    {
        filledWeek = false;
        ui->label_weekinfos->setText("Week: " + week_info.at(0) + " - Phase: " + week_info.at(1) + " - Workouts: " + QString::number(list.count()));
    }

}

void week_popup::set_weekInfos()
{
    int size = 8;
    QVector<double> stress(size),dura(size);;
    double v_stress,v_dura,max_stress = 0.0,max_dura = 0.0;
    weekchart = new QChart();
    chartview = new QChartView(weekchart);
    chartview->setRenderHint(QPainter::Antialiasing);
    ui->verticalLayout_plot->addWidget(chartview);

    stressLine = workSched->get_qLineSeries(true);
    duraBar = new QBarSet("Duration");
    duraBars = new QBarSeries();
    axisX = new QBarCategoryAxis();

    QStringList axisValues;
    QMargins chartMargins(5,5,5,5);

    QList<QDateTime> workDateList;
    qreal stressY;
    QString v_date1,v_date2;
    QDateTime workDate;

    for(int i = 0,day = 0; i < plotmodel->rowCount(); ++i)
    {
        v_date1 = plotmodel->data(plotmodel->index(i,0,QModelIndex())).toDateTime().toString("dd.MM.yyyy");
        v_stress = plotmodel->data(plotmodel->index(i,1,QModelIndex())).toDouble();
        v_dura = plotmodel->data(plotmodel->index(i,2,QModelIndex())).toDouble() / 60.0;
        v_dura = settings::set_doubleValue(v_dura,false);

        workDate = QDateTime::fromString(v_date1,"dd.MM.yyyy");
        if(i != 0)
        {
            v_date2 = plotmodel->data(plotmodel->index(i-1,0,QModelIndex())).toDateTime().toString("dd.MM.yyyy");
            if(v_date1 == v_date2)
            {
                stress[day] = stress[day] + v_stress;
                dura[day] = dura[day] + v_dura;
            }
            else
            {
                ++day;
                stress[day] = v_stress;
                dura[day] = v_dura;
                workDateList.append(workDate);
            }
        }
        else
        {
            stress[day] = v_stress;
            dura[day] = v_dura;
            workDateList.append(workDate);
        }
        if(max_stress < stress[day]) max_stress = stress[day];
        if(max_dura < dura[day]) max_dura = dura[day];
    }

    for(int i = 0; i < size-1; ++i)
    {
        axisValues << workDateList.at(i).toString("dd.MM");
        stressY = stress[i];
        duraBar->append(dura[i]);
        stressLine->append(i,stressY);
    }

    duraBar->setPen(QPen(Qt::blue));
    duraBar->setBrush(QColor(0, 0, 255, 120));

    duraBars->setLabelsFormat("@value");
    duraBars->setLabelsVisible(true);
    duraBars->append(duraBar);

    weekchart->addSeries(duraBars);
    weekchart->addSeries(stressLine);

    axisX->append(axisValues);
    axisX->setTitleText("Week");
    axisX->setTitleVisible(true);

    yStress = workSched->get_qValueAxis("Stress",true,max_stress+(max_stress*0.05),10);
    yStress->applyNiceNumbers();
    weekchart->addAxis(yStress,Qt::AlignLeft);
    stressLine->attachAxis(yStress);
    weekchart->setAxisX(axisX,stressLine);

    yDura = workSched->get_qValueAxis("Duration",true,max_dura+(max_dura*0.05),10);
    yDura->applyNiceNumbers();
    weekchart->addAxis(yDura,Qt::AlignRight);
    duraBars->attachAxis(yDura);  
    weekchart->setAxisX(axisX,duraBars);

    weekchart->setMargins(chartMargins);
    weekchart->setBackgroundRoundness(5);
    weekchart->setDropShadowEnabled(true);
    weekchart->legend()->hide();
}

void week_popup::on_pushButton_close_clicked()
{
    if(filledWeek)
    {
        this->freeMem();
    }
    reject();
}

void week_popup::on_pushButton_edit_clicked()
{
    this->freeMem();
    accept();
}
