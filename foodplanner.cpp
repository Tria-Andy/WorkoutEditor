#include "foodplanner.h"

foodplanner::foodplanner(schedule *ptrSchedule, QDate fd)
{
    schedulePtr = ptrSchedule;
    firstDayofWeek = fd;

    dayTags  << "Day" << "Meal" << "Food";
    mealTags << "name" << "port" << "cal";
    mealsHeader = settings::get_listValues("Meals");
    sumHeader << "Calories Food:" << "Conversion Base:" << "Conversion Sport:" << "Summery:" << "Difference:";
    weekSumHeader << "Week Summery";

    foodList.insert(0,QStringList() << "date");
    foodList.insert(1,QStringList() << "name");
    foodList.insert(2,QStringList() << "name" << "cal");

    dayCalBase = this->current_dayCalories() * athleteValues->value("currpal");

    for(int d = 1; d < 8; ++d)
    {
        dayHeader << QDate::longDayName(d);
    }

    mealModel = new QStandardItemModel();
    mealModel->setColumnCount(mealTags.count());
    weekPlansModel = new QStandardItemModel();
    weekPlansModel->setColumnCount(2);

    daySumModel = new QStandardItemModel(sumHeader.count(),dayHeader.count());
    daySumModel->setVerticalHeaderLabels(sumHeader);
    weekSumModel = new QStandardItemModel(sumHeader.count(),1);
    weekSumModel->setVerticalHeaderLabels(sumHeader);
    weekSumModel->setHorizontalHeaderLabels(weekSumHeader);

    filePath = settings::getStringMapPointer(settings::stingMap::GC)->value("foodplanner");

    planerXML = "foodplan.xml";
    mealXML = "meals.xml";

    if(!filePath.isEmpty())
    {
        this->check_File(filePath,planerXML);
        this->check_File(filePath,mealXML);
        this->read_foodPlan(this->load_XMLFile(filePath,planerXML));
        this->read_meals(this->load_XMLFile(filePath,mealXML));
    }

    loadedWeek = set_weekID(firstDayofWeek);

    this->fill_planList(firstDayofWeek,false);
    this->update_sumBySchedule(firstDayofWeek);
    this->update_sumByMenu(firstDayofWeek,0,NULL,false);
}

void foodplanner::read_foodPlan(QDomDocument xmldoc)
{
    QStandardItem *rootItem = weekPlansModel->invisibleRootItem();
    QDomElement childLevel;
    QDomNodeList xmlList;
    QString weekID;
    xmlList = xmldoc.firstChildElement().elementsByTagName("week");

    for(int weeks = 0; weeks < xmlList.count(); ++weeks)
    {
        QList<QStandardItem*> intItems;
        childLevel = xmlList.at(weeks).toElement();
        weekID = childLevel.attribute("id")+"_"+childLevel.attribute("year");
        intItems << new QStandardItem(weekID);
        intItems << new QStandardItem(childLevel.attribute("fdw"));
        rootItem->appendRow(intItems);
        build_weekFoodTree(childLevel,intItems.at(0));
    }
}

void foodplanner::build_weekFoodTree(QDomElement element,QStandardItem *parentItem)
{
    QDomNodeList childList = element.childNodes();
    QDomElement childElement;

    if(childList.count() > 0)
    {
        for(int childs = 0; childs < childList.count(); ++childs)
        {
            childElement = childList.at(childs).toElement();

            if(childElement.tagName() == "Day")
            {
                QList<QStandardItem*> intItems;
                intItems << new QStandardItem(childElement.attribute("date"));
                parentItem->appendRow(intItems);
                if(childElement.hasChildNodes())
                {
                    build_weekFoodTree(childElement,intItems.at(0));
                }
            }
            else if(childElement.tagName() == "Meal")
            {
                QList<QStandardItem*> intItems;
                intItems << new QStandardItem(childElement.attribute("name"));
                parentItem->appendRow(intItems);
                if(childElement.hasChildNodes())
                {
                    build_weekFoodTree(childElement,intItems.at(0));
                }
            }
            else if(childElement.tagName() == "Food")
            {
                QList<QStandardItem*> intItems;
                intItems << new QStandardItem(childElement.attribute("name"));
                intItems << new QStandardItem(childElement.attribute("cal"));
                parentItem->appendRow(intItems);
            }
        }
    }
}

