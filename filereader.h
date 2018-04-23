#ifndef FILEREADER_H
#define FILEREADER_H

#include <QStandardItemModel>
#include "settings.h"
#include "jsonhandler.h"

class fileReader : public jsonHandler
{
public:
    fileReader();

protected:
    void readJsonFiles(QStandardItem *);

private:
    QList<QStandardItem *> readFileContent(QString,QString);
    QHash<QString,QString> actInfo;
};

#endif // FILEREADER_H
