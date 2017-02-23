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

#ifndef DAY_POPUP_H
#define DAY_POPUP_H

#include <QtGui>
#include <QDialog>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <dialog_workouts.h>
#include "del_daypop.h"
#include "schedule.h"
#include "settings.h"
#include "calculation.h"

namespace Ui {
class day_popup;
}

class day_popup : public QDialog, public calculation
{
    Q_OBJECT

public:
    explicit day_popup(QWidget *parent = 0, const QDate w_date = QDate(), schedule *p_sched = 0);
    ~day_popup();

private slots:
    void on_toolButton_close_clicked();
    void on_toolButton_edit_clicked();
    void on_toolButton_editMove_clicked();
    void on_toolButton_copy_clicked();
    void on_toolButton_delete_clicked();
    void on_toolButton_stdwork_clicked();
    void load_workValues(int);
    void set_editValues(QDate);
    void setNextEditRow();
    void set_popupMode();
    void update_workValues();
    void on_tableView_day_clicked(const QModelIndex &index);
    void on_toolButton_dayEdit_clicked(bool checked);

private:
    Ui::day_popup *ui;
    schedule *workSched;
    del_daypop daypop_del;
    QStandardItemModel *dayModel;
    QSortFilterProxyModel *scheduleProxy;
    QHash<QString,QString> currWorkout;
    QDate popupDate,newDate;
    QModelIndex selIndex;
    QStringList workListHeader;
    int popupMode,selWorkout;

    void init_dayWorkouts(QDate);
    void set_controlButtons(bool);
    void set_dayData(bool);
    void set_result(QString,int);

};

#endif // DAY_POPUP_H
