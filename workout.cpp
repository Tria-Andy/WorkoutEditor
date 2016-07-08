#include <QApplication>
#include <QDebug>
#include "workout.h"

workout::workout()
{

}

void workout::add_workout(QStandardItemModel *workout_model)
{
    int row = workout_model->rowCount();

    workout_model->insertRows(row,1,QModelIndex());
    workout_model->setData(workout_model->index(row,0,QModelIndex()),get_workout_calweek());
    workout_model->setData(workout_model->index(row,1,QModelIndex()),get_workout_date());
    workout_model->setData(workout_model->index(row,2,QModelIndex()),get_workout_time());
    workout_model->setData(workout_model->index(row,3,QModelIndex()),get_workout_sport());
    workout_model->setData(workout_model->index(row,4,QModelIndex()),get_workout_code());
    workout_model->setData(workout_model->index(row,5,QModelIndex()),get_workout_title());
    workout_model->setData(workout_model->index(row,6,QModelIndex()),get_workout_duration());
    workout_model->setData(workout_model->index(row,7,QModelIndex()),QString::number(get_workout_distance()));
    workout_model->setData(workout_model->index(row,8,QModelIndex()),get_workout_stress());
}

void workout::edit_workout(QModelIndex index, QStandardItemModel *workout_model)
{
    workout_model->setData(workout_model->index(index.row(),0,QModelIndex()),get_workout_calweek());
    workout_model->setData(workout_model->index(index.row(),1,QModelIndex()),get_workout_date());
    workout_model->setData(workout_model->index(index.row(),2,QModelIndex()),get_workout_time());
    workout_model->setData(workout_model->index(index.row(),3,QModelIndex()),get_workout_sport());
    workout_model->setData(workout_model->index(index.row(),4,QModelIndex()),get_workout_code());
    workout_model->setData(workout_model->index(index.row(),5,QModelIndex()),get_workout_title());
    workout_model->setData(workout_model->index(index.row(),6,QModelIndex()),get_workout_duration());
    workout_model->setData(workout_model->index(index.row(),7,QModelIndex()),QString::number(get_workout_distance()));
    workout_model->setData(workout_model->index(index.row(),8,QModelIndex()),get_workout_stress());
}

void workout::delete_workout(QModelIndex index, QStandardItemModel *workout_model)
{
    workout_model->removeRow(index.row(),QModelIndex());
}
