#include "calculation.h"

calculation::calculation()
{
    ltsValues = settings::getdoubleMapPointer(settings::dMap::LTS);
    doubleValues = settings::getdoubleMapPointer(settings::dMap::Double);
    gcValues = settings::getStringMapPointer(settings::stingMap::GC);
    generalValues = settings::getStringMapPointer(settings::stingMap::General);
    firstdayofweek = QDate::currentDate().addDays(1 - QDate::currentDate().dayOfWeek());
}

QHash<QString,double>* calculation::thresValues = settings::getdoubleMapPointer(settings::dMap::Threshold);
QHash<QString,double>* calculation::athleteValues = settings::getdoubleMapPointer(settings::dMap::Athlete);
bool calculation::usePMData = false;

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
    return nullptr;
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
        if(sport == settings::isSwim)
        {
            speed = sec/dist;
            nr=0;
        }
        if(sport == settings::isBike)
        {
            if(min > 0)
            {
                speed = (dist/min)*60;
                nr=1;
            }
            else
            {
                speed = (dist/sec)*3600;
                nr=1;
            }
        }
        if(sport == settings::isRun)
        {
            speed = sec/dist;
            nr=2;
        }
        if(sport == settings::isAlt || sport == settings::isStrength) static_cast<void>(speed = 0.0), nr=3;

        speed = set_doubleValue(speed,false);

        if(full_label)
        {
            if(nr == 0) return (QDateTime::fromTime_t(static_cast<uint>(speed)).toUTC().toString("mm:ss") + speedLabel.at(nr) + QDateTime::fromTime_t(static_cast<uint>(speed/10.0)).toUTC().toString("mm:ss") +" min/100m");
            if(nr == 1) return (QString::number(speed) + speedLabel.at(nr));
            if(nr == 2) return (QDateTime::fromTime_t(static_cast<uint>(speed)).toUTC().toString("mm:ss") + speedLabel.at(nr));
            if(nr == 3) return speedLabel.at(nr);
        }
        else
        {
            if(nr == 0) return (QDateTime::fromTime_t(static_cast<uint>(speed/10.0)).toUTC().toString("mm:ss") +" min/100m");
            if(nr == 1) return (QString::number(speed) + speedLabel.at(nr));
            if(nr == 2) return (QDateTime::fromTime_t(static_cast<uint>(speed)).toUTC().toString("mm:ss") + speedLabel.at(nr));
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

double calculation::calc_Speed(double sec, double dist, double factor)
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
}

int calculation::calc_lapPace(QString sport, int duration, double distance)
{
    int pace;
    if(distance > 0)
    {
        if (sport == settings::isSwim)
        {
            pace = static_cast<int>(duration * (100.0/distance));
        }
        else
        {
            pace = static_cast<int>(duration / distance);
        }
        return pace;
    }
    else
    {
        return 0;
    }
}

int calculation::get_hfvalue(QString percent)
{
    double value = percent.toDouble();

    return static_cast<int>(round(thresValues->value("hfthres") * (value / 100.0)));
}

double calculation::calc_totalCal(double weight,double avgHF, double moveTime)
{
    int age = static_cast<int>(QDate::currentDate().year() - athleteValues->value("yob"));

    return ceil(((-55.0969 + (0.6309 * avgHF) + (0.1988 * weight) + (0.2017 * age))/4.184) * moveTime/60);
}

double calculation::calc_totalWork(QString sport, double pValue, double dura,int tempID)
{
    double factor = 1000.0;
    double grav = 9.81;
    double weight = settings::get_weightforDate(QDateTime::currentDateTime());
    double height = athleteValues->value("height");
    double mSec = pValue*factor/3600.0;

    if(sport == settings::isSwim)
    {
        double correctMet = 1;
        double swimThresPace = thresValues->value("swimpace");
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
        double bodyHub = (height * 0.0543) + (((3600/thresValues->value("runpace")) / pValue) / 100.0);
        return (weight * grav * mSec * bodyHub) * dura / factor;
    }
    if(sport == settings::isStrength || sport == settings::isAlt)
    {
        return (weight * grav * mSec * 0.145) * dura / factor;
    }

    return 0;
}

QString calculation::threstopace(double thresPace, double percent)
{
    return set_time(static_cast<int>(round(thresPace / (percent/100.0))));
}

double calculation::wattToSpeed(double thresPower,double currWatt)
{
    double diff = 0;

    if(currWatt == 0.0)
    {
        return 0;
    }
    else if(currWatt <= thresPower)
    {
        diff = thresPower - currWatt;
        return thresValues->value("bikespeed") - (diff * thresValues->value("wattfactor"));
    }
    else if(currWatt > thresPower)
    {
        diff = currWatt - thresPower;
        return thresValues->value("bikespeed") + (diff * thresValues->value("wattfactor"));
    }

    return 0;
}

