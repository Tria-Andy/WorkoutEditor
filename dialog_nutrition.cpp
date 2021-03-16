#include "dialog_nutrition.h"
#include "ui_dialog_nutrition.h"

Dialog_nutrition::Dialog_nutrition(QWidget *parent, foodplanner *p_food) :
    QDialog(parent),
    ui(new Ui::Dialog_nutrition)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    ui->comboBox_meals->addItem("Hauptspeise");
    ui->comboBox_meals->addItem("Beilage");
}

Dialog_nutrition::~Dialog_nutrition()
{
    delete ui;
}

void Dialog_nutrition::on_toolButton_close_clicked()
{
    reject();
}
