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
    QFile xmlFile(path + QDir::separator() + fileName);

    if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << fileName+": File not open:"+fileName;
    }

    QTextStream stream(&xmlFile);
    stream << xmlDoc->toString();

    xmlFile.close();
}
    /*
void xmlHandler::fileToModel(QDomDocument xmldoc,QString rootTag,QStandardItemModel *model, QMap<int,QStringList> *tags)
{

    QDomNodeList listLevel,listChild;
    QDomElement elementLevel,childLevel;
    QMap<int,QDomNodeList> nodeList;
    int level = 0;
    listLevel = xmldoc.firstChildElement().elementsByTagName(rootTag);

    for(int baseLevel = 0; baseLevel < listLevel.count(); ++baseLevel)
    {
        elementLevel = listLevel

    }


    for(int level0 = 0; level0 < listLevel0.count(); ++level0)
    {
        elementLevel0 = listLevel0.at(level0).toElement();


        model->insertRow(level0,QModelIndex());
        for(int col = 0; col < tags->value(0).count();++col)
        {
            //model->setData(model->index(level0,col),elementLevel0.attribute(tags->value(level).at(col)));
            qDebug() << elementLevel0.attribute(tags->value(0).at(col));
        }
        if(elementLevel0.hasChildNodes())
        {
            QDomNodeList listLevel1 = elementLevel0.childNodes();
            QDomElement elementLevel1;

            for(int level1 = 0; level1 < listLevel1.count(); ++level1)
            {
                elementLevel1 = listLevel1.at(level1).toElement();
                for(int col = 0; col < tags->value(1).count(); ++col)
                {
                    qDebug() << elementLevel1.attribute(tags->value(1).at(col));
                }
                if(elementLevel1.hasChildNodes())
                {
                    QDomNodeList listLevel2 = elementLevel1.childNodes();
                    QDomElement elementLevel2;

                }
            }

        }
        contestCount = childList.count();
        contestModel->insertRows(contestModel->rowCount(),contestCount);

        for(int contest = 0; contest < contestCount; ++contest,++rowCount)
        {
            childElement = childList.at(contest).toElement();
            for(int col = 0; col < contest_tags.count(); ++col)
            {
                contestModel->setData(contestModel->index(rowCount,col),childElement.attribute(contest_tags.at(col)));
            }
        }
    }


}
*/
void xmlHandler::readLevel()
{

}
