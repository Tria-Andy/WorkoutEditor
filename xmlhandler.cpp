#include "xmlhandler.h"

xmlHandler::xmlHandler()
{

}
void xmlHandler::check_File(QString path,QString fileName)
{
    QFile workFile(path + QDir::separator() + fileName);
    if(!workFile.exists())
    {
        qDebug() << "File not exists! Created "+fileName;
        workFile.open(QIODevice::WriteOnly | QIODevice::Text);
        workFile.close();
    }
}

void xmlHandler::write_XMLFile(QString path,QDomDocument xmlDoc,QString fileName)
{
    QFile file(path + QDir::separator() + fileName);

    if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << fileName+": File not open!";
    }

    QTextStream stream(&file);
    stream << xmlDoc.toString();

    file.close();
}
