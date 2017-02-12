#ifndef FILEREADER_H
#define FILEREADER_H

#include <QStandardItemModel>
#include "settings.h"
#include "jsonhandler.h"

class fileReader : public jsonHandler
{
public:
    fileReader(QStandardItemModel *fModel);

private:
    QStandardItemModel *fileModel;
    void readJsonFiles();
    QList<QStandardItem *> readFileContent(QString,QString);

    QHash<QString,QString> actInfo;
};

#endif // FILEREADER_H
