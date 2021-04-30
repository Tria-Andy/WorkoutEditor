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

    QStringList *menuHeader,*foodPlanTags,*foodHistTags,*foodsumHeader,*foodestHeader,*foodhistHeader,*ingredTags,*drinkTags;
    QStringList planList,mealsHeader,dayHeader,dayListHeader;

    bool copyMap_hasData();
    bool dragDrop_hasData();
    bool check_foodDay(QDate);
    QString get_mode(QDate);
    QString get_newRecipeID(QString);
    QModelIndex get_modelIndex(QStandardItemModel*,QString,int);
    QStandardItem *get_modelItem(QStandardItemModel*,QString,int);
    QVector<double> get_foodMacros(QStandardItemModel*,QString);
    QPair<QDate,QString> get_copyMeal();
    QStringList get_modelSections(QStandardItemModel*);
    QList<QStandardItem*> get_sectionItems(QStandardItemModel*,QString);
    QStandardItem* submit_recipes(QList<QStandardItem*>,QString,bool);
    QStandardItem *get_proxyItem(int);
    int get_slideValue(QDate day) {return slideMap.value(day);}
    void update_foodPlanModel(QDate,QString,QMap<int,QList<QStandardItem*>>);

    void set_currentWeek(QDate);
    void update_fromSchedule(QDate);
    void fill_copyMap(QDate,QString);
    void execute_copy(QDate,bool);
    void clear_copyMap() {copyMap.clear();}
    void clear_dragDrop();
    void set_dragDrop(QDate,QString);
    void set_dropMeal(QDate,QString);
    void save_foolPlan();
    void save_historyFile();
    void save_drinkList();
    void save_recipeList();
    void save_ingredList(int);
    void update_foodMode(QDate,QString);
    void update_ingredient(QString,QString,QVector<double>);
    void edit_mealSection(QString,int);
    void insert_newWeek(QDate);
    void remove_week(QString);
    void add_ingredient(QString, QString,QVector<double>);

public slots:
    void scheduleChanged(QStandardItem*);

private:
    schedule *schedulePtr;
    QDate firstdayofweek;
    QString dateFormat;
    QString dateSaveFormat;
    QHash<QString,QString> *gcValues;
    QQueue<QPair<QDate,QString>> copyQueue;
    QMap<QDate,QStringList> copyMap;
    QMap<QDate,int> slideMap;
    QPair<QDate,QString> dragDrop;
    QMap<QDate,QPair<QString,QVector<int>>> historyMap;
    QHash<QString,QString> *fileMap;
    QHash<QString,double> *doubleValues;

    QMap<int,QList<QStandardItem *>> get_copyItem(QDate,QString);

    void set_headerLabel(QStandardItemModel*, QStringList*,bool);
    void read_nutritionHistory();
    void update_summeryModel(QDate,QStandardItem*,bool);
    void set_summeryData(QStandardItem*);
    void update_foodHistory(QDate);
    void check_foodPlan();

};

#endif // FOODPLANNER_H
