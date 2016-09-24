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
    zone_count = 7;
    changeRowCount = false;
}

const QString Activity::isSwim = "Swim ";
const QString Activity::isBike = "Bike ";
const QString Activity::isRun = "Run ";

void Activity::act_reset()
{
    changeRowCount = false;
}

void Activity::read_jsonFile(QString fileContent)
{
    int row;
    QModelIndex data_index;

    QJsonDocument d = QJsonDocument::fromJson(fileContent.toUtf8());
    QJsonObject jsonobj = d.object();

    QJsonValue ride = jsonobj.value(QString("RIDE"));
    QJsonObject item_ride = ride.toObject();

    QJsonValue v_tags = item_ride.value(QString("TAGS"));
    QJsonObject item_tags = v_tags.toObject();

    ride_model = new QStandardItemModel(settings::get_jsoninfos().count()+1,2);

    //Get and Set RIDE and TAGS Entries
    QJsonValue v_date = item_ride["STARTTIME"];
    this->set_date(v_date.toString());
    QJsonValue v_sport = item_tags["Sport"];
    this->set_sport(v_sport.toString());
    settings::set_act_sport(v_sport.toString());
    QJsonValue v_bahn = item_tags["Pool Length"].toString().toDouble();
    swim_track = v_bahn.toDouble();
    settings::set_swimLaplen(v_bahn.toInt());

    ride_model->setData(ride_model->index(0,0,QModelIndex()),"Date:");
    ride_model->setData(ride_model->index(0,1,QModelIndex()),this->get_date());

    for(int i = 0, row = 1; i < settings::get_jsoninfos().count();++i,++row)
    {
        QJsonValue value = item_tags[settings::get_jsoninfos().at(i)];
        ride_model->setData(ride_model->index(row,0,QModelIndex()),settings::get_jsoninfos().at(i)+":");
        ride_model->setData(ride_model->index(row,1,QModelIndex()),value.toString());
    }

    QJsonArray arr_int = item_ride["INTERVALS"].toArray();
    int_model = new QStandardItemModel(arr_int.count(),3);
    edit_int_model = new QStandardItemModel(arr_int.count(),4);
    row=0;

    foreach(const QJsonValue & v_int, arr_int)
    {
        QJsonObject obj_int = v_int.toObject();
        int_model->setData(int_model->index(row,0,QModelIndex()),obj_int["NAME"].toString());
        edit_int_model->setData(edit_int_model->index(row,0,QModelIndex()),obj_int["NAME"].toString());
        int_model->setData(int_model->index(row,1,QModelIndex()),obj_int["START"].toInt());
        edit_int_model->setData(edit_int_model->index(row,1,QModelIndex()),obj_int["START"].toInt());
        int_model->setData(int_model->index(row,2,QModelIndex()),obj_int["STOP"].toDouble());
        edit_int_model->setData(edit_int_model->index(row,2,QModelIndex()),obj_int["STOP"].toDouble());
        ++row;
    }

    if(this->get_sport() == this->isSwim)
    {
        edit_int_model->setData(edit_int_model->index(0,1,QModelIndex()),0);

        QJsonArray arr_swimXdata = item_ride["XDATA"].toArray();
        QJsonObject item_xdata = arr_swimXdata.at(0).toObject();
        QJsonArray arr_lapData = item_xdata["SAMPLES"].toArray();
        swim_xdata = new QStandardItemModel(arr_lapData.count(),5);
        swim_xdata->setHorizontalHeaderLabels(settings::get_swimtime_header());
        int lapNr = 0;
        int intCount = 1;
        double lapStart,lapStartPrev = 0,lapPacePrev = 0;
        double lapSpeed,lapPace;
        row = 0;

        foreach (const QJsonValue & v_xdata, arr_lapData)
        {
            QJsonObject obj_xdata = v_xdata.toObject();     
            lapPace = round((obj_xdata["VALUES"].toArray().at(1).toDouble() - 0.1));
            if(lapPace > 0)
            {
                swim_xdata->setData(swim_xdata->index(row,0,QModelIndex()),QString::number(intCount)+"_"+QString::number(++lapNr*swim_track));
                if(lapPacePrev == 0)
                {
                    lapStart = obj_xdata["SECS"].toDouble();

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
                swim_xdata->setData(swim_xdata->index(row,0,QModelIndex()),"Break");
                lapStart = lapStartPrev + lapPacePrev;
                lapSpeed = 0;
                ++intCount;
            }

            //swim_xdata->setData(swim_xdata->index(row,1,QModelIndex()),obj_xdata["SECS"].toDouble());
            swim_xdata->setData(swim_xdata->index(row,1,QModelIndex()),lapStart);
            swim_xdata->setData(swim_xdata->index(row,2,QModelIndex()),lapPace);
            swim_xdata->setData(swim_xdata->index(row,3,QModelIndex()),obj_xdata["VALUES"].toArray().at(2).toDouble());
            swim_xdata->setData(swim_xdata->index(row,4,QModelIndex()),lapSpeed);
            lapPacePrev = lapPace;
            lapStartPrev = lapStart;
            ++row;
        }    
    }

    QJsonArray arr_samp = item_ride["SAMPLES"].toArray();
    samp_model = new QStandardItemModel(arr_samp.count(),5);
    row=0;

    foreach (const QJsonValue & v_samp, arr_samp)
    {
        QJsonObject obj_samp = v_samp.toObject();
        samp_model->setData(samp_model->index(row,0,QModelIndex()),obj_samp["SECS"].toInt());
        samp_model->setData(samp_model->index(row,1,QModelIndex()),obj_samp["KM"].toDouble());
        samp_model->setData(samp_model->index(row,2,QModelIndex()),obj_samp["KPH"].toDouble());
        samp_model->setData(samp_model->index(row,3,QModelIndex()),obj_samp["CAD"].toInt());
        samp_model->setData(samp_model->index(row,4,QModelIndex()),obj_samp["WATTS"].toDouble());
        ++row;
    }

    int_model->setData(int_model->index(int_model->rowCount()-1,2,QModelIndex()),samp_model->rowCount()-1);
    edit_int_model->setData(edit_int_model->index(edit_int_model->rowCount()-1,2,QModelIndex()),samp_model->rowCount()-1);
    edit_int_model->setHorizontalHeaderLabels(settings::get_time_header());

    edit_dist_model = new QStandardItemModel(int_model->rowCount(),2);
    edit_dist_model->setHorizontalHeaderLabels(settings::get_km_header());
    for(int i = 0; i < int_model->rowCount();++i)
    {
        edit_dist_model->setData(edit_dist_model->index(i,0,QModelIndex()),int_model->data(int_model->index(i,0,QModelIndex())).toString());
        edit_dist_model->setData(edit_dist_model->index(i,1,QModelIndex()),this->get_int_distance(i,false));
    }

    row = ride_model->rowCount();
    ride_model->insertRows(row,1,QModelIndex());
    ride_model->setData(ride_model->index(row,0,QModelIndex()),"Distance:");
    data_index = samp_model->index(samp_model->rowCount()-1,1,QModelIndex());
    ride_model->setData(ride_model->index(row,1,QModelIndex()),QString::number(samp_model->data(data_index,Qt::DisplayRole).toDouble()));

    row = ride_model->rowCount();
    ride_model->insertRows(row,1,QModelIndex());
    ride_model->setData(ride_model->index(row,0,QModelIndex()),"Duration:");
    ride_model->setData(ride_model->index(row,1,QModelIndex()),QDateTime::fromTime_t(samp_model->rowCount()).toUTC().toString("hh:mm:ss"));

    this->set_curr_act_model(false);
    this->set_dist_factor();
    this->reset_avg();

    if(this->get_sport() == this->isSwim)
    {
        p_swim_timezone.resize(zone_count+1);
        p_swim_time.resize(zone_count+1);
        p_hf_timezone.resize(zone_count);
        hf_zone_avg.resize(zone_count);
        hf_avg = 0;
        move_time = 0.0;
        this->read_swim_data();
    }
}

void Activity::set_additional_ride_info()
{
    if(settings::get_act_isrecalc())
    {
        ride_model->setData(ride_model->index(6,1,QModelIndex()),QString::number(edit_samp_model->data(edit_samp_model->index(edit_samp_model->rowCount()-1,1,QModelIndex())).toDouble()));
        ride_model->setData(ride_model->index(7,1,QModelIndex()),QDateTime::fromTime_t(edit_samp_model->rowCount()).toUTC().toString("hh:mm:ss"));
    }
    else
    {
        ride_model->setData(ride_model->index(6,1,QModelIndex()),QString::number(samp_model->data(samp_model->index(samp_model->rowCount()-1,1,QModelIndex())).toDouble()));
        ride_model->setData(ride_model->index(7,1,QModelIndex()),QDateTime::fromTime_t(samp_model->rowCount()).toUTC().toString("hh:mm:ss"));
    }
}

void Activity::read_swim_data()
{
    QStringList pace_header,hf_header;
    pace_header << "Zone" << "Low (min/100m)" << "High (min/100m)" << "Time in Zone";
    hf_header << "Zone" << "Low (1/Min)" << "High (1/min)" << "Time in Zone";

    QStringList swimZone, hfZone,levels;
    QString temp,zone_low,zone_high,hfThres,hfMax;

    //Set Tableinfos
    swim_pace_model = new QStandardItemModel(zone_count,4);
    swim_pace_model->setHorizontalHeaderLabels(pace_header);

    swim_hf_model = new QStandardItemModel(zone_count,4);
    swim_hf_model->setHorizontalHeaderLabels(hf_header);

    //Read current CV and HF Threshold
    QString temp_cv = settings::get_paceList().at(0);
    swim_cv = (3600.0 / settings::get_timesec(temp_cv)) / 10.0;
    pace_cv = settings::get_timesec(temp_cv);

    swimZone = settings::get_swimRange();
    hfZone = settings::get_hfRange();
    hfThres = settings::get_hfList().at(0);
    hf_threshold = hfThres.toInt();
    hfMax = settings::get_hfList().at(1);
    levels = settings::get_levelList();

    //Set Swim zone low and high

        for(int i = 0; i < zone_count; i++)
        {
            temp = swimZone.at(i);
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
        this->set_time_in_zones();

        for (int x = 1; x <= zone_count;x++)
        {
            swim_pace_model->setData(swim_pace_model->index(x-1,3,QModelIndex()),settings::set_time(p_swim_timezone[x]));
        }

    //Set HF zone low and high

        for(int i = 0; i < zone_count; i++)
        {
            temp = hfZone.at(i);
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
                swim_hf_model->setData(swim_hf_model->index(i,2,QModelIndex()),hfMax);
                this->set_hf_zone_avg(this->get_zone_values(zone_low.toDouble(),hf_threshold,false),hfMax.toDouble(),i);
            }
        }
}

void Activity::set_time_in_zones()
{
    int z0=0,z1=0,z2=0,z3=0,z4=0,z5=0,z6=0,z7=0;
    double paceSec;

    for(int i = 0; i < samp_model->rowCount(); i++)
    {
        paceSec = samp_model->data(samp_model->index(i,2,QModelIndex())).toDouble();

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

     //REKOM: Z1 + Z2*0,5 + Z0*0,125
     p_hf_timezone[0] = ceil(p_swim_timezone[1] + (p_swim_timezone[2]*hf_factor[2]) + (p_swim_timezone[0]*hf_factor[0]));

     //END: Z2*0,5 + Z3*0,75 + Z4*0,25 + Z0*0,125
     p_hf_timezone[1] = ceil((p_swim_timezone[2]*hf_factor[2]) + (p_swim_timezone[3]*hf_factor[3]) + (p_swim_timezone[4]*hf_factor[1]) + (p_swim_timezone[0]*hf_factor[0]));

     //TEMP: Z3*0,25 + Z4*0,5 + Z5*0,5 + Z0*0,5
     p_hf_timezone[2] = ceil((p_swim_timezone[3]*hf_factor[1]) + (p_swim_timezone[4]*hf_factor[2]) + (p_swim_timezone[5]*hf_factor[2]) + (p_swim_timezone[0]*hf_factor[2]));

     //LT: Z4*0,25 + Z5*0,25 + Z6*0,75 + Z7*0,5 + Z0*0,125
     p_hf_timezone[3] = ceil((p_swim_timezone[4]*hf_factor[1]) + (p_swim_timezone[5]*hf_factor[1]) + (p_swim_timezone[6]*hf_factor[3]) + (p_swim_timezone[7]*hf_factor[2]) + (p_swim_timezone[0]*hf_factor[0]));

     //VO2: Z5*0,25 + Z6*0,125 + Z7*0,25 + Z0*0,25
     p_hf_timezone[4] = ceil((p_swim_timezone[5]*hf_factor[1]) + (p_swim_timezone[6]*hf_factor[0]) + (p_swim_timezone[7]*hf_factor[1]) + (p_swim_timezone[0]*hf_factor[0]));

     //AC: Z6*0,125 + Z7*0,25
     p_hf_timezone[5] = ceil((p_swim_timezone[6]*hf_factor[0]) + (p_swim_timezone[7]*hf_factor[1]));

     //NEURO: in swimming hardly reached
     p_hf_timezone[6] = 0.0;

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
    if(this->get_sport() == this->isSwim || this->get_sport() == this->isBike)
    {
        return 6;
    }
    else
    {
        return 5;
    }
}

void Activity::set_dist_factor()
{
    if (this->get_sport() == isSwim)
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
    if(this->get_sport() == this->isSwim)
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
        if (this->get_sport() == this->isSwim)
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
        edit_int_model->setData(edit_int_model->index(row,3,QModelIndex()),0);
    }
    else
    {
        edit_int_model->setData(edit_int_model->index(row,3,QModelIndex()),1);
    }

    return lapcount;
}

double Activity::get_int_watts(int row)
{
    double watts = 0.0;
    int int_start,int_stop;
    int_start = int_model->data(int_model->index(row,1,QModelIndex())).toInt();
    int_stop = int_model->data(int_model->index(row,2,QModelIndex())).toInt();

    for(int i = int_start; i < int_stop; ++i)
    {
        watts = watts + samp_model->data(samp_model->index(i,4,QModelIndex())).toDouble();
    }
    watts = watts / (int_stop - int_start);

    return watts;
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
            edit_dist_model->setData(edit_dist_model->index(i,0,QModelIndex()),i);
        }
    }

    if(this->get_sport() == this->isSwim && recalc)
    {
        p_swimlaps.resize(edit_dist_model->rowCount());
        this->adjust_intervalls();
        for(int i = 0; i < edit_dist_model->rowCount();++i)
        {
            edit_int_model->setData(edit_int_model->index(i,3,QModelIndex()),this->check_is_intervall(i));
            p_swimlaps[i] = round((edit_dist_model->data(edit_dist_model->index(i,1,QModelIndex())).toDouble()*1000)) / swim_track;

            if(i == 0)
            {
                lapname = "Warmup_";
                edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),QString::number(lapcounter)+"_"+lapname+QString::number(swim_track*p_swimlaps[i]));
                ++lapcounter;
            }
            if(i != 0 && i < edit_dist_model->rowCount()-1)
            {
                if(this->check_is_intervall(i) == 1)
                {
                    lapname = "Int_";
                    edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),QString::number(lapcounter)+"_"+lapname+QString::number(swim_track*p_swimlaps[i]));
                    ++lapcounter;
                }
                else
                {
                    lapname = "Break";
                    edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),lapname);
                }
            }
            if(i == edit_dist_model->rowCount()-1)
            {
                lapname = "Cooldown_";
                edit_int_model->setData(edit_int_model->index(i,0,QModelIndex()),QString::number(lapcounter)+"_"+lapname+QString::number(swim_track*p_swimlaps[i]));
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

    this->set_edit_samp_model();
    this->set_curr_act_model(recalc);
}

