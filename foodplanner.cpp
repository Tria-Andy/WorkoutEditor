#include "foodplanner.h"

foodplanner::foodplanner(schedule *ptrSchedule, QDate fd)
{
    schedulePtr = ptrSchedule;
    firstDayofWeek = fd;

    dayTags  << "Day" << "Meal" << "Food";
    mealTags << "name" << "port" << "cal" << "carb" << "protein" << "fat" << "fiber" << "sugar" << "id";
    menuHeader << "Name" << "Port" << "Cal" << "Carb" << "Protein" << "Fat" << "Fiber" << "Sugar";
    mealsHeader = settings::get_listValues("Meals");
    sumHeader << "Calories Food:" << "Conversion Base:" << "Conversion Sport:" << "Summery:" << "Difference:";
    weekSumHeader << "Week Summery";
    estHeader << "Weight at Weekstart:" << "Avg Daily Calories:" << "Avg Daily Conversion:" <<"Avg Daily Diff:" << "Weight Change:" << "Weight at Weekend:";

    foodList.insert(0,QStringList() << "date");
    foodList.insert(1,QStringList() << "name");
    foodList.insert(2,QStringList() << "name" << "cal");

    for(int d = 1; d < 8; ++d)
    {
        dayHeader << QDate::longDayName(d);
    }

    mealModel = new QStandardItemModel();
    mealModel->setColumnCount(mealTags.count());
    mealModel->setHorizontalHeaderLabels(menuHeader);
    weekPlansModel = new QStandardItemModel();
    weekPlansModel->setColumnCount(mealTags.count()+1);

    daySumModel = new QStandardItemModel(sumHeader.count(),dayHeader.count());
    daySumModel->setVerticalHeaderLabels(sumHeader);
    weekSumModel = new QStandardItemModel(sumHeader.count(),1);
    weekSumModel->setVerticalHeaderLabels(sumHeader);
    weekSumModel->setHorizontalHeaderLabels(weekSumHeader);

    estModel = new QStandardItemModel(estHeader.count(),1);
    estModel->setVerticalHeaderLabels(estHeader);

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

    QModelIndex weekIndex = weekPlansModel->indexFromItem(weekPlansModel->findItems(loadedWeek,Qt::MatchExactly,0).at(0));
    calPercent = settings::doubleMap.value(weekPlansModel->data(weekPlansModel->index(weekIndex.row(),1)).toString());
    defaultCal = settings::doubleMap.value("Mealdefault");
    this->fill_planList(firstDayofWeek,false);
    this->update_sumBySchedule(firstDayofWeek);
    this->update_sumByMenu(firstDayofWeek,0,nullptr,false);
}

enum {ADD,DEL,EDIT};

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
        intItems << new QStandardItem(settings::get_listValues("Mode").at(childLevel.attribute("mode").toInt()));
        intItems << new QStandardItem(childLevel.attribute("fdw"));
        intItems << new QStandardItem(childLevel.attribute("weight"));

        if(intItems.at(2)->data(Qt::DisplayRole).toDate() == firstDayofWeek)
        {
            intItems.at(3)->setData(athleteValues->value("weight"),Qt::EditRole);
        }

        if(intItems.at(2)->data(Qt::DisplayRole).toDate() >= firstDayofWeek)
        {
            rootItem->appendRow(intItems);
            build_weekFoodTree(childLevel,intItems.at(0));
        }
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
                for(int i = 0; i < mealTags.count(); ++i)
                {
                    intItems << new QStandardItem(childElement.attribute(mealTags.at(i)));
                }
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
        xmlElement.setAttribute("mode",settings::get_listValues("Mode").indexOf(weekPlansModel->data(weekPlansModel->index(week,1)).toString()));
        xmlElement.setAttribute("fdw",weekPlansModel->data(weekPlansModel->index(week,2)).toString());
        xmlElement.setAttribute("weight",weekPlansModel->data(weekPlansModel->index(week,3)).toString());

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
                                foodElement.setAttribute("port",weekPlansModel->data(weekPlansModel->index(food,1,foodIndex)).toString());
                                foodElement.setAttribute("cal",weekPlansModel->data(weekPlansModel->index(food,2,foodIndex)).toString());
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
                mealItems << new QStandardItem(childElement.attribute("carb"));
                mealItems << new QStandardItem(childElement.attribute("protein"));
                mealItems << new QStandardItem(childElement.attribute("fat"));
                mealItems << new QStandardItem(childElement.attribute("fiber"));
                mealItems << new QStandardItem(childElement.attribute("sugar"));
                secItem.at(0)->appendRow(mealItems);
            }
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
    this->write_meals();
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

