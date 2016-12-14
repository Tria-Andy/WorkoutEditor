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

#include "activity.h"
#include <math.h>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonValue>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>

Activity::Activity()
{
    zone_count = settings::get_levelList().count();
    changeRowCount = false;
}

void Activity::act_reset()
{
    changeRowCount = false;
}

void Activity::prepareData()
{
    int sampCount = samp_model->rowCount();
    int intCount = int_model->rowCount();
    QModelIndex editIndex,intIndex;
    QString swimLapName;

    settings::set_swimLaplen(swim_track);
    int_model->setData(int_model->index(intCount-1,2,QModelIndex()),sampCount-1);
    edit_int_model = new QStandardItemModel(intCount,5);

    for(int row = 0; row < intCount; ++row)
    {
        for(int col = 0; col < int_model->columnCount(); ++col)
        {
            intIndex = int_model->index(row,col,QModelIndex());
            editIndex = edit_int_model->index(row,col,QModelIndex());
            edit_int_model->setData(editIndex,int_model->data(intIndex,Qt::DisplayRole));
        }
        edit_int_model->setData(edit_int_model->index(row,3,QModelIndex()),settings::set_doubleValue(this->get_int_distance(row,false),true));
    }

    edit_int_model->setData(edit_int_model->index(edit_int_model->rowCount()-1,2,QModelIndex()),sampCount-1);
    edit_int_model->setHorizontalHeaderLabels(settings::get_time_header());

    if(curr_sport == settings::isSwim)
    {
        edit_int_model->setData(edit_int_model->index(0,1,QModelIndex()),0);
        int rowCounter = xdata_model->rowCount();
        swim_xdata = new QStandardItemModel(rowCounter,7);
        swim_xdata->setHorizontalHeaderLabels(settings::get_swimtime_header());

        int lapNr = 0;
        int intCount = 1;
        int type = 0, typePrev = 0;
        double lapStart,lapStartPrev = 0,lapPacePrev = 0;
        double lapSpeed,lapPace,lapDist = 0;

        for(int row = 0; row < rowCounter; ++row)
        {
            lapPace = round(xdata_model->data(xdata_model->index(row,3,QModelIndex())).toDouble() - 0.1);
            type = xdata_model->data(xdata_model->index(row,2,QModelIndex())).toInt();
            lapDist = xdata_model->data(xdata_model->index(row,1,QModelIndex())).toDouble();

            if(lapPace > 0 && type != 0)
            {
                swimLapName = QString::number(intCount)+"_"+QString::number(++lapNr*swim_track);
                swim_xdata->setData(swim_xdata->index(row,0,QModelIndex()),swimLapName);

                if(typePrev == 0)
                {
                    lapStart = xdata_model->data(xdata_model->index(row,0,QModelIndex())).toDouble();
                    int breakTime = lapStart - lapStartPrev;
                    swim_xdata->setData(swim_xdata->index(row-1,2,QModelIndex()),breakTime);
                }
                else
                {
                    lapStart = lapStartPrev + lapPacePrev;
                }
                if(row == 1) lapStart = lapPacePrev-1;
                lapSpeed = settings::get_speed(QTime::fromString(settings::set_time(lapPace),"mm:ss"),swim_track,settings::isSwim,false).toDouble();
            }
            else
            {
                lapNr = 0;
                swim_xdata->setData(swim_xdata->index(row,0,QModelIndex()),settings::get_breakName());
                lapStart = lapStartPrev + lapPacePrev;          
                lapSpeed = 0;
                ++intCount;
            }

            swim_xdata->setData(swim_xdata->index(row,1,QModelIndex()),lapStart);
            swim_xdata->setData(swim_xdata->index(row,2,QModelIndex()),lapPace);
            swim_xdata->setData(swim_xdata->index(row,3,QModelIndex()),xdata_model->data(xdata_model->index(row,4,QModelIndex())));
            swim_xdata->setData(swim_xdata->index(row,4,QModelIndex()),lapSpeed);
            swim_xdata->setData(swim_xdata->index(row,5,QModelIndex()),lapDist);
            swim_xdata->setData(swim_xdata->index(row,6,QModelIndex()),type);
            lapPacePrev = lapPace;
            lapStartPrev = lapStart;
            typePrev = type;
        }
        xdata_model->clear();
        delete xdata_model;
    }

    ride_info.insert("Distance:",QString::number(samp_model->data(samp_model->index(sampCount-1,1,QModelIndex())).toDouble()));
    ride_info.insert("Duration:",QDateTime::fromTime_t(sampCount).toUTC().toString("hh:mm:ss"));

    curr_act_model = new QStandardItemModel(intCount,this->get_header_num());
    this->set_curr_act_model(false);
    this->set_dist_factor();
    this->reset_avg();

    if(curr_sport == settings::isSwim)
    {
        p_swim_timezone.resize(zone_count+1);
        p_swim_time.resize(zone_count+1);
        p_hf_timezone.resize(zone_count);
        hf_zone_avg.resize(zone_count);
        hf_avg = 0;
        move_time = 0.0;

        QStringList pace_header,hf_header;
        pace_header << "Zone" << "Low (min/100m)" << "High (min/100m)" << "Time in Zone";
        hf_header << "Zone" << "Low (1/Min)" << "High (1/min)" << "Time in Zone";


        //Set Tableinfos
        swim_pace_model = new QStandardItemModel(zone_count,4);
        swim_pace_model->setHorizontalHeaderLabels(pace_header);

        swim_hf_model = new QStandardItemModel(zone_count,4);
        swim_hf_model->setHorizontalHeaderLabels(hf_header);

        //Read current CV and HF Threshold
        double temp_cv = settings::get_thresValue("swimpace");
        swim_cv = (3600.0 / temp_cv) / 10.0;
        pace_cv = temp_cv;

        this->set_swim_data();
    }
}

