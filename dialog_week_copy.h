#ifndef DIALOG_WEEK_COPY_H
#define DIALOG_WEEK_COPY_H

#include <QDialog>
#include <QMessageBox>
#include "schedule.h"

namespace Ui {
class Dialog_week_copy;
}

class Dialog_week_copy : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_week_copy(QWidget *parent = 0,QString selectedWeek = 0, schedule *p_sched = 0);
    ~Dialog_week_copy();

private slots:
    void on_pushButton_cancel_clicked();
    void on_pushButton_copy_clicked();

private:
    Ui::Dialog_week_copy *ui;
    schedule *workSched;
    QStringList weekList;
};

#endif // DIALOG_WEEK_COPY_H
