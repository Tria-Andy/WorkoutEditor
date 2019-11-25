#include "foodplanner.h"

foodplanner::foodplanner(schedule *p_Schedule)
{
    schedulePtr = p_Schedule;

    firstdayofweek = settings::firstDayofWeek;
    dateFormat = settings::get_format("dateformat");
    dateSaveFormat = "yyyy-MM-dd";
    menuHeader = settings::getHeaderMap("menuheader");
    foodsumHeader = settings::getHeaderMap("foodsumheader");
    foodestHeader = settings::getHeaderMap("foodestheader");
    foodhistHeader = settings::getHeaderMap("foodhistheader");
    mealsHeader = settings::get_listValues("Meals");
    fileMap = settings::getStringMapPointer(settings::stingMap::File);

    dayListHeader << "Cal (%)"
                  << "Carbs ("+QString::number(settings::doubleVector.value("Macros").at(0))+"%)"
                  << "Protein ("+QString::number(settings::doubleVector.value("Macros").at(1))+"%)"
                  << "Fat ("+QString::number(settings::doubleVector.value("Macros").at(2))+"%)";

    for(int d = 1; d < 8; ++d)
    {
        dayHeader << QLocale().dayName(d);
    }

    mealModel = new QStandardItemModel();
    mealModel->setColumnCount(menuHeader->count());
    this->set_headerLabel(mealModel,menuHeader,false);

    foodPlanModel = new QStandardItemModel();
    foodPlanModel->setColumnCount(settings::getHeaderMap("fooditem")->count());

    historyModel = new QStandardItemModel();
    historyModel->setColumnCount(foodhistHeader->count());
    this->set_headerLabel(historyModel,foodhistHeader,false);

    if(!settings::getStringMapPointer(settings::stingMap::GC)->value("foodplanner").isEmpty())
    {
        this->fill_treeModel(fileMap->value("mealsfile"),mealModel);
        this->set_mealsMap();
        this->fill_treeModel(fileMap->value("foodfile"),foodPlanModel);
        this->fill_treeModel(fileMap->value("foodhistory"),historyModel);
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
    QModelIndex weekIndex = foodPlanModel->indexFromItem(foodPlanModel->findItems(calc_weekID(startDate),Qt::MatchExactly,0).at(0));
    return foodPlanModel->data(foodPlanModel->index(weekIndex.row(),2)).toString();
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

void foodplanner::update_foodPlanModel()
{
    QStandardItem *dayItem,*mealItem;

    for(QMap<QPair<bool,QDate>,QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>>>::const_iterator daystart = updateMap.cbegin(), dayend = updateMap.cend(); daystart != dayend; ++daystart)
    {
        if(daystart.key().first)
        {
            dayItem = this->get_modelItem(foodPlanModel,daystart.key().second.toString(dateSaveFormat));
            for(QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>>::const_iterator mealstart = daystart.value().cbegin(), mealend = daystart.value().cend(); mealstart != mealend; ++mealstart)
            {
                    mealItem = dayItem->child(mealsHeader.indexOf(mealstart.key()));
                    foodPlanModel->removeRows(0,mealItem->rowCount(),mealItem->index());

                    for(QHash<QString,QPair<QString,QPair<int,double>>>::const_iterator foodstart = mealstart.value().cbegin(), foodend = mealstart.value().cend(); foodstart != foodend; ++foodstart)
                    {
                        QList<QStandardItem*> mealList;
                        mealList << new QStandardItem(foodstart.key());
                        mealList << new QStandardItem(foodstart->first);
                        mealList << new QStandardItem(QString::number(foodstart->second.second));
                        mealList << new QStandardItem(QString::number(foodstart->second.first));
                        mealList << new QStandardItem("0");
                        mealItem->appendRow(mealList);
                    }
            }
            this->set_dayFoodValues(dayItem);
        }
    }
}

QVector<double> foodplanner::get_mealValues(QString mealId,double factor)
{
    QVector<double> mealValues(7,0);

    QModelIndex mealIndex = mealModel->indexFromItem(mealModel->findItems(mealId,Qt::MatchRecursive | Qt::MatchExactly,1).at(0));

    for(int value = 2; value < mealModel->columnCount(); ++value)
    {
        mealValues[value-2] = round(mealIndex.siblingAtColumn(value).data(Qt::DisplayRole).toInt()*(factor/mealIndex.siblingAtColumn(2).data(Qt::DisplayRole).toDouble()));
    }
    return mealValues;
}

QModelIndex foodplanner::get_modelIndex(QStandardItemModel *model, QString searchString)
{
    QList<QStandardItem*> list;

    list = model->findItems(searchString,Qt::MatchExactly | Qt::MatchRecursive,0);

    if(list.count() > 0)
    {
        return model->indexFromItem(list.at(0));
    }
    else
    {
        return QModelIndex();
    }
}

QStandardItem *foodplanner::get_modelItem(QStandardItemModel *model, QString searchString)
{
    QList<QStandardItem*> list = model->findItems(searchString,Qt::MatchExactly | Qt::MatchRecursive,0);

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
    QHash<QString,QHash<QString,QVector<double>>> mealCal;
    QVector<double> mealValues(7,0);

    for(int meal = 0; meal < dayItem->rowCount(); ++meal)
    {
        mealItem = dayItem->child(meal,0);
        for(int food = 0; food < mealItem->rowCount(); ++food)
        {
            mealValues = this->get_mealValues(mealItem->child(food,0)->data(Qt::DisplayRole).toString(),mealItem->child(food,2)->data(Qt::DisplayRole).toDouble());
            mealValues.insert(0,mealItem->child(food,3)->data(Qt::DisplayRole).toDouble());
            foodCal.insert(mealItem->child(food,0)->data(Qt::DisplayRole).toString(),mealValues);
        }
        mealCal.insert(mealItem->data(Qt::DisplayRole).toString(),foodCal);
        foodCal.clear();
    }
    foodPlanMap.insert(dayItem->data(Qt::DisplayRole).toDate(),mealCal);
    this->set_daySumMap(dayItem->data(Qt::DisplayRole).toDate());
    mealCal.clear();
}

void foodplanner::set_foodPlanData(QStandardItem *weekItem)
{
    this->set_foodPlanList(weekItem);

    for(int day = 0; day < weekItem->rowCount(); ++day)
    {
        this->set_dayFoodValues(weekItem->child(day,0));
    }
}

void foodplanner::add_toHistory(QDate day)
{
    if(!historyModel->findItems(day.toString(dateFormat),Qt::MatchExactly | Qt::MatchRecursive,2).count())
    {
        if(day == firstdayofweek)
        {
            QList<QStandardItem*> weekList;
            QStandardItem *rootItem = historyModel->invisibleRootItem();

            weekList << new QStandardItem(QString::number(day.weekNumber()));
            weekList << new QStandardItem(QString::number(day.year()));
            weekList << new QStandardItem(day.toString(dateFormat));
            weekList << new QStandardItem(foodPlanList.value(day).at(3));
            weekList << new QStandardItem(QString::number(daySumMap.value(day).at(1)));
            weekList << new QStandardItem(foodPlanList.value(day).at(2));
            rootItem->appendRow(weekList);

            QList<QStandardItem*> dayValueList;
            dayValueList << new QStandardItem(QString::number(day.dayOfWeek()-1));
            dayValueList << new QStandardItem(QString::number(daySumMap.value(day).at(2)));
            dayValueList << new QStandardItem(QString::number(daySumMap.value(day).at(0)));
            weekList.at(0)->appendRow(dayValueList);
        }
        else
        {
            QStandardItem *weekItem = historyModel->item(historyModel->rowCount()-1,0);
            QList<QStandardItem*> dayValueList;
            dayValueList << new QStandardItem(QString::number(day.dayOfWeek()-1));
            dayValueList << new QStandardItem(QString::number(daySumMap.value(day).at(2)));
            dayValueList << new QStandardItem(QString::number(daySumMap.value(day).at(0)));
            weekItem->appendRow(dayValueList);
        }
    }
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

void foodplanner::update_foodPlanData(bool singleItem,QDate copyFromDate,QDate copyToDate)
{
    QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>> updateMealSection;
    QPair<bool,QDate> copyKey(false,copyFromDate);

    if(singleItem && copyToDate.isValid())
    {
         updateMealSection = updateMap.value(copyKey);
         updateMap.insert(qMakePair(true,copyToDate),updateMealSection);
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
                if(it.key().first)
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

    QHash<QString,QVector<double>> currentMeal = foodPlanMap.value(updateDate).value(updateSection);

    if(singleItem & !dragDrop) updateMap.clear();

    for(QHash<QString,QVector<double>>::const_iterator it = currentMeal.cbegin(), end = currentMeal.cend(); it != end; ++it)
    {
        updateFood.first = mealsMap.value(it.key());
        posPort.first = static_cast<int>(it.value().at(0));
        posPort.second = it.value().at(1);
        updateFood.second = posPort;
        updateMeal.insert(it.key(),updateFood);
    }

    updateMealSection.insert(updateSection,updateMeal);
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

void foodplanner::set_foodPlanList(QStandardItem *weekItem)
{
    QModelIndex weekIndex = foodPlanModel->indexFromItem(weekItem);
    QVector<QString> weekInfo(4);
    weekInfo[0] = weekIndex.siblingAtColumn(0).data(Qt::DisplayRole).toString();
    weekInfo[1] = weekIndex.siblingAtColumn(1).data(Qt::DisplayRole).toDate().toString(dateFormat);
    weekInfo[2] = weekIndex.siblingAtColumn(2).data(Qt::DisplayRole).toString();
    weekInfo[3] = weekIndex.siblingAtColumn(3).data(Qt::DisplayRole).toString();

    foodPlanList.insert(weekIndex.siblingAtColumn(1).data(Qt::DisplayRole).toDate(),weekInfo);
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

    if(day >= firstdayofweek && day < firstdayofweek.addDays(7)) this->add_toHistory(day);
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
    this->set_foodPlanMap(INIT);
}

void foodplanner::save_foolPlan()
{
    this->read_treeModel(foodPlanModel,fileMap->value("foodfile"));
    this->read_treeModel(historyModel,fileMap->value("foodhistory"));
}

void foodplanner::save_mealList()
{
    this->read_treeModel(mealModel,fileMap->value("mealsfile"));
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

void foodplanner::add_meal(QItemSelectionModel *mealSelect)
{
    QList<QStandardItem*> mealItems;
    QStandardItem *SectionItem = mealModel->itemFromIndex(mealSelect->currentIndex());

    qDebug() << SectionItem->data(Qt::DisplayRole).toString() +"_"+ QString::number(SectionItem->rowCount()+1);

    mealItems << new QStandardItem("NewMeal");
    mealItems << new QStandardItem("0");
    mealItems << new QStandardItem("0");
    mealItems << new QStandardItem("0");
    mealItems << new QStandardItem("0");
    mealItems << new QStandardItem("0");
    mealItems << new QStandardItem("0");
    mealItems << new QStandardItem("0");

    SectionItem->insertRow(0,mealItems);
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

void foodplanner::insert_newWeek(QDate firstday)
{
    QStandardItem *rootItem = foodPlanModel->invisibleRootItem();
    QString mealDefault;
    QList<QStandardItem*> weekList;
    weekList << new QStandardItem(this->calc_weekID(firstday));
    weekList << new QStandardItem(firstday.toString(dateSaveFormat));
    weekList << new QStandardItem(settings::get_listValues("Mode").at(0));
    weekList << new QStandardItem(rootItem->child(rootItem->rowCount()-1,3)->data(Qt::DisplayRole).toString());

    for(int day = 0; day < dayHeader.count(); ++day)
    {
        QList<QStandardItem*> dayItem;
        dayItem << new QStandardItem(firstday.addDays(day).toString(dateSaveFormat));
        dayItem << new QStandardItem(QString::number(day));
        weekList.at(0)->appendRow(dayItem);

        for(int meals = 0; meals < mealsHeader.count(); ++meals)
        {
            QList<QStandardItem*> mealsItem;
            mealsItem << new QStandardItem(mealsHeader.at(meals));
            mealsItem << new QStandardItem(QString::number(meals));

            mealDefault = "100-"+QString::number(meals);

            QList<QStandardItem*> foodItem;
            foodItem << new QStandardItem(mealDefault);
            foodItem << new QStandardItem(mealsMap.value(mealDefault));
            foodItem << new QStandardItem(QString::number(get_mealData(mealDefault).second.at(0)));
            foodItem << new QStandardItem("0");
            foodItem << new QStandardItem(QString::number(get_mealData(mealDefault).second.at(1)));

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
