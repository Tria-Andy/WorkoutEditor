#ifndef FOODPLANNER_H
#define FOODPLANNER_H

#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include <QtXml>
#include <QJsonObject>
#include "schedule.h"
#include "settings.h"

class foodplanner : public xmlHandler, public calculation
{

public:
    explicit foodplanner(schedule *ptrSchedule = nullptr);

    QStandardItemModel *foodPlanModel,*summeryModel,*recipeModel,*ingredModel,*drinkModel;
    QSortFilterProxyModel *foodProxy,*sumProxy;

    QStringList *menuHeader,*foodPlanTags,*foodHistTags,*foodsumHeader,*foodweekHeader,*ingredTags,*drinkTags;
    QStringList planList,mealsHeader,dayHeader,dayListHeader;

    bool copyMap_hasData();
    bool copyQueue_hasData();
    bool dragDrop_hasData();
    bool check_foodDay(QDate);
    QString get_mode(QDate);
    QString get_newRecipeID(QString);
    QModelIndex get_modelIndex(QStandardItemModel*,QString,int);
    QStandardItem *get_modelItem(QStandardItemModel*,QString,int);
    QStandardItemModel *get_foodModel(int modelID) {return modelMap.value(modelID);}
    QVector<double> get_foodMacros(QStandardItemModel*,QString);
    QPair<QDate,QString> get_copyMeal() {return copyMap;};
    QStringList get_modelSections(QStandardItemModel*);
    QList<QStandardItem*> get_sectionItems(QStandardItemModel*,QString);
    QStandardItem* submit_recipes(QList<QStandardItem*>,QString,bool);
    QMap<QString,QList<QDate>> get_recipeMap(QString recipeID) {return recipeMap.value(recipeID);}
    QStandardItem *get_proxyItem(int);
    int get_slideValue(QDate day) {return slideMap.value(day).second;}
    QPair<double,double> get_estimateWeight(QDateTime date) {return estWeightMap.value(date);}

    void update_foodPlanModel(QDate,QString,QMap<int,QList<QStandardItem*>>);
    void update_ingredient(QString,QString,QVector<double>,int);
    void add_ingredient(QString, QString,QVector<double>,int);
    void set_dayListHeader(QString);

    void set_currentWeek(QDate);
    void update_fromSchedule();
    void update_byPalChange();
    void fill_copyMap(QDate,QString,bool);
    void execute_copy(QDate,bool);
    void clear_dragDrop();
    void check_copyQueue() {if(!copyQueue.isEmpty()) copyQueue.clear();}
    void set_dragDrop(QDate,QString);
    void set_dropMeal(QDate,QString);
    void save_foolPlan();
    void save_historyFile();
    void save_drinkList();
    void save_recipeList();
    void save_ingredList(int);
    void update_foodMode(QDate,QString);
    void add_foodSection(QString,int);
    void insert_newWeek(QDate);
    void remove_week(QString);


private:
    schedule *schedulePtr;
    QDate firstdayofweek;
    QString dateFormat;
    QString dateSaveFormat;
    QHash<QString,QString> *gcValues;
    QQueue<QPair<QDate,QString>> copyQueue;
    QPair<QDate,QString> copyMap;
    QMap<QDate,QPair<int,int>> slideMap;
    QMap<QDateTime,QPair<double,double>> estWeightMap;
    QMap<int,QStandardItemModel*> modelMap;
    QMap<QString,QMap<QString,QList<QDate>>> recipeMap;
    QPair<QDate,QString> dragDrop;
    QMap<QDate,QPair<QString,QVector<int>>> historyMap;
    QHash<QString,QString> *fileMap;
    QHash<QString,double> *doubleValues;

    QMap<int,QList<QStandardItem *>> get_copyItem(QDate,QString);

    void set_headerLabel(QStandardItemModel*, QStringList*,bool);
    void read_nutritionHistory();
    void set_recipeList(QDate,QString,QString);
    void update_summeryModel(QDate,QStandardItem*,bool);
    void set_summeryData(QStandardItem*);
    void update_foodHistory(QDate);
    void check_foodPlan();

};

#endif // FOODPLANNER_H
