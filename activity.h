#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QFile>
#include <QFileDialog>
#include <QtXml>
#include <QStandardItemModel>
#include "settings.h"

class Activity
{
private:
    settings *act_settings;
    QString v_date,v_sport;
    QStringList ride_items;
    double swim_track,avg_dist,avg_watt;
    int dist_factor,avg_counter,avg_laptime,avg_pace;

    int pace_cv,zone_count;
    int move_time;
    int swim_pace;
    int hf_threshold,hf_avg;
    double swim_cv,swim_sri;
    QVector<double> calc_speed,p_swim_time,new_dist;
    QVector<int> p_swim_timezone,p_hf_timezone,hf_zone_avg,p_swimlaps;
    bool changeRowCount;

    double get_int_distance(int,bool);
    double get_int_watts(int);
    int get_swim_laps(int,bool);
    bool check_speed(int);
    double interpolate_speed(int,int,double);
    int check_is_intervall(int);

    void read_swim_data();
    int get_zone_values(double,int,bool);
    void set_time_in_zones();

    void adjust_intervalls();
    QStandardItemModel * set_int_model_pointer(bool);
    QStandardItemModel * set_samp_model_pointer(bool);
    void set_edit_samp_model();

public:
    explicit Activity(settings *p_settings = 0);
    static const QString isSwim;
    static const QString isBike;
    static const QString isRun;

    void read_jsonFile(QString fileContent);
    void set_additional_ride_info();
    void set_curr_act_model(bool);
    void act_reset();
    QStandardItemModel *ride_model,*int_model,*samp_model,*curr_act_model,*edit_int_model,*edit_samp_model,*edit_dist_model;
    QStandardItemModel *swim_pace_model, *swim_hf_model;

    //Recalculation
    void recalculate_intervalls(bool);
    int get_int_duration(int,bool);
    int get_int_pace(int,bool);
    double get_int_speed(int,bool);
    double polish_SpeedValues(double,double,double,bool);

    //Value Getter and Setter
    void set_date(QString a_date) {v_date = a_date;}
    QString get_date() {return v_date;}
    void set_sport(QString a_sport) {v_sport = a_sport;}
    QString get_sport() {return v_sport;}
    int get_header_num();
    void set_changeRowCount(bool setCount) {changeRowCount = setCount;}

    //Averages
    void set_dist_factor();
    int get_dist_factor() {return dist_factor;}
    void set_avg_values(int,int,bool);
    void reset_avg();
    int get_avg_laptime() {return avg_laptime/avg_counter;}
    int get_avg_pace() {return avg_pace/avg_counter;}
    double get_avg_dist() {return avg_dist/avg_counter;}
    double get_avg_watts() {return avg_watt/avg_counter;}

    //Swim Calculations
    int get_swim_cv_pace(double);
    QString get_swim_pace_time(int);

    void set_swim_pace();
    int get_swim_pace() {return swim_pace;}
    void set_move_time();
    int get_move_time() {return move_time;}
    void set_swim_sri();
    double get_swim_sri() { return swim_sri;}
    void set_hf_zone_avg(double,double,int);
    double get_hf_zone_avg();
    void set_hf_avg();
    int get_hf_avg() {return hf_avg;}
    int get_hf_max() {return hf_threshold;}
    double get_swim_cv() {return swim_cv;}
    void set_hf_time_in_zone();
    double get_swim_track() {return swim_track;}
    QVector<double> * get_new_dist()
    {
        QVector<double> *dist = &new_dist;
        return dist;
    }
    QVector<double> * get_new_speed()
    {
        QVector<double> *speed = &calc_speed;
        return speed;
    }

signals:

public slots:
};

#endif // ACTIVITY_H
