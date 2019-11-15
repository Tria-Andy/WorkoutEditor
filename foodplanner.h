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
    QMap<QDate,QVector<double>> dayTarget;
    QMap<QDate,QVector<double>> dayMacros;

    QMap<QDate,QVector<double>> *get_daySumMap() {return &daySumMap;}
    QMap<QDate,QHash<QString,QHash<QString,QVector<double>>>> *get_foodPlanMap() {return &foodPlanMap;}
    QMap<QDate,QString> *get_foodPlanList() {return &foodPlanList;}
    QString get_mealName(QString key) {return mealsMap.value(key);}
    QString get_mode(QDate);
    QVector<int> get_mealData(QString,bool);
    void update_foodPlanMap(bool,QDate,QDate,QString,QString);

    void save_foolPlan();
    void save_mealList();
    void write_foodHistory();
    void edit_mealSection(QString,int);
    void add_meal(QItemSelectionModel*);
    void remove_meal(QItemSelectionModel*);
    void insert_newWeek(QDate);
    void remove_week(QString);


private:
    schedule *schedulePtr;
    QDomNodeList xmlList;
    QString loadedWeek,filePath,planerXML,mealXML,historyXML;
    QStringList dayHistTags;
    QVector<double> defaultCal;
    QMap<QDate,QHash<QString,QHash<QString,QVector<double>> > > foodPlanMap;
    QMap<QDate,QVector<double>> daySumMap;
    QHash<QString,QString> mealsMap;
    QMap<QDate,QString> foodPlanList;

    QVector<double> get_mealValues(QString,double);
    void set_headerLabel(QStandardItemModel*, QStringList*,bool);
    void set_mealsMap();
    void set_foodPlanMap(bool);

    void set_daySumMap(QDate);
    void set_foodPlanList(QStandardItem*);
    void compare_foodToMeal();

    void read_history(QDomDocument);

    QStandardItem *create_item(QDomElement,QStandardItem*);
    void fill_planList(QDate,bool);
    void addrow_mealModel(QStandardItem*,QStringList*);

};

#endif // FOODPLANNER_H
