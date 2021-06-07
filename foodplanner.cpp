#include "foodplanner.h"

foodplanner::foodplanner(schedule *p_Schedule)
{
    schedulePtr = p_Schedule;

    firstdayofweek = settings::firstDayofWeek;
    dateFormat = settings::get_format("dateformat");
    dateSaveFormat = "yyyy-MM-dd";
    gcValues = settings::getStringMapPointer(settings::stingMap::GC);
    foodPlanTags = settings::get_xmlMapping("foodplan");
    foodHistTags = settings::get_xmlMapping("histweeks");
    drinkTags = settings::get_xmlMapping("drinks");
    ingredTags = settings::get_xmlMapping("ingredients");

    menuHeader = settings::getHeaderMap("menuheader");
    foodsumHeader = settings::getHeaderMap("foodsumheader");
    foodweekHeader = settings::getHeaderMap("foodweekheader");
    mealsHeader = settings::get_listValues("Meals");
    fileMap = settings::getStringMapPointer(settings::stingMap::File);
    doubleValues = settings::getdoubleMapPointer(settings::dMap::Double);

    dayListHeader << "Cal (%)"
                  << "Carbs ("+QString::number(settings::doubleVector.value("Macros").at(0))+"%)"
                  << "Protein ("+QString::number(settings::doubleVector.value("Macros").at(1))+"%)"
                  << "Fat ("+QString::number(settings::doubleVector.value("Macros").at(2))+"%)";

    for(int d = 1; d < 8; ++d)
    {
        dayHeader << QLocale().dayName(d);
    }

    foodPlanModel = new QStandardItemModel();
    summeryModel = new QStandardItemModel();
    ingredModel = new QStandardItemModel();
    recipeModel = new QStandardItemModel();
    drinkModel = new QStandardItemModel();

    modelMap.insert(0,recipeModel);
    modelMap.insert(1,ingredModel);
    modelMap.insert(2,drinkModel);

    if(!settings::getStringMapPointer(settings::stingMap::GC)->value("foodplanner").isEmpty())
    {
        this->xml_toTreeModel(fileMap->value("recipefile"),recipeModel);
        this->xml_toTreeModel(fileMap->value("ingredfile"),ingredModel);
        this->xml_toTreeModel(fileMap->value("drinkfile"),drinkModel);
        this->xml_toTreeModel(fileMap->value("foodfile"),foodPlanModel);
    }

    foodProxy = new QSortFilterProxyModel();
    foodProxy->setSourceModel(foodPlanModel);
    sumProxy = new QSortFilterProxyModel();
    sumProxy->setSourceModel(summeryModel);

    this->set_currentWeek(firstdayofweek);
    this->check_foodPlan();
}

enum {INIT,WEEK,DAY};
enum {ADD,DEL,EDIT};

bool foodplanner::copyMap_hasData()
{
    if(copyMap.first.isValid())
    {
        return true;
    }

    return false;
}

bool foodplanner::copyQueue_hasData()
{
    if(!copyQueue.isEmpty())
    {
        return true;
    }
    return false;
}

bool foodplanner::dragDrop_hasData()
{
    if(dragDrop.first.isValid())
    {
        return true;
    }
    return false;
}

bool foodplanner::check_foodDay(QDate day)
{
    if(foodPlanModel->findItems(calc_weekID(day),Qt::MatchExactly,0).isEmpty())
    {
        return false;
    }
    else
    {
        return true;
    }
}

QString foodplanner::get_mode(QDate startDate)
{
    if(foodPlanModel->findItems(calc_weekID(startDate),Qt::MatchExactly,0).isEmpty())
    {
        return settings::get_listValues("Mode").at(0);
    }
    else
    {
        QModelIndex weekIndex = foodPlanModel->indexFromItem(foodPlanModel->findItems(calc_weekID(startDate),Qt::MatchExactly,0).at(0));
        return foodPlanModel->data(foodPlanModel->index(weekIndex.row(),2)).toString();
    }
}

