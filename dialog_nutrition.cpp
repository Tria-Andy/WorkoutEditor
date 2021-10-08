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

    ui->comboBox_recipe->addItems(foodPlan->get_modelSections(foodPlan->recipeModel));
    ui->comboBox_recipe->model()->sort(0);
    ui->comboBox_ingredients->addItems(foodPlan->get_modelSections(foodPlan->ingredModel));
    ui->comboBox_ingredients->model()->sort(0);
    ui->comboBox_drinks->addItems(foodPlan->get_modelSections(foodPlan->drinkModel));
    ui->comboBox_drinks->model()->sort(0);

    modelMap.insert(0,foodPlan->ingredModel);
    modelMap.insert(1,foodPlan->drinkModel);

    ui->toolButton_save->setEnabled(false);
    ui->lineEdit_recipeName->setEnabled(false);
    ui->toolButton_clear->setEnabled(false);

    ui->toolButton_edit->setVisible(false);
    ui->toolButton_update->setVisible(false);

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
    foodUpdated = false;
    loadedMacros.resize(7);

}

enum {INGED,DRINK};

Dialog_nutrition::~Dialog_nutrition()
{
    delete ui;
}

void Dialog_nutrition::on_toolButton_close_clicked()
{
    if(foodUpdated)
    {
        done(1);
    }
    else
    {
        reject();
    }
}

void Dialog_nutrition::on_comboBox_recipe_currentIndexChanged(const QString &value)
{
    this->set_listItems(foodPlan->recipeModel,ui->listWidget_recipes,value);
}

void Dialog_nutrition::on_comboBox_ingredients_currentIndexChanged(const QString &value)
{    
    this->set_listItems(foodPlan->ingredModel,ui->listWidget_ingredients,value);
}

void Dialog_nutrition::on_comboBox_drinks_currentIndexChanged(const QString &value)
{
    this->set_listItems(foodPlan->drinkModel,ui->listWidget_drinks,value);
}

