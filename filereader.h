#ifndef FILEREADER_H
#define FILEREADER_H

#include <QStandardItemModel>
#include "settings.h"
#include "jsonhandler.h"

class fileReader : public jsonHandler
{
public:
    fileReader(QStandardItemModel *fModel,QStandardItem *);

private:
    QStandardItemModel *fileModel;
    void readJsonFiles(QStandardItem *);
    QList<QStandardItem *> readFileContent(QString,QString);
    QHash<QString,QString> actInfo;
};

#endif // FILEREADER_H
