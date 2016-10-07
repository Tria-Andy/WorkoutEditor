#include "jsonhandler.h"
#include "settings.h"

jsonHandler::jsonHandler(QString filename,QString jsonfile, Activity *p_act)
{
    fileName = filename;
    curr_act = p_act;
    this->read_json(jsonfile);
}

void jsonHandler::fill_qmap(QMap<QString, QString> *qmap,QJsonObject *objItem)
{
    QString keyValue;
    for(int i = 0; i < objItem->keys().count(); ++i)
    {
        keyValue = objItem->keys().at(i);
        qmap->insert(keyValue,objItem->value(keyValue).toString());

    }
}

void jsonHandler::fill_keyList(QStringList *targetList,QMap<int, QString> *map, QStringList *list)
{
    for(int i = 0; i < map->count(); ++i)
    {
       targetList->insert(i,map->value(i));
    }
    for(int x = 0; x < list->count();++x)
    {
        if(!targetList->contains(list->at(x)))
        {
            (*targetList) << list->at(x);
        }
    }
}

void jsonHandler::fill_model(QStandardItemModel *model, QJsonArray *jArray, QStringList *list)
{
    QJsonObject objItem;
    for(int row = 0; row < jArray->count(); ++row)
    {
        objItem = jArray->at(row).toObject();

        for(int col = 0; col < list->count(); ++col)
        {
            model->setData(model->index(row,col,QModelIndex()),objItem[list->at(col)].toVariant());
        }
    }
}

void jsonHandler::fill_list(QJsonArray *jArray,QStringList *list)
{
    for(int i = 0; i < jArray->count(); ++i)
    {
        (*list) << jArray->at(i).toString();
    }
}

QJsonObject jsonHandler::mapToJson(QMap<QString,QString> *map)
{
    QJsonObject item;

    for(QMap<QString,QString>::const_iterator it =  map->cbegin(), end = map->cend(); it != end; ++it)
    {
        if(it.key() == "RECINTSECS")
        {
            item.insert(it.key(),1);
        }
        else
        {
            item.insert(it.key(),it.value());
        }
    }
    return item;
}

QJsonArray jsonHandler::listToJson(QStringList *list)
{
    QJsonArray jArray;

    for(int i = 0; i < list->count(); ++i)
    {
        jArray.insert(i,list->at(i));
    }

    return jArray;
}

QJsonArray jsonHandler::modelToJson(QStandardItemModel *model, QStringList *list)
{
    QJsonArray jArray;
    QVariant modelValue;
    for(int row = 0; row < model->rowCount(); ++row)
    {
        QJsonObject item_array;
        for(int col = 0; col < list->count(); ++col)
        {
            modelValue = model->data(model->index(row,col,QModelIndex()));
            if(modelValue.isValid())
            {
                item_array.insert(list->at(col),QJsonValue::fromVariant(modelValue));
            }
        }
        jArray.insert(row,item_array);
    }

    return jArray;
}