void Activity::set_additional_ride_info()
{
    if(settings::get_act_isrecalc())
    {
        ride_info.insert("Distance:",QString::number(edit_samp_model->data(edit_samp_model->index(edit_samp_model->rowCount()-1,1,QModelIndex())).toDouble()));
        ride_info.insert("Duration:",QDateTime::fromTime_t(edit_samp_model->rowCount()).toUTC().toString("hh:mm:ss"));
    }
    else
    {
        ride_info.insert("Distance:",QString::number(samp_model->data(samp_model->index(samp_model->rowCount()-1,1,QModelIndex())).toDouble()));
        ride_info.insert("Duration:",QDateTime::fromTime_t(samp_model->rowCount()).toUTC().toString("hh:mm:ss"));
    }
}

void Activity::set_swim_data()
{
    bool recalc = settings::get_act_isrecalc();
    QStringList levels = settings::get_levelList();
    QString temp,zone_low,zone_high;
    hf_threshold = settings::get_thresValue("hfthres");
    hf_max = settings::get_thresValue("hfmax");

    //Set Swim zone low and high
        for(int i = 0; i < levels.count(); i++)
        {
            temp = settings::get_rangeValue(curr_sport,levels.at(i));
            zone_low = temp.split("-").first();
            zone_high = temp.split("-").last();

            swim_pace_model->setData(swim_pace_model->index(i,0,QModelIndex()),levels.at(i));
            swim_pace_model->setData(swim_pace_model->index(i,1,QModelIndex()),settings::set_time(this->get_zone_values(zone_low.toDouble(),pace_cv,true)));

            p_swim_time[i] = swim_cv*(zone_low.toDouble()/100);

            if(i < zone_count-1)
            {
                swim_pace_model->setData(swim_pace_model->index(i,2,QModelIndex()),settings::set_time(this->get_zone_values(zone_high.toDouble(),pace_cv,true)));
            }
            else
            {
                swim_pace_model->setData(swim_pace_model->index(i,2,QModelIndex()),"MAX");
            }
            p_swim_timezone[i] = 0;
        }

        this->set_time_in_zones(recalc);

        for (int x = 1; x <= zone_count;x++)
        {
            swim_pace_model->setData(swim_pace_model->index(x-1,3,QModelIndex()),settings::set_time(p_swim_timezone[x]));
        }

    //Set HF zone low and high
        for(int i = 0; i < levels.count(); i++)
        {
            temp = settings::get_rangeValue("HF",levels.at(i));
            zone_low = temp.split("-").first();
            zone_high = temp.split("-").last();

            swim_hf_model->setData(swim_hf_model->index(i,0,QModelIndex()),levels.at(i));
            swim_hf_model->setData(swim_hf_model->index(i,1,QModelIndex()),this->get_zone_values(zone_low.toDouble(),hf_threshold,false));

            if(i < zone_count-1)
            {
                swim_hf_model->setData(swim_hf_model->index(i,2,QModelIndex()),this->get_zone_values(zone_high.toDouble(),hf_threshold,false));

                this->set_hf_zone_avg(this->get_zone_values(zone_low.toDouble(),hf_threshold,false),this->get_zone_values(zone_high.toDouble(),hf_threshold,false),i);
            }
            else
            {
                swim_hf_model->setData(swim_hf_model->index(i,2,QModelIndex()),hf_max);
                this->set_hf_zone_avg(this->get_zone_values(zone_low.toDouble(),hf_threshold,false),hf_max,i);
            }
        }
        this->set_hf_time_in_zone();
}

