#include "foodplanner.h"

foodplanner::foodplanner(schedule *p_Schedule)
{
    schedulePtr = p_Schedule;

    firstdayofweek = settings::firstDayofWeek;
    dateFormat = settings::get_format("dateformat");
    dateSaveFormat = "yyyy-MM-dd";
    foodPlanTags = settings::get_xmlMapping("foodplan");
    foodHistTags = settings::get_xmlMapping("histweeks");
    mealsTags = settings::get_xmlMapping("meals");

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

    mealModel = new QStandardItemModel();
    this->set_headerLabel(mealModel,menuHeader,false);

    foodPlanModel = new QStandardItemModel();
    historyModel = new QStandardItemModel();
    historyModel->setColumnCount(foodhistHeader->count());
    this->set_headerLabel(historyModel,foodhistHeader,false);

    if(!settings::getStringMapPointer(settings::stingMap::GC)->value("foodplanner").isEmpty())
    {
        this->xml_toTreeModel(fileMap->value("mealsfile"),mealModel);
        this->set_mealsMap();
        this->xml_toTreeModel(fileMap->value("foodfile"),foodPlanModel);
        this->xml_toTreeModel(fileMap->value("foodhistory"),historyModel);
        this->check_foodPlan();
    }
}

QHash<QString, QPair<QString, QPair<int, double>>>  foodplanner::get_mealtoUpdate(bool toUpdate,QDate updateDate, QString updateSection)
{
    QPair<bool,QDate> updateKey(toUpdate,updateDate);
    QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>> dayToUpdate = updateMap.value(updateKey);
    return dayToUpdate.value(updateSection);
}

enum {INIT,WEEK,DAY};
enum {ADD,DEL,EDIT};

