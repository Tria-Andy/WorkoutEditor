#ifndef JSONHANDLER_H
#define JSONHANDLER_H
#include "activity.h"
#include <QMap>
#include <QStandardItemModel>

class jsonHandler
{
public:
    jsonHandler(QString filename = QString(),QString jsonfile = QString(), Activity *p_act = 0);
    QString get_jsonfile() {return jsonFile;}
    void write_json();
    void set_overrideData(QString vKey,QString vValue)
    {
        overrideData.insert(vKey,vValue);
    }
    void set_overrideFlag(bool oFlag)
    {
        hasOverride = oFlag;
    }

private:
    Activity *curr_act;
    QString jsonFile,fileName;
    bool hasXdata,hasOverride;
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