QVector<int> foodplanner::get_mealData(QString mealName, bool makros)
{
    QVector<int> mealData;
    QStandardItem *sectionItem;
    QModelIndex sectionIndex,mealIndex;

    for(int row = 0; row < mealModel->rowCount(); ++row)
    {
        sectionItem = mealModel->item(row,0);
        sectionIndex = sectionItem->index();

        if(sectionItem->hasChildren())
        {
            for(int subrow = 0; subrow < sectionItem->rowCount(); ++subrow)
            {
                mealIndex = mealModel->index(subrow,0,sectionIndex);
                if(mealName == mealModel->data(mealIndex).toString())
                {
                    if(makros)
                    {
                        mealData.resize(5);
                        mealData[0] = mealModel->data(mealModel->index(subrow,3,sectionIndex)).toInt();
                        mealData[1] = mealModel->data(mealModel->index(subrow,4,sectionIndex)).toInt();
                        mealData[2] = mealModel->data(mealModel->index(subrow,5,sectionIndex)).toInt();
                        mealData[3] = mealModel->data(mealModel->index(subrow,6,sectionIndex)).toInt();
                        mealData[4] = mealModel->data(mealModel->index(subrow,7,sectionIndex)).toInt();
                    }
                    else
                    {
                        mealData.resize(2);
                        mealData[0] = mealModel->data(mealModel->index(subrow,1,sectionIndex)).toInt();
                        mealData[1] = mealModel->data(mealModel->index(subrow,2,sectionIndex)).toInt();
                    }
                    break;
                }
            }
        }
    }
    return mealData;
}

void foodplanner::insert_newWeek(QDate firstday)
{
    QStandardItem *rootItem = weekPlansModel->invisibleRootItem();
    QList<QStandardItem*> weekList;
    weekList << new QStandardItem(this->set_weekID(firstday));
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
    QModelIndex weekIndex = weekPlansModel->indexFromItem(weekPlansModel->findItems(weekID.split(" - ").first(),Qt::MatchExactly,0).at(0));
    weekPlansModel->removeRow(weekIndex.row());
    this->fill_planList(QDate(),false);
}

void foodplanner::fill_planList(QDate firstDate, bool addWeek)
{
    if(addWeek)
    {
        planList << weekPlansModel->data(weekPlansModel->index(weekPlansModel->rowCount()-1,0)).toString()+" - "+firstDate.toString("dd.MM.yyyy")+" - "+weekPlansModel->data(weekPlansModel->index(weekPlansModel->rowCount()-1,1)).toString();
    }
    else
    {
        planList.clear();
        for(int i = 0; i < weekPlansModel->rowCount(); ++i)
        {
            planList << weekPlansModel->data(weekPlansModel->index(i,0)).toString()+" - "+weekPlansModel->data(weekPlansModel->index(i,2)).toDate().toString("dd.MM.yyyy")+" - "+weekPlansModel->data(weekPlansModel->index(i,1)).toString();
        }
    }
}

