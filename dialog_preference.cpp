#include "dialog_preference.h"
#include "ui_dialog_preference.h"

Dialog_preference::Dialog_preference(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_preference)
{
    ui->setupUi(this);
}

Dialog_preference::~Dialog_preference()
{
    delete ui;
}