bool foodplanner::updateMap_hasData()
{
    if(updateMap.count() > 0)
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

void foodplanner::set_mealsMap()
{
    QStandardItem *rootitem = mealModel->invisibleRootItem();
    QStandardItem *sectionItem;

    for(int section = 0; section < rootitem->rowCount(); ++section)
    {
        sectionItem = rootitem->child(section,0);
        if(sectionItem->hasChildren())
        {
            for(int meal = 0; meal < sectionItem->rowCount(); ++meal)
            {
                mealsMap.insert(sectionItem->child(meal,1)->data(Qt::DisplayRole).toString(),sectionItem->child(meal,0)->data(Qt::DisplayRole).toString());
            }
        }
    }
}

QVector<double> foodplanner::get_mealValues(QString mealId,double factor)
{
    QVector<double> mealValues(7,0);

    QModelIndex mealIndex = mealModel->indexFromItem(mealModel->findItems(mealId,Qt::MatchRecursive | Qt::MatchExactly,1).at(0));

    for(int value = 2; value < mealModel->columnCount(); ++value)
    {
        if(value > 2)
        {
            mealValues[value-2] = round(mealIndex.siblingAtColumn(value).data(Qt::DisplayRole).toInt()*(factor/mealIndex.siblingAtColumn(2).data(Qt::DisplayRole).toDouble()));
        }
        else
        {
            mealValues[value-2] = mealIndex.siblingAtColumn(value).data(Qt::DisplayRole).toInt()*(factor/mealIndex.siblingAtColumn(2).data(Qt::DisplayRole).toDouble());
        }

    }
    return mealValues;
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

void foodplanner::set_dayFoodValues(QStandardItem *dayItem)
{
    QStandardItem *mealItem;
    QHash<QString,QVector<double>> foodCal;
    QHash<QString,QVector<double>> foodMacros;
    QHash<QString,QHash<QString,QVector<double>>> mealCal;
    QVector<double> mealValues(7,0);
    QVector<double> macroValues(5,0);

    for(int meal = 0; meal < dayItem->rowCount(); ++meal)
    {
        mealItem = dayItem->child(meal,0);
        for(int food = 0; food < mealItem->rowCount(); ++food)
        {
            mealValues = this->get_mealValues(mealItem->child(food,0)->data(Qt::DisplayRole).toString(),mealItem->child(food,2)->data(Qt::DisplayRole).toDouble());
            mealValues.insert(0,mealItem->child(food,3)->data(Qt::DisplayRole).toDouble());
            foodCal.insert(mealItem->child(food,0)->data(Qt::DisplayRole).toString(),mealValues);
            for(int macro = 0; macro < macroValues.count(); ++macro)
            {
                macroValues[macro] = macroValues.at(macro) + mealValues.at(macro+3);
            }
        }
        mealCal.insert(mealItem->data(Qt::DisplayRole).toString(),foodCal);
        foodCal.clear();
    }
    foodMacros.insert("Day",macroValues);
    dayMacroMap.insert(dayItem->data(Qt::DisplayRole).toDate(),foodMacros);
    foodPlanMap.insert(dayItem->data(Qt::DisplayRole).toDate(),mealCal);
    this->set_daySumMap(dayItem->data(Qt::DisplayRole).toDate());
    mealCal.clear();
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

    for(int day = 0; day < weekItem->rowCount(); ++day)
    {
        this->set_dayFoodValues(weekItem->child(day,0));
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


void foodplanner::update_foodPlanModel()
{
    QStandardItem *dayItem,*mealItem;
    for(QMap<QPair<bool,QDate>,QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>>>::const_iterator daystart = updateMap.cbegin(), dayend = updateMap.cend(); daystart != dayend; ++daystart)
    {
        if(daystart.key().first)
        {
            dayItem = this->get_modelItem(foodPlanModel,daystart.key().second.toString(dateSaveFormat),0);
            for(QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>>::const_iterator mealstart = daystart.value().cbegin(), mealend = daystart.value().cend(); mealstart != mealend; ++mealstart)
            {
                mealItem = dayItem->child(mealsHeader.indexOf(mealstart.key()));
                foodPlanModel->removeRows(0,mealItem->rowCount(),mealItem->index());

                for(QHash<QString,QPair<QString,QPair<int,double>>>::const_iterator foodstart = mealstart.value().cbegin(), foodend = mealstart.value().cend(); foodstart != foodend; ++foodstart)
                {
                    QList<QStandardItem*> foodList;
                    foodList << new QStandardItem(foodstart.key());
                    foodList << new QStandardItem(foodstart.value().first);
                    foodList << new QStandardItem(QString::number(foodstart.value().second.second));
                    foodList << new QStandardItem(QString::number(foodstart.value().second.first));
                    foodList.at(0)->setData(foodPlanTags->at(3),Qt::AccessibleTextRole);

                    mealItem->appendRow(foodList);
                }
            }
            this->set_dayFoodValues(dayItem);
        }
    }
    updateMap.clear();
}


void foodplanner::update_foodPlanData(bool singleItem,QDate copyFromDate,QDate copyToDate)
{
    QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>> updateMealSection;
    QPair<bool,QDate> copyKey(singleItem,copyFromDate);

    if(singleItem && copyToDate.isValid())
    {
         QPair<bool,QDate> copyToKey(singleItem,copyToDate);

         if(!updateMap.contains(copyToKey))
         {
             updateMealSection = updateMap.value(copyKey);
             updateMap.insert(qMakePair(true,copyToDate),updateMealSection);
         }
    }
    else
    {
        if(dayMealCopy.first)
        {
            updateMealSection = updateMap.value(copyKey);
            updateMap.insert(qMakePair(true,copyToDate),updateMealSection);
        }
        else if(dayMealCopy.second)
        {
            int day = 0;
            for(QMap<QPair<bool,QDate>,QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>>>::const_iterator it = updateMap.cbegin(), end = updateMap.cend(); it != end; ++it)
            {
                if(!it.key().first)
                {
                    updateMealSection = it.value();
                    updateMap.insert(qMakePair(true,copyToDate.addDays(day++)),updateMealSection);
                }
            }
        }
    }
    this->update_foodPlanModel();
}

void foodplanner::fill_updateMap(bool singleItem,bool dragDrop,QDate updateDate, QString updateSection)
{
    QPair<bool,QDate> copyKey(singleItem,updateDate);
    QPair<int,double> posPort;
    QPair<QString,QPair<int,double>> updateFood;
    QHash<QString,QPair<QString,QPair<int,double>>> updateMeal;

    QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>> updateMealSection = updateMap.value(copyKey);

    if(singleItem & !dragDrop) updateMap.clear();

    if(dragDrop)
    {
        updateMealSection.insert(updateSection,updateMap.first().value(updateMap.first().keys().first()));
    }
    else
    {
        QHash<QString,QVector<double>> currentMeal = foodPlanMap.value(updateDate).value(updateSection);

        for(QHash<QString,QVector<double>>::const_iterator it = currentMeal.cbegin(), end = currentMeal.cend(); it != end; ++it)
        {
            updateFood.first = mealsMap.value(it.key());
            posPort.first = static_cast<int>(it.value().at(0));
            posPort.second = it.value().at(1);
            updateFood.second = posPort;
            updateMeal.insert(it.key(),updateFood);
        }
        updateMealSection.insert(updateSection,updateMeal);
    }

    updateMap.insert(copyKey,updateMealSection);
}

void foodplanner::change_updateMapOrder(QPair<QDate,QString> dateSection, QMap<QString, int> changeTo)
{
    QPair<bool,QDate> updateKey(true,dateSection.first);
    QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>> editMealSection = updateMap.value(updateKey);
    QHash<QString,QPair<QString,QPair<int,double>>> editMeal = editMealSection.value(dateSection.second);
    QPair<QString,QPair<int,double>> editFood;

    for(QMap<QString, int>::const_iterator it = changeTo.cbegin(), end = changeTo.cend(); it != end; ++it)
    {
        editFood = editMeal.value(it.key());
        editFood.second.first = it.value();
        editMeal.insert(it.key(),editFood);
    }
    editMealSection.insert(dateSection.second,editMeal);
    updateMap.insert(updateKey,editMealSection);
}

void foodplanner::edit_updateMap(int foodEdit,QPair<QDate,QString> dateSection,QString mealID, double factor)
{
    QPair<bool,QDate> updateKey(true,dateSection.first);
    QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>> editMealSection = updateMap.value(updateKey);
    QHash<QString,QPair<QString,QPair<int,double>>> editMeal = editMealSection.value(dateSection.second);
    QPair<QString,QPair<int,double>> editFood;

    if(foodEdit == ADD)
    {
        editFood.first = mealsMap.value(mealID);
        editFood.second.first = editMeal.count();
        editFood.second.second = factor;
        editMeal.insert(mealID,editFood);
    }
    else if(foodEdit == DEL)
    {
        editMeal.remove(mealID);
    }
    else if(foodEdit == EDIT)
    {
        editFood = editMeal.value(mealID);
        editFood.second.second = factor;
        editMeal.insert(mealID,editFood);
    }

    editMealSection.insert(dateSection.second,editMeal);
    updateMap.insert(updateKey,editMealSection);
}

void foodplanner::set_daySumMap(QDate day)
{
    QVector<double> values(5,0); 

    values[1] = round(this->current_dayCalories(QDateTime(day)) * athleteValues->value("currpal"));

    QMap<QString,QVector<double>> sportValues = schedulePtr->get_compValues()->value(day);
    QHash<QString,QHash<QString,QVector<double>>> foodValues = foodPlanMap.value(day);

    for(QHash<QString,QHash<QString,QVector<double>> >::const_iterator it = foodValues.cbegin(), end = foodValues.cend(); it != end; ++it)
    {
        for(QHash<QString,QVector<double>>::const_iterator vit = it.value().cbegin(), vend = it.value().cend(); vit != vend; ++vit)
        {
            values[0] = values.at(0) + vit.value().at(2);
        }
    }

    for(QMap<QString,QVector<double>>::const_iterator it = sportValues.cbegin(), end = sportValues.cend(); it != end; ++it)
    {
        values[2] = values.at(2) + it.value().at(5);
    }

    values[3] = values.at(1) + values.at(2);
    values[4] = values.at(3) - values.at(0);

    daySumMap.insert(day,values);

    if(day > firstdayofweek.addDays(-1) && day < firstdayofweek.addDays(7)) this->update_foodHistory(day);

    //Set Target Macro Values
    QHash<QString,QVector<double>> foodMacros = dayMacroMap.value(day);
    QVector<double> macroValues(5,0);

    macroValues[0] = round(values.at(3) * (settings::doubleVector.value("Macros").at(0) / 100.0) / 4.1);
    macroValues[1] = round(values.at(3) * (settings::doubleVector.value("Macros").at(1) / 100.0) / 4.1);
    macroValues[2] = round(values.at(3) * (settings::doubleVector.value("Macros").at(2) / 100.0) / 9.3);
    macroValues[3] = ceil(athleteValues->value("weight") * (doubleValues->value("DayFiber") /100.0));
    macroValues[4] = round(values.at(3) * (doubleValues->value("DaySugar") / 100.0) / 4.1);

    foodMacros.insert("Target",macroValues);
    dayMacroMap.insert(day,foodMacros);
    this->set_weekSumMap();
}

void foodplanner::set_weekSumMap()
{
    QVector<double> weekValues(6,0);
    QModelIndex weekIndex;
    QStandardItem *weekItem = nullptr;

    double currentWeight = settings::get_weightforDate(QDateTime(firstdayofweek));

    for(QMap<QDate,QVector<QString>>::const_iterator week = foodPlanList.cbegin(); week != foodPlanList.cend(); ++week)
    {
        weekIndex = this->get_modelIndex(foodPlanModel,calc_weekID(week.key()),0);
        weekItem = foodPlanModel->itemFromIndex(weekIndex.siblingAtColumn(3));
        weekItem->setData(currentWeight,Qt::DisplayRole);

        for(QMap<QDate,QVector<double>>::const_iterator day = daySumMap.find(week.key()); day != daySumMap.find(week.key().addDays(7)); ++day)
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

    //Compare FoodPlan to Meals
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

    if(!historyModel->findItems(firstdayofweek.toString(dateFormat),Qt::MatchExactly | Qt::MatchRecursive,2).count())
    {
        QList<QStandardItem*> weekList;
        QStandardItem *rootItem = historyModel->invisibleRootItem();

        weekList << new QStandardItem(QString::number(firstdayofweek.weekNumber()));
        weekList << new QStandardItem(QString::number(firstdayofweek.year()));
        weekList << new QStandardItem(firstdayofweek.toString(dateFormat));
        weekList << new QStandardItem(QString::number(settings::get_weightforDate(QDateTime(firstdayofweek))));
        weekList << new QStandardItem(QString::number(round(this->current_dayCalories(QDateTime(firstdayofweek)) * athleteValues->value("currpal"))));
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

void foodplanner::save_mealList()
{
    this->treeModel_toXml(mealModel,fileMap->value("mealsfile"));
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

void foodplanner::edit_mealSection(QString sectionName,int mode)
{
    if(mode == ADD)
    {
        QStandardItem *item = new QStandardItem(sectionName);
        mealModel->invisibleRootItem()->appendRow(item);
    }
    if(mode == DEL)
    {
        QList<QStandardItem*> mealSection = mealModel->findItems(sectionName,Qt::MatchExactly,0);
        if(!mealSection.isEmpty())
        {
            QModelIndex sectionIndex = mealModel->indexFromItem(mealSection.at(0));

            if(mealSection.at(0)->hasChildren())
            {
                mealModel->removeRows(0,mealSection.at(0)->rowCount(),sectionIndex);
            }
        }
    }
    if(mode == EDIT)
    {
        QList<QStandardItem*> mealSection = mealModel->findItems(sectionName,Qt::MatchExactly,0);
        mealSection.at(0)->setData(sectionName,Qt::EditRole);
    }

}

void foodplanner::remove_meal(QItemSelectionModel *mealSelect)
{
    QModelIndex index = mealSelect->currentIndex();
    mealModel->removeRow(index.row(),index.parent());
}

QPair<QString,QVector<int>> foodplanner::get_mealData(QString mealID)
{
    QPair<QString,QVector<int>> mealData;
    QVector<int> mealValues(7,0);
    QStandardItem *mealItem = mealModel->findItems(mealID,Qt::MatchExactly | Qt::MatchRecursive,1).at(0);
    QModelIndex mealIndex = mealModel->indexFromItem(mealItem);
    mealData.first = mealIndex.siblingAtColumn(0).data(Qt::DisplayRole).toString();

    for(int col = 2; col < mealModel->columnCount(); ++col)
    {
        mealValues[col-2] = mealIndex.siblingAtColumn(col).data(Qt::DisplayRole).toInt();
    }
    mealData.second = mealValues;
    return mealData;
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
            foodItem << new QStandardItem(mealsMap.value(mealDefault));
            foodItem << new QStandardItem(QString::number(get_mealData(mealDefault).second.at(0)));
            foodItem << new QStandardItem("0");
            foodItem.at(0)->setData(foodPlanTags->at(3),Qt::AccessibleTextRole);
            mealsItem.at(0)->appendRow(foodItem);
            dayItem.at(0)->appendRow(mealsItem);
        }
    }
    rootItem->appendRow(weekList);
    this->set_foodPlanMap(WEEK);
}

void foodplanner::insert_newMeal(QString mealSection)
{
    QStandardItem *sectionItem = mealModel->findItems(mealSection,Qt::MatchExactly,0).at(0);
    QList<QStandardItem*> mealValues;

    mealValues << new QStandardItem("NewMeal");
    mealValues << new QStandardItem(get_modelIndex(mealModel,mealSection,0).siblingAtColumn(1).data(Qt::DisplayRole).toString()+"-"+QString::number(sectionItem->rowCount()+1));
    mealValues << new QStandardItem("100");
    mealValues << new QStandardItem("0");
    mealValues << new QStandardItem("0");
    mealValues << new QStandardItem("0");
    mealValues << new QStandardItem("0");
    mealValues << new QStandardItem("0");
    mealValues << new QStandardItem("0");
    mealValues.at(0)->setData(mealsTags->at(1),Qt::AccessibleTextRole);
    sectionItem->appendRow(mealValues);
}

void foodplanner::remove_week(QString weekID)
{
    QModelIndex weekIndex = foodPlanModel->indexFromItem(foodPlanModel->findItems(weekID.split(" - ").first(),Qt::MatchExactly,0).at(0));
    foodPlanModel->removeRow(weekIndex.row());
}
