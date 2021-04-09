#ifndef FOODPLANNER_H
#define FOODPLANNER_H

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QtXml>
#include "schedule.h"
#include "settings.h"

class foodplanner : public xmlHandler, public calculation
{

public:
    explicit foodplanner(schedule *ptrSchedule = nullptr);

    QStandardItemModel *foodPlanModel,*mealModel,*recipeModel,*ingredModel,*drinkModel,*historyModel;
    QStringList *menuHeader,*foodPlanTags,*foodHistTags,*mealsTags,*foodsumHeader,*foodestHeader,*foodhistHeader,*ingredTags,*drinkTags;
    QStringList planList,mealsHeader,dayHeader,dayListHeader;
    QPair<bool,bool> dayMealCopy;
    QHash<QString,QPair<QString,QPair<int,double>>> get_mealtoUpdate(bool,QDate,QString);
    QMap<QDate,QVector<double>> *get_daySumMap() {return &daySumMap;}
    QMap<QDate,QVector<double>> *get_weekSumMap() {return &weekSumMap;}
    QMap<QDate,QHash<QString,QVector<double>>> *get_dayMacroMap() {return &dayMacroMap;}
    QVector<double> get_mealValues(QString,double);
    QMap<QDate,QHash<QString,QHash<QString,QVector<double>>>> *get_foodPlanMap() {return &foodPlanMap;}
    QMap<QPair<QDate,int>,QMap<QDate,QList<QVariant>>> *get_foodHistoryMap() {return &foodHistoryMap;}
    QMap<QDate,QVector<QString>> *get_foodPlanList() {return &foodPlanList;}
    bool updateMap_hasData();
    QString get_mealName(QString key) {return mealsMap.value(key);}
    QString get_mode(QDate);
    QString get_newRecipeID(QString);

    QPair<QString,QVector<int>> get_mealData(QString);
    QMap<QDate,double> get_lastFoodWeek(QDate);
    QModelIndex get_modelIndex(QStandardItemModel*,QString,int);
    QStandardItem *get_modelItem(QStandardItemModel*,QString,int);
    QVector<double> get_foodMacros(QString);

    QStringList get_modelSections(QStandardItemModel*);
    QList<QStandardItem*> get_sectionItems(QStandardItemModel*,QString);
    QStandardItem* submit_recipes(QList<QStandardItem*>,QString,bool);

    void update_foodPlanData(bool,QDate,QDate);
    void update_foodHistory(QDate,QVector<double>);
    void update_foodPlanModel(QDate,QString,QMap<int,QList<QStandardItem*>>);

    void set_daySumMap(QDate);
    void set_weekSumMap();
    //void set_dragandDrop() {update_foodPlanModel();}
    void fill_updateMap(bool,bool,QDate,QString);
    void edit_updateMap(int,QPair<QDate,QString>,QString,double);
    void clear_updateMap() {updateMap.clear();}
    void save_foolPlan();
    void save_mealList();
    void save_recipeList();
    void save_ingredList();
    void update_foodMode(QDate,QString);
    void update_ingredient(QString,QString,QVector<double>);
    void edit_mealSection(QString,int);
    void remove_meal(QItemSelectionModel*);
    void insert_newWeek(QDate);
    void insert_newMeal(QString);
    void remove_week(QString);
    void change_updateMapOrder(QPair<QDate,QString>,QMap<QString,int>);
    void add_ingredient(QString, QString,QVector<double>);

private:
    schedule *schedulePtr;
    QMap<QDate,QList<QTableWidgetItem*>> daySummeryMap;
    QDate firstdayofweek;
    QString dateFormat;
    QString dateSaveFormat;

    QMap<QDate,QHash<QString,QHash<QString,QVector<double>> > > foodPlanMap;
    QMap<QPair<QDate,int>,QMap<QDate,QList<QVariant>>> foodHistoryMap;
    QMap<QPair<bool,QDate>,QHash<QString,QHash<QString,QPair<QString,QPair<int,double>>>>> updateMap;
    QMap<QDate,QVector<double>> daySumMap;   
    QMap<QDate,QVector<double>> weekSumMap;
    QMap<QDate,QHash<QString,QVector<double>>> dayMacroMap;
    QHash<QString,QString> mealsMap;
    QHash<QString,QString> *fileMap;
    QHash<QString,double> *doubleValues;
    QMap<QDate,QVector<QString>> foodPlanList;


    void set_headerLabel(QStandardItemModel*, QStringList*,bool);
    void set_foodHistoryValues();
    void update_foodHistoryModel(QDate);
    void set_foodPlanMap(int);
    void set_dayFoodValues(QStandardItem*);
    void set_foodPlanData(QStandardItem*);
    void update_foodHistory(QDate);
    void check_foodPlan();
};

#endif // FOODPLANNER_H
