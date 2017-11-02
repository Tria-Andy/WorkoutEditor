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
    explicit foodplanner(schedule *ptrSchedule = 0,QDate fd = QDate());

    QStandardItemModel *weekSumModel,*daySumModel,*mealModel;
    QSortFilterProxyModel *planerProxy,*mealProxy;
    QDate firstDayofWeek;
    QStringList planList,mealsHeader,dayHeader;

    void fill_plannerModel();
    void update_sumBySchedule(QDate);
    void update_sumByMenu(QDate);

private:
    schedule *schedulePtr;
    QStandardItemModel *weekPlansModel,*planerModel;
    QString filePath,planerXML,mealXML;
    QStringList dayTags,mealTags,weekHeader,mealList,sumHeader,daySumHeader,weekSumHeader;
    int dayCalBase;

    void read_foodPlan(QDomDocument);
    void read_meals(QDomDocument);
    void build_weekFoodTree(QDomElement,QStandardItem*);
    QStandardItem *create_item(QDomElement,QStandardItem*);
    void set_Models(QDate);
    void update_daySumModel();
    void update_weekSumModel();

private slots:

};

#endif // FOODPLANNER_H
