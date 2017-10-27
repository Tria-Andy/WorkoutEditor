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
    void fileToModel(QDomDocument,QString,QStandardItemModel*,QMap<int,QStringList>*);
    void readLevel();
private:

};

#endif // XMLHANDLER_H
