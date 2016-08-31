#include "standardworkouts.h"

standardWorkouts::standardWorkouts()
{
    meta_tags << "sport" << "id" << "code" << "title" << "duration" << "distance" << "stress";
    step_tags << "sport-id" << "id" << "part" << "level" << "threshold" << "int-time" << "int-dist" << "repeats" << "parent";

    this->read_standard_workouts();
}

void standardWorkouts::read_standard_workouts()
{
    workouts_meta = new QStandardItemModel(0,7);
    workouts_steps = new QStandardItemModel(0,9);
    QDomDocument meta_doc,step_doc;
    QDomElement root_meta,root_step;
    QDomNodeList meta_list,step_list;
    QDomElement work_ele,step_ele;

    QFile metaWork(settings().get_workoutsPath() + QDir::separator() + "standard_workouts_meta.xml");

    if(!metaWork.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File not open!";
    }
    else
    {
        if(!meta_doc.setContent(&metaWork))
        {
            qDebug() << "Workouts not loaded!";
        }
        metaWork.close();
    }

    root_meta = meta_doc.firstChildElement();
    meta_list = root_meta.elementsByTagName("workout");

    //META
    for(int row = 0; row < meta_list.count(); ++row)
    {
        work_ele = meta_list.at(row).toElement();
        workouts_meta->insertRows(row,1,QModelIndex());
        for(int col = 0; col < workouts_meta->columnCount(); ++col)
        {
            workouts_meta->setData(workouts_meta->index(row,col,QModelIndex()),work_ele.attribute(meta_tags.at(col)));
        }
    }

    QFile stepWork(settings().get_workoutsPath() + QDir::separator() + "standard_workouts_steps.xml");

    if(!stepWork.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "File not open!";
    }
    else
    {
        if(!step_doc.setContent(&stepWork))
        {
            qDebug() << "Workouts not loaded!";
        }
        stepWork.close();
    }

    root_step = step_doc.firstChildElement();
    step_list = root_step.elementsByTagName("step");


    //Steps
    for(int row = 0; row < step_list.count(); ++row)
    {
        step_ele = step_list.at(row).toElement();
        workouts_steps->insertRows(row,1,QModelIndex());
        for(int col = 0; col < workouts_steps->columnCount(); ++col)
        {
            workouts_steps->setData(workouts_steps->index(row,col,QModelIndex()),step_ele.attribute(step_tags.at(col)));
        }
    }

    this->set_workoutIds();
}

void standardWorkouts::write_standard_workouts()
{
    if (save_workouts)
    {
        QModelIndex index;
        QDomDocument meta_doc,step_doc;

        QDomElement xmlroot;
        xmlroot = meta_doc.createElement("workouts");
        meta_doc.appendChild(xmlroot);

        //Meta
        for(int i = 0; i < workouts_meta->rowCount(); ++i)
        {
            index = workouts_meta->index(i,2,QModelIndex());
            QDomElement std_workout = meta_doc.createElement("workout");

            for(int x = 0; x < workouts_meta->columnCount(); ++x)
            {
                index = workouts_meta->index(i,x,QModelIndex());
                std_workout.setAttribute(meta_tags.at(x),workouts_meta->data(index,Qt::DisplayRole).toString());
            }
            xmlroot.appendChild(std_workout);
        }

        QFile metaFile(settings().get_workoutsPath() + QDir::separator() + "standard_workouts_meta.xml");

        if(!metaFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "File not open!";
        }

        QTextStream metastream(&metaFile);
        metastream << meta_doc.toString();

        metaFile.close();

        //Steps
        xmlroot = step_doc.createElement("steps");
        step_doc.appendChild(xmlroot);

        for(int i = 0; i < workouts_steps->rowCount(); ++i)
        {
            index = workouts_steps->index(i,2,QModelIndex());
            QDomElement std_workout = step_doc.createElement("step");

            for(int x = 0; x < workouts_steps->columnCount(); ++x)
            {
                index = workouts_steps->index(i,x,QModelIndex());
                std_workout.setAttribute(step_tags.at(x),workouts_steps->data(index,Qt::DisplayRole).toString());
            }
            xmlroot.appendChild(std_workout);
        }

        QFile stepFile(settings().get_workoutsPath() + QDir::separator() + "standard_workouts_steps.xml");

        if(!stepFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            qDebug() << "File not open!";
        }

        QTextStream stepstream(&stepFile);
        stepstream << step_doc.toString();

        stepFile.close();
    }
}

void standardWorkouts::set_workoutIds()
{
    workoutIDs = QStringList();
    for(int i = 0; i < workouts_meta->rowCount(); ++i)
    {
        workoutIDs << workouts_meta->data(workouts_meta->index(i,1,QModelIndex())).toString();
    }
}

void standardWorkouts::delete_stdWorkout(QString workID,bool isdelete)
{
    QList<QStandardItem*> list = workouts_meta->findItems(workID,Qt::MatchExactly,1);
    workouts_meta->removeRow(workouts_meta->indexFromItem(list.at(0)).row(),QModelIndex());

    list = workouts_steps->findItems(workID,Qt::MatchExactly,0);
    for(int i = 0; i < list.count(); ++i)
    {
        workouts_steps->removeRow(workouts_steps->indexFromItem(list.at(i)).row(),QModelIndex());
    }

    if(isdelete) this->set_workoutIds();
    this->write_standard_workouts();
}

