#include "jsonhandler.h"
#include "settings.h"

jsonHandler::jsonHandler(bool readFlag,QString jsonfile, Activity *p_act)
{   
    hasFile = readFlag;
    hasXdata = false;
    if(readFlag)
    {
        curr_act = p_act;
        this->read_json(jsonfile);
    }
}

void jsonHandler::fill_qmap(QMap<QString, QString> *qmap,QJsonObject *objItem)
{
    QString keyValue;
    for(int i = 0; i < objItem->keys().count(); ++i)
    {
        keyValue = objItem->keys().at(i);
        qmap->insert(keyValue,objItem->value(keyValue).toString().trimmed());
        if(keyValue == "OVERRIDES") hasOverride = true;
    }
}

void jsonHandler::fill_keyList(QStringList *targetList,QMap<int, QString> *map, QStringList *list)
{
    for(int i = 0; i < map->count(); ++i)
    {
        if(list->contains(map->value(i)))
        {
            targetList->insert(i,map->value(i));
        }
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
            item.insert(it.key(),it.value()+" ");
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
    hasOverride = false;
    QJsonObject itemObject;
    QJsonArray itemArray;
    QStringList valueList;
    QString stgValue;
    QMap<int,QString> mapValues;
    QJsonDocument d = QJsonDocument::fromJson(jsonfile.toUtf8());
    QJsonObject jsonobj = d.object();

    QJsonObject item_ride = jsonobj.value(QString("RIDE")).toObject();
    this->fill_qmap(&rideData,&item_ride);

    if(hasOverride)
    {
        QJsonObject objOverride,objValue;
        itemArray = item_ride["OVERRIDES"].toArray();
        for(int i = 0; i < itemArray.count(); ++i)
        {
            objOverride = itemArray.at(i).toObject();
            objValue = objOverride[objOverride.keys().first()].toObject();
            overrideData.insert(objOverride.keys().first(),objValue["value"].toString());
        }
    }

    itemObject = item_ride.value(QString("TAGS")).toObject();
    this->fill_qmap(&tagData,&itemObject);
    curr_act->set_sport(tagData.value("Sport"));
    valueList = QStringList();

    curr_act->ride_info.insert("Date:",rideData.value("STARTTIME"));
    fileName = tagData.value("Filename").trimmed();

    for(int i = 0; i < settings::get_jsoninfos().count();++i)
    {
        stgValue = settings::get_jsoninfos().at(i);
        curr_act->ride_info.insert(stgValue+":",itemObject[stgValue].toString());
    }
    stgValue = QString();

    itemArray = item_ride["INTERVALS"].toArray();
    valueList << itemArray.at(0).toObject().keys();
    mapValues = settings::get_intList();
    this->fill_keyList(&intList,&mapValues,&valueList);
    curr_act->int_model = new QStandardItemModel(itemArray.count(),intList.count());
    this->fill_model(curr_act->int_model,&itemArray,&intList);

    valueList = QStringList();

    itemArray = item_ride["SAMPLES"].toArray();
    valueList << itemArray.at(0).toObject().keys();
    mapValues = settings::get_sampList();
    this->fill_keyList(&sampList,&mapValues,&valueList);
    curr_act->samp_model = new QStandardItemModel(itemArray.count(),sampList.count());
    this->fill_model(curr_act->samp_model,&itemArray,&sampList);

    if(curr_act->get_sport() == settings::isRun)
    {
        int avgHF = 0;
        int posHF = sampList.indexOf("HR");
        int sampCount = curr_act->samp_model->rowCount();
        for(int i = 0;i < sampCount; ++i)
        {
            avgHF = avgHF + curr_act->samp_model->data(curr_act->samp_model->index(i,posHF,QModelIndex())).toInt();
        }
        avgHF = (avgHF / sampCount);
        overrideData.insert("total_work",QString::number(settings::calc_totalWork(curr_act->get_sport(),tagData.value("Weight").toDouble(),avgHF,sampCount,0.0)));
        hasOverride = true;
    }

    if(item_ride.contains("XDATA"))
    {
        hasXdata = true;
        if(curr_act->get_sport() == settings::isSwim)
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
            QJsonObject obj_xdata = itemArray.at(0).toObject();

            curr_act->xdata_model = new QStandardItemModel(itemArray.count(),(obj_xdata.keys().count()+xdataValues.count()));

            double swim_track = tagData.value("Pool Length").toDouble();
            curr_act->set_swim_track(swim_track);

            for(int i = 0; i < itemArray.count(); ++i)
            {
                obj_xdata = itemArray.at(i).toObject();
                QJsonArray arrValues = obj_xdata["VALUES"].toArray();
                curr_act->xdata_model->setData(curr_act->xdata_model->index(i,0,QModelIndex()),obj_xdata["SECS"].toInt());
                curr_act->xdata_model->setData(curr_act->xdata_model->index(i,1,QModelIndex()),obj_xdata["KM"].toDouble());
                curr_act->xdata_model->setData(curr_act->xdata_model->index(i,2,QModelIndex()),arrValues.at(0).toInt());
                curr_act->xdata_model->setData(curr_act->xdata_model->index(i,3,QModelIndex()),arrValues.at(1).toDouble());
                curr_act->xdata_model->setData(curr_act->xdata_model->index(i,4,QModelIndex()),arrValues.at(2).toInt());
            }
        }
    }
    curr_act->prepareData();
}

