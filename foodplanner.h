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
    explicit foodplanner(schedule *ptrSchedule = nullptr);

    QStandardItemModel *foodPlanModel,*mealModel,*historyModel;
    QStringList *menuHeader,*foodsumHeader,*foodestHeader,*foodhistHeader;
    QStringList planList,mealsHeader,dayHeader,dayListHeader;
    bool dayCopy;
    QMap<QDate,QVector<double>> dayTarget;
    QMap<QDate,QVector<double>> dayMacros;
    QHash<QString,QPair<QString,QPair<int,double>>> get_mealtoUpdate(bool,QDate,QString);
    QMap<QDate,QVector<double>> *get_daySumMap() {return &daySumMap;}
    QVector<double> get_mealValues(QString,double);
    QMap<QDate,QHash<QString,QHash<QString,QVector<double>>>> *get_foodPlanMap() {return &foodPlanMap;}
    QMap<QDate,QVector<QString>> *get_foodPlanList() {return &foodPlanList;}
    bool updateMap_hasData();
    QString get_mealName(QString key) {return mealsMap.value(key);}
    QString get_mode(QDate);
    QPair<QString,QVector<int>> get_mealData(QString);

    void update_foodPlanData(QHash<QString,QMap<QDate,QString>>);
    void update_foodPlanData(QDate,QDate);
    void fill_updateMap(bool,QDate,QString);
    void edit_updateMap(QPair<QDate,QString>,QString,double);
    void clear_updateMap() {updateMap.clear();}
    void save_foolPlan();
    void save_mealList();
    void edit_mealSection(QString,int);
    void add_meal(QItemSelectionModel*);
    void remove_meal(QItemSelectionModel*);
    void insert_newWeek(QDate);
    void remove_week(QString);


private:
    schedule *schedulePtr;
    QMap<QDate,QHash<QString,QHash<QString,QVector<double>> > > foodPlanMap;
    QMap<QPair<bool,QDate>,QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>>> updateMap;
    QMap<QDate,QVector<double>> daySumMap;
    QHash<QString,QString> mealsMap;
    QHash<QString,QString> *fileMap;
    QMap<QDate,QVector<QString>> foodPlanList;
    QDate firstdayofweek;
    QString dateFormat;
    QString dateSaveFormat;

    QModelIndex get_modelIndex(QStandardItemModel*,QString);
    QStandardItem *get_modelItem(QStandardItemModel*,QString);
    void set_headerLabel(QStandardItemModel*, QStringList*,bool);
    void set_mealsMap();
    void update_foodPlanModel();
    void set_foodPlanMap(int);
    void set_dayFoodValues(QStandardItem*);
    void set_foodPlanData(QStandardItem*);
    void add_toHistory(QDate);
    void set_daySumMap(QDate);
    void set_foodPlanList(QStandardItem*);
    void check_foodPlan();
};

#endif // FOODPLANNER_H
