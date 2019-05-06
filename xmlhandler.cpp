#include "xmlhandler.h"
#include <QMessageBox>

xmlHandler::xmlHandler()
{
    schedulePath = settings::getStringMapPointer(settings::stingMap::GC)->value("schedule");
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
    const int IndentSize = 2;

    QFile xmlFile(path + QDir::separator() + fileName);

    if(!xmlFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << fileName+": File not open:"+fileName;
    }

    QTextStream outXML(&xmlFile);
    xmlDoc->save(outXML,IndentSize);
    xmlFile.close();
}

void xmlHandler::fill_treeModel(QDomNodeList *pList, QStandardItemModel *model, QMap<int, QStringList> *tagList)
{
    QStandardItem *rootItem = model->invisibleRootItem();
    QDomElement pElement,cElement;
    QDomNodeList cList;

    for(int parent = 0; parent < pList->count(); ++parent)
    {
        QList<QStandardItem*> pItem;
        pElement = pList->at(parent).toElement();

        for(int pTag = 0; pTag < tagList->value(0).count(); ++pTag)
        {
            pItem << new QStandardItem(pElement.attribute(tagList->value(0).at(pTag)));
        }
        rootItem->appendRow(pItem);

        if(pElement.hasChildNodes())
        {
            cList = pElement.childNodes();
            for(int child = 0; child < cList.count(); ++child)
            {
                QList<QStandardItem*> cItem;
                cElement = cList.at(child).toElement();

                for(int cTag = 0; cTag < tagList->value(1).count(); ++cTag)
                {
                    cItem << new QStandardItem(cElement.attribute(tagList->value(1).at(cTag)));
                }
                pItem.at(0)->appendRow(cItem);
            }
        }
    }
}

void xmlHandler::read_treeModel(QStandardItemModel *model, QMap<int, QStringList> *tagList)
{
    //QModelIndex index;
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement,childElement;

    QMap<QDate,QString> scheduleDates;
    QDate readDate;
    QString weekID;
    for(int i = 0; i < model->rowCount(); ++i)
    {
        readDate = QDate::fromString(model->data(model->index(i,1)).toString(),"dd.MM.yyyy");
        weekID = model->data(model->index(i,0)).toString();

        if(!scheduleDates.contains(readDate)) scheduleDates.insert(readDate,weekID);
    }
    /*
    for(QMap<QDate,QString>::const_iterator it = scheduleDates.cbegin(), end = scheduleDates.cend(); it != end; ++it)
    {
       this->filter_schedule(it.key().toString("dd.MM.yyyy"),1,true);


       for(int x = 0; x < scheduleProxy->rowCount(); ++x)
       {
           childElement = xmlDoc.createElement("workout");
           childElement.setAttribute("id",QString::number(x));

           for(int tags = 2; tags < workoutTags.count(); ++tags)
           {
               childElement.setAttribute(workoutTags.at(tags),scheduleProxy->data(scheduleProxy->index(x,tags)).toString());
           }
           xmlElement.appendChild(childElement);
       }
       xmlRoot.appendChild(xmlElement);
       xmlElement.clear();
    }
    this->write_XMLFile(schedulePath,&xmlDoc,scheduleFile);
    xmlDoc.clear();
    */
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
