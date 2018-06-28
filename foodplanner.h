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

    QStandardItemModel *weekPlansModel,*weekSumModel,*daySumModel,*mealModel,*estModel;
    QDate firstDayofWeek;
    QStringList planList,mealsHeader,dayHeader,estHeader;
    QMap<int,QStringList> foodList;

    QString set_weekID(QDate);
    void write_foodPlan();
    void write_meals();
    void edit_mealSection(QString,int);
    void add_meal(QItemSelectionModel*);
    void remove_meal(QItemSelectionModel*);
    QVector<int> get_mealData(QString);
    QVector<double> calPercent,defaultCal;
    void fill_plannerModel();
    void insert_newWeek(QDate);
    void remove_week(QString);
    void update_sumBySchedule(QDate);
    void update_sumByMenu(QDate,int, QStringList*,bool);
    QStringList get_mealList(QString);

private:
    schedule *schedulePtr;
    QString loadedWeek,filePath,planerXML,mealXML;
    QStringList dayTags,sectionTags,mealTags,weekHeader,sumHeader,daySumHeader,weekSumHeader;

    void read_foodPlan(QDomDocument);
    void read_meals(QDomDocument);
    void build_weekFoodTree(QDomElement,QStandardItem*);
    QStandardItem *create_item(QDomElement,QStandardItem*);
    void fill_planList(QDate,bool);
    void addrow_mealModel(QStandardItem*,QStringList*);

    int read_dayCalories(QDate);
    void calc_weekGoal();
    void update_weekPlanModel(QDate,int,QStringList*);
    void update_daySumModel();
    void update_weekSumModel();

private slots:

};

#endif // FOODPLANNER_H