void Dialog_nutrition::on_listWidget_recipes_itemClicked(QListWidgetItem *selItem)
{
    ui->treeWidget_recipe->clear();

    QStandardItem *recipeItem = foodPlan->get_modelItem(foodPlan->recipeModel,selItem->data(Qt::AccessibleTextRole).toString(),0);
    QTreeWidgetItem *treeItem;

    for(int row = 0; row < recipeItem->rowCount();++row)
    {
        treeItem = new QTreeWidgetItem(ui->treeWidget_recipe->invisibleRootItem());
        treeItem->setData(0,Qt::DisplayRole,recipeItem->child(row,1)->data(Qt::DisplayRole).toString());
        treeItem->setData(0,Qt::AccessibleTextRole,recipeItem->child(row,0)->data(Qt::DisplayRole).toString());
        treeItem->setData(0,Qt::UserRole,recipeItem->child(row,9)->data(Qt::DisplayRole).toInt());
        treeItem->setData(0,Qt::UserRole+1,recipeItem->child(row,10)->data(Qt::DisplayRole).toInt());
        treeItem->setData(1,Qt::DisplayRole,recipeItem->child(row,2)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(2,Qt::DisplayRole,recipeItem->child(row,3)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(3,Qt::DisplayRole,recipeItem->child(row,4)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(4,Qt::DisplayRole,recipeItem->child(row,5)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(5,Qt::DisplayRole,recipeItem->child(row,6)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(6,Qt::DisplayRole,recipeItem->child(row,7)->data(Qt::DisplayRole).toDouble());
        treeItem->setData(7,Qt::DisplayRole,recipeItem->child(row,8)->data(Qt::DisplayRole).toDouble());
    }

    ui->spinBox_port->setValue(recipeItem->index().siblingAtColumn(2).data(Qt::DisplayRole).toInt());

    ui->lineEdit_recipeName->setText(selItem->data(Qt::DisplayRole).toString());
    ui->lineEdit_recipeName->setAccessibleName(selItem->data(Qt::AccessibleTextRole).toString());
    ui->lineEdit_recipeName->setEnabled(true);
    this->calc_recipeValues();

    ui->toolButton_save->setEnabled(true);
    ui->toolButton_clear->setEnabled(true);
    ui->toolButton_createRecipe->setEnabled(false);


    QMap<QString,QList<QDate>> recipeList = foodPlan->get_recipeMap(recipeItem->data(Qt::DisplayRole).toString());
    ui->treeWidget_recipeDays->clear();
    ui->treeWidget_recipeDays->setColumnCount(2);

    for(QMap<QString,QList<QDate>>::const_iterator it = recipeList.cbegin(); it != recipeList.cend(); ++it)
    {
        for(int day = 0; day < it.value().count(); ++day)
        {
            QTreeWidgetItem *dayItem = new QTreeWidgetItem(ui->treeWidget_recipeDays->invisibleRootItem());
            dayItem->setData(0,Qt::DisplayRole,it.value().at(day).toString("dd.MM.yyyy"));
            dayItem->setData(1,Qt::DisplayRole,it.key());
        }
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

    selList->sortItems();
}

void Dialog_nutrition::set_foodMacros(QListWidgetItem *item,QVector<double> foodMacros)
{
    loadedMacros = foodMacros;

    ui->lineEdit_foodName->setText(item->data(Qt::DisplayRole).toString());
    ui->lineEdit_foodName->setAccessibleName(item->data(Qt::AccessibleTextRole).toString());

    ui->doubleSpinBox_portion->setValue(foodMacros.at(0));
    ui->doubleSpinBox_calories->setValue(foodMacros.at(1));
    ui->doubleSpinBox_carbs->setValue(foodMacros.at(2));
    ui->doubleSpinBox_protein->setValue(foodMacros.at(3));
    ui->doubleSpinBox_fat->setValue(foodMacros.at(4));
    ui->doubleSpinBox_fiber->setValue(foodMacros.at(5));
    ui->doubleSpinBox_sugar->setValue(foodMacros.at(6));

    ui->toolButton_addIngred->setVisible(false);
    ui->toolButton_edit->setVisible(true);
}

void Dialog_nutrition::on_listWidget_ingredients_itemClicked(QListWidgetItem *item)
{
    this->set_foodMacros(item,foodPlan->get_foodMacros(foodPlan->ingredModel,item->data(Qt::AccessibleTextRole).toString()));
}

void Dialog_nutrition::on_listWidget_drinks_itemClicked(QListWidgetItem *item)
{
    this->set_foodMacros(item,foodPlan->get_foodMacros(foodPlan->drinkModel,item->data(Qt::AccessibleTextRole).toString()));
}


void Dialog_nutrition::on_toolButton_add_clicked()
{
    QTreeWidgetItem *newItem = new QTreeWidgetItem(ui->treeWidget_recipe->invisibleRootItem());
    newItem->setData(0,Qt::AccessibleTextRole,ui->lineEdit_foodName->accessibleName());
    newItem->setData(0,Qt::DisplayRole,ui->lineEdit_foodName->text());
    newItem->setData(0,Qt::UserRole+1,ui->toolBox_food->currentIndex());
    newItem->setData(1,Qt::DisplayRole,ui->doubleSpinBox_portion->value());
    newItem->setData(2,Qt::DisplayRole,round(ui->doubleSpinBox_calories->value()));
    newItem->setData(3,Qt::DisplayRole,ui->doubleSpinBox_carbs->value());
    newItem->setData(4,Qt::DisplayRole,ui->doubleSpinBox_protein->value());
    newItem->setData(5,Qt::DisplayRole,ui->doubleSpinBox_fat->value());
    newItem->setData(6,Qt::DisplayRole,ui->doubleSpinBox_fiber->value());
    newItem->setData(7,Qt::DisplayRole,ui->doubleSpinBox_sugar->value());
    this->calc_recipeValues();
    this->clear_ingredValues();

    if(ui->treeWidget_recipe->invisibleRootItem()->childCount() > 0) ui->toolButton_clear->setEnabled(true);
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

    loadedMacros = foodPlan->get_foodMacros(modelMap.value(item->data(0,Qt::UserRole+1).toInt()),item->data(0,Qt::AccessibleTextRole).toString());

    ui->lineEdit_foodName->setText(item->data(0,Qt::DisplayRole).toString());
    ui->lineEdit_foodName->setAccessibleName(item->data(0,Qt::AccessibleTextRole).toString());
    ui->doubleSpinBox_portion->setValue(item->data(1,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_calories->setValue(item->data(2,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_carbs->setValue(item->data(3,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_protein->setValue(item->data(4,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_fat->setValue(item->data(5,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_fiber->setValue(item->data(6,Qt::DisplayRole).toDouble());
    ui->doubleSpinBox_sugar->setValue(item->data(7,Qt::DisplayRole).toDouble());

    ui->toolButton_add->setVisible(false);
    ui->toolButton_update->setVisible(true);
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

    ui->toolButton_update->setVisible(false);
    ui->toolButton_add->setVisible(true);
    this->calc_recipeValues();
    this->clear_ingredValues();
}

void Dialog_nutrition::update_ingredientModel(bool addNew,int listID)
{
    QComboBox *sourceBox = nullptr;
    QListWidget *foodList = nullptr;

    if(listID == INGED)
    {
        sourceBox = ui->comboBox_ingredients;
        foodList = ui->listWidget_ingredients;
    }
    else if(listID == DRINK)
    {
       sourceBox = ui->comboBox_drinks;
       foodList = ui->listWidget_drinks;
    }

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
        foodPlan->add_ingredient(sourceBox->currentText(),ui->lineEdit_foodName->text(),foodValues,listID+1);
    }
    else
    {
        foodPlan->update_ingredient(ui->lineEdit_foodName->accessibleName(),ui->lineEdit_foodName->text(),foodValues,listID+1);
    }

    foodPlan->save_ingredList(listID);

    this->clear_ingredValues();
    this->set_listItems(modelMap.value(listID),foodList,sourceBox->currentText());
    foodUpdated = true;
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
    ui->label_calories->setText(QString::number(ceil(foodValues.at(1))));

    ui->label_carbs->setText(QString::number(ceil(foodValues.at(2))));
    ui->label_protein->setText(QString::number(ceil(foodValues.at(3))));
    ui->label_fat->setText(QString::number(ceil(foodValues.at(4))));
    ui->label_fiber->setText(QString::number(ceil(foodValues.at(5))));
    ui->label_sugar->setText(QString::number(ceil(foodValues.at(6))));
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

    ui->toolButton_createRecipe->setChecked(false);
    ui->toolButton_createRecipe->setEnabled(true);
    ui->toolButton_save->setEnabled(false);
    ui->toolButton_clear->setEnabled(false);
}

void Dialog_nutrition::clear_ingredValues()
{
    ui->lineEdit_foodName->clear();
    ui->doubleSpinBox_portion->clear();
    ui->doubleSpinBox_calories->clear();
    ui->doubleSpinBox_carbs->clear();
    ui->doubleSpinBox_protein->clear();
    ui->doubleSpinBox_fat->clear();
    ui->doubleSpinBox_fiber->clear();
    ui->doubleSpinBox_sugar->clear();

    ui->toolButton_addIngred->setVisible(true);
    ui->toolButton_edit->setVisible(false);
}


void Dialog_nutrition::on_toolButton_edit_clicked()
{
    this->update_ingredientModel(false,ui->toolBox_food->currentIndex());
}

void Dialog_nutrition::on_toolButton_addIngred_clicked()
{
    this->update_ingredientModel(true,ui->toolBox_food->currentIndex());
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
    macroList.insert(2,new QStandardItem(QString::number(ui->spinBox_port->value())));
    macroList.insert(3,new QStandardItem(ui->label_calories->text()));
    macroList.insert(4,new QStandardItem(ui->label_carbs->text()));
    macroList.insert(5,new QStandardItem(ui->label_protein->text()));
    macroList.insert(6,new QStandardItem(ui->label_fat->text()));
    macroList.insert(7,new QStandardItem(ui->label_fiber->text()));
    macroList.insert(8,new QStandardItem(ui->label_sugar->text()));
    macroList.insert(9,new QStandardItem(ui->label_gramms->text()));
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
        ingredList.append(new QStandardItem(QString::number(row)));
        ingredList.append(new QStandardItem(currItem->data(0,Qt::UserRole+1).toString()));
        ingredList.at(0)->setData(recipeTags->at(2),Qt::AccessibleTextRole);
        recipeItem->appendRow(ingredList);
    }

    this->set_listItems(foodPlan->recipeModel,ui->listWidget_recipes,ui->comboBox_recipe->currentText());

    if(ui->toolButton_createRecipe->isChecked()) this->clear_recipeInfo();

    foodPlan->save_recipeList();
    foodUpdated = true;
}

void Dialog_nutrition::on_toolButton_createRecipe_clicked()
{
    ui->lineEdit_recipeName->setEnabled(true);
    ui->toolButton_save->setEnabled(true);
    ui->lineEdit_recipeName->setAccessibleName(foodPlan->get_newRecipeID(ui->comboBox_recipe->currentText()));
    ui->toolButton_createRecipe->setEnabled(false);
}

void Dialog_nutrition::on_toolButton_delete_clicked()
{
    ui->treeWidget_recipe->invisibleRootItem()->removeChild(ui->treeWidget_recipe->currentItem());
    ui->treeWidget_recipe->clearSelection();
    ui->toolButton_add->setVisible(true);
    ui->toolButton_update->setVisible(false);
    this->clear_ingredValues();
    this->calc_recipeValues();

    if(ui->treeWidget_recipe->invisibleRootItem()->childCount() == 0) ui->toolButton_clear->setEnabled(false);
}

QTreeWidgetItem* Dialog_nutrition::move_item(int movement)
{
    int currentindex = ui->treeWidget_recipe->currentIndex().row();

    QTreeWidgetItem *moveItem = ui->treeWidget_recipe->takeTopLevelItem(currentindex);
    ui->treeWidget_recipe->insertTopLevelItem(currentindex+movement,moveItem);

    return moveItem;

}

void Dialog_nutrition::on_toolButton_up_clicked()
{
    ui->treeWidget_recipe->setCurrentItem(this->move_item(-1));
}

void Dialog_nutrition::on_toolButton_down_clicked()
{
    ui->treeWidget_recipe->setCurrentItem(this->move_item(1));
}

void Dialog_nutrition::on_toolButton_reset_clicked()
{
    this->clear_ingredValues();
    ui->toolButton_add->setVisible(true);
    ui->toolButton_update->setVisible(false);
}
