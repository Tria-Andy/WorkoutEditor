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
#include "schedule.h"
#include "settings.h"

namespace Ui {
class Dialog_settings;
}

class Dialog_settings : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_settings(QWidget *parent = 0,schedule *psched = 0);
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
    void on_dateEdit_saisonStart_dateChanged(const QDate &date);
    void on_pushButton_color_clicked();
    void on_dateEdit_saisonEnd_dateChanged(const QDate &date);
    void on_lineEdit_schedule_textChanged(const QString &arg1);
    void on_lineEdit_standard_textChanged(const QString &arg1);
    void on_lineEdit_athlete_textChanged(const QString &arg1);
    void on_lineEdit_yob_textChanged(const QString &arg1);
    void on_lineEdit_addedit_textChanged(const QString &arg1);
    void on_tableView_level_doubleClicked(const QModelIndex &index);
    void on_tableView_hf_doubleClicked(const QModelIndex &index);
    void on_listWidget_useIn_doubleClicked(const QModelIndex &index);
    void on_listWidget_useIn_itemChanged(QListWidgetItem *item);
    void on_lineEdit_gcpath_textChanged(const QString &arg1);
    void on_toolButton_gcPath_clicked();
    void on_doubleSpinBox_factor_valueChanged(double arg1);
    void on_spinBox_thresPower_valueChanged(int arg1);
    void on_timeEdit_thresPace_timeChanged(const QTime &time);
    void on_toolButton_schedulePath_clicked();
    void on_toolButton_workoutsPath_clicked();
    void on_spinBox_hfThres_valueChanged(int arg1);

    void on_spinBox_hfMax_valueChanged(int arg1);

private:
    Ui::Dialog_settings *ui;
    QStandardItemModel *level_model,*hf_model;
    schedule *schedule_ptr;
    QHash<QString,QStringList> listMap;
    QStringList keyList,extkeyList,sportList,model_header;
    del_level level_del;
    double thresPower,thresPace,sportFactor;
    QString getDirectory(QString);
    bool useColor;
    void checkSetup();
    void set_listEntries(QString);
    void set_color(QColor,bool,QString);
    void save_settingsChanges();
    void set_thresholdView(QString);
    void set_thresholdModel(QString);
    void set_hfmodel(double);
    void set_ltsList();
    void checkSportUse();
    void enableSavebutton();
    void updateListMap(int,bool);
    void writeChangedValues();
    void writeRangeValues(QString);
};


#endif // DIALOG_SETTINGS_H
