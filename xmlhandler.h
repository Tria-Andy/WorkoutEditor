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
    QDomDocument load_XMLFile(QString,QString);
    void write_XMLFile(QString,QDomDocument*,QString);
    void fill_treeModel(QString,QStandardItemModel*);
    void fill_xmlToList(QDomDocument,QMap<int,QStringList>*);
    void read_listMap(QMap<int,QStringList>*,QString);
    void add_child(QDomElement,QStandardItem*);
    void read_treeModel(QStandardItemModel*,QString);
    void read_child(QDomDocument*, QDomElement*, QStandardItem *,QStandardItemModel*,QStringList*,int);
    QHash<QString,QString> rootTagMap;
    QString schedulePath,metaFile;

private:
    QString timetoSec(QString time);
    QDomDocument xmlDoc;

};

#endif // XMLHANDLER_H
