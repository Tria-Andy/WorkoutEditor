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
    void on_pushButton_ok_clicked();

    void on_radioButton_copy_clicked();

    void on_radioButton_save_clicked();

    void on_radioButton_clear_clicked();

private:
    Ui::Dialog_week_copy *ui;
    schedule *workSched;
    QStringList weekList;
    int editMode;
    void editWeek();
};

#endif // DIALOG_WEEK_COPY_H
