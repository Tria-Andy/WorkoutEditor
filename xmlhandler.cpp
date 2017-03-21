#include "xmlhandler.h"
#include <QMessageBox>

xmlHandler::xmlHandler()
{

}
void xmlHandler::check_File(QString path,QString fileName)
{
    QFile workFile(path + QDir::separator() + fileName);
    if(!workFile.exists())
    {
        qDebug() << "File not exists! Created:"+fileName;
        workFile.open(QIODevice::WriteOnly | QIODevice::Text);
        workFile.close();
    }
}

QDomDocument xmlHandler::load_XMLFile(QString path,QString fileName)
{
    QFile xmlFile(path + QDir::separator() + fileName);
    QDomDocument xmldoc = QDomDocument();

    if(!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File not open:"+fileName;
    }
    else
    {
        if(!xmldoc.setContent(&xmlFile))
        {
            qDebug() << "File content not loaded:"+fileName;
        }
        xmlFile.close();
    }
    return xmldoc;
}

void xmlHandler::write_XMLFile(QString path,QDomDocument *xmlDoc,QString fileName)
{
    QMessageBox infoBox;
    infoBox.setText("Save XML File");
    infoBox.addButton(QMessageBox::Ok);
    infoBox.exec();

    QFile xmlFile(path + QDir::separator() + fileName);

    if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << fileName+": File not open:"+fileName;
    }

    QTextStream stream(&xmlFile);
    stream << xmlDoc->toString();

    xmlFile.close();
}
