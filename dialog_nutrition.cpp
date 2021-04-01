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

    ui->toolButton_save->setEnabled(false);
    ui->lineEdit_recipeName->setEnabled(false);
    ui->toolButton_clear->setEnabled(false);

    recipeTags = settings::get_xmlMapping("recipes");

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
        treeItem->setData(2,Qt::DisplayRole,recipeItem->child(row,3)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(3,Qt::DisplayRole,recipeItem->child(row,4)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(4,Qt::DisplayRole,recipeItem->child(row,5)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(5,Qt::DisplayRole,recipeItem->child(row,6)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(6,Qt::DisplayRole,recipeItem->child(row,7)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(7,Qt::DisplayRole,recipeItem->child(row,8)->data(Qt::DisplayRole).toDouble());
    }

    ui->lineEdit_recipeName->setText(selItem->data(Qt::DisplayRole).toString());
    ui->lineEdit_recipeName->setAccessibleName(selItem->data(Qt::AccessibleTextRole).toString());
    ui->lineEdit_recipeName->setEnabled(true);
    this->calc_recipeValues();

    ui->toolButton_save->setEnabled(true);
    ui->toolButton_clear->setEnabled(true);
    ui->toolButton_createRecipe->setEnabled(false);

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

    selList->sortItems();
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
    newItem->setData(2,Qt::DisplayRole,round(ui->doubleSpinBox_calories->value()));
    newItem->setData(3,Qt::DisplayRole,ui->doubleSpinBox_carbs->value());
    newItem->setData(4,Qt::DisplayRole,ui->doubleSpinBox_protein->value());
    newItem->setData(5,Qt::DisplayRole,ui->doubleSpinBox_fat->value());
    newItem->setData(6,Qt::DisplayRole,ui->doubleSpinBox_fiber->value());
    newItem->setData(7,Qt::DisplayRole,ui->doubleSpinBox_sugar->value());
    this->calc_recipeValues();
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
    this->calc_recipeValues();
}

void Dialog_nutrition::update_ingredientModel(bool addNew)
{
    QVector<double> foodValues(7,0);

    foodValues[0] = ui->doubleSpinBox_portion->value();
    foodValues[1] = ui->doubleSpinBox_calories->value();
    foodValues[2] = ui->doubleSpinBox_carbs->value();
    foodValues[3] = ui->doubleSpinBox_protein->value();
    foodValues[4] = ui->doubleSpinBox_fat->value();
    foodValues[5] = ui->doubleSpinBox_fiber->value();
    foodValues[6] = ui->doubleSpinBox_sugar->value();

    if(addNew)
    {
        foodPlan->add_ingredient(ui->comboBox_ingredients->currentText(),ui->lineEdit_foodName->text(),foodValues);
        this->set_listItems(foodPlan->ingredModel,ui->listWidget_ingredients,ui->comboBox_ingredients->currentText());
    }
    else
    {
        foodPlan->update_ingredient(ui->lineEdit_foodName->accessibleName(),ui->lineEdit_foodName->text(),foodValues);
    }

    foodPlan->save_ingredList();
}

void Dialog_nutrition::calc_recipeValues()
{
    QTreeWidgetItem *rootItem = ui->treeWidget_recipe->invisibleRootItem();
    QVector<double> foodValues(7,0);
    QTreeWidgetItem *currItem;

    for(int row = 0; row < rootItem->childCount(); ++row)
    {
        currItem = rootItem->child(row);

        for(int col = 1, i= 0; col < recipeHeader->count(); ++col,++i)
        {
            foodValues[i] = currItem->data(col,Qt::DisplayRole).toDouble() + foodValues.at(i);
        }
    }

    ui->label_gramms->setText(QString::number(foodValues.at(0)));
    ui->label_calories->setText(QString::number(round(foodValues.at(1))));

    ui->label_carbs->setText(QString::number(round(foodValues.at(2))));
    ui->label_protein->setText(QString::number(round(foodValues.at(3))));
    ui->label_fat->setText(QString::number(round(foodValues.at(4))));
    ui->label_fiber->setText(QString::number(round(foodValues.at(5))));
    ui->label_sugar->setText(QString::number(round(foodValues.at(6))));
}

void Dialog_nutrition::clear_recipeInfo()
{
    ui->treeWidget_recipe->clear();
    ui->lineEdit_recipeName->clear();
    ui->label_gramms->setText("-");
    ui->label_calories->setText("-");
    ui->label_carbs->setText("-");
    ui->label_protein->setText("-");
    ui->label_fat->setText("-");
    ui->label_fiber->setText("-");
    ui->label_sugar->setText("-");

    ui->toolButton_createRecipe->setEnabled(true);
    ui->toolButton_save->setEnabled(false);
    ui->toolButton_clear->setEnabled(false);
}


void Dialog_nutrition::on_toolButton_edit_clicked()
{
    this->update_ingredientModel(false);
}

void Dialog_nutrition::on_toolButton_addIngred_clicked()
{
    this->update_ingredientModel(true);
}

void Dialog_nutrition::on_toolButton_clear_clicked()
{
    this->clear_recipeInfo();
}

void Dialog_nutrition::on_toolButton_save_clicked()
{
    QTreeWidgetItem *rootItem = ui->treeWidget_recipe->invisibleRootItem();
    QTreeWidgetItem *currItem;
    QList<QStandardItem*> ingredList,macroList;
    QStandardItem *recipeItem;

    macroList.insert(0,new QStandardItem(ui->lineEdit_recipeName->accessibleName()));
    macroList.insert(1,new QStandardItem(ui->lineEdit_recipeName->text()));
    macroList.insert(2,new QStandardItem(ui->label_gramms->text()));
    macroList.insert(3,new QStandardItem(ui->label_calories->text()));
    macroList.insert(4,new QStandardItem(ui->label_carbs->text()));
    macroList.insert(5,new QStandardItem(ui->label_protein->text()));
    macroList.insert(6,new QStandardItem(ui->label_fat->text()));
    macroList.insert(7,new QStandardItem(ui->label_fiber->text()));
    macroList.insert(8,new QStandardItem(ui->label_sugar->text()));
    macroList.at(0)->setData(recipeTags->at(1),Qt::AccessibleTextRole);

    recipeItem = foodPlan->submit_recipes(macroList,ui->comboBox_recipe->currentText(),ui->toolButton_createRecipe->isChecked());

    for(int row = 0; row < rootItem->childCount(); ++row)
    {
        currItem = rootItem->child(row);
        ingredList.clear();

        ingredList.insert(0,new QStandardItem(currItem->data(0,Qt::AccessibleTextRole).toString()));

        for(int col = 0,i = 1; col < recipeHeader->count(); ++col,++i)
        {
            ingredList.insert(i,new QStandardItem(currItem->data(col,Qt::DisplayRole).toString()));
        }
        ingredList.at(0)->setData(recipeTags->at(2),Qt::AccessibleTextRole);
        recipeItem->appendRow(ingredList);
    }

    this->set_listItems(foodPlan->recipeModel,ui->listWidget_recipes,ui->comboBox_recipe->currentText());

    if(ui->toolButton_createRecipe->isChecked()) this->clear_recipeInfo();
    foodPlan->save_recipeList();
}

void Dialog_nutrition::on_toolButton_createRecipe_clicked()
{
    ui->lineEdit_recipeName->setEnabled(true);
    ui->toolButton_save->setEnabled(true);
    ui->lineEdit_recipeName->setAccessibleName(foodPlan->get_newRecipeID(ui->comboBox_recipe->currentText()));
    ui->toolButton_createRecipe->setEnabled(false);
}
