#include "calculation.h"

calculation::calculation()
{
    thresValues = settings::getdoubleMapPointer(settings::dMap::Threshold);
    athleteValues = settings::getdoubleMapPointer(settings::dMap::Athlete);
    generalValues = settings::getStringMapPointer(settings::stingMap::General);
}

void calculation::set_currentSport(QString sport)
{
    currentSport = sport;
    isSwim = isBike = isRun = isStrength = isAlt = isOther = isTria = false;

    if(currentSport == settings::SwimLabel) isSwim = true;
    if(currentSport == settings::BikeLabel) isBike = true;
    if(currentSport == settings::RunLabel) isRun = true;
    if(currentSport == settings::StrengthLabel) isStrength = true;
    if(currentSport == settings::AltLabel) isAlt = true;
    if(currentSport == settings::OtherLabel) isOther = true;
    if(currentSport == settings::TriaLabel) isTria = true;
}

QString calculation::set_time(int sec) const
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

int calculation::get_timesec(QString time) const
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

QString calculation::get_workout_pace(double dist, double duration,QString sport,bool full_label) const
{
    QStringList speedLabel;
    speedLabel << " min/km - " << " km/h" << " min/km" << "no Speed";
    int nr=0;
    double speed = 0;

    int sec = static_cast<int>(duration);

    int min = static_cast<int>(duration/60.0);

    if(dist != 0.0 || min != 0)
    {
        if(isSwim)
        {
            speed = sec/dist;
            nr=0;
        }
        if(isBike)
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
        if(isRun)
        {
            speed = sec/dist;
            nr=2;
        }
        if(isAlt || isStrength) static_cast<void>(speed = 0.0), nr=3;

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

double calculation::get_speed(QTime pace,double dist,bool fixdist) const
{
    int sec = pace.minute()*60;
    sec = sec + pace.second();
    double speed;

    if(sec > 0)
    {
        if(fixdist)
        {
            if(isSwim)
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

double calculation::calc_Speed(double sec, double dist, double factor) const
{
    return 3600.0 / (sec / (dist/factor));
}

QString calculation::calc_lapSpeed(double sec) const
{
    if(isSwim)
    {
        return QString::number(set_doubleValue(360.0/sec,false))+" km/h";
    }
    else if(isBike || isRun)
    {
        return QString::number(set_doubleValue(3600.0/sec,false))+" km/h";
    }
    else
    {
        return "--";
    }
}

int calculation::calc_lapPace(int duration, double distance) const
{
    int pace;
    if(distance > 0)
    {
        if (isSwim)
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

int calculation::get_hfvalue(QString percent) const
{
    double value = percent.toDouble();

    return static_cast<int>(round(thresValues->value("hfthres") * (value / 100.0)));
}

int calculation::get_secFromTime(QTime pTime) const
{
    int sec = pTime.minute()*60;
    sec = sec + pTime.second();

    return  sec;
}

int calculation::get_baseValue(double percent)
{
    if(usePMData)
    {
        return this->calc_thresPower(percent);
    }
    else
    {
        return this->calc_thresPace(percent);
    }
}

double calculation::calc_totalCal(double weight,double avgHF, double moveTime) const
{
    int age = static_cast<int>(QDate::currentDate().year() - athleteValues->value("yob"));

    return ceil(((-55.0969 + (0.6309 * avgHF) + (0.1988 * weight) + (0.2017 * age))/4.184) * moveTime/60);
}

double calculation::calc_totalWork(QString sport, double pValue, double dura,int tempID) const
{
    double factor = 1000.0;
    double grav = 9.81;
    double weight = settings::get_weightforDate(QDateTime::currentDateTime());
    double height = athleteValues->value("height");
    double mSec = pValue*factor/3600.0;
    double totalWork = 0;
    sport = "";
    if(isSwim)
    {
        double correctMet = 1;
        double sriFactor = 0;
        double sri = 0;

        if(tempID != 0)
        {
            sri = pow(thresPace/pValue,3.0);
            sriFactor = sqrt(sri) / 10.0;
            if(sri < 1.0) sriFactor = sriFactor * -1.0;
        }
        correctMet = get_corrected_MET(weight,tempID)+sriFactor;

        totalWork = (correctMet * 3.5 * weight / 200.0) * (dura/60.0);
    }
    else if(isBike)
    {
        totalWork = (dura * pValue) / factor;
    }
    else if(isRun)
    {
        if(usePMData)
        {
            totalWork = (dura * pValue) / factor;
        }
        else
        {
            double bodyHub = (height * 0.0543) + (((3600/thresPace) / pValue) / 100.0);
            totalWork = (weight * grav * mSec * bodyHub) * dura / factor;
        }

    }
    else if(isStrength || isAlt)
    {
        totalWork = (weight * grav * mSec * 0.145) * dura / factor;
    }

    return set_doubleValue(totalWork,false);
}

double calculation::wattToSpeed(double thresPower,double currWatt) const
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

int calculation::calc_thresPace(double percent) const
{
    if(percent > 0)
    {
        return static_cast<int>(round(thresPace / (percent/100.0)));
    }
    else
    {
        return 0;
    }
}

double calculation::calc_thresSpeed(double percent) const
{
    if(percent > 0)
    {
        return set_doubleValue(thresSpeed / (percent/100.0),false);
    }
    else
    {
        return 0;
    }
}

int calculation::calc_thresPower(double percent) const
{
    if(percent > 0)
    {
        return static_cast<int>(round(thresPower *(percent/100.0)));
    }
    else
    {
        return 0;
    }
}

double calculation::calc_distance(int duration, double pace) const
{
    if(pace > 0)
    {
        return set_doubleValue(duration / pace,true);
    }
    return 0;
}

QTime calculation::calc_duration(double dist, int pace) const
{
    QTime duration;
    int sec = 0;

    if(isSwim)
    {
        sec = static_cast<int>(pace * (dist*10));
    }
    else
    {
        sec = static_cast<int>(pace * dist);
    }

    duration.setHMS(0,sec/60,sec%60);
    return duration;
}


double calculation::calc_lnp(double speed,double athleteHeight,double athleteWeight) const
{
    double athleteF = (0.2025*pow(athleteHeight,0.725)*pow(athleteWeight,0.425))*0.266;
    double cAero = 0.5*1.2*0.9*athleteF*pow(speed,2)/athleteWeight;
    double athleteEff = (0.25+0.054*speed)*(1 - 0.5*speed/8.33);

    return (cAero+3.6*athleteEff)*speed*athleteWeight;
}

double calculation::current_dayCalories(QDateTime calcDate) const
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

double calculation::calc_swim_xpower(double distance,double pace,double time,double athleteWeight) const
{
    double K = 2 + 0.35 * athleteWeight;
    double velo = distance / calc_thresPace(pace);
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

double calculation::get_corrected_MET(double weight, int style) const
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

double calculation::calc_stressScore(double baseValue, int duration) const
{
    double athleteWeight = athleteValues->value("weight");
    double athleteHeight = athleteValues->value("height");
    double est_stress = 0;
    double est_power = 0;
    double raw_effort = 0;
    double cv_effort = 0;
    double baseCalc = 0;


    if(isSwim)
    {
        if(baseValue > 0)
        {
            est_power = calc_swim_xpower(100,baseValue,duration,athleteWeight);
            raw_effort = (duration * est_power) * (est_power / thresPower);
        }
        else
        {
            return 0;
        }
    }
    else if(isBike)
    {
        if(usePMData)
        {
            baseCalc = calc_thresPower(baseValue);
            raw_effort = (duration * baseCalc) * (baseCalc / thresPower);
        }
        else
        {
            baseCalc = calc_thresPace(baseValue);
        }
    }
    else if(isRun)
    {
        if(usePMData)
        {
            baseCalc = calc_thresPower(baseValue);
            est_stress = 0.03 * duration * pow((baseCalc/thresPower),3.5);
            return set_doubleValue(est_stress,false);
        }
        else
        {
            baseCalc = calc_thresPace(baseValue);
            est_power = calc_lnp(baseCalc,athleteHeight,athleteWeight);
            raw_effort = est_power * duration * (est_power / thresPower);
        }
    }
    else if(isStrength || isAlt)
    {
        baseCalc = calc_thresPower(baseValue);
        raw_effort = (duration * baseCalc) * (baseCalc / thresPower);
    }

    cv_effort = thresPower * 3600;
    est_stress = (raw_effort / cv_effort) * 100;

    return set_doubleValue(est_stress,false);

}


double calculation::estimate_stress(QString sport, int p_goal, int duration,bool usePM) const
{
    sport = "";
    double goal = 0;
    double est_stress = 0;
    double est_power = 0;
    double raw_effort = 0;
    double cv_effort = 0;
    double thresPower = 0;
    double athleteWeight = athleteValues->value("weight");
    double athleteHeight = athleteValues->value("height");

    if(isSwim)
    {
        goal = p_goal;
    }
    if(isBike)
    {
        if(usePM)
        {
            goal = p_goal;
        }
        else
        {
            goal = get_speed(QTime::fromString(set_time(p_goal),"mm:ss"),0,true)/3.6;
        }
    }
    if(isRun)
    {
        if(usePM)
        {
            goal = p_goal;
        }
        else
        {
            goal = get_speed(QTime::fromString(set_time(p_goal),"mm:ss"),0,true)/3.6;
        }
    }
    if(isStrength)
    {
        goal = p_goal;
    }
    if(isAlt)
    {
        goal = p_goal;
    }

    if(goal > 0)
    {
        if(isSwim)
        {
            thresPower = thresValues->value("swimpower");
            est_power = calc_swim_xpower(100,goal,duration,athleteWeight);
            raw_effort = (duration * est_power) * (est_power / thresPower);         
        }
        if(isBike)
        {
            thresPower = thresValues->value("bikepower");
            raw_effort = (duration * goal) * (goal / thresPower);
        }
        if(isRun)
        {
            thresPower = thresValues->value("runpower");
            est_power = calc_lnp(goal,athleteHeight,athleteWeight);
            raw_effort = est_power * duration * (est_power / thresPower);
        }
        if(isStrength)
        {
            thresPower = thresValues->value("stgpower");
            raw_effort = (duration * goal) * (goal / thresPower);
        }
        if(isAlt)
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

double calculation::set_doubleValue(double value, bool setthree) const
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

int calculation::get_thresPercent(QString level, bool max) const
{
    QString range = settings::get_rangeValue(currentSport,level);

    if(max)
    {
        return range.split("-").last().toInt();
    }
    else
    {
        return range.split("-").first().toInt();
    }
}