QString foodplanner::get_newRecipeID(QString section)
{
    QStandardItem *item = recipeModel->findItems(section,Qt::MatchExactly,0).at(0);

    return section+"-"+QString::number(item->rowCount());
}

void foodplanner::set_headerLabel(QStandardItemModel *model, QStringList *list, bool vert)
{
    for(int i = 0; i < list->count(); ++i)
    {
        if(vert)
        {
            model->setVerticalHeaderItem(i,new QStandardItem(list->at(i)));
        }
        else
        {
            model->setHorizontalHeaderItem(i,new QStandardItem(list->at(i)));
        }
    }
}

void foodplanner::read_nutritionHistory()
{
    QFile file(gcValues->value("confpath") + QDir::separator() + gcValues->value("nutritionfile"));
    file.open(QFile::ReadOnly | QFile::Text);

    QString jsonFile = file.readAll();
    QJsonDocument d = QJsonDocument::fromJson(jsonFile.toUtf8());
    QJsonObject jsonobj = d.object();
    QStringList *mapList,*valueList;
    QJsonArray nutrition = jsonobj[settings::get_xmlMapping("nutrition")->at(0)].toArray();
    file.close();

    QDate day;
    QJsonObject nutritionInfo;
    QVector<int> histValues(10,0);
    QPair<QString,QVector<int>> histInfo;

    mapList = settings::get_xmlMapping("nutritinfo");
    valueList = settings::get_xmlMapping("nutrivalues");

    for(int i = 0; i < nutrition.count(); ++i)
    {
        nutritionInfo = nutrition.at(i).toObject();

        day = QDateTime().fromSecsSinceEpoch(nutritionInfo.value(mapList->at(0)).toInt()).date();

        if(day.daysTo(QDate().currentDate()) <= doubleValues->value("keephistory"))
        {
            for(int value = 0; value < valueList->count(); ++value)
            {
                histValues[value] = nutritionInfo.value(valueList->at(value)).toInt();
            }

            histInfo.first = nutritionInfo.value(mapList->at(1)).toString();
            histInfo.second = histValues;

            historyMap.insert(day,histInfo);
        }
    }
}

void foodplanner::set_recipeList(QDate day, QString recipeID, QString section)
{
    QMap<QString,QList<QDate>> recipeAt = recipeMap.value(recipeID);
    QList<QDate> atSection = recipeAt.value(section);
    atSection.append(day);    
    recipeAt.insert(section,atSection);
    recipeMap.insert(recipeID,recipeAt);
}

QModelIndex foodplanner::get_modelIndex(QStandardItemModel *model, QString searchString,int col)
{
    QList<QStandardItem*> list = model->findItems(searchString,Qt::MatchExactly | Qt::MatchRecursive,col);

    if(list.count() > 0)
    {
        return model->indexFromItem(list.at(0));
    }
    else
    {
        return QModelIndex();
    }
}

QStandardItem *foodplanner::get_modelItem(QStandardItemModel *model, QString searchString,int col)
{
    QList<QStandardItem*> list = model->findItems(searchString,Qt::MatchExactly | Qt::MatchRecursive,col);

    if(list.count() > 0)
    {
        return list.at(0);
    }
    else
    {
        return nullptr;
    }
}

QVector<double> foodplanner::get_foodMacros(QStandardItemModel *model,QString foodID)
{
    QVector<double> macros(7,0);

    QModelIndex index = this->get_modelIndex(model,foodID,0);

    macros[0] = index.siblingAtColumn(2).data(Qt::DisplayRole).toDouble();
    macros[1] = index.siblingAtColumn(3).data(Qt::DisplayRole).toDouble();
    macros[2] = index.siblingAtColumn(4).data(Qt::DisplayRole).toDouble();
    macros[3] = index.siblingAtColumn(5).data(Qt::DisplayRole).toDouble();
    macros[4] = index.siblingAtColumn(6).data(Qt::DisplayRole).toDouble();
    macros[5] = index.siblingAtColumn(7).data(Qt::DisplayRole).toDouble();
    macros[6] = index.siblingAtColumn(8).data(Qt::DisplayRole).toDouble();

    return macros;
}

