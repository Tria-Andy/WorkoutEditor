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

#ifndef WEEK_POPUP_H
#define WEEK_POPUP_H

#include <QDialog>
#include <QStandardItemModel>
#include "schedule.h"
#include "settings.h"
#include "calculation.h"

namespace Ui {
class week_popup;
}

class week_popup : public QDialog, public calculation
{
    Q_OBJECT

public:
    explicit week_popup(QWidget *parent = 0,QString weekinfo = 0,schedule *p_sched = 0);
    ~week_popup();

private slots:
    void on_toolButton_close_clicked();
    void on_toolButton_edit_clicked();
    void on_comboBox_yValue_currentIndexChanged(int index);

private:
    Ui::week_popup *ui;
    QStringList week_info,barSelection;
    QSortFilterProxyModel *workProxy;
    schedule *workSched;
    QDateTime firstDay;
    bool isLoad;
    QList<QDateTime> weekDates;
    QVector<double> xStress,xLTS,xBar,xWorks,maxValues;
    QVector<double> yStress,yLTS,yDura,yDist,yWorks,yWorkCount,yValues;
    int dayCount;
    QCPGraph *get_QCPLine(QString,QColor,QVector<double> &xdata,QVector<double> &ydata,bool);
    QCPBars *get_QCPBar(QColor,int,bool);
    void set_itemTracer(QCPGraph*,QVector<double> &xdata,QColor,int);
    void set_itemLineText(QFont,QVector<double> &xdata,QVector<double> &ydata,int);
    void set_itemBarText(QFont,QColor,QVector<double> &xdata,QVector<double> &ydata,QVector<double> &ytext,int,bool);
    void set_plotValues();
    void set_graph();
    void set_weekPlot(int);
};

#endif // WEEK_POPUP_H
