#ifndef DIALOG_WORKOUTS_H
#define DIALOG_WORKOUTS_H

#include "settings.h"
#include <QDialog>
#include <QStandardItemModel>
#include <QAbstractItemModel>
#include <QtXml>
#include <QFile>
#include "standardworkouts.h"

namespace Ui {
class Dialog_workouts;
}

class Dialog_workouts : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_workouts(QWidget *parent = 0, QString w_sport = QString(), standardWorkouts *p_workouts = 0);
    ~Dialog_workouts();

    //Getter
    //QString get_workout_sport() {return w_sport;}
    QString get_workout_code() {return w_code;}
    QString get_workout_title() {return w_title;}
    QTime get_workout_duration() {return QTime::fromString(w_duration,"hh:mm:ss");}
    double get_workout_distance() {return w_distance;}
    int get_workout_stress() {return w_stress;}

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_close_clicked();
    void on_listView_workouts_clicked(const QModelIndex &index);

private:
    Ui::Dialog_workouts *ui;
    standardWorkouts *stdWorkouts;
    QStandardItemModel *workout_model;
    QList<QStandardItem*> selected_workout;
    QString w_id,w_sport,w_code,w_title,w_duration;
    double w_distance;
    int w_stress;

    void create_workout_list();
};

#endif // DIALOG_WORKOUTS_H
