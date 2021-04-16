#include "foodplanner.h"

foodplanner::foodplanner(schedule *p_Schedule)
{
    schedulePtr = p_Schedule;

    firstdayofweek = settings::firstDayofWeek;
    dateFormat = settings::get_format("dateformat");
    dateSaveFormat = "yyyy-MM-dd";
    foodPlanTags = settings::get_xmlMapping("foodplan");
    foodHistTags = settings::get_xmlMapping("histweeks");
    drinkTags = settings::get_xmlMapping("drinks");
    ingredTags = settings::get_xmlMapping("ingredients");

    menuHeader = settings::getHeaderMap("menuheader");
    foodsumHeader = settings::getHeaderMap("foodsumheader");
    foodestHeader = settings::getHeaderMap("foodestheader");
    foodhistHeader = settings::getHeaderMap("foodhistheader");
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
    ingredModel = new QStandardItemModel();
    recipeModel = new QStandardItemModel();
    drinkModel = new QStandardItemModel();
    historyModel = new QStandardItemModel();
    historyModel->setColumnCount(foodhistHeader->count());
    this->set_headerLabel(historyModel,foodhistHeader,false);

    if(!settings::getStringMapPointer(settings::stingMap::GC)->value("foodplanner").isEmpty())
    {
        this->xml_toTreeModel(fileMap->value("recipefile"),recipeModel);
        this->xml_toTreeModel(fileMap->value("ingredfile"),ingredModel);
        this->xml_toTreeModel(fileMap->value("drinkfile"),drinkModel);
        this->xml_toTreeModel(fileMap->value("foodfile"),foodPlanModel);
        this->xml_toTreeModel(fileMap->value("foodhistory"),historyModel);
        this->check_foodPlan();
    }
}

enum {INIT,WEEK,DAY};
enum {ADD,DEL,EDIT};

