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

#ifndef DIALOG_WORKOUTS_H
#define DIALOG_WORKOUTS_H

#include <QDialog>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include "standardworkouts.h"

namespace Ui {
class Dialog_workouts;
}

class Dialog_workouts : public QDialog, public standardWorkouts
{
    Q_OBJECT

public:
    explicit Dialog_workouts(QWidget *parent = 0, QString w_sport = QString());
    ~Dialog_workouts();

    QHash<int,QString> workData;
    //Getter

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_close_clicked();
    void on_listView_workouts_clicked(const QModelIndex &index);

private:
    Ui::Dialog_workouts *ui;
    QStandardItemModel *listModel;
    QSortFilterProxyModel *metaProxy;
    QString workoutSport;
    void create_workout_list();
};

#endif // DIALOG_WORKOUTS_H
