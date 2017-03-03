#ifndef XMLHANDLER_H
#define XMLHANDLER_H
#include <QtXml>
#include <QFile>
#include "settings.h"

class xmlHandler
{
public:
    xmlHandler();

protected:
    void check_File(QString,QString);
    QDomDocument load_XMLFile(QString,QString);
    void write_XMLFile(QString,QDomDocument*,QString);

private:

};

#endif // XMLHANDLER_H