void jsonHandler::write_json()
{
    QJsonDocument jsonDoc;
    QJsonObject rideFile,item_ride;
    QJsonArray intArray;

    item_ride = mapToJson(&rideData);
    item_ride["TAGS"] = mapToJson(&tagData);

    if(hasFile)
    {
        bool isrecalc = settings::get_act_isrecalc();
        p_int = curr_act->set_int_model_pointer(isrecalc);
        p_samp = curr_act->set_samp_model_pointer(isrecalc);
        item_ride["INTERVALS"] = modelToJson(p_int,&intList);
        item_ride["SAMPLES"] = modelToJson(p_samp,&sampList);
    }

    if(hasOverride)
    {
        int i = 0;
        for(QMap<QString,QString>::const_iterator it =  overrideData.cbegin(), end = overrideData.cend(); it != end; ++it,++i)
        {
            QJsonObject objOverride,objValue;
            objValue.insert("value",it.value());
            objOverride.insert(it.key(),objValue);
            intArray.insert(i,objOverride);
        }
        item_ride["OVERRIDES"] = intArray;
    }

    if(hasXdata)
    {
        QJsonArray item_xdata;
        QJsonObject xdataObj;

        xdataObj.insert("NAME",xData.value("NAME"));
        xdataObj.insert("UNITS",listToJson(&xdataUnits));
        xdataObj.insert("VALUES",listToJson(&xdataValues));

        intArray = QJsonArray();
        for(int i = 0; i < curr_act->xdata_model->rowCount(); ++i)
        {
            QJsonObject item_array;
            QJsonArray value_array;
            item_array.insert("SECS",QJsonValue::fromVariant(curr_act->swim_xdata->data(curr_act->swim_xdata->index(i,1,QModelIndex()))));
            item_array.insert("KM",QJsonValue::fromVariant(curr_act->swim_xdata->data(curr_act->swim_xdata->index(i,5,QModelIndex()))));
            value_array.insert(0,QJsonValue::fromVariant(curr_act->swim_xdata->data(curr_act->swim_xdata->index(i,6,QModelIndex()))));
            value_array.insert(1,QJsonValue::fromVariant(curr_act->swim_xdata->data(curr_act->swim_xdata->index(i,2,QModelIndex()))));
            value_array.insert(2,QJsonValue::fromVariant(curr_act->swim_xdata->data(curr_act->swim_xdata->index(i,3,QModelIndex()))));
            item_array["VALUES"] = value_array;
            intArray.insert(i,item_array);
        }
        xdataObj.insert("SAMPLES",intArray);
        item_xdata.insert(0,xdataObj);
        item_ride["XDATA"] = item_xdata;
    }

    rideFile["RIDE"] = item_ride;
    jsonDoc.setObject(rideFile);
    this->write_file(jsonDoc);
}

void jsonHandler::write_file(QJsonDocument jsondoc)
{
    QFile file(settings::get_gcPath() + QDir::separator() + fileName);
    //QFile file(QCoreApplication::applicationDirPath() + QDir::separator() + fileName);
    if(!file.open(QFile::WriteOnly))
    {
        qDebug() << "File not open!";
        return;
    }

    file.write(jsondoc.toJson(QJsonDocument::Compact));
    file.flush();
    file.close();
}
