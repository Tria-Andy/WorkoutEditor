#include "schedule.h"

schedule::schedule(settings *p_settings)
{
    sched_settings = p_settings;
    workoutTags << "week" << "date" << "time" << "sport" << "code" << "title" << "duration" << "distance" << "stress";
    metaTags << "id" << "week" << "name" << "fdw";
    contentTags << "id" << "week" << "swim" << "bike" << "run" << "strength" << "alternativ" << "summery";
    firstdayofweek = QDate::currentDate().addDays(1 - QDate::currentDate().dayOfWeek());
    load_workouts_file();
}

void schedule::load_workouts_file()
{
    QFile workouts(sched_settings->get_schedulePath() + QDir::separator() + "workout_schedule.xml");
    QDomDocument doc_workouts;

    if(!workouts.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File not open!";
    }
    else
    {
        if(!doc_workouts.setContent(&workouts))
        {
            qDebug() << "Workouts not loaded!";
        }
        workouts.close();
    }

    QFile weekMeta(sched_settings->get_schedulePath() + QDir::separator() + "workout_phase_meta.xml");
    QDomDocument doc_week_meta;
    if(!weekMeta.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File not open!";
    }
    else
    {
        if(!doc_week_meta.setContent(&weekMeta))
        {
            qDebug() << "Workouts not loaded!";
        }
        weekMeta.close();
    }

    QFile weekContent(sched_settings->get_schedulePath() + QDir::separator() + "workout_phase_content.xml");
    QDomDocument doc_week_content;
    if(!weekContent.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File not open!";
    }
    else
    {
        if(!doc_week_content.setContent(&weekContent))
        {
            qDebug() << "Workouts not loaded!";
        }
        weekContent.close();
    }

    this->read_week_values(doc_week_meta,doc_week_content);
    this->read_workout_values(doc_workouts);
}

void schedule::save_workout_file()
{
        QModelIndex index;
        QDomDocument document;

        QDomElement xmlroot = document.createElement("workouts");
        document.appendChild(xmlroot);

        for(int i = 0; i < workout_schedule->rowCount(); ++i)
        {
            index = workout_schedule->index(i,1,QModelIndex());
            if(QDate::fromString(workout_schedule->data(index,Qt::DisplayRole).toString(),"dd.MM.yyyy") < firstdayofweek ) continue;

            QDomElement xml_workout = document.createElement("workout");

            for(int x = 0; x < workout_schedule->columnCount(); ++x)
            {
                index = workout_schedule->index(i,x,QModelIndex());
                xml_workout.setAttribute(workoutTags.at(x),workout_schedule->data(index,Qt::DisplayRole).toString());
            }
            xmlroot.appendChild(xml_workout);
        }

        QFile file(sched_settings->get_schedulePath() + QDir::separator() + "workout_schedule.xml");

        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "File not open!";
        }

        QTextStream stream(&file);
        stream << document.toString();

        file.close();
}

void schedule::read_week_values(QDomDocument weekMeta, QDomDocument weekContent)
{
    QDomElement root_meta = weekMeta.firstChildElement();
    QDomElement root_content = weekContent.firstChildElement();
    QDomNodeList meta_list,content_list;
    meta_list = root_meta.elementsByTagName("phase");
    content_list = root_content.elementsByTagName("content");

    week_meta = new QStandardItemModel(meta_list.count(),4);
    week_content = new QStandardItemModel(content_list.count(),8);

    for(int i = 0; i < meta_list.count(); ++i)
    {
        QDomElement meta_element;
        QDomNode meta_node = meta_list.at(i);
        meta_element = meta_node.toElement();
        for(int col = 0; col < week_meta->columnCount(); ++col)
        {
            if(col == 0)
            {
                week_meta->setData(week_meta->index(i,col,QModelIndex()),meta_element.attribute(metaTags.at(col)).toInt());
            }
            else
            {
                week_meta->setData(week_meta->index(i,col,QModelIndex()),meta_element.attribute(metaTags.at(col)));
            }
        }

    }
    week_meta->sort(0);

    for(int i = 0; i < content_list.count(); ++i)
    {
        QDomElement content_element;
        QDomNode content_node = content_list.at(i);
        content_element = content_node.toElement();
        for(int col = 0; col < week_content->columnCount(); ++col)
        {
            if(col == 0)
            {
                week_content->setData(week_content->index(i,col,QModelIndex()),content_element.attribute(contentTags.at(col)).toInt());
            }
            else
            {
                week_content->setData(week_content->index(i,col,QModelIndex()),content_element.attribute(contentTags.at(col)));
            }
        }
    }

}