QStringList foodplanner::get_modelSections(QStandardItemModel *model)
{
    QStringList sectionList;

    for(int i = 0; i < model->rowCount(); i++)
    {
        sectionList << model->item(i,0)->data(Qt::DisplayRole).toString();
    }

    return sectionList;
}

QList<QStandardItem*> foodplanner::get_sectionItems(QStandardItemModel *model, QString section)
{
    QStandardItem *item = model->findItems(section,Qt::MatchExactly,0).at(0);
    QList<QStandardItem*> sectionItems;
    QStandardItem *sectionItem;

    for(int i = 0; i < item->rowCount(); i++)
    {
        sectionItem = new QStandardItem();
        sectionItem->setData(item->child(i,1)->data(Qt::DisplayRole).toString(),Qt::DisplayRole);
        sectionItem->setData(item->child(i,0)->data(Qt::DisplayRole).toString(),Qt::AccessibleTextRole);
        sectionItems.append(sectionItem);
    }

    return sectionItems;
}

void foodplanner::set_summeryData(QStandardItem *weekItem)
{
    QModelIndex weekIndex = foodPlanModel->indexFromItem(weekItem);
    QDate firstDay = weekIndex.siblingAtColumn(1).data(Qt::DisplayRole).toDate();

    QString foodMode = weekIndex.siblingAtColumn(2).data(Qt::DisplayRole).toString();

    //Set SummeryModel
    QStandardItem *sumRootItem = summeryModel->invisibleRootItem();
    QList<QStandardItem*> sumList,weekList;
    weekList.insert(0,new QStandardItem(weekIndex.siblingAtColumn(0).data(Qt::DisplayRole).toString()));
    weekList.insert(1,new QStandardItem(firstDay.toString(dateFormat)));

    for(int day = 0; day < weekItem->rowCount(); ++day)
    {
        sumList.insert(0,new QStandardItem());
        sumList.at(0)->setData(day,Qt::DisplayRole);
        sumList.at(0)->setData(foodMode,Qt::AccessibleTextRole);
        sumList.insert(1,new QStandardItem());
        sumList.at(1)->setData(firstDay.addDays(day),Qt::DisplayRole);

        weekList.at(0)->appendRow(sumList);
        sumList.clear();
    }
    sumRootItem->appendRow(weekList);

    for(int day = 0; day < weekItem->rowCount(); ++day)
    {
        this->update_summeryModel(firstDay.addDays(day),weekItem->child(day,0),false);
    }

}

void foodplanner::update_foodPlanModel(QDate mealDate, QString mealSection, QMap<int,QList<QStandardItem *>> mealMap)
{
    QStandardItem *dayItem,*mealItem;
    dayItem = this->get_modelItem(foodPlanModel,mealDate.toString(dateSaveFormat),0);
    mealItem = dayItem->child(mealsHeader.indexOf(mealSection),0);
    int calories = 0;

    foodPlanModel->removeRows(0,mealItem->rowCount(),mealItem->index());

    for(QMap<int,QList<QStandardItem *>>::const_iterator mealStart = mealMap.cbegin(); mealStart != mealMap.cend(); ++mealStart)
    {
        calories = calories + mealStart.value().at(3)->data(Qt::DisplayRole).toInt();
        mealStart.value().first()->setData(foodPlanTags->at(3),Qt::AccessibleTextRole);
        mealItem->appendRow(mealStart.value());
    }
    this->update_summeryModel(mealDate,mealItem,true);
}

void foodplanner::set_currentWeek(QDate weekDate)
{
    QString weekID = calc_weekID(weekDate);
    foodProxy->invalidate();
    foodProxy->setFilterFixedString(weekID);
    sumProxy->invalidate();
    sumProxy->setFilterFixedString(weekID);
}

void foodplanner::update_fromSchedule()
{
    QDate editDay;
    QString weekID;
    QStandardItem *weekItem;

    while(!schedulePtr->changedDays.isEmpty())
    {
        editDay = schedulePtr->changedDays.dequeue();
        weekID = calc_weekID(editDay);
        weekItem = this->get_modelItem(foodPlanModel,weekID,0);
        if(weekItem != nullptr)
        {
            this->update_summeryModel(editDay,weekItem->child(editDay.dayOfWeek()-1),false);
        }
    }
}

