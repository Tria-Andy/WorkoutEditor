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
#include <QSet>
#include <QStandardItemModel>
#include <QJsonObject>

class jsonHandler
{
public:
    jsonHandler();

protected:
    QSet<QString> activityKeys;
    QMap<int,QVector<double>> xDataValues;
    QMap<int,QVariantMap> xDataMap;
    QMap<int,QPair<int,int>> intervallMap;
    QMap<int,QString> intNameMap;
    QMap<int,QVector<double>> sampleMap;

    bool hasXdata,hasOverride,powerFlag;
    QHash<QString,QString> rideData,tagData,overrideData;
    QHash<QString,QHash<QString,QVariant>> activityData;

    QStringList *xdataHeader,*xValuesKeys;
    QStringList sampleUseKeys;
    QHash<QString,QString> *gcValues,*generalValues;

    QString read_jsonContent(QString,bool);
    QVector<QString> read_activityMeta(QString,int);
    void prepareWrite_JsonFile();


private:
    void check_keyList(QStringList*,QStringList*,QStringList*);
    void fill_activityData(QString,QJsonObject*);
    void fill_qmap(QHash<QString,QString>*,QJsonObject*);
    void fill_list(QJsonArray*,QStringList*);
    QJsonObject mapToJson(QHash<QString,QString>*);
};

#endif // JSONHANDLER_H
