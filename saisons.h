#ifndef SAISONS_H
#define SAISONS_H

#include <QStandardItemModel>
#include <QtXml>
#include "settings.h"
#include "xmlhandler.h"


class saisons : public xmlHandler
{
public:
    saisons();
    QStandardItemModel *saisonsModel,*contestModel;

    void add_saison(QString,QDate,QDate,int);
    void remove_saison(int);

private:
    QStringList saison_tags,contest_tags;
    QString saisonPath,saisonFile;

    void read_saisonInfo(QDomDocument);
};

#endif // SAISONS_H
