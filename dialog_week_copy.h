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

#ifndef DIALOG_WEEK_COPY_H
#define DIALOG_WEEK_COPY_H

#include <QDialog>
#include <QMessageBox>
#include <QStandardItemModel>
#include "schedule.h"
#include "xmlhandler.h"
#include "calculation.h"
#include "settings.h"

namespace Ui {
class Dialog_week_copy;
}

class Dialog_week_copy : public QDialog, public xmlHandler, public calculation
{
    Q_OBJECT

public:
    explicit Dialog_week_copy(QWidget *parent = nullptr,QString selectedWeek = nullptr, schedule *p_sched = nullptr,bool hasWeek = false);
    ~Dialog_week_copy();

private slots:
    void on_pushButton_cancel_clicked();
    void on_pushButton_ok_clicked();
    void on_radioButton_copy_clicked();
    void on_radioButton_save_clicked();
    void on_radioButton_clear_clicked();
    void on_radioButton_load_clicked();
    void on_listView_weeks_clicked(const QModelIndex &index);
    void on_toolButton_addweek_clicked();
    void on_lineEdit_saveas_textChanged(const QString &arg1);
    void on_comboBox_select_currentIndexChanged(const QString &arg1);
    void on_toolButton_delete_clicked();

private:
    Ui::Dialog_week_copy *ui;
    schedule *workSched;
    QStandardItemModel *saveWeekModel,*listModel;
    QStringList weekTags;
    QString sourceWeek,saveweekFile;
    QModelIndex listIndex;
    bool isSaveWeek;
    int editMode;

    void readSaveweeks(QDomDocument);
    void write_weekList();
    void addWeek();
    void saveWeek();
};

#endif // DIALOG_WEEK_COPY_H
