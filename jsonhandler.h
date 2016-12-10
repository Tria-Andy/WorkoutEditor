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
#include "activity.h"
#include <QMap>
#include <QStandardItemModel>

class Activity;
class jsonHandler
{
public:
    jsonHandler(bool readFlag = false,QString jsonfile = QString(), Activity *p_act = 0);
    QString get_jsonfile() {return jsonFile;}
    void set_filename(QString fname) {fileName = fname;}
    void write_json();
    void set_overrideData(QString vKey,QString vValue)
    {
        overrideData.insert(vKey,vValue);
    }
    QString get_overrideDate(QString vKey)
    {
        return overrideData.value(vKey);
    }
    void set_overrideFlag(bool oFlag)
    {
        hasOverride = oFlag;
    }
    void set_rideData(QString vKey,QString vValue)
    {
        rideData.insert(vKey,vValue);
    }
    QString get_tagData(QString vKey)
    {
        return tagData.value(vKey);
    }
    void set_tagData(QString vKey,QString vValue)
    {
        tagData.insert(vKey,vValue);
    }
    void reset_maps()
    {
        rideData.clear();
        tagData.clear();
        overrideData.clear();
    }

private:
    Activity *curr_act;
    QString jsonFile,fileName;
    bool hasXdata,hasOverride,hasFile;
    QStandardItemModel *p_int,*p_samp;
    QStringList intList,sampList,xdataValues,xdataUnits;
    QMap<QString,QString> rideData;
    QMap<QString,QString> tagData;
    QMap<QString,QString> xData;
    QMap<QString,QString> overrideData;

    void read_json(QString);   
    void write_file(QJsonDocument);

    void fill_qmap(QMap<QString,QString>*,QJsonObject*);
    void fill_keyList(QStringList*,QMap<int,QString>*,QStringList*);
    void fill_model(QStandardItemModel*,QJsonArray*,QStringList*);
    void fill_list(QJsonArray*,QStringList*);
    QJsonObject mapToJson(QMap<QString,QString>*);
    QJsonArray listToJson(QStringList*);
    QJsonArray modelToJson(QStandardItemModel*,QStringList*);
};

#endif // JSONHANDLER_H
