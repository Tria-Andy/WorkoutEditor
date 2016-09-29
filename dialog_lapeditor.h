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
    void on_pushButton_ok_clicked();
    void on_pushButton_close_clicked();
    void on_radioButton_add_clicked();
    void on_radioButton_update_clicked();
    void on_radioButton_del_clicked();
    void on_comboBox_lap_currentIndexChanged(int index);

    void on_comboBox_edit_currentIndexChanged(int index);

    void on_spinBox_starttime_valueChanged(int arg1);

    void on_spinBox_endtime_valueChanged(int arg1);

private:
    Ui::Dialog_lapeditor *ui;
    Activity *curr_act;
    QStandardItemModel *editModel;

    void set_lapinfo();
    void set_components(bool);
    void set_visible(bool,bool);
    void edit_laps(int,int);
    int calc_strokes(int);
};

#endif // DIALOG_LAPEDITOR_H
