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
#include "calculation.h"

namespace Ui {
class Dialog_addweek;
}

class Dialog_addweek : public QDialog, public calculation
{
    Q_OBJECT

public:
    explicit Dialog_addweek(QWidget *parent = 0,QString sel_week = QString(), schedule *p_sched = 0);
    ~Dialog_addweek();

private slots:
    void on_dateEdit_selectDate_dateChanged(const QDate &date);
    void on_toolButton_update_clicked();
    void on_toolButton_close_clicked();

private:
    Ui::Dialog_addweek *ui;
    schedule *workSched;
    del_addweek week_del;
    QStandardItemModel *weekModel;
    QSortFilterProxyModel *metaProxy,*contentProxy;
    QString openID,selYear,weekID,timeFormat,empty;
    QStringList sportuseList,weekHeader,weekMeta,weekContent;
    bool update;

    void fill_values(QString);
    QStringList create_values();
    void store_values();
};

#endif // DIALOG_ADDWEEK_H
