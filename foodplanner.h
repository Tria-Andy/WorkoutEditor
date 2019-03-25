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
    explicit foodplanner(schedule *ptrSchedule = nullptr,QDate fd = QDate());

    QStandardItemModel *weekPlansModel,*weekSumModel,*daySumModel,*mealModel,*estModel,*historyModel;
    QSortFilterProxyModel *daySumProxy;
    QDate firstDayofWeek;
    QStringList planList,mealsHeader,menuHeader,dayHeader,dayListHeader,estHeader;
    QMap<int,QStringList> foodList;

    void write_foodPlan();
    void write_meals(bool);
    void write_foodHistory();
    void edit_mealSection(QString,int);
    void add_meal(QItemSelectionModel*);
    void remove_meal(QItemSelectionModel*);
    QVector<int> get_mealData(QString,bool);
    QVector<double> calPercent,defaultCal;
    void fill_plannerModel();
    void insert_newWeek(QDate);
    void remove_week(QString);
    void update_sumBySchedule(QDate);
    void update_sumByMenu(QDate,int, QStringList*,bool);
    QVector<int> calc_FoodMacros(QString, double);
    QStringList get_mealList(QString);
    QMap<QDate,QVector<double>> dayTarget;
    QMap<QDate,QVector<double>> dayMacros;


private:
    schedule *schedulePtr;
    QString loadedWeek,filePath,planerXML,mealXML,historyXML;
    QStringList dayTags,weekTags,dayHistTags,sectionTags,mealTags,sumHeader,daySumHeader,weekSumHeader,histHeader;

    void read_foodPlan(QDomDocument);
    void read_meals(QDomDocument);
    void read_history(QDomDocument);
    void build_weekFoodTree(QDomElement,QStandardItem*);
    QStandardItem *create_item(QDomElement,QStandardItem*);
    void fill_planList(QDate,bool);
    void addrow_mealModel(QStandardItem*,QStringList*);

    int read_dayCalories(QDate);
    void calc_weekGoal();
    void update_weekPlanModel(QDate,int,QStringList*);
    void update_historyModel();
    void set_foodMacros(QDate,QString,double);

private slots:
    void update_daySumModel();
    void update_weekSumModel();
};

#endif // FOODPLANNER_H
