#include "foodplanner.h"

foodplanner::foodplanner(schedule *ptrSchedule)
{
    schedulePtr = ptrSchedule;
    dayHistTags << "id" << "food" << "sport";
    menuHeader = settings::getHeaderMap("menuheader");
    foodsumHeader = settings::getHeaderMap("foodsumheader");
    foodestHeader = settings::getHeaderMap("foodestheader");
    foodhistHeader = settings::getHeaderMap("foodhistheader");
    mealsHeader = settings::get_listValues("Meals");

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

    historyModel = new QStandardItemModel();
    foodPlanModel = new QStandardItemModel();
    foodPlanModel->setColumnCount(settings::getHeaderMap("fooditem")->count());

    filePath = settings::getStringMapPointer(settings::stingMap::GC)->value("foodplanner");

    planerXML = "foodplan.xml";
    mealXML = "meals.xml";
    historyXML = "foodhistory.xml";

    loadedWeek = this->calc_weekID(firstdayofweek);
    defaultCal = settings::doubleVector.value("Mealdefault");

    if(!filePath.isEmpty())
    {
        this->check_File(filePath,mealXML);
        xmlList = this->load_XMLFile(filePath,mealXML).firstChildElement().elementsByTagName("mealsection");
        this->fill_treeModel(&xmlList,mealModel);
        this->set_mealsMap();

        this->check_File(filePath,planerXML);
        xmlList = this->load_XMLFile(filePath,planerXML).firstChildElement().elementsByTagName("foodweek");
        this->fill_treeModel(&xmlList,foodPlanModel);
        this->compare_foodToMeal();


        this->check_File(filePath,historyXML);
        this->read_history(this->load_XMLFile(filePath,historyXML));
    }
}

QString foodplanner::get_mode(QDate startDate)
{
    QModelIndex weekIndex = foodPlanModel->indexFromItem(foodPlanModel->findItems(calc_weekID(startDate),Qt::MatchExactly,0).at(0));
    return foodPlanModel->data(foodPlanModel->index(weekIndex.row(),2)).toString();
}

enum {ADD,DEL,EDIT};

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

