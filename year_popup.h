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

#ifndef YEAR_POPUP_H
#define YEAR_POPUP_H

#include <QDialog>
#include <QStandardItemModel>
#include <QtCharts>
#include "schedule.h"
#include "settings.h"

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class year_popup;
}

class year_popup : public QDialog
{
    Q_OBJECT

public:
    explicit year_popup(QWidget *parent = 0, QString pInfo = 0,int position = 0,schedule *p_sched = 0, QString pPhase = 0,int pIndex = 0);
    ~year_popup();

private slots:
    void on_pushButton_close_clicked();

    void on_comboBox_select_currentIndexChanged(int index);

private:
    Ui::year_popup *ui;

    QStringList partInfo,phaseList,selectList;
    QString phase;
    int col,phaseindex,widthFactor,selectAxis,weekcount;
    double max_stress;
    schedule *workSched;
    QVector<double> y_stress,y_dura,y_dist,y_work,maxValues;

    QChart *phasechart;
    QChartView *chartview;
    QValueAxis *yStress,*yBars;
    QBarCategoryAxis *axisX;
    QLineSeries *stressLine;
    QBarSet *selectBar;
    QBarSeries *selectBars;

    void fill_values();
    void set_plot(int);
};

#endif // YEAR_POPUP_H