void Activity::set_time_in_zones(bool recalc)
{
    QStandardItemModel *model = this->set_samp_model_pointer(recalc);

    int z0=0,z1=0,z2=0,z3=0,z4=0,z5=0,z6=0,z7=0;
    double paceSec;

    for(int i = 0; i < model->rowCount(); i++)
    {
        paceSec = model->data(model->index(i,2,QModelIndex())).toDouble();

        if(paceSec <= p_swim_time[0])
        {
            ++z0;
            p_swim_timezone[0] = z0;
        }

        if(paceSec > p_swim_time[0] && paceSec <= p_swim_time[1])
        {
            ++z1;
            p_swim_timezone[1] = z1;
        }

        if(paceSec > p_swim_time[1] && paceSec <= p_swim_time[2])
        {
            ++z2;
            p_swim_timezone[2] = z2;
        }

        if(paceSec > p_swim_time[2] && paceSec <= p_swim_time[3])
        {
            ++z3;
            p_swim_timezone[3] = z3;
        }

        if(paceSec > p_swim_time[3] && paceSec <= p_swim_time[4])
        {
            ++z4;
            p_swim_timezone[4] = z4;
        }

        if(paceSec > p_swim_time[4] && paceSec <= p_swim_time[5])
        {
            ++z5;
            p_swim_timezone[5] = z5;
        }

        if(paceSec > p_swim_time[5] && paceSec <= p_swim_time[6])
        {
            ++z6;
            p_swim_timezone[6] = z6;
        }
        if(paceSec > p_swim_time[6])
        {
            ++z7;
            p_swim_timezone[7] = z7;
        }
    }
    this->set_move_time();
}

void Activity::set_hf_time_in_zone()
{
     double hf_factor[] = {0.125,0.25,0.5,0.75};

     //REKOM: Z0*0,125 + Z1 + Z2*0,5
     p_hf_timezone[0] = ceil((p_swim_timezone[0]*hf_factor[0]) + p_swim_timezone[1] + (p_swim_timezone[2]*hf_factor[2]));

     //END: Z0*0,5 + Z2*0,5 + Z3*0,75
     p_hf_timezone[1] = ceil((p_swim_timezone[0]*hf_factor[2]) + (p_swim_timezone[2]*hf_factor[2]) + (p_swim_timezone[3]*hf_factor[3]));

     //TEMP: Z0*0,25 + Z3*0,25 + Z4*0,75 + Z5*0,5
     p_hf_timezone[2] = ceil((p_swim_timezone[0]*hf_factor[1]) + (p_swim_timezone[3]*hf_factor[1]) + p_swim_timezone[4] + (p_swim_timezone[5]*hf_factor[2]));

     //LT: Z5*0,25 + Z6*0,75 + Z7*0,5
     p_hf_timezone[3] = ceil((p_swim_timezone[5]*hf_factor[1]) + (p_swim_timezone[6]*hf_factor[3]) + (p_swim_timezone[7]*hf_factor[2]));

     //VO2: Z5*0,25 + Z6*0,125 + Z7*0,25
     p_hf_timezone[4] = ceil((p_swim_timezone[5]*hf_factor[1]) + (p_swim_timezone[6]*hf_factor[0]) + (p_swim_timezone[7]*hf_factor[1]));

     //AC: Z6*0,125 + Z7*0,125
     p_hf_timezone[5] = ceil((p_swim_timezone[6]*hf_factor[0]) + (p_swim_timezone[7]*hf_factor[0]));

     //NEURO: Z7*0,125
     p_hf_timezone[6] = ceil(p_swim_timezone[7]*hf_factor[0]);

     for(int i = 0; i < zone_count; i++)
     {
         swim_hf_model->setData(swim_hf_model->index(i,3,QModelIndex()),settings::set_time(p_hf_timezone[i]));
     }
    hf_avg = 0;
    this->set_hf_avg();
}

void Activity::set_hf_avg()
{
    double hf_part;
    for(int i = 0; i < 7; i++)
    {
        hf_part = static_cast<double>(hf_zone_avg[i]) * (static_cast<double>(p_hf_timezone[i])/static_cast<double>(samp_model->rowCount()));
        hf_avg = hf_avg + ceil(hf_part);
    }
}

void Activity::set_move_time()
{
    move_time = 0;
    for(int i = 1; i <= 7;i++)
    {
        move_time = move_time + p_swim_timezone[i];
    }
    this->set_swim_pace();
}

void Activity::set_hf_zone_avg(double low,double high, int pos)
{
    hf_zone_avg[pos] = ceil((low + high) / 2);
}

void Activity::set_swim_pace()
{
    swim_pace = ceil(static_cast<double>(move_time) / (samp_model->data(samp_model->index(samp_model->rowCount()-1,1,QModelIndex())).toDouble()*10));
    this->set_swim_sri();
}

void Activity::set_swim_sri()
{
    double goal = sqrt(pow(static_cast<double>(swim_pace),3.0))/10;
    swim_sri = static_cast<double>(pace_cv) / goal;
}

int Activity::get_zone_values(double factor, int max, bool ispace)
{
    int zone_value;
    if(ispace)
    {
        zone_value = ceil(max/(factor/100));
    }
    else
    {
        zone_value = ceil(max*(factor/100));
    }

    return zone_value;
}

