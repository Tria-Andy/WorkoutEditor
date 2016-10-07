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

private:
    Activity *curr_act;
    QString jsonFile,fileName;
    bool hasXdata;
    QStandardItemModel *int_model,*samp_model,*xdata_model;
    QStringList intList,sampList,xdataValues,xdataUnits;
    QMap<QString,QString> rideData;
    QMap<QString,QString> tagData;
    QMap<QString,QString> xData;

    void read_json(QString);
    void write_json();
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
