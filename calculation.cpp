#include "calculation.h"

calculation::calculation()
{
}

QString calculation::set_time(int sec)
{
    if(sec < 3600)
    {
        return QDateTime::fromTime_t(sec).toUTC().toString("mm:ss");
    }
    if(sec >= 3600 && sec < 86400)
    {
        return QDateTime::fromTime_t(sec).toUTC().toString("hh:mm:ss");
    }
    if(sec >=86400)
    {
        QString timeVal;
        int hours, minutes;
        hours = sec / 60 / 60;
        minutes = (sec - (hours*60*60)) / 60;
        timeVal = QString::number(hours) + ":" + QString::number(minutes);
        return timeVal;
    }
    return 0;
}

int calculation::get_timesec(QString time)
{
    int sec = 0;
    if(time.length() == 8)
    {
        QTime durtime = QTime::fromString(time,"hh:mm:ss");
        sec = durtime.hour()*60*60;
        sec = sec + durtime.minute()*60;
        sec = sec + durtime.second();
    }
    if(time.length() == 5)
    {
        QTime durtime = QTime::fromString(time,"mm:ss");
        sec = durtime.minute()*60;
        sec = sec + durtime.second();
    }

    return sec;
}

QString calculation::get_workout_pace(double dist, QTime duration,QString sport,bool full_label)
{
    QStringList speedLabel;
    speedLabel << " min/km - " << " km/h" << " min/km" << "no Speed";
    int nr=0;
    double speed = 0;

    int sec = duration.hour()*60*60;
    sec = sec + duration.minute()*60;
    sec = sec + duration.second();

    int min = duration.hour()*60;
    min = min + duration.minute();

    if(dist != 0.0 || min != 0)
    {
        if(sport == settings::isSwim) speed = sec/dist, nr=0;
        if(sport == settings::isBike)
        {
            if(min > 0)
            {
                speed = (dist/min)*60, nr=1;
            }
            else
            {
                speed = (dist/sec)*3600, nr=1;
            }
        }
        if(sport == settings::isRun)  speed = sec/dist, nr=2;
        if(sport == settings::isAlt || sport == settings::isStrength) speed = 0.0, nr=3;

        speed = set_doubleValue(speed,false);

        if(full_label)
        {
            if(nr == 0) return (QDateTime::fromTime_t(speed).toUTC().toString("mm:ss") + speedLabel.at(nr) + QDateTime::fromTime_t(speed/10.0).toUTC().toString("mm:ss") +" min/100m");
            if(nr == 1) return (QString::number(speed) + speedLabel.at(nr));
            if(nr == 2) return (QDateTime::fromTime_t(speed).toUTC().toString("mm:ss") + speedLabel.at(nr));
            if(nr == 3) return speedLabel.at(nr);
        }
        else
        {
            if(nr == 0) return (QDateTime::fromTime_t(speed/10.0).toUTC().toString("mm:ss") +" min/100m");
            if(nr == 1) return (QString::number(speed) + speedLabel.at(nr));
            if(nr == 2) return (QDateTime::fromTime_t(speed).toUTC().toString("mm:ss") + speedLabel.at(nr));
            if(nr == 3) return speedLabel.at(nr);
        }
    }
    else
    {
        speed = 0.0;
    }

    return speedLabel.at(3);
}

QString calculation::get_speed(QTime pace,int dist,QString sport,bool fixdist)
{
    int sec = pace.minute()*60;
    sec = sec + pace.second();
    double speed;

    if(sec > 0)
    {
        if(fixdist)
        {
            if(sport == settings::isSwim)
            {
                speed = 360.0/sec;
            }
            else
            {
                speed = 3600.0/sec;

            }
        }
        else
        {
            speed = (3600.0/sec) / (1000.0/dist);
        }
        return QString::number(speed);
    }

    return "--";
}

int calculation::get_hfvalue(QString percent)
{
    double value = percent.toDouble();

    return static_cast<int>(round(settings::get_thresValue("hfthres") * (value / 100.0)));
}

double calculation::calc_totalWork(double weight,double avgHF, double moveTime)
{
    int age = QDate::currentDate().year() - settings::get_gcInfo("yob").toInt();

    return ceil(((-55.0969 + (0.6309 * avgHF) + (0.1988 * weight) + (0.2017 * age))/4.184) * moveTime/60);
}

double calculation::estimate_stress(QString sport, QString p_goal, int duration)
{
    double goal = 0;
    double est_stress = 0;
    double est_power = 0;
    double raw_effort = 0;
    double cv_effort = 0;
    double thresPower = 0;
    if(sport == settings::isSwim)
    {
        goal = get_timesec(p_goal);
    }
    if(sport == settings::isBike)
    {
        goal = p_goal.toDouble();
    }
    if(sport == settings::isRun)
    {
        goal = get_timesec(p_goal);
    }
    if(sport == settings::isStrength)
    {
        goal = p_goal.toDouble();
    }


    if(goal > 0)
    {
        if(sport == settings::isSwim)
        {
            thresPower = settings::get_thresValue("swimpower");
            goal = settings::get_thresValue("swimpace") / goal;
            goal = pow(goal,3.0);
            est_power = thresPower * goal;
            raw_effort = (duration * est_power) * (est_power / thresPower);
            cv_effort = thresPower * 3600;

        }
        if(sport == settings::isBike)
        {
            thresPower = settings::get_thresValue("bikepower");
            raw_effort = (duration * goal) * (goal / thresPower);
            cv_effort = thresPower * 3600;
        }
        if(sport == settings::isRun)
        {
            thresPower = settings::get_thresValue("runpower");
            est_power = thresPower * (settings::get_thresValue("runpace")/goal);
            raw_effort = (duration * est_power) * (est_power / thresPower);
            cv_effort = thresPower * 3600;
        }
        if(sport == settings::isStrength)
        {
            thresPower = settings::get_thresValue("stgpower");
            raw_effort = (duration * goal) * (goal / thresPower);
            cv_effort = thresPower * 3600;
        }
        est_stress = (raw_effort / cv_effort) * 100;
        return set_doubleValue(est_stress,false);
    }
    return 0;
}

double calculation::set_doubleValue(double value, bool isthree)
{
    if(isthree)
    {
        return value = round( value * 1000.0 ) / 1000.0;
    }
    else
    {
        return ((static_cast<int>(value *100 +.5)) / 100.0);
    }
    return 0;
}
