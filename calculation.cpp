#include "calculation.h"

calculation::calculation()
{
}

QString calculation::set_time(int sec)
{
    QTime vTime;
    vTime.setHMS(0,0,0);

    if(sec < 3600)
    {
        return vTime.addSecs(sec).toString("mm:ss");
    }
    if(sec >= 3600 && sec < 86400)
    {
        return vTime.addSecs(sec).toString("hh:mm:ss");
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

double calculation::get_speed(QTime pace,int dist,QString sport,bool fixdist)
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
        return speed;
    }

    return 0;
}

double calculation::calcSpeed(double sec, double dist, double factor)
{
    return 3600.0 / (sec / (dist/factor));
}

QString calculation::calc_lapSpeed(QString sport,double sec)
{
    if(sport == settings::isSwim)
    {
        return QString::number(set_doubleValue(360.0/sec,false))+" km/h";
    }
    else if(sport == settings::isBike || sport == settings::isRun)
    {
        return QString::number(set_doubleValue(3600.0/sec,false))+" km/h";
    }
    else
    {
        return "--";
    }

    return 0;
}

int calculation::calc_lapPace(QString sport, int duration, double distance)
{
    int pace;
    if(distance > 0)
    {
        if (sport == settings::isSwim)
        {
            pace = duration * (100.0/distance);
        }
        else
        {
            pace = duration / distance;
        }
        return pace;
    }
    else
    {
        return 0;
    }
    return 0;
}

int calculation::get_hfvalue(QString percent)
{
    double value = percent.toDouble();

    return static_cast<int>(round(settings::get_thresValue("hfthres") * (value / 100.0)));
}

double calculation::calc_totalCal(double weight,double avgHF, double moveTime)
{
    int age = QDate::currentDate().year() - settings::get_athleteValue("yob");

    return ceil(((-55.0969 + (0.6309 * avgHF) + (0.1988 * weight) + (0.2017 * age))/4.184) * moveTime/60);
}

double calculation::calc_totalWork(QString sport, double pValue, double dura,int tempID)
{
    double factor = 1000.0;
    double grav = 9.81;
    double weight = settings::get_weightforDate(QDateTime::currentDateTime());
    double height = settings::get_athleteValue("height");
    double mSec = pValue*factor/3600.0;

    if(sport == settings::isSwim)
    {
        double correctMet = 1;
        double swimThresPace = settings::get_thresValue("swimpace");
        double sriFactor = 0;
        double sri = 0;

        if(tempID != 0)
        {
            sri = pow(swimThresPace/pValue,3.0);
            sriFactor = sqrt(sri) / 10.0;
            if(sri < 1.0) sriFactor = sriFactor * -1.0;
        }
        correctMet = get_corrected_MET(weight,tempID)+sriFactor;

        return (correctMet * 3.5 * weight / 200.0) * (dura/60.0);
    }
    if(sport == settings::isBike)
    {
        return dura * pValue / factor;
    }
    if(sport == settings::isRun)
    {
        double bodyHub = (height * 0.057) + (((3600/settings::get_thresValue("runpace")) / pValue) / 100.0);
        return (weight * grav * mSec * bodyHub) * dura / factor;
    }
    if(sport == settings::isStrength)
    {
        return (weight * grav * mSec * 0.218) * dura / factor;
    }
    if(sport == settings::isAlt)
    {
        return (weight * grav * mSec * 0.146) * dura / factor;
    }

    return 0;
}

QString calculation::threstopace(double thresPace, double percent)
{
    return set_time(static_cast<int>(round(thresPace / (percent/100.0))));
}

double calculation::wattToSpeed(double thresPower,double thresSpeed,double currWatt)
{
    double diff = 0;
    double speedStep = 0.3;

    if(currWatt < thresPower)
    {
        diff = thresPower - currWatt;
        return thresSpeed - ((diff/5.0) * speedStep);
    }
    else
    {
        diff = currWatt - thresPower;
        return thresSpeed + ((diff/5.0) * speedStep);
    }

    return 0;
}

QString calculation::calc_threshold(QString sport,double threshold,double percent)
{
    QString thresValue = "00:00";

    if(percent > 0)
    {
        if(sport == settings::isSwim || sport == settings::isRun)
        {
            thresValue = set_time(static_cast<int>(round(threshold / (percent/100.0))));
        }
        if(sport == settings::isBike || sport == settings::isStrength || sport == settings::isAlt)
        {
            thresValue = QString::number(threshold * (percent/100));
        }
    }

    return thresValue;
}

double calculation::calc_distance(QString duration, double pace)
{
    if(pace > 0)
    {
        return set_doubleValue(get_timesec(duration) / pace,true);
    }
    return 0;
}

