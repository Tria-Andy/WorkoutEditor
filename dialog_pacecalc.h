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

#ifndef DIALOG_PACECALC_H
#define DIALOG_PACECALC_H

#include <QDialog>
#include <QtCore>
#include <QStandardItemModel>
#include "settings.h"

namespace Ui {
class Dialog_paceCalc;
}

class Dialog_paceCalc : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_paceCalc(QWidget *parent = 0);
    ~Dialog_paceCalc();

private slots:
    void on_pushButton_close_clicked();
    void on_comboBox_sport_currentTextChanged(const QString &arg1);
    void on_timeEdit_pace_timeChanged(const QTime &time);
    void on_lineEdit_dist_textChanged(const QString &arg1);
    void on_spinBox_factor_valueChanged(int arg1);
    void on_timeEdit_intTime_timeChanged(const QTime &time);
    void on_spinBox_IntDist_valueChanged(int arg1);
    void on_pushButton_clicked();
    void on_toolButton_copy_clicked();

private:
    Ui::Dialog_paceCalc *ui;
    QStringList sportList,model_header;
    QStandardItemModel *pace_model;
    QVector<double> dist;
    int distFactor;
    void init_paceView();
    void set_pace();
    void set_freeField(int);
};

#endif // DIALOG_PACECALC_H
