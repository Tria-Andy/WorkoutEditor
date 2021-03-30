#include "dialog_nutrition.h"
#include "ui_dialog_nutrition.h"

Dialog_nutrition::Dialog_nutrition(QWidget *parent, foodplanner *p_food) :
    QDialog(parent),
    ui(new Ui::Dialog_nutrition)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->setFixedSize(settings::screenSize.first/1.5,settings::screenSize.second/1.6);

    foodPlan = p_food;

    foodProxy = new QSortFilterProxyModel();

    ui->comboBox_recipe->addItems(foodPlan->get_modelSections(foodPlan->recipeModel));
    ui->comboBox_ingredients->addItems(foodPlan->get_modelSections(foodPlan->ingredModel));

    recipeHeader = settings::getHeaderMap("recipeheader");
    QTreeWidgetItem *headerItem = new QTreeWidgetItem();
    for(int i = 0; i < recipeHeader->count(); ++i)
    {
        headerItem->setData(i,Qt::DisplayRole,recipeHeader->at(i));
    }
    ui->treeWidget_recipe->setHeaderItem(headerItem);
    ui->treeWidget_recipe->header()->setSectionResizeMode(QHeaderView::Stretch);
    ui->treeWidget_recipe->header()->setSectionResizeMode(0,QHeaderView::ResizeToContents);
    ui->treeWidget_recipe->setAcceptDrops(true);
    ui->treeWidget_recipe->setDragEnabled(true);
    ui->treeWidget_recipe->setDragDropMode(QAbstractItemView::DragDrop);
    ui->treeWidget_recipe->setEnabled(true);

    loadedMacros.resize(7);

}

Dialog_nutrition::~Dialog_nutrition()
{
    delete ui;
}

void Dialog_nutrition::on_toolButton_close_clicked()
{
    reject();
}

void Dialog_nutrition::on_comboBox_recipe_currentIndexChanged(const QString &value)
{
    this->set_listItems(foodPlan->recipeModel,ui->listWidget_recipes,value);
}

void Dialog_nutrition::on_comboBox_ingredients_currentIndexChanged(const QString &value)
{    
    this->set_listItems(foodPlan->ingredModel,ui->listWidget_ingredients,value);
}

