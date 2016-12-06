/*
 * Copyright (c) 2016 Andreas Hunner (andy-atech@gmx.net)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef ACTIVITY_H
#define ACTIVITY_H

#include <QFile>
#include <QFileDialog>
#include <QtXml>
#include <QStandardItemModel>
#include "settings.h"
#include "jsonhandler.h"

class jsonHandler;

class Activity
{
private:
    jsonHandler *jsonhandler;
    QString v_date,curr_sport;
    QStringList ride_items;
    QVector<double> calc_speed,calc_cadence,p_swim_time,new_dist;
    QVector<int> p_swim_timezone,p_hf_timezone,hf_zone_avg,p_swimlaps,vect_lapstart;
    double swim_track,avg_dist,avg_watt,avg_cad,avg_laptime,swim_cv,swim_sri,polishFactor;
    int dist_factor,avg_counter,avg_pace,pace_cv,zone_count,move_time,swim_pace,hf_threshold,hf_avg;
    bool changeRowCount;


    void set_time_in_zones(bool);
    void adjust_intervalls();
    void set_edit_samp_model(int);

    double get_int_distance(int,bool);
    double get_int_value(int,int,bool);
    double interpolate_speed(int,int,double);

    int get_swim_laps(int,bool);
    int check_is_intervall(int);
    int get_zone_values(double,int,bool);

    bool check_speed(int);

public:
    explicit Activity();
    void set_jsonhandler(jsonHandler *p) {jsonhandler = p;}
    void prepareData();
    void set_additional_ride_info();
    void set_curr_act_model(bool);
    void act_reset();
    QStandardItemModel *int_model,*samp_model,*curr_act_model,*edit_int_model,*xdata_model,*swim_xdata,*edit_samp_model;
    QStandardItemModel *swim_pace_model, *swim_hf_model;
    QMap<QString,QString> ride_info;

    //Recalculation
    QStandardItemModel * set_int_model_pointer(bool);
    QStandardItemModel * set_samp_model_pointer(bool);
    void recalculate_intervalls(bool);
    int get_int_duration(int,bool);
    int get_int_pace(int,bool);
    double get_int_speed(int,bool);
    double polish_SpeedValues(double,double,double,bool);

    //Value Getter and Setter
    void set_changeRowCount(bool setCount) {changeRowCount = setCount;}
    void set_polishFactor(double vFactor) {polishFactor = vFactor;}
    int get_header_num();
    void set_date(QString a_date) {v_date = a_date;}
    QString get_date() {return v_date;}
    void set_sport(QString a_sport) {curr_sport = a_sport;}
    QString get_sport() {return curr_sport;}


    //Averages
    void reset_avg();
    void set_dist_factor();
    void set_avg_values(int,int,bool);
    int get_dist_factor() {return dist_factor;}
    int get_avg_laptime() {return round(avg_laptime/avg_counter);}
    int get_avg_pace() {return avg_pace/avg_counter;}
    double get_avg_dist() {return avg_dist/avg_counter;}
    double get_avg_watts() {return avg_watt/avg_counter;}
    double get_avg_cad() {return avg_cad/avg_counter;}

    //Swim Calculations
    void set_swim_data();
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
    void set_swim_track(double trackLen) {swim_track = trackLen;}
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

};

#endif // ACTIVITY_H
