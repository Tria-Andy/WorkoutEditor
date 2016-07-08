#ifndef WORKOUT_H
#define WORKOUT_H

#include <QObject>
#include <QDebug>
#include <QStandardItemModel>
#include <math.h>
#include <QDateTime>

class workout
{

private:
     QString workout_date;
     QString workout_time;
     QString workout_calweek;
     QString workout_phase;
     QString workout_sport;
     QString workout_code;
     QString workout_title;
     QString workout_duration;

     float workout_distance;
     int workout_stress_score;

public:
    workout();

    //Setter
    void set_workout_date(QString w_date) {workout_date = w_date;}
    void set_workout_time(QString w_time) {workout_time = w_time;}
    void set_workout_calweek(QString w_calweek) {workout_calweek = w_calweek;}
    void set_workout_sport(QString sport) {workout_sport = sport;}
    void set_workout_code(QString wcode) {workout_code = wcode;}
    void set_workout_title(QString wtitle) {workout_title = wtitle;}
    void set_workout_duration(QString wduration) {workout_duration = wduration;}
    void set_workout_distance(double wdistance) {workout_distance = wdistance;}
    void set_workout_stress(int wstress) {workout_stress_score = wstress;}

    //Getter
    QString get_workout_date() {return workout_date;}
    QString get_workout_time() {return workout_time;}
    QString get_workout_calweek() {return workout_calweek;}
    QString get_workout_sport() {return workout_sport;}
    QString get_workout_code() {return workout_code;}
    QString get_workout_title() {return workout_title;}
    QString get_workout_duration() {return workout_duration;}
    float get_workout_distance() {return workout_distance;}
    int get_workout_stress() {return workout_stress_score;}

    //edit Workouts
    void add_workout(QStandardItemModel*);
    void edit_workout(QModelIndex,QStandardItemModel*);
    void delete_workout(QModelIndex,QStandardItemModel*);

};

#endif // WORKOUT_H
