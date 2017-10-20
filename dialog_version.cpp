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

#include "dialog_version.h"
#include "ui_dialog_version.h"
#include "settings.h"

Dialog_version::Dialog_version(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_version)
{
    ui->setupUi(this);

    if(VERSION_BUILD == 0)
    {
        appVersion = QString::number(VERSION_MAJOR)+"."+QString::number(VERSION_MINOR);
    }
    else
    {
        if(DEV_BUILD == 1)
        {
            appVersion = QString::number(VERSION_MAJOR)+"."+QString::number(VERSION_MINOR)+"."+QString::number(VERSION_BUILD)+" DEV";
        }
        else
        {
            appVersion = QString::number(VERSION_MAJOR)+"."+QString::number(VERSION_MINOR)+"."+QString::number(VERSION_BUILD);
        }

    }

    ui->textBrowser_info->setText("<center>"
                                  "<h2>WorkoutEditor</h2>"
                                  "<h2>for GoldenCheetah</h2>"
                                  "<h3>Version "+appVersion+"</h3><br>"
                                  "Planing year and week workout schedule and export it to GC.<br>"
                                  "Forecast Trainingsload based on Stress History and planned Workouts.<br>"
                                  "Edit existing workouts out of GC.<br>"
                                  "<br>Developed by Andreas Hunner"
                                  "</center>"
                                  );
}

Dialog_version::~Dialog_version()
{
    delete ui;
}

