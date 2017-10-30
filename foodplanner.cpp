#include "foodplanner.h"

foodplanner::foodplanner(schedule *ptrSchedule)
{
    schedulePtr = ptrSchedule;
    firstDayofWeek = QDate().currentDate().addDays(1 - QDate().currentDate().dayOfWeek());

    dayTags  << "Day" << "Meal" << "Food";
    mealTags << "id" << "section" << "name" << "cal";
    mealsHeader = settings::get_listValues("meals");
    sumHeader << "Calories Food:" << "Conversion Base:" << "Conversion Sport:" << "Summery:" << "Difference:";

    dayCalBase = this->current_dayCalories() * athleteValues->value("currpal");

    for(int d = 1; d < 8; ++d)
    {
        dayHeader << QDate::longDayName(d);
    }

    planerModel = new QStandardItemModel(mealsHeader.count(),7);
    planerModel->setHorizontalHeaderLabels(dayHeader);
    planerModel->setVerticalHeaderLabels(mealsHeader);
    mealModel = new QStandardItemModel();
    mealModel->setColumnCount(mealTags.count()+1);
    mealProxy = new QSortFilterProxyModel;
    mealProxy->setSourceModel(mealModel);
    weekModel = new QStandardItemModel();
    weekModel->setColumnCount(2);

    daySumModel = new QStandardItemModel(sumHeader.count(),dayHeader.count());
    daySumModel->setVerticalHeaderLabels(sumHeader);
    weekSumModel = new QStandardItemModel(sumHeader.count(),1);
    weekSumModel->setVerticalHeaderLabels(sumHeader);

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
}

void foodplanner::set_headerInfo(QDate firstDay)
{
    this->fill_weekModel(firstDay);
}


void foodplanner::read_foodPlan(QDomDocument xmldoc)
{
    QStandardItem *rootItem = weekModel->invisibleRootItem();
    QDomElement childLevel;
    QDomNodeList xmlList;

    xmlList = xmldoc.firstChildElement().elementsByTagName("week");

    for(int weeks = 0; weeks < xmlList.count(); ++weeks)
    {
        QList<QStandardItem*> intItems;
        childLevel = xmlList.at(weeks).toElement();
        intItems << new QStandardItem(childLevel.attribute("id")+"_"+childLevel.attribute("year"));
        rootItem->appendRow(intItems);
        build_weekFoodTree(childLevel,intItems.at(0));
    }
}

void foodplanner::read_meals(QDomDocument xmlDoc)
{
    QDomNodeList xmlList;
    QDomElement xmlElement;

    xmlList = xmlDoc.firstChildElement().elementsByTagName("meal");

    for(int i = 0; i < xmlList.count(); ++i)
    {
        xmlElement = xmlList.at(i).toElement();
        mealModel->insertRow(i);
        mealModel->setData(mealModel->index(i,0),xmlElement.attribute("name")+" - "+xmlElement.attribute("cal"));

        for(int col = 0; col < mealTags.count(); ++col)
        {
            mealModel->setData(mealModel->index(i,col+1),xmlElement.attribute(mealTags.at(col)));
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
                intItems << new QStandardItem(childElement.attribute("name"));
                intItems << new QStandardItem(childElement.attribute("cal"));
                parentItem->appendRow(intItems);
            }
        }
    }
}

void foodplanner::fill_weekModel(QDate firstday)
{
    QString weekID = QString::number(firstday.weekNumber())+"_"+QString::number(firstday.year());
    QList<QStandardItem*> week;
    week = weekModel->findItems(weekID,Qt::MatchExactly,0);
    QModelIndex weekIndex = weekModel->indexFromItem(week.at(0));
    QModelIndex dayIndex;
    QModelIndex mealIndex;
    int foodCount = 0;
    int dayCalories = 0;
    QString foodString;

    for(int day = 0; day < dayHeader.count(); ++day)
    {
        dayIndex = weekModel->index(day,0,weekIndex);
        dayCalories = 0;
        for(int meal = 0; meal < mealsHeader.count(); ++meal)
        {
            mealIndex = weekModel->index(meal,0,dayIndex);
            foodCount = weekModel->itemFromIndex(mealIndex)->rowCount();
            foodString.clear();

            for(int food = 0; food < foodCount; ++food)
            {
                dayCalories = dayCalories + weekModel->data(weekModel->index(food,1,mealIndex)).toInt();
                foodString = foodString +weekModel->data(weekModel->index(food,0,mealIndex)).toString()+" - "+weekModel->data(weekModel->index(food,1,mealIndex)).toString()+"\n";
            }
            foodString.truncate(foodString.count()-1);

            planerModel->setData(planerModel->index(meal,day),foodString);
            daySumModel->setData(daySumModel->index(0,day),dayCalories);
            daySumModel->setData(daySumModel->index(1,day),dayCalBase);
        }
    }
}
