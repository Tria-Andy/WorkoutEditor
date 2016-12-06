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

#ifndef DIALOG_VERSION_H
#define DIALOG_VERSION_H

#include <QDialog>

namespace Ui {
class Dialog_version;
}

class Dialog_version : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_version(QWidget *parent = 0);
    ~Dialog_version();

private:
    Ui::Dialog_version *ui;
    QString appVersion;
};

#endif // DIALOG_VERSION_H