void foodplanner::write_foodPlan()
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement,dayElement,mealElement,foodElement;
    xmlRoot = xmlDoc.createElement("weekplans");
    xmlDoc.appendChild(xmlRoot);
    QStandardItem *weekItem,*dayItem,*mealItem;
    QString weekID;

    for(int week = 0; week < weekPlansModel->rowCount(); ++week)
    {
        weekID = weekPlansModel->data(weekPlansModel->index(week,0)).toString();
        xmlElement = xmlDoc.createElement("week");
        xmlElement.setAttribute("id",weekID.split("_").first());
        xmlElement.setAttribute("year",weekID.split("_").last());
        xmlElement.setAttribute("fdw",weekPlansModel->data(weekPlansModel->index(week,1)).toString());

        weekItem = weekPlansModel->item(week,0);

        if(weekItem->hasChildren())
        {
            QModelIndex weekIndex = weekPlansModel->indexFromItem(weekItem);

            for(int day = 0; day < weekItem->rowCount(); ++day)
            {
                dayElement = xmlDoc.createElement("Day");
                dayElement.setAttribute("id",day);
                dayElement.setAttribute("date",weekPlansModel->data(weekPlansModel->index(day,0,weekIndex)).toString());

                dayItem = weekPlansModel->itemFromIndex(weekPlansModel->index(day,0,weekIndex));

                if(dayItem->hasChildren())
                {
                    QModelIndex mealIndex = weekPlansModel->indexFromItem(dayItem);

                    for(int meal = 0; meal < dayItem->rowCount(); ++meal)
                    {
                        mealElement = xmlDoc.createElement("Meal");
                        mealElement.setAttribute("id",meal);
                        mealElement.setAttribute("name",weekPlansModel->data(weekPlansModel->index(meal,0,mealIndex)).toString());

                        mealItem = weekPlansModel->itemFromIndex(weekPlansModel->index(meal,0,mealIndex));

                       if(mealItem->hasChildren())
                       {
                            QModelIndex foodIndex = weekPlansModel->indexFromItem(mealItem);

                            for(int food = 0; food < mealItem->rowCount(); ++food)
                            {
                                foodElement = xmlDoc.createElement("Food");
                                foodElement.setAttribute("id",food);
                                foodElement.setAttribute("name",weekPlansModel->data(weekPlansModel->index(food,0,foodIndex)).toString());
                                foodElement.setAttribute("cal",weekPlansModel->data(weekPlansModel->index(food,1,foodIndex)).toString());
                                mealElement.appendChild(foodElement);
                                foodElement.clear();
                            }
                        }
                       dayElement.appendChild(mealElement);
                       mealElement.clear();
                    }
                }
                xmlElement.appendChild(dayElement);
                dayElement.clear();
            }
        }
        xmlRoot.appendChild(xmlElement);
        xmlElement.clear();
    }
    this->write_XMLFile(filePath,&xmlDoc,planerXML);
    xmlDoc.clear();
}


void foodplanner::read_meals(QDomDocument xmlDoc)
{
    QStandardItem *rootItem = mealModel->invisibleRootItem();
    QDomNodeList xmlList,childList;
    QDomElement xmlElement,childElement;

    xmlList = xmlDoc.firstChildElement().elementsByTagName("section");

    for(int i = 0; i < xmlList.count(); ++i)
    {
        QList<QStandardItem*> secItem;
        xmlElement = xmlList.at(i).toElement();
        secItem << new QStandardItem(xmlElement.attribute("name"));
        rootItem->appendRow(secItem);

        if(xmlElement.hasChildNodes())
        {
            childList = xmlElement.childNodes();
            for(int child = 0; child < childList.count(); ++child)
            {
                QList<QStandardItem*> mealItems;
                childElement = childList.at(child).toElement();
                mealItems << new QStandardItem(childElement.attribute("name"));
                mealItems << new QStandardItem(childElement.attribute("port"));
                mealItems << new QStandardItem(childElement.attribute("cal"));
                secItem.at(0)->appendRow(mealItems);
            }
        }
    }
}

void foodplanner::write_meals()
{
    QDomDocument xmlDoc;
    QDomElement xmlRoot,xmlElement,childElement;

    xmlRoot = xmlDoc.createElement("meals");
    xmlDoc.appendChild(xmlRoot);

    for(int section = 0; section < mealModel->rowCount(); ++section)
    {
        xmlElement = xmlDoc.createElement("section");
        xmlElement.setAttribute("id",section);
        xmlElement.setAttribute("name",mealModel->data(mealModel->index(section,0)).toString());

        if(mealModel->item(section,0)->hasChildren())
        {
            QModelIndex sectionIndex = mealModel->indexFromItem(mealModel->item(section,0));

            for(int meal = 0; meal < mealModel->item(section,0)->rowCount(); ++meal)
            {
                childElement = xmlDoc.createElement("meal");
                childElement.setAttribute("id",meal);

                for(int att = 0; att < mealTags.count(); ++att)
                {
                    childElement.setAttribute(mealTags.at(att),mealModel->data(mealModel->index(meal,att,sectionIndex)).toString());
                }
                xmlElement.appendChild(childElement);
                childElement.clear();
            }
        }
        xmlRoot.appendChild(xmlElement);
        xmlElement.clear();
    }
    this->write_XMLFile(filePath,&xmlDoc,mealXML);
    xmlDoc.clear();
}

