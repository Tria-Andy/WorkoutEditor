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

    QTreeWidgetItem *rootItem = ui->treeWidget_recipe->invisibleRootItem();
    QTreeWidgetItem *treeItem;

    qDebug() << recipeItem->rowCount();

    for(int row = 0; row < recipeItem->rowCount();++row)
    {
        treeItem = new QTreeWidgetItem(rootItem);
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
    QVector<double> foodMacros = foodPlan->get_foodMacros(item->data(Qt::AccessibleTextRole).toString());

    ui->lineEdit_foodName->setText(item->data(Qt::DisplayRole).toString());
    ui->doubleSpinBox_portion->setValue(foodMacros.at(0));
    ui->doubleSpinBox_calories->setValue(foodMacros.at(1));
    ui->doubleSpinBox_carbs->setValue(foodMacros.at(2));
    ui->doubleSpinBox_protein->setValue(foodMacros.at(3));
    ui->doubleSpinBox_fat->setValue(foodMacros.at(4));
    ui->doubleSpinBox_fiber->setValue(foodMacros.at(5));
    ui->doubleSpinBox_sugar->setValue(foodMacros.at(6));
}

void Dialog_nutrition::on_toolButton_add_clicked()
{
    //QTreeWidgetItem *newItem;



}
