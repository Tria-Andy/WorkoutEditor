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

#ifndef DIALOG_EXPORT_H
#define DIALOG_EXPORT_H

#include <QDialog>
#include <QStandardItemModel>
#include <QFile>
#include "jsonhandler.h"
#include "schedule.h"
#include "settings.h"
#include "calculation.h"

namespace Ui {
class Dialog_export;
}

class Dialog_export : public QDialog, public jsonHandler
{
    Q_OBJECT

public:
    explicit Dialog_export(QWidget *parent = nullptr,schedule *p_schedule = nullptr);
    ~Dialog_export();

private slots:
    void on_pushButton_close_clicked();
    void on_pushButton_export_clicked();
    void on_radioButton_day_clicked();
    void on_radioButton_week_clicked();
    void on_comboBox_week_export_currentIndexChanged(const QString &arg1);
    void on_dateEdit_export_dateChanged(const QDate &date);
    void on_comboBox_time_export_currentIndexChanged(const QString &time);

private:
    Ui::Dialog_export *ui;
    QSortFilterProxyModel *exportProxy;
    QString fileContent;
    int exportMode;

    void workout_export();
    void set_filecontent(int);
    void set_exportselection(bool,bool);
    void set_infolabel(int);
    void get_exportinfo(QString,int);
    void set_comboBox_time();
};

#endif // DIALOG_EXPORT_H
