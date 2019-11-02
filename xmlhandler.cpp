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

void xmlHandler::fill_treeModel(QDomNodeList *pList, QStandardItemModel *model)
{
    QStandardItem *rootItem = model->invisibleRootItem();
    QDomElement pElement,cElement;
    QDomNodeList cList;
    QDomNamedNodeMap nodeMap;
    QStringList *tagList;

    for(int parent = 0; parent < pList->count(); ++parent)
    {
        QList<QStandardItem*> pItem;
        pElement = pList->at(parent).toElement();
        nodeMap = pElement.attributes();

        tagList = settings::getHeaderMap(pElement.tagName());

        for(int node = 0; node < nodeMap.count(); ++node)
        {
            pItem << new QStandardItem(pElement.attribute(tagList->at(node)));
        }

        rootItem->appendRow(pItem);

        if(pElement.hasChildNodes())
        {
            this->add_child(pElement,pItem.at(0));
        }
    }
}

void xmlHandler::add_child(QDomElement element, QStandardItem *item)
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
        tagList = settings::getHeaderMap(child.tagName());
        for(int nodeAtt = 0; nodeAtt < nodeMap.count(); ++nodeAtt)
        {
            cItem << new QStandardItem(child.attribute(tagList->at(nodeAtt)));
        }

        item->appendRow(cItem);
        if(child.hasChildNodes())
        {
            add_child(child,cItem.at(0));
        }
    }
}

void xmlHandler::read_treeModel(QStandardItemModel *model,QString rootTag, QString xmlFile)
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement;
    QStringList *elementList = settings::getHeaderMap(rootTag);
    QStringList *tagList = settings::getHeaderMap(elementList->at(0));
    QStandardItem *item;
    xmlRoot = xmlDoc.createElement(rootTag);
    xmlDoc.appendChild(xmlRoot);

    for(int row = 0; row < model->rowCount(); ++row)
    {
        item = model->item(row);
        xmlElement = xmlDoc.createElement(elementList->at(0));
        for(int att = 0; att < tagList->count();++att)
        {
            xmlElement.setAttribute(tagList->at(att),model->data(model->index(row,att)).toString());
        }
        xmlRoot.appendChild(xmlElement);

        if(item->hasChildren())
        {
            this->read_child(&xmlDoc,&xmlElement,item,model,elementList,1);
        }
    }
    this->write_XMLFile(schedulePath,&xmlDoc,xmlFile);
}

void xmlHandler::read_child(QDomDocument *xmlDoc, QDomElement *xmlElement, QStandardItem *parent, QStandardItemModel *model,QStringList *list,int counter)
{
    QStringList *tagList = settings::getHeaderMap(list->at(counter));
    QDomElement xmlChild;
    QStandardItem *item;
    QModelIndex index = model->indexFromItem(parent);
    QModelIndex childIndex;

    for(int row = 0; row < parent->rowCount(); ++row)
    {
        item = model->itemFromIndex(model->index(row,0,index));
        childIndex = model->indexFromItem(item);
        xmlChild = xmlDoc->createElement(list->at(counter));

        for(int att = 0; att < tagList->count(); ++att)
        {
            xmlChild.setAttribute(tagList->at(att),model->data(childIndex.siblingAtColumn(att)).toString());
        }

        xmlElement->appendChild(xmlChild);

        if(item->hasChildren())
        {
            this->read_child(xmlDoc,&xmlChild,item,model,list,list->indexOf(list->at(counter))+1);
        }
    }
}
