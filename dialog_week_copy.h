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
#include "schedule.h"

namespace Ui {
class Dialog_week_copy;
}

class Dialog_week_copy : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_week_copy(QWidget *parent = 0,QString selectedWeek = 0, schedule *p_sched = 0);
    ~Dialog_week_copy();

private slots:
    void on_pushButton_cancel_clicked();
    void on_pushButton_ok_clicked();

    void on_radioButton_copy_clicked();

    void on_radioButton_save_clicked();

    void on_radioButton_clear_clicked();

private:
    Ui::Dialog_week_copy *ui;
    schedule *workSched;
    QStringList weekList;
    int editMode;
    void editWeek();
};

#endif // DIALOG_WEEK_COPY_H
