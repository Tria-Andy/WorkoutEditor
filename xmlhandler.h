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
    void fileToModel_single(QStandardItemModel*,QStringList*,QString);
    void fileToModel_multi(QStandardItemModel*,QStringList*,QStandardItemModel*,QStringList*);

private:

};

#endif // XMLHANDLER_H
