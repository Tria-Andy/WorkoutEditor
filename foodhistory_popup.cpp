#include "foodhistory_popup.h"
#include "ui_foodhistory_popup.h"

foodhistory_popup::foodhistory_popup(QWidget *parent,foodplanner *pFood) :
    QDialog(parent),
    ui(new Ui::foodhistory_popup)
{
    ui->setupUi(this);

    foodplan = pFood;
    ui->treeView_foodhistory->setModel(foodplan->historyModel);
}

foodhistory_popup::~foodhistory_popup()
{
    delete ui;
}
