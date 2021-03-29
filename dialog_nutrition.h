#ifndef DIALOG_NUTRITION_H
#define DIALOG_NUTRITION_H

#include <QDialog>
#include "foodplanner.h"

namespace Ui {
class Dialog_nutrition;
}

class Dialog_nutrition : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_nutrition(QWidget *parent = nullptr,foodplanner *pFood = nullptr);
    ~Dialog_nutrition();

private slots:
    void on_toolButton_close_clicked();
    void on_comboBox_recipe_currentIndexChanged(const QString &arg1);
    void on_comboBox_ingredients_currentIndexChanged(const QString &arg1);
    void on_listWidget_recipes_itemClicked(QListWidgetItem *item);

    void on_listWidget_ingredients_itemClicked(QListWidgetItem *item);

    void on_toolButton_add_clicked();

private:
    Ui::Dialog_nutrition *ui;
    foodplanner *foodPlan;
    QSortFilterProxyModel *foodProxy;
    QStringList *recipeHeader;


    void set_listItems(QStandardItemModel*,QListWidget*,QString);
};

#endif // DIALOG_NUTRITION_H
