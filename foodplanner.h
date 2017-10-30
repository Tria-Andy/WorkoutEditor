#ifndef FOODPLANNER_H
#define FOODPLANNER_H

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QtXml>
#include "xmlhandler.h"
#include "schedule.h"
#include "settings.h"

class foodplanner : public xmlHandler, public calculation
{
public:
    explicit foodplanner(schedule *ptrSchedule = 0);

    QStandardItemModel *planerModel,*weekModel,*weekSumModel,*daySumModel,*mealModel;
    QSortFilterProxyModel *mealProxy;
    QDate firstDayofWeek;

    void set_headerInfo(QDate);

private:
    schedule *schedulePtr;
    QString filePath,planerXML,mealXML;
    QStringList dayTags,mealTags,mealsHeader,weekHeader,dayHeader,mealList,sumHeader,daySumHeader;
    int dayCalBase;

    void read_foodPlan(QDomDocument);
    void read_meals(QDomDocument);
    void build_weekFoodTree(QDomElement,QStandardItem*);
    QStandardItem *create_item(QDomElement,QStandardItem*);
    void fill_weekModel(QDate);
};

#endif // FOODPLANNER_H