void foodplanner::update_byPalChange()
{
    QStandardItem *weekItem;

    for(int week = 0; week < foodPlanModel->rowCount(); ++week)
    {
        weekItem = foodPlanModel->item(week,0);

        for(int day = 0; day < weekItem->rowCount(); ++day)
        {
            this->update_summeryModel(weekItem->child(day,0)->data(Qt::DisplayRole).toDate(),weekItem->child(day,0),false);
        }
    }
}

void foodplanner::add_ingredient(QString section, QString foodName,QVector<double> values,int modelID)
{
    QStandardItem *sectionItem = nullptr;
    QStringList *tagList = nullptr;

    if(modelID == 1)
    {
        sectionItem = this->get_modelItem(ingredModel,section,0);
        tagList = ingredTags;
    }
    else if(modelID == 2)
    {
        sectionItem = this->get_modelItem(drinkModel,section,0);
        tagList = drinkTags;
    }

    QList<QStandardItem*> itemList;
    QStandardItem *item;

    itemList.append(new QStandardItem(section+"-"+QString::number(sectionItem->rowCount()+1)));
    itemList.append(new QStandardItem(foodName));

    for(int i = 0; i < values.count(); ++i)
    {
        item = new QStandardItem(QString::number(values.at(i)));
        itemList.append(item);
    }
    itemList.at(0)->setData(tagList->at(1),Qt::AccessibleTextRole);
    sectionItem->appendRow(itemList);
}

void foodplanner::update_ingredient(QString ingredID,QString name,QVector<double> values,int modelID)
{
    QModelIndex index;

    QStandardItem *sectionItem = nullptr;

    if(modelID == 1)
    {
        index = this->get_modelIndex(ingredModel,ingredID,0);
        sectionItem = ingredModel->itemFromIndex(index)->parent();
    }
    else if(modelID == 2)
    {
        index = this->get_modelIndex(drinkModel,ingredID,0);
        sectionItem = drinkModel->itemFromIndex(index)->parent();
    }

    sectionItem->child(index.row(),1)->setData(name,Qt::DisplayRole);

    for(int i = 0,item=2; i < values.count(); ++i,++item)
    {
        sectionItem->child(index.row(),item)->setData(values.at(i),Qt::DisplayRole);
    }
}

QStandardItem* foodplanner::submit_recipes(QList<QStandardItem *> recipeMeta, QString section,bool newRecipe)
{
    QStandardItem *sectionItem = this->get_modelItem(recipeModel,section,0);
    QStandardItem *recipeItem;
    QString recipeID = recipeMeta.at(0)->data(Qt::DisplayRole).toString();

    if(newRecipe)
    {
        sectionItem->appendRow(recipeMeta);
        recipeItem = sectionItem->child(sectionItem->rowCount()-1,0);
    }
    else
    {
        QModelIndex recipeIndex = this->get_modelIndex(recipeModel,recipeID,0);
        recipeItem = recipeModel->itemFromIndex(recipeIndex);

        if(recipeItem->hasChildren())
        {
            recipeModel->removeRows(0,recipeItem->rowCount(),recipeIndex);
        }

        for(int i = 0; i < recipeMeta.count(); ++i)
        {
            recipeItem = recipeModel->itemFromIndex(recipeIndex.siblingAtColumn(i));
            recipeItem->setData(recipeMeta.at(i)->data(Qt::DisplayRole),Qt::DisplayRole);
        }
        recipeItem = this->get_modelItem(recipeModel,recipeID,0);
    }

    return recipeItem;
}

QStandardItem *foodplanner::get_proxyItem(int id)
{
    if(id == 0)
    {
        return foodPlanModel->itemFromIndex(foodProxy->mapToSource(foodProxy->index(0,0)));
    }
    else if(id == 1)
    {
        return summeryModel->itemFromIndex(sumProxy->mapToSource(sumProxy->index(0,0)));
    }
    return nullptr;

}

