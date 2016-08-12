#ifndef DIALOG_LAPEDITOR_H
#define DIALOG_LAPEDITOR_H

#include <QDialog>
#include "activity.h"

namespace Ui {
class Dialog_lapeditor;
}

class Dialog_lapeditor : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_lapeditor(QWidget *parent = 0,Activity *p_act = 0);
    ~Dialog_lapeditor();

private slots:
    void on_pushButton_close_clicked();
    void on_radioButton_add_clicked();
    void on_radioButton_del_clicked();
    void on_pushButton_ok_clicked();

private:
    Ui::Dialog_lapeditor *ui;
    Activity *curr_act;
    void set_components(bool);
    void edit_laps(bool,int);
};

#endif // DIALOG_LAPEDITOR_H
