#include "saisons.h"

saisons::saisons()
{
    saison_tags << "name" << "start" << "end" << "weeks";
    contest_tags << "id" <<"saisonid" <<"date" << "sport" << "name" << "distance" << "stress";
    saisonPath = settings::getStringMapPointer(settings::stingMap::GC)->value("saisons");

    saisonFile = "saisons.xml";

    if(!saisonPath.isEmpty())
    {
        saisonsModel = new QStandardItemModel(0,saison_tags.count());
        contestModel = new QStandardItemModel(0,contest_tags.count());
        this->check_File(saisonPath,saisonFile);
        this->read_saisonInfo(this->load_XMLFile(saisonPath,saisonFile));
    }
}

void saisons::read_saisonInfo(QDomDocument xmldoc)
{
    QDomElement rootTag;
    QDomNodeList xmlList,childList;
    QDomElement xmlElement,childElement;
    int contestCount = 0;
    int rowCount = 0;
    rootTag = xmldoc.firstChildElement();
    xmlList = rootTag.elementsByTagName("saison");

    for(int saison = 0; saison < xmlList.count(); ++saison)
    {
        xmlElement = xmlList.at(saison).toElement();
        saisonsModel->insertRow(saison,QModelIndex());
        for(int col = 0; col < saison_tags.count();++col)
        {
            saisonsModel->setData(saisonsModel->index(saison,col),xmlElement.attribute(saison_tags.at(col)));
        }

        childList = xmlElement.childNodes();
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
    saisonsModel->sort(0);
    contestModel->sort(0);
    this->fill_saisonMap();
}

void saisons::write_saisonInfo()
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement,childElement;
    QSortFilterProxyModel *contestProxy = new QSortFilterProxyModel;
    contestProxy->setSourceModel(contestModel);

    xmlRoot = xmlDoc.createElement("saisons");
    xmlDoc.appendChild(xmlRoot);

    for(int saison = 0; saison < saisonsModel->rowCount(); ++saison)
    {
        xmlElement = xmlDoc.createElement("saison");

        for(int att = 0; att < saison_tags.count(); ++att)
        {
            xmlElement.setAttribute(saison_tags.at(att),saisonsModel->data(saisonsModel->index(saison,att)).toString());
        }

        contestProxy->setFilterFixedString(saisonsModel->data(saisonsModel->index(saison,0)).toString());
        contestProxy->setFilterKeyColumn(1);

        if(contestProxy->rowCount() > 0)
        {
            for(int contest = 0; contest < contestProxy->rowCount(); ++contest)
            {
                childElement = xmlDoc.createElement("contest");
                for(int conAtt = 0; conAtt < contest_tags.count(); ++conAtt)
                {
                    childElement.setAttribute(contest_tags.at(conAtt),contestProxy->data(contestProxy->index(contest,conAtt)).toString());
                }
                xmlElement.appendChild(childElement);
                childElement.clear();
            }
        }
        contestProxy->invalidate();
        xmlRoot.appendChild(xmlElement);
        xmlElement.clear();
    }
    this->write_XMLFile(saisonPath,&xmlDoc,saisonFile);
    xmlDoc.clear();
}

void saisons::fill_saisonMap()
{
    QMap<QString,QVariant> tempMap;
    saisonMap.clear();

    for(int i = 0; i < saisonsModel->rowCount(); ++i)
    {
        for(int x = 1; x < saison_tags.count(); ++x)
        {
            tempMap.insert(saison_tags.at(x),saisonsModel->data(saisonsModel->index(i,x)));
        }
        saisonMap.insert(saisonsModel->data(saisonsModel->index(i,0)).toString(),tempMap);
        currSaison = this->saison_atDate(QDate::currentDate());
    }
}

void saisons::update_saison(bool addNew,int row,QString name, QDate startDate, QDate endDate, int weeks)
{

    if(addNew)
    {
        row = saisonsModel->rowCount();
        saisonsModel->insertRow(row,QModelIndex());
    }

    saisonsModel->setData(saisonsModel->index(row,0),name);
    saisonsModel->setData(saisonsModel->index(row,1),startDate);
    saisonsModel->setData(saisonsModel->index(row,2),endDate);
    saisonsModel->setData(saisonsModel->index(row,3),weeks);

    this->fill_saisonMap();
}

void saisons::remove_saison(int id)
{
    saisonsModel->removeRow(id);
    this->fill_saisonMap();
}

QVariant saisons::get_saisonInfo(QString saison, QString key)
{
    return saisonMap.value(saison).value(key);
}

QString saisons::saison_atDate(QDate checkDate)
{
    QDate saisonDate;

    for(QHash<QString,QMap<QString,QVariant> >::const_iterator it = saisonMap.cbegin(), end = saisonMap.cend(); it != end;++it)
    {
        saisonDate = it.value().value("start").toDate();
        if(checkDate >= saisonDate)
        {
            saisonDate = it.value().value("end").toDate();
            if(checkDate <= saisonDate)
            {
                return it.key();
            }
        }
    }

    return 0;
}