void foodplanner::update_summeryModel(QDate day,QStandardItem *calcItem,bool isMeal)
{
    const int minutes = 1440;
    double calMinute = round(this->current_dayCalories(day) * settings::doubleVector.value("palday").at(day.dayOfWeek()-1)) / minutes;
    double sportMinutes = 0;
    QMap<QString,QVector<double>> sportValues = schedulePtr->get_compValues()->value(day);

    QVector<int> sumValues(10,0);
    QVector<int> macroValues(5,0);

    QStandardItem *dayItem,*sectionItem;

    if (isMeal)
    {
        dayItem = calcItem->parent();
    }
    else
    {
        dayItem = calcItem;
    }

    for(int section = 0; section < dayItem->rowCount(); ++section)
    {
        sectionItem = dayItem->child(section);

        for(int meal = 0; meal < sectionItem->rowCount(); ++meal)
        {
            sumValues[0] = sumValues.at(0) + sectionItem->child(meal,3)->data(Qt::DisplayRole).toInt();
            sumValues[5] = sumValues.at(5) + sectionItem->child(meal,4)->data(Qt::DisplayRole).toInt();
            sumValues[6] = sumValues.at(6) + sectionItem->child(meal,5)->data(Qt::DisplayRole).toInt();
            sumValues[7] = sumValues.at(7) + sectionItem->child(meal,6)->data(Qt::DisplayRole).toInt();
            sumValues[8] = sumValues.at(8) + sectionItem->child(meal,7)->data(Qt::DisplayRole).toInt();
            sumValues[9] = sumValues.at(9) + sectionItem->child(meal,8)->data(Qt::DisplayRole).toInt();
        }
    }

    for(QMap<QString,QVector<double>>::const_iterator it = sportValues.cbegin(), end = sportValues.cend(); it != end; ++it)
    {
        sumValues[2] = sumValues.at(2) + it.value().at(5);
        sportMinutes = sportMinutes + (it.value().at(1)/60.0);
    }

    sumValues[1] = ceil((minutes - sportMinutes) * calMinute);
    sumValues[3] = sumValues.at(1) + sumValues.at(2);
    sumValues[4] = sumValues.at(3) - sumValues.at(0);

    double currentWeight = settings::get_weightforDate(day);

    macroValues[0] = round(sumValues.at(3) * (settings::doubleVector.value("Macros").at(0) / 100.0) / 4.1);
    macroValues[1] = round(sumValues.at(3) * (settings::doubleVector.value("Macros").at(1) / 100.0) / 4.1);
    macroValues[2] = round(sumValues.at(3) * (settings::doubleVector.value("Macros").at(2) / 100.0) / 9.3);
    macroValues[3] = ceil(currentWeight * (doubleValues->value("DayFiber") /100.0));
    macroValues[4] = ceil(currentWeight * (doubleValues->value("DaySugar")));

    if(day.daysTo(firstdayofweek) <= 1)
    {
        QPair<double,double> weightChange;

        weightChange.first = set_doubleValue(sumValues.at(4) / athleteValues->value("BodyFatCal") / 1000.0,true)*-1;
        weightChange.second = currentWeight;

        estWeightMap.insert(day,weightChange);

        if(estWeightMap.firstKey().daysTo(day) >= 1)
        {
            for(QMap<QDate,QPair<double,double>>::iterator it = estWeightMap.find(day); it != estWeightMap.end(); ++it)
            {
                weightChange.second = estWeightMap.value(it.key().addDays(-1)).second + weightChange.first;
                weightChange.first = it.value().first;
                estWeightMap.insert(it.key(),weightChange);
            }
        }
    }

    QStandardItem *weekItem = this->get_modelItem(summeryModel,calc_weekID(day),0);
    dayItem = weekItem->child(day.dayOfWeek()-1,0);

    QPair<int,int> slideValue;
    slideValue.first = sumValues.at(4);
    slideValue.second = 0;

    slideMap.insert(day,slideValue);

    int slideSum = 0;

    if(slideMap.firstKey().daysTo(day) >= 2)
    {
        for(QMap<QDate,QPair<int,int>>::Iterator it = slideMap.find(day); it != slideMap.end() ;++it)
        {
            slideValue = it.value();

            for(QMap<QDate,QPair<int,int>>::Iterator slide = slideMap.find(it.key().addDays(-2)); slide != slideMap.find(it.key()) ;++slide)
            {
                slideSum = slideSum + slide.value().first;
            }

            slideValue.second = round((it.value().first+slideSum)/3);
            slideMap.insert(it.key(),slideValue);
            slideSum = 0;
        }
    }

    for(int value = 0; value < sumValues.count(); ++value)
    {
        dayItem->setData(sumValues.at(value),Qt::UserRole+value);
    }

    dayItem = weekItem->child(day.dayOfWeek()-1,1);

    for(int macro = 0; macro < macroValues.count(); ++macro)
    {
        dayItem->setData(macroValues.at(macro),Qt::UserRole+macro);
    }
}