void foodplanner::insert_newWeek(QDate firstday)
{
    QStandardItem *rootItem = weekPlansModel->invisibleRootItem();
    QList<QStandardItem*> weekList;
    weekList << new QStandardItem(this->set_weekID(firstday));
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
            foodItem << new QStandardItem("Food");
            mealsItem.at(0)->appendRow(foodItem);
        }
    }
    this->fill_planList(firstday,true);
}

void foodplanner::remove_week(QString weekID)
{
    QModelIndex weekIndex = weekPlansModel->indexFromItem(weekPlansModel->findItems(weekID.split(" - ").first(),Qt::MatchExactly,0).at(0));
    weekPlansModel->removeRow(weekIndex.row());
}

void foodplanner::fill_planList(QDate firstDate, bool addWeek)
{
    if(addWeek)
    {
        planList << weekPlansModel->data(weekPlansModel->index(weekPlansModel->rowCount()-1,0)).toString()+" - "+firstDate.toString("dd.MM.yyyy");
    }
    else
    {
        for(int i = 0; i < weekPlansModel->rowCount(); ++i)
        {
            planList << weekPlansModel->data(weekPlansModel->index(i,0)).toString()+" - "+weekPlansModel->data(weekPlansModel->index(i,1)).toDate().toString("dd.MM.yyyy");
        }
    }
}

QString foodplanner::set_weekID(QDate vDate)
{
    return QString::number(vDate.weekNumber())+"_"+QString::number(vDate.year());
}

void foodplanner::update_mealModel(QString section,QStringList *mealList)
{
    if(!section.isEmpty())
    {
        QStringList mealString;
        QString tempString;
        QList<QStandardItem*> mealSection = mealModel->findItems(section,Qt::MatchExactly,0);
        QModelIndex sectionIndex = mealModel->indexFromItem(mealSection.at(0));

        if(mealSection.at(0)->hasChildren())
        {
            mealModel->removeRows(0,mealSection.at(0)->rowCount(),sectionIndex);

            for(int i = 0; i < mealList->count(); ++i)
            {
                tempString = mealList->at(i);
                mealString = tempString.split(" - ");
                QList<QStandardItem*> mealItems;
                mealItems << new QStandardItem(mealString.at(0));
                mealItems << new QStandardItem(mealString.at(1));
                mealItems << new QStandardItem(mealString.at(2));
                mealSection.at(0)->appendRow(mealItems);
            }
        }
    }
}

void foodplanner::update_weekPlanModel(QDate vDate, int mealID, QStringList *foodList)
{
    QModelIndex weekIndex = weekPlansModel->indexFromItem(weekPlansModel->findItems(set_weekID(vDate),Qt::MatchExactly,0).at(0));
    QModelIndex dayIndex;
    QModelIndex mealIndex;
    QDate dayDate;
    QString mealString;
    QString foodString;
    int foodCount = 0;

    for(int day = 0; day < dayHeader.count(); ++day)
    {
        dayDate = weekPlansModel->data(weekPlansModel->index(day,0,weekIndex)).toDate();

        if(dayDate == vDate)
        {
            dayIndex = weekPlansModel->index(day,0,weekIndex);
            for(int meal = 0; meal < mealsHeader.count(); ++meal)
            {
                mealIndex = weekPlansModel->index(meal,0,dayIndex);
                mealString = weekPlansModel->data(weekPlansModel->index(meal,0,dayIndex)).toString();
                if(mealString == mealsHeader.at(mealID))
                {
                    foodCount = weekPlansModel->itemFromIndex(mealIndex)->rowCount();
                    weekPlansModel->removeRows(0,foodCount,mealIndex);

                    for(int food = 0; food < foodList->count(); ++food)
                    {
                        foodString = foodList->at(food);
                        QList<QStandardItem*> intItems;
                        intItems << new QStandardItem(foodString.split(" - ").first());
                        intItems << new QStandardItem(foodString.split(" - ").last());
                        weekPlansModel->itemFromIndex(mealIndex)->appendRow(intItems);
                    }
                    break;
                }
            }
            break;
        }
    }
}

