#ifndef DIALOG_STRESSCALC_H
#define DIALOG_STRESSCALC_H

#include <QDialog>
#include "settings.h"


namespace Ui {
class Dialog_stresscalc;
}

class Dialog_stresscalc : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_stresscalc(QWidget *parent = 0);
    ~Dialog_stresscalc();

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_calc_clicked();

    void on_comboBox_sport_currentIndexChanged(int index);

    void on_pushButton_clear_clicked();

private:
    Ui::Dialog_stresscalc *ui;
    void read_threshold_values();
    double *t_power, *t_pace;

    int sport_index,est_stress;
    double est_power,raw_effort,cv_effort;

    QStringList combo_sport;
    QString sport;
    QString lab_power,lab_workout;
    void set_sport_threshold();
    void estimate_stress();
    void reset_calc();

};

#endif // DIALOG_STRESSCALC_H
