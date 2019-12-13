#include "datahandler.h"

datahandler::datahandler()
{

}

enum {SAISON,SCHEDULE,WORKOUTS};

QDomDocument datahandler::xmlDoc;
QString datahandler::filePath;
QHash<QString,QString>* datahandler::fileMap;
QHash<QString,QPair<QString,int>> datahandler::rootTagMap;
QStandardItemModel* datahandler::scheduleModel = new QStandardItemModel();
QStandardItemModel* datahandler::phaseModel = new QStandardItemModel();
QStandardItemModel* datahandler::stdWorkoutsModel = new QStandardItemModel();
QMap<int,QStringList> datahandler::mapList;
QMap<QDate,QVector<double>> datahandler::stressMap;
int datahandler::attributeCount = 0;

void datahandler::load_data()
{
    filePath = settings::getStringMapPointer(settings::stingMap::GC)->value("schedule");
    fileMap = settings::getStringMapPointer(settings::stingMap::File);

    xml_toTreeModel(fileMap->value("schedulefile"),scheduleModel);
    xml_toTreeModel(fileMap->value("saisonfile"),phaseModel);
    xml_toTreeModel(fileMap->value("standardworkoutfile"),stdWorkoutsModel);
    xml_toListMap(fileMap->value("stressfile"),&mapList);
}


void datahandler::save_data(int saveData)
{
    if(saveData == SAISON)
    {
        treeModel_toXml(phaseModel,fileMap->value("saisonfile"));
    }
    else if(saveData == SCHEDULE)
    {
        treeModel_toXml(scheduleModel,fileMap->value("schedulefile"));
    }
    else if(saveData == WORKOUTS)
    {
        treeModel_toXml(stdWorkoutsModel,fileMap->value("standardworkoutfile"));
    }
}

void datahandler::check_File(QString path,QString fileName)
{
    QFile workFile(path + QDir::separator() + fileName);
    if(!workFile.exists())
    {
        qDebug() << "File not exists! Created:"+fileName;
        workFile.open(QIODevice::WriteOnly | QIODevice::Text);
        workFile.close();
    }
}

QDomDocument datahandler::load_XMLFile(QString path,QString fileName)
{
    check_File(path,fileName);
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
    xmlDoc = xmldoc;
    return xmldoc;
}

void datahandler::write_XMLFile(QString path,QDomDocument *xmlDoc,QString fileName)
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

void datahandler::xml_toTreeModel(QString xmlFile, QStandardItemModel *model)
{
    QStandardItem *rootItem = model->invisibleRootItem();
    QDomElement element;
    QDomNamedNodeMap nodeMap;
    QStringList *tagList;

    QDomElement rootTag = load_XMLFile(filePath,xmlFile).documentElement();

    QDomNodeList childNodes = rootTag.childNodes();

    for(int parent = 0; parent < childNodes.count(); ++parent)
    {
        QList<QStandardItem*> pItem;
        element = childNodes.at(parent).toElement();
        nodeMap = element.attributes();
        tagList = settings::get_xmlMapping(element.tagName());

        if(attributeCount < nodeMap.count()) attributeCount = nodeMap.count();

        for(int nodeAtt = 0; nodeAtt < nodeMap.count(); ++nodeAtt)
        {
            pItem.insert(nodeAtt,new QStandardItem(element.attribute(tagList->at(nodeAtt))));
        }
        pItem.at(0)->setData(element.tagName(),Qt::AccessibleTextRole);
        rootItem->appendRow(pItem);

        if(element.hasChildNodes())
        {
            xml_childToTreeModel(element,pItem.at(0));
        }
    }
    rootTagMap.insert(xmlFile,qMakePair(rootTag.tagName(),attributeCount));
    model->setColumnCount(attributeCount);
    attributeCount = 0;
}

void datahandler::xml_childToTreeModel(QDomElement element, QStandardItem *item)
{
    QDomElement child;
    QDomNodeList nodeList = element.childNodes();
    QDomNamedNodeMap nodeMap;
    QStringList *tagList;

    for(int node = 0; node < nodeList.count(); ++node)
    {
        QList<QStandardItem*> cItem;
        child = nodeList.at(node).toElement();
        nodeMap = child.attributes();

        if(attributeCount < nodeMap.count()) attributeCount = nodeMap.count();

        tagList = settings::get_xmlMapping(child.tagName());

        for(int nodeAtt = 0; nodeAtt < nodeMap.count(); ++nodeAtt)
        {
            cItem.insert(nodeAtt,new QStandardItem(child.attribute(tagList->at(nodeAtt))));
        }
        cItem.at(0)->setData(child.tagName(),Qt::AccessibleTextRole);
        item->appendRow(cItem);

        if(child.hasChildNodes())
        {
            xml_childToTreeModel(child,cItem.at(0));
        }
    }
}