int foodplanner::read_dayCalories(QDate vDate)
{
    QModelIndex weekIndex = weekPlansModel->indexFromItem(weekPlansModel->findItems(set_weekID(vDate),Qt::MatchExactly,0).at(0));
    QModelIndex dayIndex;
    QModelIndex mealIndex;
    QDate dayDate;
    QString mealString;
    int foodCount = 0;
    int dayCalories = 0;

    for(int day = 0; day < dayHeader.count(); ++day)
    {
        dayDate = weekPlansModel->data(weekPlansModel->index(day,0,weekIndex)).toDate();

        if(dayDate == vDate)
        {
            dayIndex = weekPlansModel->index(day,0,weekIndex);
            for(int meal = 0; meal < mealsHeader.count(); ++meal)
            {
                mealIndex = weekPlansModel->index(meal,0,dayIndex);
                mealString = weekPlansModel->data(weekPlansModel->index(meal,0,dayIndex)).toString();

                foodCount = weekPlansModel->itemFromIndex(mealIndex)->rowCount();
                for(int food = 0; food < foodCount; ++food)
                {
                    dayCalories = dayCalories + weekPlansModel->data(weekPlansModel->index(food,1,mealIndex)).toInt();
                }
            }
        }
    }

    return dayCalories;
}

void foodplanner::update_daySumModel()
{
    int sum = 0;
    int diff = 0;

    for(int i = 0; i < dayHeader.count(); ++i)
    {
        sum = daySumModel->data(daySumModel->index(1,i)).toInt() + daySumModel->data(daySumModel->index(2,i)).toInt();
        diff = sum - daySumModel->data(daySumModel->index(0,i)).toInt();
        daySumModel->setData(daySumModel->index(1,i),dayCalBase);
        daySumModel->setData(daySumModel->index(3,i),sum);
        daySumModel->setData(daySumModel->index(4,i),diff);
    }
    this->update_weekSumModel();
}

void foodplanner::update_weekSumModel()
{
    QVector<int> weekSum(5);

    for(int i = 0; i < dayHeader.count(); ++i)
    {
        for(int x = 0; x < daySumModel->rowCount(); ++x)
        {
            weekSum[x] = weekSum[x] + daySumModel->data(daySumModel->index(x,i)).toInt();
        }
    }

    for(int i = 0; i < weekSum.count(); ++i)
    {
        weekSumModel->setData(weekSumModel->index(i,0),weekSum[i]);
    }
}

void foodplanner::update_sumBySchedule(QDate firstday)
{
    int day = 0;
    int dayWork = 0;
    QString weekID = set_weekID(firstday);

    if(weekID == loadedWeek)
    {
        schedulePtr->scheduleProxy->invalidate();
        schedulePtr->scheduleProxy->setFilterFixedString(weekID);
        schedulePtr->scheduleProxy->setFilterKeyColumn(0);

        for(int i = 0; i < dayHeader.count(); ++i)
        {
            daySumModel->setData(daySumModel->index(2,i),0);
        }

        for(int i = 0; i < schedulePtr->scheduleProxy->rowCount(); ++i)
        {
            day = QDate::fromString(schedulePtr->scheduleProxy->data(schedulePtr->scheduleProxy->index(i,1)).toString(),"dd.MM.yyyy").dayOfWeek()-1;
            dayWork = daySumModel->data(daySumModel->index(2,day)).toInt() + schedulePtr->scheduleProxy->data(schedulePtr->scheduleProxy->index(i,9)).toInt();
            daySumModel->setData(daySumModel->index(2,day),dayWork);
        }
        this->update_daySumModel();
    }
}

void foodplanner::update_sumByMenu(QDate firstDay, int meal, QStringList *foodList, bool isUpdate)
{
    if(isUpdate)
    {
        this->update_weekPlanModel(firstDay,meal,foodList);
        daySumModel->setData(daySumModel->index(0,firstDay.dayOfWeek()-1),read_dayCalories(firstDay));
    }
    else
    {
        for(int day = 0; day < dayHeader.count(); ++day)
        {
            daySumModel->setData(daySumModel->index(0,day),read_dayCalories(firstDay.addDays(day)));
        }
    }

    loadedWeek = set_weekID(firstDay);
    this->update_daySumModel();
}

QStringList foodplanner::get_mealList(QString section)
{
    QStringList mealList;

    if(!section.isEmpty())
    {
        QList<QStandardItem*> mealSection = mealModel->findItems(section,Qt::MatchExactly,0);

        QModelIndex sectionIndex = mealModel->indexFromItem(mealSection.at(0));

        if(mealSection.at(0)->hasChildren())
        {
            for(int i = 0; i < mealSection.at(0)->rowCount(); ++i)
            {
                mealList << mealModel->data(mealModel->index(i,0,sectionIndex)).toString()+ " - "+mealModel->data(mealModel->index(i,1,sectionIndex)).toString()+ " - "+mealModel->data(mealModel->index(i,2,sectionIndex)).toString();
            }
        }
    }
    return mealList;
}
