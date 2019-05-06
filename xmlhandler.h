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
    void fill_treeModel(QDomNodeList*, QStandardItemModel*,QMap<int,QStringList>*);
    void read_treeModel(QStandardItemModel*,QMap<int,QStringList>*);
    void fileToModel(QDomDocument,QString,QStandardItemModel*,QMap<int,QStringList>*);
    void readLevel();
private:
    QString schedulePath,workoutFile,scheduleFile,metaFile,contentFile,ltsFile;

};

#endif // XMLHANDLER_H
