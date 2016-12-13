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

#ifndef DIALOG_SETTINGS_H
#define DIALOG_SETTINGS_H

#include <QtCore>
#include <QDialog>
#include <QListWidget>
#include <QStandardItemModel>
#include "del_level.h"
#include "settings.h"

namespace Ui {
class Dialog_settings;
}

class Dialog_settings : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_settings(QWidget *parent = 0);
    ~Dialog_settings();

private slots:
    void on_pushButton_cancel_clicked();
    void on_comboBox_selInfo_currentTextChanged(const QString &arg1);
    void on_listWidget_selection_itemDoubleClicked(QListWidgetItem *item);
    void on_pushButton_up_clicked();
    void on_pushButton_down_clicked();
    void on_pushButton_add_clicked();
    void on_pushButton_delete_clicked();
    void on_pushButton_edit_clicked();
    void on_comboBox_thresSport_currentTextChanged(const QString &arg1);
    void on_pushButton_save_clicked();
    void on_lineEdit_thresPace_textEdited(const QString &arg1);
    void on_lineEdit_thresPower_textEdited(const QString &arg1);
    void on_lineEdit_hfThres_textEdited(const QString &arg1);
    void on_lineEdit_hfmax_textEdited(const QString &arg1);
    void on_dateEdit_saisonStart_dateChanged(const QDate &date);
    void on_pushButton_color_clicked();
    void on_dateEdit_saisonEnd_dateChanged(const QDate &date);
    void on_pushButton_schedulePath_clicked();
    void on_pushButton_workoutsPath_clicked();
    void on_lineEdit_schedule_textChanged(const QString &arg1);
    void on_lineEdit_standard_textChanged(const QString &arg1);
    void on_lineEdit_athlete_textChanged(const QString &arg1);
    void on_lineEdit_yob_textChanged(const QString &arg1);
    void on_lineEdit_addedit_textChanged(const QString &arg1);
    void on_tableView_level_doubleClicked(const QModelIndex &index);
    void on_tableView_hf_doubleClicked(const QModelIndex &index);

private:
    Ui::Dialog_settings *ui;
    QStandardItemModel *level_model,*hf_model;
    QStringList sportList,model_header;
    del_level level_del;
    double thresPower,thresPace,sportFactor;
    bool useColor;
    void checkSetup();
    void set_listEntries(QString);
    void set_color(QColor,bool,QString);
    void save_settingsChanges();
    void set_thresholdView(QString);
    void set_thresholdModel(QString);
    void set_hfmodel();
    void enableSavebutton();
    void writeChangedValues();
    void writeRangeValues(QString);
};

#endif // DIALOG_SETTINGS_H