bool foodplanner::copyMap_hasData()
{
    if(copyMap.count() > 0)
    {
        return true;
    }

    return false;
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

QPair<QDate, QString> foodplanner::get_mealToCopy()
{
    QPair<QDate,QString> mealCopy;
    mealCopy.first = copyMap.firstKey();
    mealCopy.second = copyMap.value(copyMap.firstKey()).at(0);
    return mealCopy;
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

void foodplanner::set_foodHistoryValues()
{
    QStandardItem *rootItem = historyModel->invisibleRootItem();
    QStandardItem *weekItem;
    QPair<QDate,int> foodKey;
    QMap<QDate,QList<QVariant>> foodHistMap;
    QList<QVariant> foodValues;
    QDate histDate;
    int baseCal = 0,sportCal = 0,foodCal = 0;

    for(int week = 0; week < historyModel->rowCount(); ++week)
    {
        weekItem = rootItem->child(week,0);
        histDate = QDate::fromString(rootItem->child(week,2)->data(Qt::DisplayRole).toString(),dateFormat);

        foodKey.first = histDate;
        foodKey.second = rootItem->child(week,0)->data(Qt::DisplayRole).toInt();

        baseCal = rootItem->child(week,4)->data(Qt::DisplayRole).toInt();

        for(int day = 0; day < weekItem->rowCount(); ++day)
        {
            sportCal = weekItem->child(day,1)->data(Qt::DisplayRole).toInt();
            foodCal = weekItem->child(day,2)->data(Qt::DisplayRole).toInt();
            foodValues.append(baseCal);
            foodValues.append(foodCal);
            foodValues.append(sportCal);
            foodValues.append(baseCal + sportCal);
            foodValues.append(foodCal - (baseCal + sportCal));
            foodHistMap.insert(histDate.addDays(day),foodValues);
            foodValues.clear();
        }
        foodHistoryMap.insert(foodKey,foodHistMap);
        foodHistMap.clear();
    }
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

void foodplanner::set_foodPlanData(QStandardItem *weekItem)
{
    //Fill FoodPlans List
    QModelIndex weekIndex = foodPlanModel->indexFromItem(weekItem);
    QVector<QString> weekInfo(4);
    weekInfo[0] = weekIndex.siblingAtColumn(0).data(Qt::DisplayRole).toString();
    weekInfo[1] = weekIndex.siblingAtColumn(1).data(Qt::DisplayRole).toDate().toString(dateFormat);
    weekInfo[2] = weekIndex.siblingAtColumn(2).data(Qt::DisplayRole).toString();
    weekInfo[3] = weekIndex.siblingAtColumn(3).data(Qt::DisplayRole).toString();

    foodPlanList.insert(weekIndex.siblingAtColumn(1).data(Qt::DisplayRole).toDate(),weekInfo);

    QStandardItem *dayItem,*sectionItem;
    QHash<QString,QVector<double>> foodMacros;
    QVector<int> mealValues(7,0);
    QVector<double> macroValues(5,0);

    for(int day = 0; day < weekItem->rowCount(); ++day)
    {
        dayItem = weekItem->child(day,0);

        for(int section = 0; section < dayItem->rowCount(); ++section)
        {
            sectionItem = dayItem->child(section,0);

            for(int food = 0; food < sectionItem->rowCount(); ++food)
            {
                for(int value = 3,pos = 0; value < 9; ++value,++pos)
                {
                    mealValues[pos] = mealValues.at(pos) + sectionItem->child(food,value)->data(Qt::DisplayRole).toInt();
                }

                mealValues.insert(0,sectionItem->child(food,3)->data(Qt::DisplayRole).toDouble());

                for(int macro = 0; macro < macroValues.count(); ++macro)
                {
                    macroValues[macro] = macroValues.at(macro) + mealValues.at(macro+3);
                }
            }
        }
        foodMacros.insert("Day",macroValues);
        dayMacroMap.insert(dayItem->data(Qt::DisplayRole).toDate(),foodMacros);
        //foodPlanMap.insert(dayItem->data(Qt::DisplayRole).toDate(),mealCal);

        this->set_daySumMap(dayItem->data(Qt::DisplayRole).toDate());
    }
}

void foodplanner::update_foodHistory(QDate day)
{
    QModelIndex weekIndex = historyModel->indexFromItem(historyModel->findItems(firstdayofweek.toString(dateFormat),Qt::MatchExactly | Qt::MatchRecursive,2).at(0));

    QPair<QDate,int> foodHistKey(firstdayofweek,day.weekNumber());
    QMap<QDate,QList<QVariant>> foodHistMap = foodHistoryMap.value(foodHistKey);
    QList<QVariant> foodValues = foodHistMap.value(day);

    foodValues[0] = daySumMap.value(day).at(1);
    foodValues[1] = daySumMap.value(day).at(0);
    foodValues[2] = daySumMap.value(day).at(2);
    foodValues[3] = daySumMap.value(day).at(3);
    foodValues[4] = daySumMap.value(day).at(4)*-1;

    foodHistMap.insert(day,foodValues);
    foodHistoryMap.insert(foodHistKey,foodHistMap);

    QStandardItem *weekItem = historyModel->itemFromIndex(weekIndex.siblingAtColumn(0));
    weekItem->child(day.dayOfWeek()-1,1)->setData(daySumMap.value(day).at(2),Qt::DisplayRole);
    weekItem->child(day.dayOfWeek()-1,2)->setData(daySumMap.value(day).at(0),Qt::DisplayRole);
}

void foodplanner::update_foodHistory(QDate day, QVector<double> dayValues)
{
    QDate firstDay = day.addDays(1 - day.dayOfWeek());

    QModelIndex weekIndex = historyModel->indexFromItem(historyModel->findItems(firstDay.toString(dateFormat),Qt::MatchExactly | Qt::MatchRecursive,2).at(0));

    QPair<QDate,int> foodHistKey(firstDay,day.weekNumber());
    QMap<QDate,QList<QVariant>> foodHistMap = foodHistoryMap.value(foodHistKey);
    QList<QVariant> foodValues = foodHistMap.value(day);

    foodValues[0] = dayValues.at(0);
    foodValues[1] = dayValues.at(1);
    foodValues[2] = dayValues.at(2);
    foodValues[3] = dayValues.at(3);
    foodValues[4] = dayValues.at(4)*-1;

    foodHistMap.insert(day,foodValues);
    foodHistoryMap.insert(foodHistKey,foodHistMap);

    QStandardItem *weekItem = historyModel->itemFromIndex(weekIndex.siblingAtColumn(0));
    weekItem->child(day.dayOfWeek()-1,1)->setData(dayValues.at(2),Qt::DisplayRole);
    weekItem->child(day.dayOfWeek()-1,2)->setData(dayValues.at(1),Qt::DisplayRole);
}

void foodplanner::update_foodPlanModel(QDate mealDate, QString mealSection, QMap<int,QList<QStandardItem *>> mealMap)
{
    QStandardItem *dayItem,*mealItem;
    dayItem = this->get_modelItem(foodPlanModel,mealDate.toString(dateSaveFormat),0);
    mealItem = dayItem->child(mealsHeader.indexOf(mealSection),0);

    foodPlanModel->removeRows(0,mealItem->rowCount(),mealItem->index());

    for(QMap<int,QList<QStandardItem *>>::const_iterator mealStart = mealMap.cbegin(); mealStart != mealMap.cend(); ++mealStart)
    {
        mealStart.value().first()->setData(foodPlanTags->at(3),Qt::AccessibleTextRole);
        mealItem->appendRow(mealStart.value());
    }

    this->set_daySumMap(mealDate);

    //this->set_dayFoodValues(dayItem);
}

void foodplanner::set_foodPlanMap(int update)
{
    QStandardItem *rootItem = foodPlanModel->invisibleRootItem();

    if(update == INIT)
    {
        for(int week = 0; week < rootItem->rowCount(); ++week)
        {
            this->set_foodPlanData(rootItem->child(week,0));
        }
    }
    else if(update == WEEK)
    {
        this->set_foodPlanData(rootItem->child(rootItem->rowCount()-1,0));
    }
}

void foodplanner::add_ingredient(QString section, QString foodName,QVector<double> values)
{
    QStandardItem *sectionItem = this->get_modelItem(ingredModel,section,0);
    QList<QStandardItem*> itemList;
    QStandardItem *item;

    itemList.append(new QStandardItem(section+"-"+QString::number(sectionItem->rowCount())));
    itemList.append(new QStandardItem(foodName));

    for(int i = 0; i < values.count(); ++i)
    {
        item = new QStandardItem(QString::number(values.at(i)));
        itemList.append(item);
    }
    itemList.at(0)->setData(ingredTags->at(1),Qt::AccessibleTextRole);
    sectionItem->appendRow(itemList);
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

void foodplanner::set_daySumMap(QDate day)
{
    const int minutes = 1440;
    double calMinute = round(this->current_dayCalories(day.startOfDay()) * settings::doubleVector.value("palday").at(day.dayOfWeek()-1)) / minutes;

    double sportMinutes = 0;
    QMap<QString,QVector<double>> sportValues = schedulePtr->get_compValues()->value(day);
    QVector<double> values(5,0);


    QStandardItem *dayItem,*sectionItem;
    dayItem = this->get_modelItem(foodPlanModel,day.toString(dateSaveFormat),0);

    for(int section = 0; section < dayItem->rowCount(); ++section)
    {
        sectionItem = dayItem->child(section);

        if(sectionItem->hasChildren())
        {
            for(int meal = 0; meal < sectionItem->rowCount(); ++meal)
            {
                values[0] = values.at(0) + sectionItem->child(meal,3)->data(Qt::DisplayRole).toDouble();
            }
        }
    }

    QHash<QString,QVector<double>> foodMacros = dayMacroMap.value(day);
    QVector<double> macroValues(5,0);

    for(QMap<QString,QVector<double>>::const_iterator it = sportValues.cbegin(), end = sportValues.cend(); it != end; ++it)
    {
        values[2] = values.at(2) + it.value().at(5);
        sportMinutes = sportMinutes + (it.value().at(1)/60.0);
    }

    values[1] = ceil((minutes - sportMinutes) * calMinute);
    values[3] = values.at(1) + values.at(2);
    values[4] = values.at(3) - values.at(0);

    macroValues[0] = round(values.at(3) * (settings::doubleVector.value("Macros").at(0) / 100.0) / 4.1);
    macroValues[1] = round(values.at(3) * (settings::doubleVector.value("Macros").at(1) / 100.0) / 4.1);
    macroValues[2] = round(values.at(3) * (settings::doubleVector.value("Macros").at(2) / 100.0) / 9.3);
    macroValues[3] = ceil(athleteValues->value("weight") * (doubleValues->value("DayFiber") /100.0));
    macroValues[4] = ceil(athleteValues->value("weight") * (doubleValues->value("DaySugar")));

    daySumMap.insert(day,values);
    foodMacros.insert("Target",macroValues);
    dayMacroMap.insert(day,foodMacros);
    this->set_weekSumMap();
}

void foodplanner::set_weekSumMap()
{
    QVector<double> weekValues(6,0);
    QModelIndex weekIndex;
    QStandardItem *weekItem = nullptr;

    double currentWeight = settings::get_weightforDate(firstdayofweek.startOfDay());

    for(QMap<QDate,QVector<QString>>::const_iterator week = foodPlanList.cbegin(); week != foodPlanList.cend(); ++week)
    {
        weekIndex = this->get_modelIndex(foodPlanModel,calc_weekID(week.key()),0);
        weekItem = foodPlanModel->itemFromIndex(weekIndex.siblingAtColumn(3));
        weekItem->setData(currentWeight,Qt::DisplayRole);

        for(QMap<QDate,QVector<double>>::iterator day = daySumMap.find(week.key()); day != daySumMap.find(week.key().addDays(7)); ++day)
        {
            weekValues[1] = weekValues.at(1) + day.value().at(0);
            weekValues[2] = weekValues.at(2) + day.value().at(3);
            weekValues[3] = weekValues.at(3) + day.value().at(4);
        }

        weekValues[0] = currentWeight;
        weekValues[1] = round(weekValues.at(1) / 7);
        weekValues[2] = round(weekValues.at(2) / 7);
        weekValues[4] = set_doubleValue(weekValues.at(3) / athleteValues->value("BodyFatCal") / 1000.0,true)*-1;
        weekValues[3] = round((weekValues.at(3) / 7)*-1);
        weekValues[5] = set_doubleValue(currentWeight + weekValues.at(4),false);
        currentWeight = weekValues.at(5);

        weekSumMap.insert(week.key(),weekValues);
        weekValues.fill(0);
    }
}

void foodplanner::fill_copyMap(QDate copyDate, QString copySection)
{
    QStringList sectionList;

    if(copyMap.contains(copyDate))
    {
        sectionList = copyMap.value(copyDate);
    }

    sectionList.append(copySection);
    copyMap.insert(copyDate,sectionList);
}

void foodplanner::execute_copy(QDate copyDate)
{
    QStandardItem *dayItem,*sectionItem;
    QMap<int,QList<QStandardItem *>> mealMap;
    QList<QStandardItem *> itemList;
    int dayCount = 0;

    for(QMap<QDate,QStringList>::const_iterator it = copyMap.cbegin(); it != copyMap.cend(); ++it)
    {
        dayItem = this->get_modelItem(foodPlanModel,it.key().toString(dateSaveFormat),0);

        for(int section = 0; section < it.value().count(); ++section)
        {
            sectionItem = dayItem->child(section,0);

            for(int meal = 0; meal < sectionItem->rowCount(); ++meal)
            {
                for(int value = 0; value < sectionItem->columnCount(); ++value)
                {
                    itemList.insert(value,new QStandardItem(sectionItem->child(meal,value)->data(Qt::DisplayRole).toString()));
                }
                mealMap.insert(meal,itemList);
                itemList.clear();
            }
            this->update_foodPlanModel(copyDate.addDays(dayCount),it.value().at(section),mealMap);
        }
        ++dayCount;
    }
}

void foodplanner::check_foodPlan()
{
    QStandardItem *rootItem = foodPlanModel->invisibleRootItem();
    QStandardItem *weekItem,*dayItem,*mealItem;
    QList<QStandardItem*> weekList;

    //check Past Weeks
    for(int week = 0; week < rootItem->rowCount(); ++week)
    {
        weekItem = rootItem->child(week,0);
        if(weekItem->index().siblingAtColumn(1).data().toDate() < firstdayofweek)
        {
            weekList << weekItem;
        }
    }

    for(int i = 0; i < weekList.count(); ++i)
    {
        foodPlanModel->removeRows(0,weekList.at(i)->rowCount(),weekList.at(i)->index());
        foodPlanModel->removeRow(weekList.at(i)->index().row(),weekList.at(i)->index().parent());
    }

    /* Compare FoodPlan to Meals
    for(int week = 0; week < rootItem->rowCount(); ++week)
    {
        weekItem = rootItem->child(week,0);
        for(int day = 0; day < weekItem->rowCount(); ++day)
        {
            dayItem = weekItem->child(day,0);
            for(int meal = 0; meal < dayItem->rowCount(); ++meal)
            {
                mealItem = dayItem->child(meal,0);
                for(int food = 0; food < mealItem->rowCount(); ++food)
                {
                    if(mealItem->child(food,0)->data(Qt::DisplayRole).toString().compare(mealsMap.key(mealItem->child(food,1)->text())) != 0)
                    {
                        mealItem->child(food,0)->setData(mealsMap.key(mealItem->child(food,1)->text()),Qt::EditRole);
                    }
                    else
                    {
                        if(mealItem->child(food,1)->data(Qt::DisplayRole).toString().compare(mealsMap.value(mealItem->child(food,0)->text())) != 0)
                        {
                            mealItem->child(food,1)->setData(mealsMap.value(mealItem->child(food,0)->text()),Qt::EditRole);
                        }
                    }
                }
            }
        }
    }
    */
    if(!historyModel->findItems(firstdayofweek.toString(dateFormat),Qt::MatchExactly | Qt::MatchRecursive,2).count())
    {
        QList<QStandardItem*> weekList;
        QStandardItem *rootItem = historyModel->invisibleRootItem();

        weekList << new QStandardItem(QString::number(firstdayofweek.weekNumber()));
        weekList << new QStandardItem(QString::number(firstdayofweek.year()));
        weekList << new QStandardItem(firstdayofweek.toString(dateFormat));
        weekList << new QStandardItem(QString::number(settings::get_weightforDate(firstdayofweek.startOfDay())));
        weekList << new QStandardItem(QString::number(round(this->current_dayCalories(firstdayofweek.startOfDay()) * athleteValues->value("currpal"))));
        weekList << new QStandardItem(foodPlanModel->data(foodPlanModel->index(0,2)).toString());
        weekList.at(0)->setData(foodHistTags->at(0),Qt::AccessibleTextRole);
        rootItem->appendRow(weekList);

        QList<QStandardItem*> dayValueList;

        for(int i = 0; i < dayHeader.count(); ++i)
        {
            dayValueList << new QStandardItem(QString::number(i));
            dayValueList << new QStandardItem(QString::number(0));
            dayValueList << new QStandardItem(QString::number(0));
            dayValueList.at(0)->setData(foodHistTags->at(1),Qt::AccessibleTextRole);
            weekList.at(0)->appendRow(dayValueList);
            dayValueList.clear();
        }
    }

    this->set_foodHistoryValues();
    this->set_foodPlanMap(INIT);
}

void foodplanner::save_foolPlan()
{
    this->treeModel_toXml(foodPlanModel,fileMap->value("foodfile"));
    this->treeModel_toXml(historyModel,fileMap->value("foodhistory"));
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

    QVector<QString> weekInfo = foodPlanList.value(weekStart);
    weekInfo[2] = newMode;
    foodPlanList.insert(weekStart,weekInfo);

    if(weekStart == firstdayofweek)
    {
        weekIndex = this->get_modelIndex(historyModel,weekStart.toString(dateFormat),2);
        weekItem = historyModel->itemFromIndex(weekIndex.siblingAtColumn(5));
        weekItem->setData(newMode,Qt::DisplayRole);
    }
}

void foodplanner::update_ingredient(QString ingredID,QString name,QVector<double> values)
{
    QModelIndex index = this->get_modelIndex(ingredModel,ingredID,0);
    QStandardItem *sectionItem = ingredModel->itemFromIndex(index)->parent();

    sectionItem->child(index.row(),1)->setData(name,Qt::DisplayRole);

    for(int i = 0,item=2; i < values.count(); ++i,++item)
    {
        sectionItem->child(index.row(),item)->setData(values.at(i),Qt::DisplayRole);
    }
}

void foodplanner::edit_mealSection(QString sectionName,int mode)
{
    qDebug() << sectionName << mode;

}

QMap<QDate, double> foodplanner::get_lastFoodWeek(QDate firstDay)
{
    QMap<QDate,double> dayValues;

    QStandardItem *weekItem = historyModel->findItems(firstDay.toString(dateFormat),Qt::MatchExactly | Qt::MatchRecursive,2).at(0);
    QModelIndex weekIndex = historyModel->indexFromItem(weekItem).siblingAtColumn(0);

    weekItem = historyModel->itemFromIndex(weekIndex);
    int baseCal = weekIndex.siblingAtColumn(4).data(Qt::DisplayRole).toInt();

    for(int i = 0; i < 7;++i)
    {
        dayValues.insert(firstDay.addDays(i),(weekItem->child(i,1)->data(Qt::DisplayRole).toInt()+baseCal) - weekItem->child(i,2)->data(Qt::DisplayRole).toInt());
    }

    return dayValues;
}

void foodplanner::insert_newWeek(QDate firstday)
{
    QStandardItem *rootItem = foodPlanModel->invisibleRootItem();
    QString mealDefault;

    QList<QStandardItem*> weekList;
    weekList << new QStandardItem(this->calc_weekID(firstday));
    weekList << new QStandardItem(firstday.toString(dateSaveFormat));
    weekList << new QStandardItem(settings::get_listValues("Mode").at(0));
    weekList << new QStandardItem(rootItem->child(rootItem->rowCount()-1,3)->data(Qt::DisplayRole).toString());
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
            mealDefault = "100-"+QString::number(meals);

            QList<QStandardItem*> foodItem;
            foodItem << new QStandardItem(mealDefault);
            foodItem << new QStandardItem("-");
            foodItem << new QStandardItem("0");
            foodItem << new QStandardItem("0");
            foodItem.at(0)->setData(foodPlanTags->at(3),Qt::AccessibleTextRole);
            mealsItem.at(0)->appendRow(foodItem);
            dayItem.at(0)->appendRow(mealsItem);
        }
    }
    rootItem->appendRow(weekList);
    this->set_foodPlanMap(WEEK);
}

void foodplanner::remove_week(QString weekID)
{
    QModelIndex weekIndex = foodPlanModel->indexFromItem(foodPlanModel->findItems(weekID.split(" - ").first(),Qt::MatchExactly,0).at(0));
    foodPlanModel->removeRow(weekIndex.row());
}
