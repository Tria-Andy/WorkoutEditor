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
    QPair<bool,bool> dayMealCopy;
    QHash<QString,QPair<QString,QPair<int,double>>> get_mealtoUpdate(bool,QDate,QString);
    QMap<QDate,QVector<double>> *get_daySumMap() {return &daySumMap;}
    QMap<QDate,QHash<QString,QVector<double>>> *get_dayMacroMap() {return &dayMacroMap;}
    QVector<double> get_mealValues(QString,double);
    QMap<QDate,QHash<QString,QHash<QString,QVector<double>>>> *get_foodPlanMap() {return &foodPlanMap;}
    QMap<QDate,QVector<QString>> *get_foodPlanList() {return &foodPlanList;}
    bool updateMap_hasData();
    QString get_mealName(QString key) {return mealsMap.value(key);}
    QString get_mode(QDate);
    QPair<QString,QVector<int>> get_mealData(QString);

    void update_foodPlanData(bool,QDate,QDate);
    void set_dragandDrop() {update_foodPlanModel();}
    void fill_updateMap(bool,bool,QDate,QString);
    void edit_updateMap(int,QPair<QDate,QString>,QString,double);
    void clear_updateMap() {updateMap.clear();}
    void save_foolPlan();
    void save_mealList();
    void edit_mealSection(QString,int);
    void add_meal(QItemSelectionModel*);
    void remove_meal(QItemSelectionModel*);
    void insert_newWeek(QDate);
    void remove_week(QString);
    void change_updateMapOrder(QPair<QDate,QString>,QMap<QString,int>);


private:
    schedule *schedulePtr;
    QMap<QDate,QHash<QString,QHash<QString,QVector<double>> > > foodPlanMap;
    QMap<QPair<bool,QDate>,QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>>> updateMap;
    QMap<QDate,QVector<double>> daySumMap;
    QMap<QDate,QHash<QString,QVector<double>>> dayMacroMap;
    QHash<QString,QString> mealsMap;
    QHash<QString,QString> *fileMap;
    QHash<QString,double> *doubleValues;
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
