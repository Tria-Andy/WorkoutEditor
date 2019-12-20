#ifndef XMLHANDLER_H
#define XMLHANDLER_H
#include <QtXml>
#include <QFile>
#include <QStandardItemModel>
#include "settings.h"

class xmlHandler
{
public:
    xmlHandler();

protected:
    void check_File(QString,QString);
    void write_XMLFile(QString,QDomDocument*,QString);

    void xml_toTreeModel(QString,QStandardItemModel*);
    void treeModel_toXml(QStandardItemModel*,QString);

    void xml_toListMap(QString);
    void listMap_toXml(QString);

    QHash<QString,QPair<QString,int>> rootTagMap;
    QMap<int,QStringList> mapList;
    QString schedulePath;

private:
    QDomDocument xmlDoc;
    int attributeCount;
    QDomDocument load_XMLFile(QString,QString);

    void xml_childToTreeModel(QDomElement,QStandardItem*);
    void treeModelChild_toXml(QDomDocument*, QDomElement*, QStandardItem *,QStandardItemModel*);

    QString timetoSec(QString time);
};

#endif // XMLHANDLER_H