void jsonHandler::read_json(QString jsonfile)
{
    hasXdata = false;
    QJsonObject itemObject;
    QJsonArray itemArray;
    QStringList valueList;
    QMap<int,QString> mapValues;
    QJsonDocument d = QJsonDocument::fromJson(jsonfile.toUtf8());
    QJsonObject jsonobj = d.object();

    QJsonObject item_ride = jsonobj.value(QString("RIDE")).toObject();
    this->fill_qmap(&rideData,&item_ride);

    itemObject = item_ride.value(QString("TAGS")).toObject();
    this->fill_qmap(&tagData,&itemObject);

    itemArray = item_ride["INTERVALS"].toArray();
    valueList << itemArray.at(0).toObject().keys();
    mapValues = settings::get_intList();
    this->fill_keyList(&intList,&mapValues,&valueList);
    int_model = new QStandardItemModel(itemArray.count(),intList.count());
    this->fill_model(int_model,&itemArray,&intList);

    valueList = QStringList();

    itemArray = item_ride["SAMPLES"].toArray();
    valueList << itemArray.at(0).toObject().keys();
    mapValues = settings::get_sampList();
    this->fill_keyList(&sampList,&mapValues,&valueList);
    samp_model = new QStandardItemModel(itemArray.count(),sampList.count());
    this->fill_model(samp_model,&itemArray,&sampList);

    if(item_ride.contains("XDATA"))
    {
        hasXdata = true;
        if(curr_act->get_sport() == curr_act->isSwim)
        {
            QJsonObject item_xdata = item_ride["XDATA"].toArray().at(0).toObject();
            this->fill_qmap(&xData,&item_xdata);

            itemArray = QJsonArray();
            itemArray = item_xdata["UNITS"].toArray();
            this->fill_list(&itemArray,&xdataUnits);

            itemArray = QJsonArray();
            itemArray = item_xdata["VALUES"].toArray();
            this->fill_list(&itemArray,&xdataValues);

            itemArray = QJsonArray();
            itemArray = item_xdata["SAMPLES"].toArray();
            QJsonObject xSample = itemArray.at(0).toObject();

            xdata_model = new QStandardItemModel(itemArray.count(),xSample.keys().count()+xdataValues.count());

            for(int i = 0; i < itemArray.count(); ++i)
            {
                xSample = itemArray.at(i).toObject();
                QJsonArray arrValues = xSample["VALUES"].toArray();
                xdata_model->setData(xdata_model->index(i,0,QModelIndex()),xSample["SECS"].toInt());
                xdata_model->setData(xdata_model->index(i,1,QModelIndex()),xSample["KM"].toDouble());
                xdata_model->setData(xdata_model->index(i,2,QModelIndex()),arrValues.at(0).toInt());
                xdata_model->setData(xdata_model->index(i,3,QModelIndex()),arrValues.at(1).toDouble());
                xdata_model->setData(xdata_model->index(i,4,QModelIndex()),arrValues.at(2).toInt());
            }
        }
    }
    this->write_json();
}

void jsonHandler::write_json()
{
    QByteArray jsonByte;
    QMimeData *mimeData = new QMimeData();
    QJsonDocument jsonDoc;
    QJsonObject rideFile,item_ride;
    QJsonArray intArray;

    item_ride = mapToJson(&rideData);
    item_ride["TAGS"] = mapToJson(&tagData);
    item_ride["INTERVALS"] = modelToJson(int_model,&intList);
    item_ride["SAMPLES"] = modelToJson(samp_model,&sampList);

    if(hasXdata)
    {
        QJsonObject item_xdata = mapToJson(&xData);
        item_xdata["UNITS"] = listToJson(&xdataUnits);
        item_xdata["VALUES"] = listToJson(&xdataValues);

        intArray = QJsonArray();
        for(int i = 0; i < xdata_model->rowCount(); ++i)
        {
            QJsonObject item_array;
            QJsonArray value_array;
            item_array.insert("SECS",QJsonValue::fromVariant(xdata_model->data(xdata_model->index(i,0,QModelIndex()))));
            item_array.insert("KM",QJsonValue::fromVariant(xdata_model->data(xdata_model->index(i,1,QModelIndex()))));
            value_array.insert(0,QJsonValue::fromVariant(xdata_model->data(xdata_model->index(i,2,QModelIndex()))));
            value_array.insert(1,QJsonValue::fromVariant(xdata_model->data(xdata_model->index(i,3,QModelIndex()))));
            value_array.insert(2,QJsonValue::fromVariant(xdata_model->data(xdata_model->index(i,4,QModelIndex()))));
            item_array["VALUES"] = value_array;
            intArray.insert(i,item_array);
        }
        item_xdata["SAMPLES"] = intArray;
        item_ride["XDATA"] = item_xdata;
    }

    rideFile["RIDE"] = item_ride;

    jsonDoc.setObject(rideFile);

    jsonByte = jsonDoc.toJson();
    mimeData->setData("text/plain",jsonByte);
    jsonFile = mimeData->text().toUtf8();
    this->write_file(jsonDoc);
}

void jsonHandler::write_file(QJsonDocument jsondoc)
{
    //QFile file(settings::get_gcPath() + QDir::separator() + fileName);
    QFile file(QCoreApplication::applicationDirPath() + QDir::separator() + fileName);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "File not open!";
        return;
    }

    file.write(jsondoc.toJson());
    file.flush();
    file.close();
}
