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

#ifndef DIALOG_LAPEDITOR_H
#define DIALOG_LAPEDITOR_H

#include <QDialog>
#include "activity.h"
#include "calculation.h"

namespace Ui {
class Dialog_lapeditor;
}

class Dialog_lapeditor : public QDialog, public calculation
{
    Q_OBJECT

public:
    explicit Dialog_lapeditor(QWidget *parent = 0,Activity *p_act = 0,QModelIndex p_index = QModelIndex());
    ~Dialog_lapeditor();

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_close_clicked();
    void on_radioButton_add_clicked();
    void on_radioButton_update_clicked();
    void on_radioButton_del_clicked();
    void on_comboBox_lap_currentIndexChanged(int index);
    void on_comboBox_edit_currentIndexChanged(int index);
    void on_spinBox_starttime_valueChanged(int arg1);
    void on_spinBox_endtime_valueChanged(int arg1);
    void on_comboBox_lap_activated(int index);

    void on_doubleSpinBox_distance_valueChanged(double arg1);

private:
    Ui::Dialog_lapeditor *ui;
    Activity *curr_act;
    QStandardItemModel *editModel;
    QModelIndex selIndex;
    QStringList typeList;
    QString breakName;
    int selRow;
    double swimLapLen;

    void set_lapinfo();
    void set_components(bool);
    void set_visible(bool,bool);
    void set_duration();
    void set_lapSpeed(double);
    void edit_laps(int,int);
    void updateSwimModel(int,int,double,int);
    void updateIntModel(int);
    void recalulateData(int);
    int calc_strokes(int);
};

#endif // DIALOG_LAPEDITOR_H