void Dialog_nutrition::on_listWidget_recipes_itemClicked(QListWidgetItem *selItem)
{
    ui->treeWidget_recipe->clear();

    QStandardItem *recipeItem = foodPlan->get_modelItem(foodPlan->recipeModel,selItem->data(Qt::AccessibleTextRole).toString(),0);
    QTreeWidgetItem *treeItem;

    for(int row = 0; row < recipeItem->rowCount();++row)
    {
        treeItem = new QTreeWidgetItem(ui->treeWidget_recipe->invisibleRootItem());
        treeItem->setData(0,Qt::AccessibleTextRole,recipeItem->child(row,0)->data(Qt::DisplayRole).toString());
        treeItem->setData(0,Qt::DisplayRole,recipeItem->child(row,1)->data(Qt::DisplayRole).toString());
        treeItem->setData(1,Qt::DisplayRole,recipeItem->child(row,2)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(2,Qt::DisplayRole,recipeItem->child(row,3)->data(Qt::DisplayRole).toInt());
        treeItem->setData(3,Qt::DisplayRole,recipeItem->child(row,4)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(4,Qt::DisplayRole,recipeItem->child(row,5)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(5,Qt::DisplayRole,recipeItem->child(row,6)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(6,Qt::DisplayRole,recipeItem->child(row,7)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(7,Qt::DisplayRole,recipeItem->child(row,8)->data(Qt::DisplayRole).toDouble());
    }
}

void Dialog_nutrition::set_listItems(QStandardItemModel *model,QListWidget *selList,QString value)
{
    selList->clear();
    QList<QStandardItem*> items = foodPlan->get_sectionItems(model,value);

    QListWidgetItem *listItem;

    for(int count = 0; count < items.count(); ++count)
    {
        listItem = new QListWidgetItem();
        listItem->setData(Qt::DisplayRole,items.at(count)->data(Qt::DisplayRole).toString());
        listItem->setData(Qt::AccessibleTextRole,items.at(count)->data(Qt::AccessibleTextRole).toString());
        selList->addItem(listItem);
    }
}

void Dialog_nutrition::on_listWidget_ingredients_itemClicked(QListWidgetItem *item)
{
    loadedMacros = foodPlan->get_foodMacros(item->data(Qt::AccessibleTextRole).toString());

    ui->lineEdit_foodName->setText(item->data(Qt::DisplayRole).toString());
    ui->lineEdit_foodName->setAccessibleName(item->data(Qt::AccessibleTextRole).toString());
    ui->doubleSpinBox_portion->setValue(loadedMacros.at(0));
    ui->doubleSpinBox_calories->setValue(loadedMacros.at(1));
    ui->doubleSpinBox_carbs->setValue(loadedMacros.at(2));
    ui->doubleSpinBox_protein->setValue(loadedMacros.at(3));
    ui->doubleSpinBox_fat->setValue(loadedMacros.at(4));
    ui->doubleSpinBox_fiber->setValue(loadedMacros.at(5));
    ui->doubleSpinBox_sugar->setValue(loadedMacros.at(6));
}

void Dialog_nutrition::on_toolButton_add_clicked()
{
    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->treeWidget_recipe->invisibleRootItem());
    newItem->setData(0,Qt::AccessibleTextRole,ui->lineEdit_foodName->accessibleName());
    newItem->setData(0,Qt::DisplayRole,ui->lineEdit_foodName->text());
    newItem->setData(1,Qt::DisplayRole,ui->doubleSpinBox_portion->value());
    newItem->setData(2,Qt::DisplayRole,ui->doubleSpinBox_calories->value());
    newItem->setData(3,Qt::DisplayRole,ui->doubleSpinBox_carbs->value());
    newItem->setData(4,Qt::DisplayRole,ui->doubleSpinBox_protein->value());
    newItem->setData(5,Qt::DisplayRole,ui->doubleSpinBox_fat->value());
    newItem->setData(6,Qt::DisplayRole,ui->doubleSpinBox_fiber->value());
    newItem->setData(7,Qt::DisplayRole,ui->doubleSpinBox_sugar->value());
}

void Dialog_nutrition::on_doubleSpinBox_portion_valueChanged(double value)
{
    double factor = value / 100.0;

    ui->doubleSpinBox_calories->setValue(loadedMacros.at(1)*factor);
    ui->doubleSpinBox_carbs->setValue(loadedMacros.at(2)*factor);
    ui->doubleSpinBox_protein->setValue(loadedMacros.at(3)*factor);
    ui->doubleSpinBox_fat->setValue(loadedMacros.at(4)*factor);
    ui->doubleSpinBox_fiber->setValue(loadedMacros.at(5)*factor);
    ui->doubleSpinBox_sugar->setValue(loadedMacros.at(6)*factor);
}

void Dialog_nutrition::on_treeWidget_recipe_itemClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column)

    loadedMacros = foodPlan->get_foodMacros(item->data(0,Qt::AccessibleTextRole).toString());

    ui->lineEdit_foodName->setText(item->data(0,Qt::DisplayRole).toString());
    ui->lineEdit_foodName->setAccessibleName(item->data(0,Qt::AccessibleTextRole).toString());
    ui->doubleSpinBox_portion->setValue(item->data(1,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_calories->setValue(item->data(2,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_carbs->setValue(item->data(3,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_protein->setValue(item->data(4,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_fat->setValue(item->data(5,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_fiber->setValue(item->data(6,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_sugar->setValue(item->data(7,Qt::DisplayRole).toDouble());
}

void Dialog_nutrition::on_toolButton_update_clicked()
{
    QTreeWidgetItem *currItem = ui->treeWidget_recipe->currentItem();
    currItem->setData(1,Qt::DisplayRole,ui->doubleSpinBox_portion->value());
    currItem->setData(2,Qt::DisplayRole,ui->doubleSpinBox_calories->value());
    currItem->setData(3,Qt::DisplayRole,ui->doubleSpinBox_carbs->value());
    currItem->setData(4,Qt::DisplayRole,ui->doubleSpinBox_protein->value());
    currItem->setData(5,Qt::DisplayRole,ui->doubleSpinBox_fat->value());
    currItem->setData(6,Qt::DisplayRole,ui->doubleSpinBox_fiber->value());
    currItem->setData(7,Qt::DisplayRole,ui->doubleSpinBox_sugar->value());
}

void Dialog_nutrition::on_toolButton_edit_clicked()
{
    QVector<double> foodValues(6,0);

    foodValues[0] = ui->doubleSpinBox_calories->value();
    foodValues[1] = ui->doubleSpinBox_carbs->value();
    foodValues[2] = ui->doubleSpinBox_protein->value();
    foodValues[3] = ui->doubleSpinBox_fat->value();
    foodValues[4] = ui->doubleSpinBox_fiber->value();
    foodValues[5] = ui->doubleSpinBox_sugar->value();

    foodPlan->update_ingredient(ui->lineEdit_foodName->accessibleName(),foodValues);
}