int Activity::get_header_num()
{
    if(curr_sport == settings::isSwim)
    {
        return 6;
    }
    if(curr_sport == settings::isBike)
    {
        return 7;
    }
    if(curr_sport == settings::isRun)
    {
        return 5;
    }
    if(curr_sport == settings::isTria)
    {
        return 5;
    }

    return 0;
}

void Activity::set_dist_factor()
{
    if (curr_sport == settings::isSwim)
    {
        dist_factor = 1000;
    }
    else
    {
        dist_factor = 1;
    }
}

int Activity::get_int_duration(int row,bool recalc)
{
    QStandardItemModel *p_int_model = this->set_int_model_pointer(recalc);
    int duration;

    duration = p_int_model->data(p_int_model->index(row,2,QModelIndex()),Qt::DisplayRole).toInt() - p_int_model->data(p_int_model->index(row,1,QModelIndex()),Qt::DisplayRole).toInt();

    return duration;
}

double Activity::get_int_distance(int row,bool recalc)
{
    QStandardItemModel *p_int_model,*p_samp_model;
    p_int_model = this->set_int_model_pointer(recalc);
    p_samp_model = this->set_samp_model_pointer(recalc);
    double dist,dist_start,dist_stop,tempdist;
    int int_start,int_stop;

    if(row == 0)
    {
        int_stop = p_int_model->data(p_int_model->index(row,2,QModelIndex()),Qt::DisplayRole).toInt();
        dist = p_samp_model->data(p_samp_model->index(int_stop,1,QModelIndex()),Qt::DisplayRole).toDouble();
    }
    else
    {
        int_start = p_int_model->data(p_int_model->index(row,1,QModelIndex()),Qt::DisplayRole).toInt();
        int_stop = p_int_model->data(p_int_model->index(row,2,QModelIndex()),Qt::DisplayRole).toInt();
        dist_start = p_samp_model->data(p_samp_model->index(int_start,1,QModelIndex()),Qt::DisplayRole).toDouble();
        dist_stop = p_samp_model->data(p_samp_model->index(int_stop,1,QModelIndex()),Qt::DisplayRole).toDouble();
        dist = dist_stop - dist_start;
    }
    if(curr_sport == settings::isSwim)
    {
        tempdist = ceil(round(dist*100));
        return tempdist/100.0;
    }

    return dist;
}

int Activity::get_int_pace(int row,bool recalc)
{
    int pace;
    if(this->get_int_distance(row,recalc) == 0)
    {
        pace = 0;
    }
    else
    {
        if (curr_sport == settings::isSwim)
        {
            pace = this->get_int_duration(row,recalc) / (this->get_int_distance(row,recalc)*10);
        }
        else
        {
            pace = this->get_int_duration(row,recalc) / this->get_int_distance(row,recalc);
        }
    }

    return pace;
}

int Activity::get_swim_cv_pace(double swim_cv)
{
    const int hour = 60;
    double min_km = hour/swim_cv;
    double min_100 = min_km/10;

    int pace = ceil(hour*min_100);

    return pace;
}

QString Activity::get_swim_pace_time(int pace)
{
    return settings::set_time(pace);
}

int Activity::get_swim_laps(int row,bool recalc)
{
    int lapcount;

    if(recalc)
    {
        lapcount = p_swimlaps[row];
    }
    else
    {
        lapcount = round((this->get_int_distance(row,false)*1000)/swim_track);
    }

    if(lapcount == 0)
    {
        edit_int_model->setData(edit_int_model->index(row,4,QModelIndex()),0);
    }
    else
    {
        edit_int_model->setData(edit_int_model->index(row,4,QModelIndex()),1);
    }

    return lapcount;
}

double Activity::get_int_value(int row,int col, bool recalc)
{
    QStandardItemModel *model = this->set_samp_model_pointer(recalc);
    double value = 0.0;
    int int_start,int_stop;
    int_start = int_model->data(int_model->index(row,1,QModelIndex())).toInt();
    int_stop = int_model->data(int_model->index(row,2,QModelIndex())).toInt();

    for(int i = int_start; i < int_stop; ++i)
    {
        value = value + model->data(model->index(i,col,QModelIndex())).toDouble();
    }
    value = value / (int_stop - int_start);

    return value;
}

QStandardItemModel * Activity::set_int_model_pointer(bool recalc)
{
    if(recalc)
    {
        return edit_int_model;
    }
    else
    {
        return int_model;
    }
}

QStandardItemModel * Activity::set_samp_model_pointer(bool recalc)
{
    if(recalc)
    {
        return edit_samp_model;
    }
    else
    {
        return samp_model;
    }
}

