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
#include <QTimeEdit>
#include <QTextBrowser>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QPushButton>
#include "dialog_edit.h"
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

private:
    Ui::day_popup *ui;
    schedule *workSched;
    const QDate *workout_date;
    QString weekPhase;
    void show_workouts();

};

#endif // DAY_POPUP_H