void datahandler::treeModel_toXml(QStandardItemModel *model, QString xmlFile)
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement;
    QStringList *tagList;
    QStandardItem *item;

    xmlRoot = xmlDoc.createElement(rootTagMap.value(xmlFile).first);
    xmlDoc.appendChild(xmlRoot);

    for(int row = 0; row < model->rowCount(); ++row)
    {
        item = model->item(row);
        tagList = settings::get_xmlMapping(item->data(Qt::AccessibleTextRole).toString());
        xmlElement = xmlDoc.createElement(item->data(Qt::AccessibleTextRole).toString());

        for(int att = 0; att < tagList->count();++att)
        {
            xmlElement.setAttribute(tagList->at(att),model->data(model->index(row,att)).toString());
        }
        xmlRoot.appendChild(xmlElement);

        if(item->hasChildren())
        {
            treeModelChild_toXml(&xmlDoc,&xmlElement,item,model);
        }
    }
    write_XMLFile(filePath,&xmlDoc,xmlFile);
}

void datahandler::treeModelChild_toXml(QDomDocument *xmlDoc, QDomElement *xmlElement, QStandardItem *parent, QStandardItemModel *model)
{
    QStringList *tagList;
    QDomElement xmlChild;
    QStandardItem *item;
    QModelIndex index = model->indexFromItem(parent);
    QModelIndex childIndex;

    for(int row = 0; row < parent->rowCount(); ++row)
    {
        item = model->itemFromIndex(model->index(row,0,index));
        tagList = settings::get_xmlMapping(item->data(Qt::AccessibleTextRole).toString());
        childIndex = model->indexFromItem(item);
        xmlChild = xmlDoc->createElement(item->data(Qt::AccessibleTextRole).toString());

        for(int att = 0; att < tagList->count(); ++att)
        {
            xmlChild.setAttribute(tagList->at(att),model->data(childIndex.siblingAtColumn(att)).toString());
        }
        xmlElement->appendChild(xmlChild);

        if(item->hasChildren())
        {
            treeModelChild_toXml(xmlDoc,&xmlChild,item,model);
        }
    }
}

void datahandler::xml_toListMap(QString xmlFile,QMap<int,QStringList> *list)
{
    QDomElement rootTag,childTag;
    QDomNodeList xmlList;
    QDomElement xmlElement;
    QStringList *tagList;
    QStringList mapList;

    rootTag = load_XMLFile(filePath,xmlFile).documentElement();

    rootTag = xmlDoc.firstChildElement();
    childTag = rootTag.firstChild().toElement();
    xmlList = rootTag.elementsByTagName(childTag.tagName());
    tagList = settings::get_xmlMapping(childTag.tagName());

    for(int counter = 0; counter < xmlList.count(); ++counter)
    {
        xmlElement = xmlList.at(counter).toElement();

        if(attributeCount < tagList->count()) attributeCount = tagList->count();

        for(int tag = 0; tag < tagList->count(); ++tag)
        {
            mapList << xmlElement.attribute(tagList->at(tag));
        }
        list->insert(counter,mapList);
        mapList.clear();
    }
    rootTagMap.insert(xmlFile,qMakePair(rootTag.tagName(),attributeCount));
}

void datahandler::listMap_toXml(QMap<int, QStringList> *mapList,QString xmlFile)
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement;
    QStringList *elementList = settings::get_xmlMapping(rootTagMap.value(xmlFile).first);
    QStringList *tagList = settings::get_xmlMapping(elementList->at(0));

    xmlRoot = xmlDoc.createElement(rootTagMap.value(xmlFile).first);
    xmlDoc.appendChild(xmlRoot);

    for(QMap<int, QStringList>::const_iterator it = mapList->cbegin(), end = mapList->cend(); it != end; ++it)
    {
        xmlElement = xmlDoc.createElement(elementList->at(0));
        for(int attr = 0; attr < tagList->count(); ++attr)
        {
            xmlElement.setAttribute(tagList->at(attr),it.value().at(attr));
        }
        xmlRoot.appendChild(xmlElement);
    }
    write_XMLFile(filePath,&xmlDoc,xmlFile);
}