void foodplanner::fill_copyMap(QDate copyDate, QString copySection,bool day)
{
    QStringList sectionList;
    QPair<QDate,QString> queueItem;
    queueItem.first = copyDate;
    queueItem.second = copySection;

    if(day)
    {
        copyMap = queueItem;
    }
    else
    {
        copyQueue.enqueue(queueItem);
    }
}

QMap<int, QList<QStandardItem *> > foodplanner::get_copyItem(QDate copyDate, QString copySection)
{
    QStandardItem *dayItem,*sectionItem;
    QMap<int,QList<QStandardItem *>> mealMap;
    QList<QStandardItem *> itemList;

    dayItem = this->get_modelItem(foodPlanModel,copyDate.toString(dateSaveFormat),0);
    sectionItem = dayItem->child(mealsHeader.indexOf(copySection));

    for(int meal = 0; meal < sectionItem->rowCount(); ++meal)
    {
        for(int value = 0; value < sectionItem->columnCount(); ++value)
        {
            itemList.insert(value,new QStandardItem(sectionItem->child(meal,value)->data(Qt::DisplayRole).toString()));
        }
        mealMap.insert(meal,itemList);
        itemList.clear();
    }

    return mealMap;
}

void foodplanner::execute_copy(QDate copyDate,bool section)
{
    int dayCount = 0;

    QPair<QDate,QString> queueItem;

    while(!copyQueue.isEmpty())
    {
        queueItem = copyQueue.dequeue();
        this->update_foodPlanModel(copyDate.addDays(dayCount),queueItem.second,this->get_copyItem(queueItem.first,queueItem.second));
        if(section) ++dayCount;
    }
}

void foodplanner::clear_dragDrop()
{
    dragDrop.first = QDate();
    dragDrop.second.clear();
}

void foodplanner::set_dragDrop(QDate copyDate, QString copySection)
{
    dragDrop.first = copyDate;
    dragDrop.second = copySection;
}

void foodplanner::set_dropMeal(QDate toDate, QString toSection)
{
    if(dragDrop.first.isValid())
    {
        if(toDate.isValid())
        {
            this->update_foodPlanModel(toDate,toSection,this->get_copyItem(dragDrop.first,dragDrop.second));
        }
        this->clear_dragDrop();
    }
}