void schedule::save_week_files()
{
        QDomDocument document;
        QDomElement xmlroot;
        xmlroot = document.createElement("phases");
        document.appendChild(xmlroot);

        for(int i = 0; i < week_meta->rowCount(); ++i)
        {
            QDomElement xml_phase = document.createElement("phase");
            for(int col = 0; col < week_meta->columnCount(); ++col)
            {
                xml_phase.setAttribute(metaTags.at(col) ,week_meta->data(week_meta->index(i,col,QModelIndex())).toString());
            }
            xmlroot.appendChild(xml_phase);
        }

        QFile meta_file(sched_settings->get_schedulePath() + QDir::separator() + "workout_phase_meta.xml");

        if(!meta_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "File not open!";
        }

        QTextStream meta(&meta_file);
        meta << document.toString();

        meta_file.close();

        document.clear();
        xmlroot = document.createElement("contents");
        document.appendChild(xmlroot);

        for(int i = 0; i < week_content->rowCount(); ++i)
        {
            QDomElement xml_phase = document.createElement("content");
            for(int col = 0; col < week_content->columnCount(); ++col)
            {
                xml_phase.setAttribute(contentTags.at(col) ,week_content->data(week_content->index(i,col,QModelIndex())).toString());
            }
            xmlroot.appendChild(xml_phase);
        }

        QFile content_file(sched_settings->get_schedulePath() + QDir::separator() + "workout_phase_content.xml");

        if(!content_file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "File not open!";
        }

        QTextStream content(&content_file);
        content << document.toString();

        content_file.close();
}

void schedule::read_workout_values(QDomDocument workouts)
{
    QDomElement root_workouts = workouts.firstChildElement();
    QDomNodeList workout_list;

    if(workouts.hasChildNodes())
    {
        workout_list = root_workouts.elementsByTagName("workout");
    }

    workout_schedule = new QStandardItemModel(workout_list.count(),9);

    for(int i = 0; i < workout_list.count(); ++i)
    {
        QDomElement workout_element;

        QDomNode workout_node = workout_list.at(i);

        workout_element = workout_node.toElement();
        for(int col = 0; col < workout_schedule->columnCount(); ++col)
        {
            workout_schedule->setData(workout_schedule->index(i,col,QModelIndex()),workout_element.attribute(workoutTags.at(col)));
        }
    }
    workout_schedule->sort(2);
}

void schedule::copyWeek()
{
    QModelIndex index;
    QList<QStandardItem*> fromList,toList;
    QString fromWeek,toWeek,fromYear,toYear,workdate;
    int fromWeek_int,fromYear_int,toWeek_int,toYear_int,addfactor = 0;
    int days = 7;
    fromWeek = copyFrom.split("_").first();
    fromYear = copyFrom.split("_").last();
    toWeek = copyTo.split("_").first();
    toYear = copyTo.split("_").last();

    QDate lastDay(fromYear.toInt(),12,31);
    QDate workoutDate;
    fromWeek_int = fromWeek.toInt();
    fromYear_int = fromYear.toInt();
    toWeek_int = toWeek.toInt();
    toYear_int = toYear.toInt();

    fromList = workout_schedule->findItems(copyFrom,Qt::MatchExactly,0);
    toList = workout_schedule->findItems(copyTo,Qt::MatchExactly,0);

    if(toYear_int == fromYear_int)
    {
        addfactor = toWeek_int - fromWeek_int;
    }
    if(toYear_int > fromYear_int)
    {
       addfactor = (lastDay.weekNumber() - fromWeek_int) + toWeek_int;
    }

    if(!toList.isEmpty())
    {
        for(int i = 0; i < toList.count(); ++i)
        {
            this->delete_workout(workout_schedule->indexFromItem(toList.at(i)));
        }
    }

    for(int i = 0; i < fromList.count(); ++i)
    {
        index = workout_schedule->indexFromItem(fromList.at(i));
        workdate = workout_schedule->item(index.row(),1)->text();

        workout_calweek = copyTo;
        workout_date = workoutDate.fromString(workdate,"dd.MM.yyyy").addDays(days*addfactor).toString("dd.MM.yyyy");
        workout_time = workout_schedule->item(index.row(),2)->text();
        workout_sport = workout_schedule->item(index.row(),3)->text();
        workout_title = workout_schedule->item(index.row(),5)->text();
        workout_duration = workout_schedule->item(index.row(),6)->text();
        workout_distance = workout_schedule->item(index.row(),7)->text().toDouble();
        workout_stress_score = workout_schedule->item(index.row(),8)->text().toInt();

        this->add_workout();
    }
}