void Activity::recalculate_intervalls(bool recalc)
{
    QString lapname;
    int lapcounter = 1;
    int swimStart = 0;
    int swimStop = 0;
    int lapoffset = 0;
    int lastSec = samp_model->data(samp_model->index(samp_model->rowCount()-1,0,QModelIndex())).toInt();

    if(changeRowCount)
    {
        for(int i = 0; i < edit_int_model->rowCount(); ++i)
        {
            edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),i);
        }
    }

    if(curr_sport == settings::isSwim && recalc)
    {
        int rowCount = edit_int_model->rowCount();
        p_swimlaps.resize(rowCount);
        this->adjust_intervalls();
        for(int i = 0; i < rowCount;++i)
        {
            edit_int_model->setData(edit_int_model->index(i,4,QModelIndex()),this->check_is_intervall(i));
            p_swimlaps[i] = round((edit_int_model->data(edit_int_model->index(i,3,QModelIndex())).toDouble()*1000)) / swim_track;

            if(i == 0)
            {
                lapname = QString::number(lapcounter)+"_Warmup_"+QString::number(swim_track*p_swimlaps[i]);
                edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),lapname);
                ++lapcounter;
            }
            if(i != 0 && i < rowCount-1)
            {
                if(this->check_is_intervall(i) == 1)
                {
                    lapname = QString::number(lapcounter)+"_Int_"+QString::number(swim_track*p_swimlaps[i]);
                    edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),lapname);
                    ++lapcounter;
                }
                else
                {
                    lapname = settings::get_breakName();
                    edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),lapname);
                }
            }
            if(i == rowCount-1)
            {
                lapname = QString::number(lapcounter)+"_Cooldown_"+QString::number(swim_track*p_swimlaps[i]);
                edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),lapname);
            }
            if(i == 0)
            {
                swimStop = swim_xdata->data(swim_xdata->index(p_swimlaps[i],1,QModelIndex())).toInt();
                lapoffset = lapoffset + p_swimlaps[i];
                edit_int_model->setData(edit_int_model->index(i,1,QModelIndex()),swimStart);
                edit_int_model->setData(edit_int_model->index(i,2,QModelIndex()),swimStop);
            }
            else
            {
                if(p_swimlaps[i] > 0)
                {
                    lapoffset = lapoffset + p_swimlaps[i];
                }
                else
                {
                    lapoffset = lapoffset + 1;
                }
                swimStart = swimStop;
                swimStop = swim_xdata->data(swim_xdata->index(lapoffset,1,QModelIndex())).toInt();
                edit_int_model->setData(edit_int_model->index(i,1,QModelIndex()),swimStart);
                edit_int_model->setData(edit_int_model->index(i,2,QModelIndex()),swimStop);
            }
        }
        lastSec = swim_xdata->data(swim_xdata->index(swim_xdata->rowCount()-1,1,QModelIndex())).toInt()
                + swim_xdata->data(swim_xdata->index(swim_xdata->rowCount()-1,2,QModelIndex())).toInt();
        edit_int_model->setData(edit_int_model->index(edit_int_model->rowCount()-1,2,QModelIndex()),lastSec);
    }
    if(!recalc)
    {
        for(int i = 0; i < int_model->rowCount(); ++i)
        {
            edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),int_model->data(int_model->index(i,0,QModelIndex())).toString());
            edit_int_model->setData(edit_int_model->index(i,1,QModelIndex()),int_model->data(int_model->index(i,1,QModelIndex())).toInt());
            edit_int_model->setData(edit_int_model->index(i,2,QModelIndex()),int_model->data(int_model->index(i,2,QModelIndex())).toInt());
        }
    }
    this->set_edit_samp_model(++lastSec);
    this->set_curr_act_model(recalc);
}

double Activity::get_int_speed(int row,bool recalc)
{
    double speed;

    double pace = this->get_int_duration(row,recalc) / (edit_int_model->data(edit_int_model->index(row,3,QModelIndex())).toDouble() * 10.0);

    speed = 360.0 / pace;

    return speed;
}

double Activity::polish_SpeedValues(double currSpeed,double avgSpeed,double factor,bool setrand)
{
    double randfact = 0;
    if(curr_sport == settings::isRun)
    {
        randfact = ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) / (currSpeed/((factor*100)+1.0)));
    }
    if(curr_sport == settings::isBike)
    {
        randfact = ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) / (currSpeed/((factor*1000)+1.0)));
    }

    double avgLow = avgSpeed-(avgSpeed*factor);
    double avgHigh = avgSpeed+(avgSpeed*factor);

    if(setrand)
    {
        if(currSpeed < avgLow)
        {
            return avgLow+randfact;
        }
        if(currSpeed > avgHigh)
        {
            return avgHigh-randfact;
        }
        if(currSpeed > avgLow && currSpeed < avgHigh)
        {
            return currSpeed;
        }
        return currSpeed + randfact;
    }
    else
    {
        if(currSpeed < avgLow)
        {
            return avgLow;
        }
        if(currSpeed > avgHigh)
        {
            return avgHigh;
        }
        if(currSpeed > avgLow && currSpeed < avgHigh)
        {
            return currSpeed;
        }
    }
    return 0;
}

