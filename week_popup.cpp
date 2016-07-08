#include "week_popup.h"
#include "ui_week_popup.h"

week_popup::week_popup(QWidget *parent,QString weekinfo,schedule *p_sched,settings *p_settings) :
    QDialog(parent),
    ui(new Ui::week_popup)
{
    ui->setupUi(this);
    week_info << weekinfo.split("#");
    workSched = p_sched;
    pop_settings = p_settings;
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->set_plotModel();
}

week_popup::~week_popup()
{
    delete ui;
    delete plotmodel;
}

void week_popup::set_plotModel()
{
    QModelIndex index;
    QList<QStandardItem*> list = workSched->workout_schedule->findItems(week_info.at(0),Qt::MatchExactly,0);
    QDateTime workoutDate;
    plotmodel = new QStandardItemModel(list.count(),3);

    for(int i = 0; i < list.count(); ++i)
    {
        index = workSched->workout_schedule->indexFromItem(list.at(i));
        workoutDate = QDateTime::fromString(workSched->workout_schedule->item(index.row(),1)->text(),"dd.MM.yyyy");
        plotmodel->setData(plotmodel->index(i,0,QModelIndex()),workoutDate);
        plotmodel->setData(plotmodel->index(i,1,QModelIndex()),workSched->workout_schedule->item(index.row(),8)->text().toInt());
        plotmodel->setData(plotmodel->index(i,2,QModelIndex()),pop_settings->get_timesec(workSched->workout_schedule->item(index.row(),6)->text()) / 60.0);
    }
    plotmodel->sort(0);

    ui->label_weekinfos->setText("Week: " + week_info.at(0) + " - Phase: " + week_info.at(1) + " - Workouts: " + QString::number(list.count()));
    this->set_weekInfos();
}

void week_popup::set_weekInfos()
{
    int size = 8;
    QVector<double> x(size),y_stress(size),y_dura(size);
    QVector<double> ticks;
    QVector<QString> x_labels(size);
    ticks << 0 << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8;
    QString v_date1,v_date2;
    QDateTime workDate;
    double v_stress,v_dura, max_stress = 0.0, max_dura = 0.0;

    for(int i = 0,day = 0; i < plotmodel->rowCount(); ++i)
    {
        v_date1 = plotmodel->data(plotmodel->index(i,0,QModelIndex())).toDateTime().toString("dd.MM.yyyy");
        v_stress = plotmodel->data(plotmodel->index(i,1,QModelIndex())).toDouble();
        v_dura = plotmodel->data(plotmodel->index(i,2,QModelIndex())).toDouble() / 60.0;
        workDate = QDateTime::fromString(v_date1,"dd.MM.yyyy");
            if(i != 0)
            {
                v_date2 = plotmodel->data(plotmodel->index(i-1,0,QModelIndex())).toDateTime().toString("dd.MM.yyyy");
                if(v_date1 == v_date2)
                {
                    y_stress[day] = y_stress[day] + v_stress;
                    y_dura[day] = y_dura[day] + v_dura;
                }
                else
                {
                    ++day;
                    y_stress[day] = v_stress;
                    y_dura[day] = v_dura;
                }
            }
            else
            {
                y_stress[day] = v_stress;
                y_dura[day] = v_dura;
            }
            x[day] = day;
            x_labels[day] = workDate.toString("dd.MM.");
            if(max_stress < y_stress[day]) max_stress = y_stress[day];
            if(max_dura < y_dura[day]) max_dura = y_dura[day];
    }
    ui->widget_plot->xAxis->setAutoTicks(false);
    ui->widget_plot->xAxis->setAutoTickLabels(false);
    ui->widget_plot->xAxis->setTickVector(ticks);
    ui->widget_plot->xAxis->setTickVectorLabels(x_labels);
    ui->widget_plot->xAxis->setLabel("Date");
    ui->widget_plot->xAxis->setRange(-1,7);

    ui->widget_plot->yAxis->setLabel("Stress Score");
    ui->widget_plot->yAxis->setRange(0,max_stress + 20.0);

    QCPGraph *stress = ui->widget_plot->addGraph();
    stress->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssStar));
    stress->setData(x,y_stress);
    stress->setPen(QPen(Qt::red));
    stress->setBrush(QColor(255,0,0,60));

    QCPBars *duration = new QCPBars(ui->widget_plot->xAxis,ui->widget_plot->yAxis2);
    ui->widget_plot->addPlottable(duration);
    ui->widget_plot->yAxis2->setLabel("Duration");
    ui->widget_plot->yAxis2->setRange(0,max_dura + 0.5);
    ui->widget_plot->yAxis2->setVisible(true);
    duration->setWidth(4/(double)x.size());
    duration->setData(x,y_dura);
    duration->setPen(QPen(Qt::blue));
    duration->setBrush(QColor(0, 0, 255, 120));
}

void week_popup::on_pushButton_clicked()
{
    reject();
}

void week_popup::on_pushButton_2_clicked()
{
    accept();
}
