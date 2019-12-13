#ifndef DATAHANDLER_H
#define DATAHANDLER_H
#include <QtXml>
#include <QFile>
#include <QStandardItemModel>
#include "settings.h"

class datahandler
{
public:
    datahandler();
    static void load_data();
    static QStandardItemModel *scheduleModel,*phaseModel,*stdWorkoutsModel;
    static QMap<QDate,QVector<double>> stressMap;

protected:
    void save_data(int);
    static QHash<QString,QString> *fileMap;
    static QMap<int,QStringList> mapList;
    static void xml_toTreeModel(QString,QStandardItemModel*);
    static void treeModel_toXml(QStandardItemModel*,QString);

    static void xml_toListMap(QString,QMap<int,QStringList>*);
    static void listMap_toXml(QMap<int,QStringList>*,QString);

private:
    static QDomDocument xmlDoc;
    static QString filePath;
    static QHash<QString,QPair<QString,int>> rootTagMap;
    static int attributeCount;

    static QDomDocument load_XMLFile(QString,QString);

    static void check_File(QString,QString);
    static void write_XMLFile(QString,QDomDocument*,QString);

    static void xml_childToTreeModel(QDomElement,QStandardItem*);
    static void treeModelChild_toXml(QDomDocument*, QDomElement*, QStandardItem *,QStandardItemModel*);

};

#endif // DATAHANDLER_H
