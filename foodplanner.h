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

    QStandardItemModel *foodPlanModel,*historyModel,*recipeModel,*ingredModel,*drinkModel;
    QStringList *menuHeader,*foodPlanTags,*foodHistTags,*foodsumHeader,*foodestHeader,*foodhistHeader,*ingredTags,*drinkTags;
    QStringList planList,mealsHeader,dayHeader,dayListHeader;

    QMap<QDate,QVector<double>> *get_daySumMap() {return &daySumMap;}
    QMap<QDate,QVector<double>> *get_weekSumMap() {return &weekSumMap;}
    QMap<QDate,QHash<QString,QVector<double>>> *get_dayMacroMap() {return &dayMacroMap;}
    QMap<QPair<QDate,int>,QMap<QDate,QList<QVariant>>> *get_foodHistoryMap() {return &foodHistoryMap;}
    QMap<QDate,QVector<QString>> *get_foodPlanList() {return &foodPlanList;}
    bool copyMap_hasData();
    bool dragDrop_hasData();
    QString get_mode(QDate);
    QString get_newRecipeID(QString);
    QMap<QDate,double> get_lastFoodWeek(QDate);
    QModelIndex get_modelIndex(QStandardItemModel*,QString,int);
    QStandardItem *get_modelItem(QStandardItemModel*,QString,int);
    QVector<double> get_foodMacros(QStandardItemModel*,QString);
    QPair<QDate,QString> get_copyMeal();
    QStringList get_modelSections(QStandardItemModel*);
    QList<QStandardItem*> get_sectionItems(QStandardItemModel*,QString);
    QStandardItem* submit_recipes(QList<QStandardItem*>,QString,bool);

    void update_foodHistory(QDate,QVector<double>);
    void update_foodPlanModel(QDate,QString,QMap<int,QList<QStandardItem*>>);

    void set_daySumMap(QDate);
    void set_weekSumMap();
    void fill_copyMap(QDate,QString);
    void execute_copy(QDate);
    void clear_copyMap() {copyMap.clear();}
    void clear_dragDrop();
    void set_dragDrop(QDate,QString);
    void set_dropMeal(QDate,QString);
    void save_foolPlan();
    void save_drinkList();
    void save_recipeList();
    void save_ingredList(int);
    void update_foodMode(QDate,QString);
    void update_ingredient(QString,QString,QVector<double>);
    void edit_mealSection(QString,int);
    void insert_newWeek(QDate);
    void remove_week(QString);
    void add_ingredient(QString, QString,QVector<double>);

private:
    schedule *schedulePtr;
    QDate firstdayofweek;
    QString dateFormat;
    QString dateSaveFormat;
    QQueue<QMap<QDate,QStringList>> copyQueue;
    QMap<QDate,QStringList> copyMap;
    QPair<QDate,QString> dragDrop;
    QMap<QPair<QDate,int>,QMap<QDate,QList<QVariant>>> foodHistoryMap;
    QMap<QDate,QVector<double>> daySumMap;   
    QMap<QDate,QVector<double>> weekSumMap;
    QMap<QDate,QHash<QString,QVector<double>>> dayMacroMap;
    QHash<QString,QString> *fileMap;
    QHash<QString,double> *doubleValues;
    QMap<QDate,QVector<QString>> foodPlanList;

    QMap<int,QList<QStandardItem *>> get_copyItem(QDate,QString);

    void set_headerLabel(QStandardItemModel*, QStringList*,bool);
    void set_foodHistoryValues();
    void update_foodHistoryModel(QDate);
    void set_foodPlanMap(int);
    void set_foodPlanData(QStandardItem*);
    void update_foodHistory(QDate);
    void check_foodPlan();
};

#endif // FOODPLANNER_H