QString calculation::calc_thresPace(double threshold,double percent)
{
    QString thresValue = "00:00";

    if(percent > 0)
    {
        thresValue = set_time(static_cast<int>(round(threshold / (percent/100.0))));
    }

    return thresValue;
}

double calculation::calc_thresPower(double threshold, double percent)
{
    return round(threshold *(percent/100.0));
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
        return set_time(static_cast<int>(get_timesec(pace) * (dist*10)));
    }
    else
    {
        return set_time(static_cast<int>(get_timesec(pace) * dist));
    }
}

double calculation::calc_lnp(double speed,double athleteHeight,double athleteWeight)
{
    double athleteF = (0.2025*pow(athleteHeight,0.725)*pow(athleteWeight,0.425))*0.266;
    double cAero = 0.5*1.2*0.9*athleteF*pow(speed,2)/athleteWeight;
    double athleteEff = (0.25+0.054*speed)*(1 - 0.5*speed/8.33);

    return (cAero+3.6*athleteEff)*speed*athleteWeight;
}

double calculation::current_dayCalories(QDateTime calcDate)
{
    int calMethode = static_cast<int>(athleteValues->value("methode"));
    double weight = settings::get_weightforDate(calcDate);

    if(calMethode == 0)
    {
        //Mifflin-St.Jeor-Formel
        double height = athleteValues->value("height")*100;
        double age = QDate::currentDate().year() - athleteValues->value("yob");
        int sF = athleteValues->value("sex") == 0.0 ? 5 : -161;

        return round((10*weight)+(6.25*height)-(5*age)+sF);
    }
    if(calMethode == 1)
    {
        //Katch-McArdle Formel
        double bodyFat = athleteValues->value("bodyfat");
        return round(370+(21.6 *(weight - (weight*(bodyFat/100.0)))));
    }
    return 0.0;
}

QString calculation::calc_weekID(QDate workoutDate)
{
    QString weeknumber = QString::number(workoutDate.weekNumber());

    if(workoutDate.year() != workoutDate.addDays(7 - workoutDate.dayOfWeek()).year())
    {
        weeknumber = weeknumber+"_"+QString::number(workoutDate.addDays(7 - workoutDate.dayOfWeek()).year());
    }
    else
    {
        weeknumber = weeknumber+"_"+QString::number(workoutDate.year());
    }

    //qDebug() << workoutDate << weeknumber;

    return weeknumber;
}

double calculation::calc_swim_xpower(double distance,double pace,double time,double athleteWeight)
{
    double K = 2 + 0.35 * athleteWeight;
    double velo = distance / pace;
    double alpha = 2.0 /10;
    QVector<double> rawEWMA(static_cast<int>(time));
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
    double mlkgmin = (((current_dayCalories(QDateTime::currentDateTime())/1440)/5)/weight)*1000.0;

    if(style == 0)
    {
        return swimMET;
    }
    else
    {
        return swimMET * (3.5/mlkgmin);
    }
}

double calculation::estimate_stress(QString sport, QString p_goal, int duration,int usePM)
{
    double goal = 0;
    double est_stress = 0;
    double est_power = 0;
    double raw_effort = 0;
    double cv_effort = 0;
    double thresPower = 0;
    double athleteWeight = athleteValues->value("weight");
    double athleteHeight = athleteValues->value("height");

    if(sport == settings::isSwim)
    {
        goal = get_timesec(p_goal);
    }
    if(sport == settings::isBike)
    {
        if(usePM == 0)
        {
            goal = get_speed(QTime::fromString(p_goal,"mm:ss"),0,settings::isRun,true)/3.6;
        }
        else
        {
            goal = p_goal.toDouble();
        }
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
            thresPower = thresValues->value("swimpower");
            est_power = calc_swim_xpower(100,goal,duration,athleteWeight);
            raw_effort = (duration * est_power) * (est_power / thresPower);         
        }
        if(sport == settings::isBike)
        {
            thresPower = thresValues->value("bikepower");
            raw_effort = (duration * goal) * (goal / thresPower);
        }
        if(sport == settings::isRun)
        {
            thresPower = thresValues->value("runpower");
            est_power = calc_lnp(goal,athleteHeight,athleteWeight);
            raw_effort = est_power * duration * (est_power / thresPower);
        }
        if(sport == settings::isStrength)
        {
            thresPower = thresValues->value("stgpower");
            raw_effort = (duration * goal) * (goal / thresPower);
        }
        if(sport == settings::isAlt)
        {
            thresPower = thresValues->value("runpower");
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
