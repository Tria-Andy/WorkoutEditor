#include "xmlhandler.h"
#include <QMessageBox>

xmlHandler::xmlHandler()
{
    schedulePath = settings::getStringMapPointer(settings::stingMap::GC)->value("schedule");
    attributeCount = 0;
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
    this->check_File(path,fileName);
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

QString xmlHandler::timetoSec(QString time)
{
    int sec = 0;
    if(time.length() == 8)
    {
        QTime durtime = QTime::fromString(time,"hh:mm:ss");
        sec = durtime.hour()*60*60;
        sec = sec + durtime.minute()*60;
        sec = sec + durtime.second();
    }
    if(time.length() == 5)
    {
        QTime durtime = QTime::fromString(time,"hh:mm");
        sec = durtime.hour()*60*60;
        sec = sec + durtime.minute()*60;
        sec = sec + durtime.second();
    }

    return QString::number(sec);
}

void xmlHandler::xml_toTreeModel(QString xmlFile, QStandardItemModel *model)
{
    QStandardItem *rootItem = model->invisibleRootItem();
    QDomElement element;
    QDomNamedNodeMap nodeMap;
    QStringList *tagList;

    QDomElement rootTag = this->load_XMLFile(schedulePath,xmlFile).documentElement();

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
            this->xml_childToTreeModel(element,pItem.at(0));
        }
    }
    rootTagMap.insert(xmlFile,qMakePair(rootTag.tagName(),attributeCount));
    model->setColumnCount(attributeCount);
    attributeCount = 0;
}

void xmlHandler::xml_childToTreeModel(QDomElement element, QStandardItem *item)
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

void xmlHandler::treeModel_toXml(QStandardItemModel *model, QString xmlFile)
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
            this->treeModelChild_toXml(&xmlDoc,&xmlElement,item,model);
        }
    }
    this->write_XMLFile(schedulePath,&xmlDoc,xmlFile);
}

void xmlHandler::treeModelChild_toXml(QDomDocument *xmlDoc, QDomElement *xmlElement, QStandardItem *parent, QStandardItemModel *model)
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
            this->treeModelChild_toXml(xmlDoc,&xmlChild,item,model);
        }
    }
}

void xmlHandler::xml_toListMap(QString xmlFile)
{
    QDomElement rootTag,childTag;
    QDomNodeList xmlList;
    QDomElement xmlElement;
    QStringList *tagList;
    QStringList listValues;

    rootTag = this->load_XMLFile(schedulePath,xmlFile).documentElement();

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
            listValues << xmlElement.attribute(tagList->at(tag));
        }
        mapList.insert(counter,listValues);
        listValues.clear();
    }
    rootTagMap.insert(xmlFile,qMakePair(rootTag.tagName(),attributeCount));
    attributeCount = 0;
}

void xmlHandler::listMap_toXml(QString xmlFile)
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement;
    QStringList *elementList = settings::get_xmlMapping(rootTagMap.value(xmlFile).first);
    QStringList *tagList = settings::get_xmlMapping(elementList->at(0));

    xmlRoot = xmlDoc.createElement(rootTagMap.value(xmlFile).first);
    xmlDoc.appendChild(xmlRoot);

    for(QMap<int, QStringList>::const_iterator it = mapList.cbegin(), end = mapList.cend(); it != end; ++it)
    {
        xmlElement = xmlDoc.createElement(elementList->at(0));
        for(int attr = 0; attr < tagList->count(); ++attr)
        {
            xmlElement.setAttribute(tagList->at(attr),it.value().at(attr));
        }
        xmlRoot.appendChild(xmlElement);
    }
    this->write_XMLFile(schedulePath,&xmlDoc,xmlFile);
}
