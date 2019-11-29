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
    void xml_toTreeModel(QString,QStandardItemModel*);

    void read_treeModel(QStandardItemModel*,QString);
    void read_child(QDomDocument*, QDomElement*, QStandardItem *,QStandardItemModel*,QStringList*,int);

    void fill_xmlToList(QString,QMap<int,QStringList>*);
    void read_listMap(QMap<int,QStringList>*,QString);

    QHash<QString,QPair<QString,int>> rootTagMap;
    QHash<QString,QMap<int,QString>> xmlTagMap;
    QString schedulePath;

private:
    void xml_childToTreeModel(QDomElement,QStandardItem*);
    void add_child(QDomElement,QStandardItem*);
    QString timetoSec(QString time);
    void set_xmlTagMap(QString,QString,int);
    QDomDocument xmlDoc;
    int attributeCount;


};

#endif // XMLHANDLER_H
