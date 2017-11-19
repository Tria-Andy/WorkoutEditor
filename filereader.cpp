#include "filereader.h"
#include "settings.h"

fileReader::fileReader()
{

}

void fileReader::readJsonFiles(QStandardItem *rootItem)
{
    QFile file;
    QString filePath;
    int jsonMaxFiles = generalValues->value("filecount").toInt();
    QDir directory(gcValues->value("actpath"));
    directory.setSorting(QDir::Name | QDir::Reversed);
    directory.setFilter(QDir::Files);
    QFileInfoList fileList = directory.entryInfoList();
    int fileCount = fileList.count() > jsonMaxFiles ? jsonMaxFiles : fileList.count();

    for(int i = 0; i < fileCount; ++i)
    {
        filePath = fileList.at(i).path()+QDir::separator()+fileList.at(i).fileName();
        file.setFileName(filePath);
        file.open(QFile::ReadOnly | QFile::Text);
        rootItem->appendRow(this->readFileContent(file.readAll(),filePath));
        file.close();
    }
}

QList<QStandardItem *> fileReader::readFileContent(QString jsonfile,QString filePath)
{
    QList<QStandardItem *> listItems;
    QStringList valueList;

    valueList = settings::get_listValues("JsonFile");
    QDateTime workDateTime;
    workDateTime.setTimeSpec(Qt::UTC);

    QDateTime localTime(QDateTime::currentDateTime());
    localTime.setTimeSpec(Qt::LocalTime);

    QJsonObject rideObject,tagObject;
    QJsonDocument d = QJsonDocument::fromJson(jsonfile.toUtf8());
    QJsonObject jsonobj = d.object();

    rideObject = jsonobj.value(QString("RIDE")).toObject();
    this->fill_qmap(&actInfo,&rideObject);

    tagObject = rideObject.value(QString("TAGS")).toObject();
    this->fill_qmap(&actInfo,&tagObject);

    workDateTime = QDateTime::fromString(actInfo.value("STARTTIME"),"yyyy/MM/dd hh:mm:ss UTC").addSecs(localTime.offsetFromUtc());

    listItems << new QStandardItem(QDate().shortDayName(workDateTime.date().dayOfWeek()));
    listItems << new QStandardItem(workDateTime.toString("dd.MM.yyyy hh:mm:ss"));
    listItems << new QStandardItem(actInfo.value("Sport"));
    listItems << new QStandardItem(actInfo.value("Workout Code"));
    listItems << new QStandardItem(filePath);

    return listItems;
}
