#include "dialog_week_copy.h"
#include "ui_dialog_week_copy.h"

Dialog_week_copy::Dialog_week_copy(QWidget *parent,QString selected_week,schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::Dialog_week_copy)
{
    ui->setupUi(this);
    ui->lineEdit_currweek->setText(selected_week.split("#").first());
    workSched = p_sched;

    for(int i = 0; i < workSched->week_meta->rowCount(); ++i)
    {
        weekList << workSched->week_meta->data(workSched->week_meta->index(i,1,QModelIndex())).toString();
    }
    ui->comboBox_copyto->addItems(weekList);

}

Dialog_week_copy::~Dialog_week_copy()
{
    delete ui;
}

void Dialog_week_copy::on_pushButton_cancel_clicked()
{
    reject();
}

void Dialog_week_copy::on_pushButton_copy_clicked()
{
    QString wFrom = ui->lineEdit_currweek->text();
    QString wTo = ui->comboBox_copyto->currentText();

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this,
                                  "Cody Week",
                                  "Copy Week "+wFrom+" to Week "+wTo+"?",
                                  QMessageBox::Yes|QMessageBox::No
                                  );
    if (reply == QMessageBox::Yes)
    {
        workSched->set_copyWeeks(wFrom,wTo);
        accept();
    }
}