double Activity::get_int_speed(int row,bool recalc)
{
    double speed;

    double pace = this->get_int_duration(row,recalc) / (edit_dist_model->data(edit_dist_model->index(row,1,QModelIndex())).toDouble() * 10.0);

    speed = 360.0 / pace;

    return speed;
}

double Activity::polish_SpeedValues(double currSpeed,double avgSpeed,double factor,bool setrand)
{
    double randfact = ((static_cast<double>(rand()) / static_cast<double>(RAND_MAX)) / (currSpeed/((factor*100)+1.0)));
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
    isInt = edit_dist_model->data(edit_dist_model->index(row,1,QModelIndex())).toDouble();
    if(isInt == 0)
    {
        return 0;
    }
    else
    {
        return 1;
    }
}

void Activity::set_edit_samp_model()
{
    int sampRowCount = samp_model->rowCount();
    edit_samp_model = new QStandardItemModel(sampRowCount,5);
    new_dist.resize(sampRowCount);
    calc_speed.resize(sampRowCount);
    calc_cadence.resize(sampRowCount);
    double msec = 0.0;
    int int_start,int_stop,sportindex,swimLaps;
    double overall = 0.0,lowLimit;
    double swimPace,swimSpeed,swimCycle;
    bool isBreak = true;
    if(this->get_sport() != this->isSwim)
    {
        if(this->get_sport() == this->isBike) sportindex = 1;
        if(this->get_sport() == this->isRun) sportindex = 2;
        lowLimit = settings::get_speed(QTime::fromString(settings::get_paceList().at(sportindex),"mm:ss"),0,v_sport.trimmed(),true).toDouble();
        lowLimit = lowLimit - (lowLimit*0.20);
    }

    if(this->get_sport() == this->isSwim)
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

            if(swimPace > 0)
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
    }
    else
    {
      for(int c_int = 0; c_int < edit_dist_model->rowCount(); ++c_int)
      {
         int_start = edit_int_model->data(edit_int_model->index(c_int,1,QModelIndex())).toInt();
         int_stop = edit_int_model->data(edit_int_model->index(c_int,2,QModelIndex())).toInt();
         msec = edit_dist_model->data(edit_dist_model->index(c_int,1,QModelIndex())).toDouble() / this->get_int_duration(c_int,true);

         for(int c_dist = int_start;c_dist <= int_stop; ++c_dist)
         {
            if(c_dist == 0)
            {
                new_dist[0] = 0.0000;
            }
            else
            {
                if(this->get_sport() == this->isRun)
                {
                    calc_speed[c_dist] = this->interpolate_speed(c_int,c_dist,lowLimit);
                }
                else
                {
                    calc_speed[c_dist] = samp_model->data(samp_model->index(c_dist,2,QModelIndex())).toDouble();
                    calc_cadence[c_dist] = samp_model->data(samp_model->index(c_dist,3,QModelIndex())).toDouble();
                }
                new_dist[c_dist] = new_dist[c_dist-1] + msec;
            }
         }
     }
   }

      for(int i = 0; i < samp_model->rowCount();++i)
      {
          edit_samp_model->setData(edit_samp_model->index(i,0,QModelIndex()),samp_model->data(samp_model->index(i,0,QModelIndex())).toInt());
          edit_samp_model->setData(edit_samp_model->index(i,1,QModelIndex()),QString::number(new_dist[i]));
          edit_samp_model->setData(edit_samp_model->index(i,2,QModelIndex()),QString::number(calc_speed[i]));
          edit_samp_model->setData(edit_samp_model->index(i,3,QModelIndex()),QString::number(calc_cadence[i]));
          edit_samp_model->setData(edit_samp_model->index(i,4,QModelIndex()),samp_model->data(samp_model->index(i,4,QModelIndex())).toDouble());
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
    curr_act_model = new QStandardItemModel(int_model->rowCount(),this->get_header_num());

    p_int_model = this->set_int_model_pointer(recalc);
    p_samp_model = this->set_samp_model_pointer(recalc);

    for(int row = 0; row < int_model->rowCount();++row)
    {
            data_index = p_int_model->index(row,0,QModelIndex());
            curr_act_model->setData(curr_act_model->index(row,0,QModelIndex()),p_int_model->data(data_index,Qt::DisplayRole).toString());
            curr_act_model->setData(curr_act_model->index(row,1,QModelIndex()),settings::set_time(this->get_int_duration(row,recalc)));
            data_index = p_samp_model->index(p_int_model->data(p_int_model->index(row,2,QModelIndex()),Qt::DisplayRole).toInt()-1,1,QModelIndex());
            curr_act_model->setData(curr_act_model->index(row,2,QModelIndex()),p_samp_model->data(data_index,Qt::DisplayRole).toDouble());
            curr_act_model->setData(curr_act_model->index(row,3,QModelIndex()),this->get_int_distance(row,recalc));
            curr_act_model->setData(curr_act_model->index(row,4,QModelIndex()),settings::set_time(this->get_int_pace(row,recalc)));
            if(this->get_sport() == this->isSwim) curr_act_model->setData(curr_act_model->index(row,5,QModelIndex()),this->get_swim_laps(row,recalc));
            if(this->get_sport() == this->isBike) curr_act_model->setData(curr_act_model->index(row,5,QModelIndex()),this->get_int_watts(row));
    }

    curr_act_model->setHorizontalHeaderLabels(settings::get_int_header());
}

void Activity::set_avg_values(int counter, int row, bool add)
{
    avg_counter = counter;
    double t_laptime = static_cast<double>(settings::get_timesec(curr_act_model->data(curr_act_model->index(row,1,QModelIndex())).toString()));
    int t_pace = settings::get_timesec(curr_act_model->data(curr_act_model->index(row,4,QModelIndex())).toString());
    double t_dist = curr_act_model->data(curr_act_model->index(row,3,QModelIndex())).toDouble();
    double t_watt = 0.0;
    if(this->get_sport() == this->isBike) t_watt = curr_act_model->data(curr_act_model->index(row,5,QModelIndex())).toDouble();

    if(counter != 0)
    {
        if(add)
        {
            avg_laptime = avg_laptime + t_laptime;
            avg_pace = avg_pace + t_pace;
            avg_dist = avg_dist + t_dist;
            avg_watt = avg_watt + t_watt;
        }
        else
        {
            avg_laptime = avg_laptime - t_laptime;
            avg_pace = avg_pace - t_pace;
            avg_dist = avg_dist - t_dist;
            avg_watt = avg_watt - t_watt;
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
}
