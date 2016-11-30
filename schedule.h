#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QStandardItemModel>
#include <QtXml>
#include <QtCharts>
#include <QMessageBox>
#include "settings.h"
#include "logger.h"

QT_CHARTS_USE_NAMESPACE

class schedule
{

public:
    schedule();
    QStandardItemModel *workout_schedule,*week_meta,*week_content;
    void check_workoutFiles();
    void load_workouts_file();
    void save_workout_file();
    void read_workout_values(QDomDocument);
    void read_week_values(QDomDocument,QDomDocument);
    void save_week_files();
    void changeYear();
    QString get_weekPhase(QDate);
    void copyWeek();

    void set_copyWeeks(QString p_from,QString p_to)
    {
        copyFrom = p_from;
        copyTo = p_to;
    }

//Workout
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

    //edit Workouts
    void add_workout();
    void edit_workout(QModelIndex);
    void delete_workout(QModelIndex);

//Charts Objects
    QValueAxis* get_qValueAxis(QString,bool,double,int);
    QLineSeries* get_qLineSeries(bool);
    QBarSeries* get_qBarSeries();
    QBarSet* get_qBarSet();

private:
    logger *logFile;
    QStringList workoutTags,metaTags,contentTags;
    QString schedulePath,copyFrom, copyTo;
    QDate firstdayofweek;

    //Workout Var
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

};

#endif // SCHEDULE_H