void foodplanner::check_foodPlan()
{
    QStandardItem *rootItem = foodPlanModel->invisibleRootItem();
    QStandardItem *weekItem,*dayItem,*sectionItem;
    QList<QStandardItem*> weekList;
    QList<QStandardItem*> sumList;

    //check Past Weeks
    for(int week = 0; week < rootItem->rowCount(); ++week)
    {
        weekItem = rootItem->child(week,0);

        if(weekItem->index().siblingAtColumn(1).data().toDate().daysTo(firstdayofweek) > 7)
        {
            weekList << weekItem;
        }
        else
        {
            sumList.append(weekItem);
        }

        for(int day = 0; day < weekItem->rowCount(); ++day)
        {
            dayItem = weekItem->child(day,0);
            for(int section = 0; section < dayItem->rowCount(); ++section)
            {
                sectionItem = dayItem->child(section,0);
                for(int meal = 0; meal < sectionItem->rowCount(); ++meal)
                {
                    if(sectionItem->child(meal,10)->data(Qt::DisplayRole).toInt() == 0)
                    {
                        this->set_recipeList(dayItem->data(Qt::DisplayRole).toDate(), sectionItem->child(meal,0)->data(Qt::DisplayRole).toString(),sectionItem->data(Qt::DisplayRole).toString());
                    }
                }
            }
        }
    }

    for(int i = 0; i < weekList.count(); ++i)
    {
        foodPlanModel->removeRows(0,weekList.at(i)->rowCount(),weekList.at(i)->index());
        foodPlanModel->removeRow(weekList.at(i)->index().row(),weekList.at(i)->index().parent());
    }

    for(int i = 0; i < sumList.count(); ++i)
    {
        this->set_summeryData(sumList.at(i));
    }

    //add days to history

    this->read_nutritionHistory();

    QVector<int> histValues(10,0);
    QPair<QString,QVector<int>> histInfo;
    bool newDay = false;

    for(int week = 0; week < summeryModel->rowCount();++week)
    {
        weekItem = summeryModel->item(week,0);

        for(int day = 0; day < weekItem->rowCount(); ++day)
        {
            if(weekItem->child(day,1)->data(Qt::DisplayRole).toDate().daysTo(QDate().currentDate()) > 0)
            {
                histValues[0] = weekItem->child(day,0)->data(Qt::UserRole+1).toInt();
                histValues[1] = weekItem->child(day,0)->data(Qt::UserRole+2).toInt();
                histValues[2] = weekItem->child(day,0)->data(Qt::UserRole).toInt();
                histValues[3] = weekItem->child(day,0)->data(Qt::UserRole+4).toInt();
                histValues[4] = weekItem->child(day,0)->data(Qt::UserRole+5).toInt();
                histValues[5] = weekItem->child(day,0)->data(Qt::UserRole+6).toInt();
                histValues[6] = weekItem->child(day,0)->data(Qt::UserRole+7).toInt();
                histValues[7] = weekItem->child(day,0)->data(Qt::UserRole+8).toInt();
                histValues[8] = weekItem->child(day,0)->data(Qt::UserRole+9).toInt();
                histValues[9] = 0;

                histInfo.first = weekItem->child(day,0)->data(Qt::AccessibleTextRole).toString();
                histInfo.second = histValues;

                historyMap.insert(weekItem->child(day,1)->data(Qt::DisplayRole).toDate(),histInfo);
                newDay = true;
            }
        }
    }

    if(newDay) this->save_historyFile();
}

void foodplanner::save_foolPlan()
{
    this->treeModel_toXml(foodPlanModel,fileMap->value("foodfile"));
}

void foodplanner::save_historyFile()
{
    if(settings::get_intValue("savehistory"))
    {
        QJsonArray jsonArray;
        QJsonObject histItem = QJsonObject();
        QJsonObject objValue;
        int counter = 0;
        QStringList *mapList,*valueList;

        mapList = settings::get_xmlMapping("nutritinfo");
        valueList = settings::get_xmlMapping("nutrivalues");

        for(QMap<QDate,QPair<QString,QVector<int>>>::const_iterator it = historyMap.cbegin(); it != historyMap.cend(); ++it)
        {
            objValue.insert(mapList->at(0),it.key().startOfDay().toSecsSinceEpoch());
            objValue.insert(mapList->at(1),it.value().first);
            objValue.insert(mapList->at(2),"");

            for(int value = 0; value < valueList->count(); ++value)
            {
                objValue.insert(valueList->at(value),it.value().second.at(value));
            }

            jsonArray.insert(counter++,objValue);
        }

        QJsonDocument jsonDoc;
        QJsonObject nutritionFile;

        nutritionFile[settings::get_xmlMapping("nutrition")->at(0)] = jsonArray;
        jsonDoc.setObject(nutritionFile);

        QFile file(gcValues->value("confpath") + QDir::separator() + gcValues->value("nutritionfile"));
        file.open(QFile::WriteOnly);
        file.write(jsonDoc.toJson(QJsonDocument::Compact));
        file.flush();
        file.close();
    }
}

