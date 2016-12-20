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

#ifndef DIALOG_ADDWEEK_H
#define DIALOG_ADDWEEK_H

#include <QDialog>
#include "schedule.h"
#include "del_addweek.h"
#include "settings.h"

namespace Ui {
class Dialog_addweek;
}

class Dialog_addweek : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_addweek(QWidget *parent = 0,QString sel_week = QString(), schedule *p_sched = 0);
    ~Dialog_addweek();

private slots:
    void on_pushButton_cancel_clicked();    
    void on_pushButton_ok_clicked();
    void on_dateEdit_selectDate_dateChanged(const QDate &date);

private:
    Ui::Dialog_addweek *ui;
    schedule *workSched;
    del_addweek week_del;
    QStandardItemModel *weekModel;
    QHash<QString,QString> sportValues;
    QString openID,selYear,weekID,timeFormat,empty;
    QStringList sportuseList,weekHeader,weekMeta,weekContent;
    bool update;

    double calc_percent(int, int);
    int sum_int(QStandardItemModel *,QStringList *, int);
    double sum_double(QStandardItemModel *,QStringList *, int);
    QTime sum_time(QStandardItemModel *,QStringList *, int);

    void fill_values(QString);
    QStringList create_values();
    void store_values();
};

#endif // DIALOG_ADDWEEK_H