double Activity::interpolate_speed(int row,int sec,double limit)
{
    double curr_speed = samp_model->data(samp_model->index(sec,2,QModelIndex())).toDouble();
    double avg_speed = this->get_int_speed(row,settings::get_act_isrecalc());

    if(curr_speed == 0)
    {
        curr_speed = limit;
    }

    if(row == 0 && sec < 5)
    {
        return (static_cast<double>(sec) + ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)))) * 1.2;
    }
    else
    {
        if(avg_speed >= limit)
        {
            return this->polish_SpeedValues(curr_speed,avg_speed,polishFactor,true);
        }
        else
        {
            return curr_speed;
        }
    }
    return 0;
}

bool Activity::check_speed(int sec)
{
    double speed = samp_model->data(samp_model->index(sec,2,QModelIndex())).toDouble();

    if(speed == 0.0)
    {
        return false;
    }
    else
    {
        return true;
    }
}

int Activity::check_is_intervall(int row)
{
    double isInt;
    isInt = edit_int_model->data(edit_int_model->index(row,3,QModelIndex())).toDouble();
    if(isInt == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void Activity::set_edit_samp_model(int rowcount)
{
    int sampRowCount = rowcount;
    int sampColCount = samp_model->columnCount();
    edit_samp_model = new QStandardItemModel(sampRowCount,sampColCount);
    new_dist.resize(sampRowCount);
    calc_speed.resize(sampRowCount);
    calc_cadence.resize(sampRowCount);
    double msec = 0.0;
    int int_start,int_stop,sportpace,swimLaps;
    double overall = 0.0,lowLimit = 0.0,limitFactor = 0.0;
    double swimPace,swimSpeed,swimCycle;
    bool isBreak = true;
    if(curr_sport != settings::isSwim && curr_sport != settings::isTria)
    {
        if(curr_sport == settings::isBike)
        {
            sportpace = settings::get_thresValue("bikepace");
            limitFactor = 0.35;
        }
        if(curr_sport == settings::isRun)
        {
            sportpace = settings::get_thresValue("runpace");
            limitFactor = 0.20;
        }
        lowLimit = settings::get_speed(QTime::fromString(settings::set_time(sportpace),"mm:ss"),0,curr_sport,true).toDouble();
        lowLimit = lowLimit - (lowLimit*limitFactor);
    }
    if(curr_sport == settings::isTria)
    {
        lowLimit = 1.0;
    }

    if(curr_sport == settings::isSwim)
    {
        swimLaps = 0;
        for(int sLap = 0; sLap < swim_xdata->rowCount();++sLap)
        {
            int_start = swim_xdata->data(swim_xdata->index(sLap,1,QModelIndex()),Qt::DisplayRole).toInt();
            swimPace = swim_xdata->data(swim_xdata->index(sLap,2,QModelIndex()),Qt::DisplayRole).toDouble();
            swimSpeed = swim_xdata->data(swim_xdata->index(sLap,4,QModelIndex()),Qt::DisplayRole).toDouble();
            swimCycle = swim_xdata->data(swim_xdata->index(sLap,3,QModelIndex()),Qt::DisplayRole).toDouble();

            if(sLap == swim_xdata->rowCount()-1)
            {
                int_stop = sampRowCount-1;
            }
            else
            {
                int_stop = swim_xdata->data(swim_xdata->index(sLap+1,1,QModelIndex()),Qt::DisplayRole).toInt();
            }

            if(swimSpeed > 0)
            {
                ++swimLaps;
                msec = (swim_track / swimPace) / 1000;
                overall = (swim_track * swimLaps) / 1000;
                isBreak = false;
            }
            else
            {
                msec = 0;
                isBreak = true;
            }

            for(int lapsec = int_start; lapsec <= int_stop; ++lapsec)
            {
                if(lapsec == 0)
                {
                    new_dist[lapsec] = 0.001;
                }
                else
                {
                    if(lapsec == int_start && isBreak)
                    {
                        new_dist[lapsec] = overall;
                    }
                    else
                    {
                        new_dist[lapsec] = new_dist[lapsec-1] + msec;
                    }
                    if(lapsec == int_stop)
                    {
                        new_dist[int_stop-1] = overall;
                        new_dist[int_stop] = overall;
                    }
                }
                calc_speed[lapsec] = swimSpeed;
                calc_cadence[lapsec] = swimCycle;
            }
        }

        for(int row = 0; row < sampRowCount;++row)
        {
            edit_samp_model->setData(edit_samp_model->index(row,0,QModelIndex()),row);
            edit_samp_model->setData(edit_samp_model->index(row,1,QModelIndex()),new_dist[row]);
            edit_samp_model->setData(edit_samp_model->index(row,2,QModelIndex()),calc_speed[row]);
            edit_samp_model->setData(edit_samp_model->index(row,3,QModelIndex()),calc_cadence[row]);
        }
        this->set_swim_data();
    }
    else
    {
      for(int c_int = 0; c_int < edit_int_model->rowCount(); ++c_int)
      {
         int_start = edit_int_model->data(edit_int_model->index(c_int,1,QModelIndex())).toInt();
         int_stop = edit_int_model->data(edit_int_model->index(c_int,2,QModelIndex())).toInt();
         msec = edit_int_model->data(edit_int_model->index(c_int,3,QModelIndex())).toDouble() / this->get_int_duration(c_int,true);

         if(curr_sport == settings::isRun)
         {
             for(int c_dist = int_start;c_dist <= int_stop; ++c_dist)
             {
                if(c_dist == 0)
                {
                    new_dist[0] = 0.0000;
                }
                else
                {
                    calc_speed[c_dist] = this->interpolate_speed(c_int,c_dist,lowLimit);
                    new_dist[c_dist] = new_dist[c_dist-1] + msec;
                }
             }
         }

         if(curr_sport == settings::isBike)
         {
             qDebug() << c_int << int_start << int_stop;
             for(int c_dist = int_start;c_dist <= int_stop; ++c_dist)
             {
                if(c_dist == 0)
                {

                    new_dist[0] = 0.0000;
                }
                else
                {
                    calc_speed[c_dist] = this->interpolate_speed(c_int,c_dist,lowLimit);
                    calc_cadence[c_dist] = samp_model->data(samp_model->index(c_dist,3,QModelIndex())).toDouble();
                    new_dist[c_dist] = new_dist[c_dist-1] + msec;
                }
             }
          }
        }
      }

      if(curr_sport == settings::isBike)
      {
          for(int row = 0; row < sampRowCount;++row)
          {
              edit_samp_model->setData(edit_samp_model->index(row,0,QModelIndex()),row);
              edit_samp_model->setData(edit_samp_model->index(row,1,QModelIndex()),new_dist[row]);
              edit_samp_model->setData(edit_samp_model->index(row,2,QModelIndex()),calc_speed[row]);
              edit_samp_model->setData(edit_samp_model->index(row,3,QModelIndex()),calc_cadence[row]);
              edit_samp_model->setData(edit_samp_model->index(row,4,QModelIndex()),samp_model->data(samp_model->index(row,4,QModelIndex())).toDouble());
              for(int col = 5; col < sampColCount; ++col)
              {
                  edit_samp_model->setData(edit_samp_model->index(row,col,QModelIndex()),samp_model->data(samp_model->index(row,col,QModelIndex())).toDouble());
              }
          }
      }

      if(curr_sport == settings::isRun)
      {
          for(int row = 0; row < sampRowCount;++row)
          {
              edit_samp_model->setData(edit_samp_model->index(row,0,QModelIndex()),row);
              edit_samp_model->setData(edit_samp_model->index(row,1,QModelIndex()),new_dist[row]);
              edit_samp_model->setData(edit_samp_model->index(row,2,QModelIndex()),calc_speed[row]);
              for(int col = 3; col < sampColCount; ++col)
              {
                  edit_samp_model->setData(edit_samp_model->index(row,col,QModelIndex()),samp_model->data(samp_model->index(row,col,QModelIndex())).toDouble());
              }
          }
      }

      if(curr_sport == settings::isTria)
      {
          double triValue = 0,sportValue = 0;

          for(int row = 0; row < sampRowCount;++row)
          {
              edit_samp_model->setData(edit_samp_model->index(row,0,QModelIndex()),row);
              edit_samp_model->setData(edit_samp_model->index(row,1,QModelIndex()),new_dist[row]);
              edit_samp_model->setData(edit_samp_model->index(row,2,QModelIndex()),calc_speed[row]);
              edit_samp_model->setData(edit_samp_model->index(row,3,QModelIndex()),calc_cadence[row]);
              for(int col = 4; col < sampColCount; ++col)
              {
                  edit_samp_model->setData(edit_samp_model->index(row,col,QModelIndex()),samp_model->data(samp_model->index(row,col,QModelIndex())).toDouble());
              }
          }
          jsonhandler->set_overrideFlag(true);
          sportValue = round(settings::estimate_stress(settings::isSwim,settings::set_time(this->get_int_pace(0,true)/10),this->get_int_duration(0,true)));
          jsonhandler->set_overrideData("swimscore",QString::number(sportValue));
          triValue = triValue + sportValue;
          sportValue = round(settings::estimate_stress(settings::isBike,QString::number(this->get_int_value(2,4,true)),this->get_int_duration(2,true)));
          jsonhandler->set_overrideData("skiba_bike_score",QString::number(sportValue));
          triValue = triValue + sportValue;
          sportValue = round(settings::estimate_stress(settings::isRun,settings::set_time(this->get_int_pace(4,true)),this->get_int_duration(4,true)));
          jsonhandler->set_overrideData("govss",QString::number(sportValue));
          triValue = triValue + sportValue;
          jsonhandler->set_overrideData("triscore",QString::number(triValue));
      }
}

void Activity::adjust_intervalls()
{
    int i_start = 0;
    int i_stop = 0;
    int isInt;

    for(int i = 0; i < edit_int_model->rowCount(); ++i)
    {
        i_start = edit_int_model->data(edit_int_model->index(i,1,QModelIndex())).toInt();
        i_stop = edit_int_model->data(edit_int_model->index(i-1,2,QModelIndex())).toInt();
        isInt = edit_int_model->data(edit_int_model->index(i,3,QModelIndex())).toBool();

        edit_int_model->setData(edit_int_model->index(i,1,QModelIndex()),i_start);

        if(isInt)
        {
            if(i != 0)
            {
                if(i_stop < i_start)
                {
                    edit_int_model->setData(edit_int_model->index(i-1,2,QModelIndex()),i_start);
                }
                else if(i_start < i_stop)
                {
                    edit_int_model->setData(edit_int_model->index(i,1,QModelIndex()),i_stop);
                }
            }
        }
        else
        {
            if(i != 0)
            {
                if(i_stop > i_start)
                {
                    edit_int_model->setData(edit_int_model->index(i-1,2,QModelIndex()),i_start);
                }
                else if(i_start > i_stop)
                {
                    edit_int_model->setData(edit_int_model->index(i,1,QModelIndex()),i_stop);
                }
            }
        }
    }
}

void Activity::set_curr_act_model(bool recalc)
{
    QModelIndex data_index;
    QStandardItemModel *p_int_model,*p_samp_model;
    p_int_model = this->set_int_model_pointer(recalc);
    p_samp_model = this->set_samp_model_pointer(recalc);

    for(int row = 0; row < p_int_model->rowCount();++row)
    {
            data_index = p_int_model->index(row,0,QModelIndex());
            curr_act_model->setData(curr_act_model->index(row,0,QModelIndex()),p_int_model->data(data_index,Qt::DisplayRole).toString());
            curr_act_model->setData(curr_act_model->index(row,1,QModelIndex()),settings::set_time(this->get_int_duration(row,recalc)));
            data_index = p_samp_model->index(p_int_model->data(p_int_model->index(row,2,QModelIndex()),Qt::DisplayRole).toInt()-1,1,QModelIndex());
            curr_act_model->setData(curr_act_model->index(row,2,QModelIndex()),settings::set_doubleValue(p_samp_model->data(data_index,Qt::DisplayRole).toDouble(),true));
            curr_act_model->setData(curr_act_model->index(row,3,QModelIndex()),settings::set_doubleValue(this->get_int_distance(row,recalc),true));
            curr_act_model->setData(curr_act_model->index(row,4,QModelIndex()),settings::set_time(this->get_int_pace(row,recalc)));
            if(curr_sport == settings::isSwim) curr_act_model->setData(curr_act_model->index(row,5,QModelIndex()),this->get_swim_laps(row,recalc));
            if(curr_sport == settings::isBike)
            {
                curr_act_model->setData(curr_act_model->index(row,5,QModelIndex()),settings::set_doubleValue(this->get_int_value(row,4,recalc),false));
                curr_act_model->setData(curr_act_model->index(row,6,QModelIndex()),round(this->get_int_value(row,3,recalc)));
            }
    }

    curr_act_model->setHorizontalHeaderLabels(settings::get_int_header(curr_sport));
}

void Activity::set_avg_values(int counter, int row, bool add)
{
    avg_counter = counter;
    double t_laptime = static_cast<double>(settings::get_timesec(curr_act_model->data(curr_act_model->index(row,1,QModelIndex())).toString()));
    int t_pace = settings::get_timesec(curr_act_model->data(curr_act_model->index(row,4,QModelIndex())).toString());
    double t_dist = curr_act_model->data(curr_act_model->index(row,3,QModelIndex())).toDouble();
    double t_watt = 0.0,t_cad = 0.0;
    if(curr_sport == settings::isBike)
    {
        t_watt = curr_act_model->data(curr_act_model->index(row,5,QModelIndex())).toDouble();
        t_cad = curr_act_model->data(curr_act_model->index(row,6,QModelIndex())).toDouble();
    }
    if(counter != 0)
    {
        if(add)
        {
            avg_laptime = avg_laptime + t_laptime;
            avg_pace = avg_pace + t_pace;
            avg_dist = avg_dist + t_dist;
            avg_watt = avg_watt + t_watt;
            avg_cad = avg_cad + t_cad;
        }
        else
        {
            avg_laptime = avg_laptime - t_laptime;
            avg_pace = avg_pace - t_pace;
            avg_dist = avg_dist - t_dist;
            avg_watt = avg_watt - t_watt;
            avg_cad = avg_cad + t_cad;
        }
    }
    else
    {
       avg_counter = 1;
       this->reset_avg();
    }
}

void Activity::reset_avg()
{
    avg_counter = 1;
    avg_dist = 0.0;
    avg_laptime = 0;
    avg_pace = 0;
    avg_watt = 0.0;
    avg_cad = 0.0;
}
