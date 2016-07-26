#ifndef WEEK_POPUP_H
#define WEEK_POPUP_H

#include <QDialog>
#include <QStandardItemModel>
#include <QtCharts>
#include "schedule.h"
#include "settings.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class week_popup;
}

class week_popup : public QDialog
{
    Q_OBJECT

public:
    explicit week_popup(QWidget *parent = 0,QString weekinfo = 0,schedule *p_sched = 0,settings *p_settings = 0);
    ~week_popup();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

private:
    Ui::week_popup *ui;
    QStringList week_info;
    QStandardItemModel *plotmodel;
    settings *pop_settings;
    schedule *workSched;

    QChart *weekchart;
    QChartView *chartview;
    QValueAxis *yStress,*yDura;
    QBarCategoryAxis *axisX;
    QLineSeries *stressLine;
    QBarSet *duraBar;
    QBarSeries *duraBars;


    void set_plotModel();
    void set_weekInfos();

    void freeMem();
};

#endif // WEEK_POPUP_H