void foodplanner::set_foodPlanMap(bool update)
{
    QStandardItem *rootItem = foodPlanModel->invisibleRootItem();
    QStandardItem *weekItem,*dayItem,*mealItem;
    QHash<QString,QVector<double>> foodCal;
    QHash<QString,QHash<QString,QVector<double>>> mealCal;
    QVector<double> mealValues(7,0);

    if(update)
    {
        qDebug() << "";
    }
    else
    {
        for(int week = 0; week < rootItem->rowCount(); ++week)
        {
            weekItem = rootItem->child(week,0);
            set_foodPlanList(weekItem);
            for(int day = 0; day < weekItem->rowCount(); ++day)
            {
                dayItem = weekItem->child(day,0);
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
        }
    }
}

void foodplanner::update_foodPlanMap(bool copy, QHash<QString,QMap<QDate,QString>> foodUpdateMap)
{
    QHash<QString,QVector<double>> updateMeal;
    QHash<QString,QHash<QString,QVector<double>>> updateDay;
    QDate updateDate;
    QString updateSection;

    if(copy)
    {
        updateDate = foodUpdateMap.value("CopyTo").firstKey();
        updateSection =  foodUpdateMap.value("CopyTo").first();
        updateMeal = foodPlanMap.value(foodUpdateMap.value("CopyFrom").firstKey()).value(foodUpdateMap.value("CopyFrom").first());
        updateDay = foodPlanMap.value(updateDate);
        updateDay.insert(updateSection,updateMeal);
        foodPlanMap.insert(updateDate,updateDay);
    }
    else
    {
        updateDate = foodUpdateMap.value("Update").firstKey();
        updateSection = foodUpdateMap.value("Update").first();
        updateDay = foodPlanMap.value(updateDate);
        updateDay.insert(updateSection,updateMap.value(updateDate));
        foodPlanMap.insert(updateDate,updateDay);
    }

}

void foodplanner::set_updateMap(QDate updateDate, QString updateSection)
{
    updateMap.clear();
    QHash<QString,QVector<double>> section = foodPlanMap.value(updateDate).value(updateSection);
    updateMap.insert(updateDate,section);
}


void foodplanner::set_foodPlanList(QStandardItem *weekItem)
{
    QModelIndex weekIndex = foodPlanModel->indexFromItem(weekItem);

    foodPlanList.insert(weekIndex.siblingAtColumn(1).data(Qt::DisplayRole).toDate(),
                        weekIndex.siblingAtColumn(0).data(Qt::DisplayRole).toString() +" - "+
                        weekIndex.siblingAtColumn(1).data(Qt::DisplayRole).toDate().toString(dateFormat) +" - "+
                        weekIndex.siblingAtColumn(2).data(Qt::DisplayRole).toString());
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
}

void foodplanner::compare_foodToMeal()
{
    QStandardItem *rootItem = foodPlanModel->invisibleRootItem();
    QStandardItem *weekItem,*dayItem,*mealItem;

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
    this->set_foodPlanMap(false);
}

void foodplanner::save_foolPlan()
{
    this->read_treeModel(foodPlanModel,"foodplan",planerXML);
}

void foodplanner::save_mealList()
{
    this->read_treeModel(mealModel,"meals",mealXML);
}

void foodplanner::read_history(QDomDocument xmlDoc)
{
    historyModel->clear();
    historyModel->setColumnCount(foodhistHeader->count());
    this->set_headerLabel(historyModel,foodhistHeader,false);

    QStandardItem *rootItem = historyModel->invisibleRootItem();
    QDomNodeList xmlList,childList;
    QDomElement xmlElement,childElement;
    QDate dayName;
    QList<QStandardItem*> dayItem,dayItems;
    int metaRate = 0;
    int sumSport = 0;
    int sumFood = 0;
    QMap<int,int> calSport,calFood;

    xmlList = xmlDoc.firstChildElement().elementsByTagName("week");

    for(int i = 0; i < xmlList.count(); ++i)
    {
        xmlElement = xmlList.at(i).toElement();
        metaRate = xmlElement.attribute("base").toInt();

        dayItem << new QStandardItem(xmlElement.attribute("id")+"_"+xmlElement.attribute("year"));
        dayItem << new QStandardItem(xmlElement.attribute("fdw"));
        dayItem << new QStandardItem(xmlElement.attribute("weight"));
        dayItem << new QStandardItem(QString::number(metaRate*7));
        dayItem << new QStandardItem("0");
        dayItem << new QStandardItem("0");
        dayItem << new QStandardItem("0");
        dayItem << new QStandardItem(settings::get_listValues("Mode").at(xmlElement.attribute("mode").toInt()));

        rootItem->appendRow(dayItem);
        dayName =  QDate().fromString(xmlElement.attribute("fdw"),dateFormat);

        if(xmlElement.hasChildNodes())
        {
            childList = xmlElement.childNodes();
            for(int child = 0; child < childList.count(); ++child)
            {
                childElement = childList.at(child).toElement();
                calFood.insert(child,childElement.attribute(dayHistTags.at(1)).toInt());
                calSport.insert(child,childElement.attribute(dayHistTags.at(2)).toInt());

                dayItems << new QStandardItem(QLocale().dayName(child+1,QLocale::ShortFormat));
                dayItems << new QStandardItem(dayName.addDays(child).toString(dateFormat));
                dayItems << new QStandardItem("-");
                dayItems << new QStandardItem(QString::number(metaRate));
                dayItems << new QStandardItem(QString::number(calSport.value(child)));
                dayItems << new QStandardItem(QString::number(calFood.value(child)));
                dayItems << new QStandardItem(QString::number((metaRate + calSport.value(child)) - calFood.value(child)));
                dayItem.at(0)->appendRow(dayItems);
                dayItems.clear();
            }

            for(int i = 0; i < 7; ++i)
            {
                sumSport = sumSport + calSport.value(i);
                sumFood = sumFood + calFood.value(i);
            }

            dayItem.at(4)->setData(sumSport,Qt::EditRole);
            dayItem.at(5)->setData(sumFood,Qt::EditRole);
            dayItem.at(6)->setData(((metaRate*7) + sumSport) - sumFood,Qt::EditRole);

        }
        sumSport = 0;
        sumFood = 0;
        dayItem.clear();
    }

    int lastRow = historyModel->rowCount()-1;

    QString currentWeek = historyModel->data(historyModel->index(lastRow,0)).toString();

    if(loadedWeek.compare(currentWeek) != 0)
    {
        dayItem << new QStandardItem(this->calc_weekID(firstdayofweek));
        dayItem << new QStandardItem(firstdayofweek.toString(dateFormat));
        dayItem << new QStandardItem(QString::number(athleteValues->value("weight")));
        dayItem << new QStandardItem("0");
        dayItem << new QStandardItem("0");
        dayItem << new QStandardItem("0");
        dayItem << new QStandardItem("-");
        dayItem << new QStandardItem(foodPlanModel->data(foodPlanModel->index(0,1)).toString());
        rootItem->appendRow(dayItem);

        for(int i = 0; i < 7; ++i)
        {
            dayItems << new QStandardItem(QLocale().dayName(i+1,QLocale::ShortFormat));
            dayItems << new QStandardItem(firstdayofweek.addDays(i).toString(dateFormat));
            dayItems << new QStandardItem("-");
            dayItems << new QStandardItem("0");
            dayItems << new QStandardItem("0");
            dayItems << new QStandardItem("0");
            dayItems << new QStandardItem("0");
            dayItem.at(0)->appendRow(dayItems);
            dayItems.clear();
        }
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

void foodplanner::write_foodHistory()
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement,childElement;

    xmlRoot = xmlDoc.createElement("weeks");
    xmlDoc.appendChild(xmlRoot);

    QString weekID;
    QStringList modeList = settings::get_listValues("Mode");


    for(int section = 0; section < historyModel->rowCount(); ++section)
    {
        weekID = historyModel->data(historyModel->index(section,0)).toString();

        xmlElement = xmlDoc.createElement("week");
        xmlElement.setAttribute("id",weekID.split("_").first());
        xmlElement.setAttribute("year",weekID.split("_").last());
        xmlElement.setAttribute("fdw",historyModel->data(historyModel->index(section,1)).toString());
        xmlElement.setAttribute("weight",historyModel->data(historyModel->index(section,2)).toString());
        xmlElement.setAttribute("base",QString::number(historyModel->data(historyModel->index(section,3)).toInt() / 7));
        xmlElement.setAttribute("mode",modeList.indexOf(historyModel->data(historyModel->index(section,7)).toString()));

        if(historyModel->item(section,0)->hasChildren())
        {
            QModelIndex sectionIndex = historyModel->indexFromItem(historyModel->item(section,0));

            for(int day = 0; day < historyModel->item(section,0)->rowCount(); ++day)
            {
                childElement = xmlDoc.createElement("day");
                childElement.setAttribute("id",day);
                childElement.setAttribute("sport",historyModel->data(historyModel->index(day,4,sectionIndex)).toString());
                childElement.setAttribute("food",historyModel->data(historyModel->index(day,5,sectionIndex)).toString());
                xmlElement.appendChild(childElement);
                childElement.clear();
            }
        }
        xmlRoot.appendChild(xmlElement);
        xmlElement.clear();
    }
    this->write_XMLFile(filePath,&xmlDoc,historyXML);
    xmlDoc.clear();
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
    QString mealName = mealIndex.siblingAtColumn(0).data(Qt::DisplayRole).toString();

    for(int col = 2; col < mealModel->columnCount(); ++col)
    {
        mealValues[col-2] = mealIndex.siblingAtColumn(col).data(Qt::DisplayRole).toInt();
    }
    mealData.first = mealName;
    mealData.second = mealValues;
    return mealData;
}

void foodplanner::insert_newWeek(QDate firstday)
{
    QStandardItem *rootItem = foodPlanModel->invisibleRootItem();
    QList<QStandardItem*> weekList;
    weekList << new QStandardItem(this->calc_weekID(firstday));
    weekList << new QStandardItem(settings::get_listValues("Mode").at(0));
    weekList << new QStandardItem(firstday.toString("yyyy-MM-dd"));   
    rootItem->appendRow(weekList);

    for(int day = 0; day < dayHeader.count(); ++day)
    {
        QList<QStandardItem*> dayItem;
        dayItem << new QStandardItem(firstday.addDays(day).toString("yyyy-MM-dd"));
        weekList.at(0)->appendRow(dayItem);

        for(int meals = 0; meals < mealsHeader.count(); ++meals)
        {
            QList<QStandardItem*> mealsItem;
            mealsItem << new QStandardItem(mealsHeader.at(meals));
            dayItem.at(0)->appendRow(mealsItem);

            QList<QStandardItem*> foodItem;
            foodItem << new QStandardItem("Default ");
            foodItem << new QStandardItem("1");
            foodItem << new QStandardItem(QString::number(defaultCal.at(meals)));
            mealsItem.at(0)->appendRow(foodItem);
        }
    }
    this->fill_planList(firstday,false);
}

void foodplanner::remove_week(QString weekID)
{
    QModelIndex weekIndex = foodPlanModel->indexFromItem(foodPlanModel->findItems(weekID.split(" - ").first(),Qt::MatchExactly,0).at(0));
    foodPlanModel->removeRow(weekIndex.row());
    this->fill_planList(QDate(),false);
}

void foodplanner::fill_planList(QDate firstDate, bool addWeek)
{
    if(addWeek)
    {
        planList << foodPlanModel->data(foodPlanModel->index(foodPlanModel->rowCount()-1,0)).toString()+" - "+firstDate.toString(dateFormat)+" - "+foodPlanModel->data(foodPlanModel->index(foodPlanModel->rowCount()-1,1)).toString();
    }
    else
    {
        planList.clear();
        for(int i = 0; i < foodPlanModel->rowCount(); ++i)
        {
            planList << foodPlanModel->data(foodPlanModel->index(i,0)).toString()+" - "+foodPlanModel->data(foodPlanModel->index(i,1)).toDate().toString(dateFormat)+" - "+foodPlanModel->data(foodPlanModel->index(i,2)).toString();
        }
    }
}

void foodplanner::addrow_mealModel(QStandardItem *item, QStringList *itemList)
{
    QStringList mealString;
    QString tempString;

    for(int i = 0; i < itemList->count(); ++i)
    {
        tempString = itemList->at(i);
        mealString = tempString.split(" - ");
        QList<QStandardItem*> mealItems;
        mealItems << new QStandardItem(mealString.at(0));
        mealItems << new QStandardItem(mealString.at(1));
        mealItems << new QStandardItem(mealString.at(2));
        item->appendRow(mealItems);
    }
}