QString schedule::get_weekPhase(QDate currDate)
{
    QString weekID = QString::number(currDate.weekNumber()) +"_"+ QString::number(currDate.year());
    QList<QStandardItem*> metaPhase = week_meta->findItems(weekID,Qt::MatchExactly,1);
    QModelIndex index;
    if(!metaPhase.isEmpty())
    {
        index = week_meta->indexFromItem(metaPhase.at(0));
        return week_meta->item(index.row(),2)->text();
    }
    return 0;
}

void schedule::changeYear()
{
    QDate startDate = QDate::fromString(sched_settings->get_saisonFDW(),"dd.MM.yyyy");
    QString weekid;
    int id;

    week_meta->sort(0);
    week_content->sort(0);

    for(int week = 0; week < sched_settings->get_saisonWeeks(); ++week)
    {
        id = week_meta->data(week_meta->index(week,0,QModelIndex())).toInt();
        if(id == week+1)
        {
            weekid = QString::number(startDate.addDays(week*7).weekNumber()) +"_"+ QString::number(startDate.addDays(week*7).year());
            qDebug() << id << weekid << startDate.addDays(week*7).toString("dd.MM.yyyy");
            /*
            week_meta->setData(week_meta->index(week,1,QModelIndex()),weekid);
            week_meta->setData(week_meta->index(week,3,QModelIndex()),startDate.addDays(week*7).toString("dd.MM.yyyy"));
            week_content->setData(week_content->index(week,1,QModelIndex()),weekid);
            */
        }
    }
    this->save_week_files();
}

void schedule::add_workout()
{
    int row = workout_schedule->rowCount();

    workout_schedule->insertRows(row,1,QModelIndex());
    workout_schedule->setData(workout_schedule->index(row,0,QModelIndex()),workout_calweek);
    workout_schedule->setData(workout_schedule->index(row,1,QModelIndex()),workout_date);
    workout_schedule->setData(workout_schedule->index(row,2,QModelIndex()),workout_time);
    workout_schedule->setData(workout_schedule->index(row,3,QModelIndex()),workout_sport);
    workout_schedule->setData(workout_schedule->index(row,4,QModelIndex()),workout_code);
    workout_schedule->setData(workout_schedule->index(row,5,QModelIndex()),workout_title);
    workout_schedule->setData(workout_schedule->index(row,6,QModelIndex()),workout_duration);
    workout_schedule->setData(workout_schedule->index(row,7,QModelIndex()),QString::number(workout_distance));
    workout_schedule->setData(workout_schedule->index(row,8,QModelIndex()),workout_stress_score);
}

void schedule::edit_workout(QModelIndex index)
{
    workout_schedule->setData(workout_schedule->index(index.row(),0,QModelIndex()),workout_calweek);
    workout_schedule->setData(workout_schedule->index(index.row(),1,QModelIndex()),workout_date);
    workout_schedule->setData(workout_schedule->index(index.row(),2,QModelIndex()),workout_time);
    workout_schedule->setData(workout_schedule->index(index.row(),3,QModelIndex()),workout_sport);
    workout_schedule->setData(workout_schedule->index(index.row(),4,QModelIndex()),workout_code);
    workout_schedule->setData(workout_schedule->index(index.row(),5,QModelIndex()),workout_title);
    workout_schedule->setData(workout_schedule->index(index.row(),6,QModelIndex()),workout_duration);
    workout_schedule->setData(workout_schedule->index(index.row(),7,QModelIndex()),QString::number(workout_distance));
    workout_schedule->setData(workout_schedule->index(index.row(),8,QModelIndex()),workout_stress_score);
}

void schedule::delete_workout(QModelIndex index)
{
    workout_schedule->removeRow(index.row(),QModelIndex());
}

QLineSeries* schedule::get_qLineSeries(bool p_Visible)
{
    QPen linePen;
    linePen.setColor(QColor(255,0,0,120));
    linePen.setWidth(2);

    QLineSeries *line = new QLineSeries();
    line->setPointLabelsVisible(p_Visible);
    line->setPointLabelsClipping(false);
    line->setPointLabelsFormat("@yPoint");
    line->setPointLabelsColor(QColor(Qt::darkRed));
    line->setPointsVisible(p_Visible);
    line->setPen(linePen);

    return line;
}

QValueAxis* schedule::get_qValueAxis(QString title,bool p_visible,double max,int tickcount)
{
    QValueAxis *axis = new QValueAxis;
    axis->setTitleText(title);
    axis->setTitleVisible(p_visible);
    axis->setRange(0,max);
    axis->setTickCount(tickcount);

    return axis;
}