QString calculation::calc_duration(QString sport,double dist, QString pace)
{
    if(sport == settings::isSwim)
    {
        return set_time(get_timesec(pace) * (dist*10));
    }
    else
    {
        return set_time(get_timesec(pace) * dist);
    }
    return 0;
}

double calculation::calc_lnp(double speed,double athleteHeight,double athleteWeight)
{
    double athleteF = (0.2025*pow(athleteHeight,0.725)*pow(athleteWeight,0.425))*0.266;
    double cAero = 0.5*1.2*0.9*athleteF*pow(speed,2)/athleteWeight;
    double athleteEff = (0.25+0.054*speed)*(1 - 0.5*speed/8.33);

    return (cAero+3.6*athleteEff)*speed*athleteWeight;
}

double calculation::current_dayCalories()
{
    double weight = settings::get_weightforDate(QDateTime::currentDateTime());
    double height = settings::get_athleteValue("height")*100;
    double age = QDate::currentDate().year() - settings::get_athleteValue("yob");

    return round((10*weight)+(6.25*height)-(5*age)+5);
}

double calculation::calc_swim_xpower(double distance,double pace,double time,double athleteWeight)
{
    double K = 2 + 0.35 * athleteWeight;
    double velo = distance / pace;
    double alpha = 2.0 /10;
    QVector<double> rawEWMA(time);
    double xPower = (K/0.6)*pow(velo,3);
    double xPowerSum = 0.0;

    for(int i = 0; i < rawEWMA.count(); ++i)
    {
        if(i == 0)
        {
            rawEWMA[i] = alpha*xPower+(1-alpha)*i;
        }
        else
        {
            rawEWMA[i] = alpha*xPower+(1-alpha)*rawEWMA[i-1];
        }
        xPowerSum += pow(rawEWMA[i],3);
    }
    return pow(xPowerSum / time,0.33);
}

double calculation::get_corrected_MET(double weight, int style)
{
    QString factor = settings::get_listValues("SwimMET").at(style);
    double swimMET = factor.toDouble();
    double mlkgmin = (((current_dayCalories()/1440)/5)/weight)*1000.0;

    if(style == 0)
    {
        return swimMET;
    }
    else
    {
        return swimMET * (3.5/mlkgmin);
    }

    return 10;
}

double calculation::estimate_stress(QString sport, QString p_goal, int duration)
{
    double goal = 0;
    double est_stress = 0;
    double est_power = 0;
    double raw_effort = 0;
    double cv_effort = 0;
    double thresPower = 0;
    double athleteWeight = settings::get_athleteValue("weight");
    double athleteHeight = settings::get_athleteValue("height");

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
        goal = get_speed(QTime::fromString(p_goal,"mm:ss"),0,settings::isRun,true)/3.6;
    }
    if(sport == settings::isStrength)
    {
        goal = p_goal.toDouble();
    }
    if(sport == settings::isAlt)
    {
        goal = p_goal.toDouble();
    }

    if(goal > 0)
    {
        if(sport == settings::isSwim)
        {
            thresPower = settings::get_thresValue("swimpower");
            est_power = calc_swim_xpower(100,goal,duration,athleteWeight);
            raw_effort = (duration * est_power) * (est_power / thresPower);         
        }
        if(sport == settings::isBike)
        {
            thresPower = settings::get_thresValue("bikepower");
            raw_effort = (duration * goal) * (goal / thresPower);
        }
        if(sport == settings::isRun)
        {
            thresPower = settings::get_thresValue("runpower");
            est_power = calc_lnp(goal,athleteHeight,athleteWeight);
            raw_effort = est_power * duration * (est_power / thresPower);
        }
        if(sport == settings::isStrength)
        {
            thresPower = settings::get_thresValue("stgpower");
            raw_effort = (duration * goal) * (goal / thresPower);
        }
        if(sport == settings::isAlt)
        {
            thresPower = settings::get_thresValue("runpower");
            raw_effort = (duration * goal) * (goal / thresPower);
        }
        cv_effort = thresPower * 3600;
        est_stress = (raw_effort / cv_effort) * 100;
        return set_doubleValue(est_stress,false);
    }
    return 0;
}

double calculation::set_doubleValue(double value, bool setthree)
{
    if(setthree)
    {
        return value = round( value * 1000.0 ) / 1000.0;
    }
    else
    {
        return ((static_cast<int>(value *100 +.5)) / 100.0);
    }
    return 0;
}

double calculation::get_thresPercent(QString sport, QString level, bool max)
{
    QString range = settings::get_rangeValue(sport,level);
    QString value;

    if(max)
    {
        value = range.split("-").last();
        return value.toDouble();
    }
    else
    {
        value = range.split("-").first();
        return value.toDouble();
    }
}
