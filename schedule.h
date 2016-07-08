#ifndef SCHEDULE_H
#define SCHEDULE_H

#include <QStandardItemModel>
#include <QtXml>
#include <QMessageBox>
#include "settings.h"
#include "workout.h"

class schedule
{

public:
    schedule(settings *p_settings = 0);
    workout *copyworkout;
    QStandardItemModel *workout_schedule,*week_meta,*week_content;
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

private:
    settings *sched_settings;
    QStringList workoutTags,metaTags,contentTags;
    QString copyFrom, copyTo;
    QDate firstdayofweek;

};

#endif // SCHEDULE_H
