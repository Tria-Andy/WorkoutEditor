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
    void fill_treeModel(QDomNodeList*, QStandardItemModel*);
    void add_child(QDomElement,QStandardItem*);
    void read_treeModel(QStandardItemModel*,QString,QString);
    void read_child(QDomDocument*, QDomElement*, QStandardItem *,QStandardItemModel*,QStringList*,int);

private:
    QString schedulePath,workoutFile,scheduleFile,metaFile,contentFile,ltsFile;
    QString timetoSec(QString time);
};

#endif // XMLHANDLER_H
