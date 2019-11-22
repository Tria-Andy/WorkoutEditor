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

#ifndef JSONHANDLER_H
#define JSONHANDLER_H
#include <QMap>
#include <QHash>
#include <QStandardItemModel>
#include "calculation.h"

class jsonHandler : public calculation
{
public:
    jsonHandler();

protected:
    QString readJsonContent(QString);
    void fill_qmap(QHash<QString,QString>*,QJsonObject*);
    void init_actModel(QString,QMap<int,QString> *,QStandardItemModel*,QStringList*,int);
    void init_xdataModel(QStandardItemModel*);
    void init_jsonFile();
    void write_actModel(QString,QStandardItemModel*,QStringList*);
    void write_xdataModel(QString,QStandardItemModel*);
    void write_jsonFile();
    bool hasXdata,hasOverride,hasPMData;
    QHash<QString,QString> rideData,tagData,overrideData;
    QStringList intList,sampList;
    QString fileName;
    QHash<QString,QString> *gcValues;

private:
    QJsonObject activityItem;
    QString jsonFile;
    QStringList xdataValues,xdataUnits;
    QHash<QString,QString> xData;

    void fill_keyList(QStringList*,QMap<int,QString>*,QStringList*);
    void fill_model(QStandardItemModel*,QJsonArray*,QStringList*);
    void fill_list(QJsonArray*,QStringList*);
    QJsonObject mapToJson(QHash<QString,QString>*);
    QJsonArray listToJson(QStringList*);
    QJsonArray modelToJson(QStandardItemModel*,QStringList*);
};

#endif // JSONHANDLER_H