void foodplanner::save_recipeList()
{
    this->treeModel_toXml(recipeModel,fileMap->value("recipefile"));
}

void foodplanner::save_ingredList(int list)
{
    if(list == 0) this->treeModel_toXml(ingredModel,fileMap->value("ingredfile"));
    if(list == 1) this->treeModel_toXml(drinkModel,fileMap->value("drinkfile"));
}

void foodplanner::update_foodMode(QDate weekStart,QString newMode)
{
    QStandardItem *weekItem;
    QModelIndex weekIndex = this->get_modelIndex(foodPlanModel,calc_weekID(weekStart),0);
    weekItem = foodPlanModel->itemFromIndex(weekIndex.siblingAtColumn(2));
    weekItem->setData(newMode,Qt::DisplayRole);
}

void foodplanner::add_foodSection(QString sectionName,int modelID)
{
    QList<QStandardItem*> sectionList;
    sectionList.insert(0,new QStandardItem(sectionName));
    sectionList.insert(1,new QStandardItem(modelMap.value(modelID)->rowCount()));
    modelMap.value(modelID)->invisibleRootItem()->appendRow(sectionList);
}

void foodplanner::insert_newWeek(QDate firstday)
{
    QStandardItem *rootItem = foodPlanModel->invisibleRootItem();

    QVector<double> mealDefault = settings::doubleVector.value("Mealdefault");

    QList<QStandardItem*> weekList;
    weekList << new QStandardItem(this->calc_weekID(firstday));
    weekList << new QStandardItem(firstday.toString(dateSaveFormat));
    weekList << new QStandardItem(settings::get_listValues("Mode").at(0));
    weekList.at(0)->setData(foodPlanTags->at(0),Qt::AccessibleTextRole);

    for(int day = 0; day < dayHeader.count(); ++day)
    {
        QList<QStandardItem*> dayItem;
        dayItem << new QStandardItem(firstday.addDays(day).toString(dateSaveFormat));
        dayItem << new QStandardItem(QString::number(day));
        dayItem.at(0)->setData(foodPlanTags->at(1),Qt::AccessibleTextRole);
        weekList.at(0)->appendRow(dayItem);

        for(int meals = 0; meals < mealsHeader.count(); ++meals)
        {
            QList<QStandardItem*> mealsItem;
            mealsItem << new QStandardItem(mealsHeader.at(meals));
            mealsItem << new QStandardItem(QString::number(meals));
            mealsItem.at(0)->setData(foodPlanTags->at(2),Qt::AccessibleTextRole);

            QList<QStandardItem*> foodItem;
            foodItem << new QStandardItem(mealsHeader.at(meals)+"-0");
            foodItem << new QStandardItem("Default");
            foodItem << new QStandardItem("1");
            foodItem << new QStandardItem(QString::number(mealDefault.at(meals)));
            foodItem << new QStandardItem("0");
            foodItem << new QStandardItem("0");
            foodItem << new QStandardItem("0");
            foodItem << new QStandardItem("0");
            foodItem << new QStandardItem("0");
            foodItem << new QStandardItem("0");
            foodItem << new QStandardItem("0");
            foodItem.at(0)->setData(foodPlanTags->at(3),Qt::AccessibleTextRole);
            mealsItem.at(0)->appendRow(foodItem);
            dayItem.at(0)->appendRow(mealsItem);
        }
    }
    rootItem->appendRow(weekList);
    this->set_summeryData(rootItem->child(rootItem->rowCount()-1,0));
}

void foodplanner::remove_week(QString weekID)
{
    QModelIndex weekIndex = foodPlanModel->indexFromItem(foodPlanModel->findItems(weekID.split(" - ").first(),Qt::MatchExactly,0).at(0));
    foodPlanModel->removeRow(weekIndex.row());
}
