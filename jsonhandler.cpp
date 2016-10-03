#include "jsonhandler.h"

jsonHandler::jsonHandler(QString jsonfile, Activity *p_act)
{
    curr_act = p_act;
    this->read_json(jsonfile);

}

void jsonHandler::read_json(QString jsonfile)
{
    QJsonDocument d = QJsonDocument::fromJson(jsonfile.toUtf8());
    QJsonObject jsonobj = d.object();

    QJsonValue ride = jsonobj.value(QString("RIDE"));
    QJsonObject item_ride = ride.toObject();

    QJsonValue v_tags = item_ride.value(QString("TAGS"));
    QJsonObject item_tags = v_tags.toObject();

    qDebug() << item_ride.keys();
}
