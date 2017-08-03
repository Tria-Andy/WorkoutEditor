#include "saisons.h"

saisons::saisons()
{
    saison_tags << "name" << "start" << "end" << "weeks";
    contest_tags << "id" <<"saisonid" <<"date" << "sport" << "name" << "distance" << "stress";
    saisonPath = settings::get_gcInfo("saisons");

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
}

void saisons::add_saison(QString name, QDate startDate, QDate endDate, int weeks)
{
    int rowCount = saisonsModel->rowCount();

    saisonsModel->insertRow(rowCount,QModelIndex());
    saisonsModel->setData(saisonsModel->index(rowCount,0),name);
    saisonsModel->setData(saisonsModel->index(rowCount,1),startDate);
    saisonsModel->setData(saisonsModel->index(rowCount,2),endDate);
    saisonsModel->setData(saisonsModel->index(rowCount,3),weeks);
}

void saisons::remove_saison(int id)
{
    saisonsModel->removeRow(id);
}
