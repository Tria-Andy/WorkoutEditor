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

#ifndef DIALOG_STRESSCALC_H
#define DIALOG_STRESSCALC_H

#include <QDialog>
#include "settings.h"
#include "calculation.h"

namespace Ui {
class Dialog_stresscalc;
}

class Dialog_stresscalc : public QDialog, public calculation
{
    Q_OBJECT

public:
    explicit Dialog_stresscalc(QWidget *parent = nullptr);
    ~Dialog_stresscalc();

private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_calc_clicked();
    void on_comboBox_sport_currentIndexChanged(int index);
    void on_pushButton_clear_clicked();

private:
    Ui::Dialog_stresscalc *ui;
    QStringList combo_sport;
    QString sport;
    QString lab_power,lab_workout;
    double thresPower,thresPace;

    int sport_index,est_stress;
    double *t_pace;
    double est_power,raw_effort,cv_effort;

    void read_threshold_values();
    void set_sport_threshold();
    void estimateStress();
    void reset_calc();

};

#endif // DIALOG_STRESSCALC_H
