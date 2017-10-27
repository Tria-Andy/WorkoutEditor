#ifndef DIALOG_FOOD_H
#define DIALOG_FOOD_H

#include <QDialog>
#include <QStandardItemModel>
#include <QStyledItemDelegate>
#include "xmlhandler.h"
#include "schedule.h"

namespace Ui {
class Dialog_food;
}

class Dialog_food : public QDialog, public xmlHandler
{
    Q_OBJECT

public:
    explicit Dialog_food(QWidget *parent = 0,schedule *ptrSchedule = 0);
    ~Dialog_food();

private slots:
    void on_toolButton_close_clicked();

    void on_toolButton_next_clicked();

    void on_toolButton_prev_clicked();

private:
    Ui::Dialog_food *ui;

    schedule *schedulePtr;
    QString filePath,planerXML,mealXML;
    QStandardItemModel *planerModel,*ingredModel,*weekModel,*sumModel;
    QSortFilterProxyModel *weekProxy;
    QListModel *mealModel;
    QStringList dayTags,mealsHeader,weekHeader,dayHeader;
    QDate firstDayofWeek;

    void set_headerInfo(QDate);
    void read_foodPlan(QDomDocument);
    void build_weekFoodTree(QDomElement,QStandardItem*,int);
    QStandardItem *create_item(QDomElement,QStandardItem*);
    void fill_weekModel(QDate);
};

#endif // DIALOG_FOOD_H
