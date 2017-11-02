#include "foodplanner.h"

foodplanner::foodplanner(schedule *ptrSchedule, QDate fd)
{
    schedulePtr = ptrSchedule;
    firstDayofWeek = fd;

    dayTags  << "Day" << "Meal" << "Food";
    mealTags << "id" << "section" << "name" << "cal";
    mealsHeader = settings::get_listValues("meals");
    sumHeader << "Calories Food:" << "Conversion Base:" << "Conversion Sport:" << "Summery:" << "Difference:";
    weekSumHeader << "Week Summery";

    dayCalBase = this->current_dayCalories() * athleteValues->value("currpal");

    for(int d = 1; d < 8; ++d)
    {
        dayHeader << QDate::longDayName(d);
    }

    planerModel = new QStandardItemModel(0,5);
    planerProxy = new QSortFilterProxyModel;
    planerProxy->setSourceModel(planerModel);
    mealModel = new QStandardItemModel();
    mealModel->setColumnCount(mealTags.count()+1);
    mealProxy = new QSortFilterProxyModel;
    mealProxy->setSourceModel(mealModel);
    weekPlansModel = new QStandardItemModel();
    weekPlansModel->setColumnCount(2);

    daySumModel = new QStandardItemModel(sumHeader.count(),dayHeader.count());
    daySumModel->setVerticalHeaderLabels(sumHeader);
    weekSumModel = new QStandardItemModel(sumHeader.count(),1);
    weekSumModel->setVerticalHeaderLabels(sumHeader);
    weekSumModel->setHorizontalHeaderLabels(weekSumHeader );

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

    this->set_Models(fd);

}

void foodplanner::set_Models(QDate firstDay)
{
    this->fill_plannerModel();
    this->update_sumBySchedule(firstDay);
    this->update_sumByMenu(firstDay);
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
        planList << weekID;
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

void foodplanner::fill_plannerModel()
{
    QString weekID = QString::number(firstDayofWeek.weekNumber())+"_"+QString::number(firstDayofWeek.year());
    QList<QStandardItem*> week;
    week = weekPlansModel->findItems(weekID,Qt::MatchExactly,0);
    QModelIndex weekIndex = weekPlansModel->indexFromItem(week.at(0));
    QModelIndex dayIndex;
    QModelIndex mealIndex;
    int foodCount = 0;
    int rowCount = 0;
    int foodRow = 0;

    for(int day = 0; day < dayHeader.count(); ++day)
    {
        dayIndex = weekPlansModel->index(day,0,weekIndex);

        for(int meal = 0; meal < mealsHeader.count(); ++meal)
        {
            mealIndex = weekPlansModel->index(meal,0,dayIndex);
            foodCount = weekPlansModel->itemFromIndex(mealIndex)->rowCount();
            planerModel->insertRows(rowCount,foodCount);

            for(int food = 0; food < foodCount; ++food)
            {
               foodRow =  rowCount+food;
               planerModel->setData(planerModel->index(foodRow,0),weekID);
               planerModel->setData(planerModel->index(foodRow,1),firstDayofWeek.addDays(day).toString("dd.MM.yyyy"));
               planerModel->setData(planerModel->index(foodRow,2),mealsHeader.at(meal));
               planerModel->setData(planerModel->index(foodRow,3),weekPlansModel->data(weekPlansModel->index(food,0,mealIndex)).toString());
               planerModel->setData(planerModel->index(foodRow,4),weekPlansModel->data(weekPlansModel->index(food,1,mealIndex)).toInt());
            }
            rowCount = planerModel->rowCount();
        }
    }
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
    QString weekID = QString::number(firstday.weekNumber())+"_"+QString::number(firstday.year());
    schedulePtr->scheduleProxy->invalidate();
    schedulePtr->scheduleProxy->setFilterFixedString(weekID);
    schedulePtr->scheduleProxy->setFilterKeyColumn(0);

    for(int i = 0; i < schedulePtr->scheduleProxy->rowCount(); ++i)
    {
        day = QDate::fromString(schedulePtr->scheduleProxy->data(schedulePtr->scheduleProxy->index(i,1)).toString(),"dd.MM.yyyy").dayOfWeek()-1;
        dayWork = daySumModel->data(daySumModel->index(2,day)).toInt() + schedulePtr->scheduleProxy->data(schedulePtr->scheduleProxy->index(i,9)).toInt();
        daySumModel->setData(daySumModel->index(2,day),dayWork);
    }
    this->update_daySumModel();
}

void foodplanner::update_sumByMenu(QDate firstDay)
{
    int dayCalories = 0;
    QDate dateofday;
    planerProxy->invalidate();
    planerProxy->setFilterKeyColumn(1);

    for(int day = 0; day < dayHeader.count(); ++day)
    {
        dateofday = firstDay.addDays(day);
        planerProxy->setFilterRegExp("\\b"+dateofday.toString("dd.MM.yyyy")+"\\b");

        for(int row = 0 ; row < planerProxy->rowCount(); ++row)
        {
            dayCalories = dayCalories + planerProxy->data(planerProxy->index(row,4)).toInt();
        }

        daySumModel->setData(daySumModel->index(0,day),dayCalories);
        dayCalories = 0;
    }
    this->update_daySumModel();
}
