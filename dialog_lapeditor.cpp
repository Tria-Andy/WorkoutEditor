#include "dialog_lapeditor.h"
#include "ui_dialog_lapeditor.h"

Dialog_lapeditor::Dialog_lapeditor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_lapeditor)
{
    ui->setupUi(this);
}

Dialog_lapeditor::~Dialog_lapeditor()
{
    delete ui;
}