QString foodplanner::set_weekID(QDate vDate)
{
    return QString::number(vDate.weekNumber())+"_"+QString::number(vDate.year());
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

void foodplanner::update_weekPlanModel(QDate vDate, int mealID, QStringList *foodList)
{
    QModelIndex weekIndex = weekPlansModel->indexFromItem(weekPlansModel->findItems(set_weekID(vDate),Qt::MatchExactly,0).at(0));
    QModelIndex dayIndex;
    QModelIndex mealIndex;
    QDate dayDate;
    QString mealString;
    QString foodString;
    QString port,cal;
    int foodCount = 0;
    int portPos,calPos;

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
                        portPos = foodString.indexOf("(")+1;
                        calPos = foodString.indexOf("-")+1;
                        port = foodString.mid(portPos,calPos-portPos-1);
                        cal = foodString.mid(calPos,foodString.indexOf(")")-calPos);
                        intItems << new QStandardItem(foodString.mid(0,foodString.lastIndexOf(" ")));
                        intItems << new QStandardItem(port);
                        intItems << new QStandardItem(cal);
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
    double foodPort;

    for(int day = 0; day < dayHeader.count(); ++day)
    {
        dayDate = weekPlansModel->data(weekPlansModel->index(day,0,weekIndex)).toDate();

        if(dayDate == vDate)
        {
            dayMacros.insert(dayDate,QVector<double>(5));
            dayIndex = weekPlansModel->index(day,0,weekIndex);
            for(int meal = 0; meal < mealsHeader.count(); ++meal)
            {
                mealIndex = weekPlansModel->index(meal,0,dayIndex);
                foodCount = weekPlansModel->itemFromIndex(mealIndex)->rowCount();
                for(int food = 0; food < foodCount; ++food)
                {
                    dayCalories = dayCalories + weekPlansModel->data(weekPlansModel->index(food,2,mealIndex)).toInt();
                    mealString = weekPlansModel->data(weekPlansModel->index(food,0,mealIndex)).toString();
                    foodPort = weekPlansModel->data(weekPlansModel->index(food,1,mealIndex)).toDouble();
                    this->set_foodMacros(dayDate,mealString,foodPort);
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
    int dayRoutineCal = 0;
    bool dayRoutine = generalValues->value("DayRoutine").toInt();
    double currPal = athleteValues->value("currpal");
    int dayCalBase = 0;
    QDateTime calcDay;
    calcDay.setDate(firstDayofWeek);
    QVector<double> temp(5);

    if(dayRoutine) dayRoutineCal = generalValues->value("DayRoutineCal").toInt();

    double minCal = 0;
    double maxCal = 0;

    for(int i = 0; i < dayHeader.count(); ++i)
    {
        dayCalBase = static_cast<int>(this->current_dayCalories(calcDay.addDays(i).addSecs(120)) * currPal) + dayRoutineCal;
        //sum = daySumModel->data(daySumModel->index(1,i)).toInt() + daySumModel->data(daySumModel->index(2,i)).toInt();
        sum = dayCalBase + daySumModel->data(daySumModel->index(2,i)).toInt();
        diff = sum - daySumModel->data(daySumModel->index(0,i)).toInt();

        daySumModel->setData(daySumModel->index(1,i),dayCalBase);
        daySumModel->setData(daySumModel->index(3,i),sum);
        daySumModel->setData(daySumModel->index(4,i),diff);

        temp[0] = round(sum * (settings::doubleMap.value("Macros").at(0) / 100.0) / 4.0);
        temp[1] = round(sum * (settings::doubleMap.value("Macros").at(1) / 100.0) / 4.0);
        temp[2] = round(sum * (settings::doubleMap.value("Macros").at(2) / 100.0) / 9.0);
        temp[3] = ceil(athleteValues->value("weight") / 2.0);
        temp[4] = round(sum * (generalValues->value("DaySugar").toDouble() / 100.0) / 4.0);
        dayTarget.insert(calcDay.date().addDays(i),temp);

        maxCal = round(sum * (calPercent.at(1)/100.0));
        minCal = round(sum * (calPercent.at(2)/100.0));

        daySumModel->item(4,i)->setToolTip("Range: "+QString::number(minCal)+"-"+QString::number(maxCal));
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

    QDateTime calcDay;
    calcDay.setDate(firstDayofWeek);

    double weekWeight;
    double bodyfatcal = athleteValues->value("BodyFatCal");
    QModelIndex weekIndex = weekPlansModel->indexFromItem(weekPlansModel->findItems(loadedWeek,Qt::MatchExactly,0).at(0));

    if(firstDayofWeek == QDate::currentDate().addDays(1 - QDate::currentDate().dayOfWeek()))
    {
        weekWeight = settings::get_weightforDate(calcDay.addSecs(120));
    }
    else
    {
        weekWeight = weekPlansModel->data(weekPlansModel->index(weekIndex.row(),3)).toDouble();
    }

    double weekSave = weekSumModel->data(weekSumModel->index(4,0)).toDouble();
    double weekLoss = round(weekSave/bodyfatcal)/1000.0 * -1;
    double nextWeek = this->set_doubleValue(weekWeight + weekLoss,false);

    estModel->setData(estModel->index(0,0),QString::number(weekWeight) + " Kg");
    estModel->setData(estModel->index(1,0),QString::number(round(weekSumModel->data(weekSumModel->index(0,0)).toDouble()/7.0))+ " Cal");
    estModel->setData(estModel->index(2,0),QString::number(round(weekSumModel->data(weekSumModel->index(3,0)).toDouble()/7.0))+ " Cal");
    estModel->setData(estModel->index(3,0),QString::number(round((weekSave/7.0)*-1)) + " Cal");
    estModel->setData(estModel->index(4,0),QString::number(weekLoss) + " Kg");
    estModel->setData(estModel->index(5,0),QString::number(nextWeek) + " Kg");

    weekPlansModel->setData(weekPlansModel->index(weekIndex.row()+1,3),nextWeek);

    double maxCal = round(weekSum[3] * (calPercent.at(1)/100.0));
    double minCal = round(weekSum[3] * (calPercent.at(2)/100.0));

    weekSumModel->item(4,0)->setToolTip("Range: "+QString::number(minCal)+"-"+QString::number(maxCal));

}

QVector<int> foodplanner::calc_FoodMacros(QString foodString, double foodPort)
{
    QVector<int> foodMacros(5);
    QVector<double> temp(5);
    double factor = 0;
    double portion = 0;

    if(foodString.contains("Default") || foodString.contains("None") || foodString.isEmpty())
    {
        foodMacros.fill(0);
    }
    else
    {
        portion = this->get_mealData(foodString,false).at(0);
        foodMacros = this->get_mealData(foodString,true);

        if(portion >= 1 && portion < 100)
        {
            factor = foodPort;
        }
        else if(portion >= 100)
        {
            factor = foodPort / 100.0;
        }

        for(int i = 0; i < 5; ++i)
        {
            foodMacros[i] = static_cast<int>(round(foodMacros.at(i) * factor));
        }
    }

    return foodMacros;
}

void foodplanner::set_foodMacros(QDate day,QString foodString,double foodPort)
{
    QVector<int> foodMacros(5);
    QVector<double> temp(5);

    foodMacros = this->calc_FoodMacros(foodString,foodPort);

    temp = dayMacros.value(day);

    for(int x = 0; x < temp.count(); ++x)
    {
        temp[x] = temp[x] + foodMacros[x];
    }

    dayMacros.insert(day,temp);
}

void foodplanner::update_sumBySchedule(QDate firstDay)
{
    int day = 0;
    int dayWork = 0;
    QString weekID = set_weekID(firstDay);
    QVector<double> addMoving = settings::doubleMap.value("Moveday");

    if(weekID == loadedWeek)
    {
        schedulePtr->filter_schedule(weekID,0,false);

        for(int i = 0; i < dayHeader.count(); ++i)
        {
            daySumModel->setData(daySumModel->index(2,i),0);
        }

        for(int i = 0; i < schedulePtr->scheduleProxy->rowCount(); ++i)
        {
            day = QDate::fromString(schedulePtr->scheduleProxy->data(schedulePtr->scheduleProxy->index(i,1)).toString(),"dd.MM.yyyy").dayOfWeek()-1;
            dayWork = daySumModel->data(daySumModel->index(2,day)).toInt() + schedulePtr->scheduleProxy->data(schedulePtr->scheduleProxy->index(i,9)).toInt();

            if(schedulePtr->scheduleProxy->data(schedulePtr->scheduleProxy->index(i,3)).toString() == settings::isSwim && addMoving.at(day) == 1)
            {
                dayWork = dayWork + generalValues->value("AddMoving").toInt();
            }
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
        if(!mealSection.isEmpty())
        {
            QModelIndex sectionIndex = mealModel->indexFromItem(mealSection.at(0));
            if(mealSection.at(0)->hasChildren())
            {
                for(int i = 0; i < mealSection.at(0)->rowCount(); ++i)
                {
                    mealList << mealModel->data(mealModel->index(i,0,sectionIndex)).toString()+ " - "+mealModel->data(mealModel->index(i,1,sectionIndex)).toString()+ " - "+mealModel->data(mealModel->index(i,2,sectionIndex)).toString();
                }
            }
        }
        else
        {
            mealList << "No Meals";
        }
    }
    qSort(mealList.begin(),mealList.end());

    return mealList;
}
