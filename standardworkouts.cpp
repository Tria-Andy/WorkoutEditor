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

#include "standardworkouts.h"

standardWorkouts::standardWorkouts()
{
    meta_tags << "sport" << "id" << "code" << "title" << "duration" << "distance" << "stress" << "timebase";
    step_tags << "sport-id" << "id" << "part" << "level" << "threshold" << "int-time" << "int-dist" << "repeats" << "parent";
    workoutPath = settings::get_gcInfo("workouts");

    metaFile = "standard_workouts_meta.xml";
    stepFile = "standard_workouts_steps.xml";

    if(!workoutPath.isEmpty())
    {
        this->check_File(workoutPath,metaFile);
        this->check_File(workoutPath,stepFile);
        this->read_standard_workouts(this->load_XMLFile(workoutPath,metaFile),this->load_XMLFile(workoutPath,stepFile));
    }

}

void standardWorkouts::read_standard_workouts(QDomDocument meta_doc,QDomDocument step_doc)
{
    workouts_meta = new QStandardItemModel(0,8);
    workouts_steps = new QStandardItemModel(0,9);
    QDomElement rootTag;
    QDomNodeList xmlList;
    QDomElement xmlElement;

    rootTag = meta_doc.firstChildElement();
    xmlList = rootTag.elementsByTagName("workout");

    //META
    for(int row = 0; row < xmlList.count(); ++row)
    {
        xmlElement = xmlList.at(row).toElement();
        workouts_meta->insertRows(row,1,QModelIndex());
        for(int col = 0; col < workouts_meta->columnCount(); ++col)
        {
            workouts_meta->setData(workouts_meta->index(row,col,QModelIndex()),xmlElement.attribute(meta_tags.at(col)));
        }
    }

    rootTag = step_doc.firstChildElement();
    xmlList = rootTag.elementsByTagName("step");

    //Steps
    for(int row = 0; row < xmlList.count(); ++row)
    {
        xmlElement = xmlList.at(row).toElement();
        workouts_steps->insertRows(row,1,QModelIndex());
        for(int col = 0; col < workouts_steps->columnCount(); ++col)
        {
            workouts_steps->setData(workouts_steps->index(row,col,QModelIndex()),xmlElement.attribute(step_tags.at(col)));
        }
    }

    this->set_workoutIds();
}

void standardWorkouts::write_standard_workouts()
{
    if(save_workouts)
    {
        QModelIndex index;
        QDomDocument xmlDoc;

        QDomElement xmlRoot,xmlElement;
        xmlRoot = xmlDoc.createElement("workouts");
        xmlDoc.appendChild(xmlRoot);

        //Meta
        for(int i = 0; i < workouts_meta->rowCount(); ++i)
        {
            index = workouts_meta->index(i,2,QModelIndex());
            xmlElement = xmlDoc.createElement("workout");

            for(int x = 0; x < workouts_meta->columnCount(); ++x)
            {
                index = workouts_meta->index(i,x,QModelIndex());
                xmlElement.setAttribute(meta_tags.at(x),workouts_meta->data(index,Qt::DisplayRole).toString());
            }
            xmlRoot.appendChild(xmlElement);
        }
        this->write_XMLFile(workoutPath,&xmlDoc,metaFile);
        xmlDoc.clear();

        //Steps
        xmlRoot = xmlDoc.createElement("steps");
        xmlDoc.appendChild(xmlRoot);

        for(int i = 0; i < workouts_steps->rowCount(); ++i)
        {
            index = workouts_steps->index(i,2,QModelIndex());
            xmlElement = xmlDoc.createElement("step");

            for(int x = 0; x < workouts_steps->columnCount(); ++x)
            {
                index = workouts_steps->index(i,x,QModelIndex());
                xmlElement.setAttribute(step_tags.at(x),workouts_steps->data(index,Qt::DisplayRole).toString());
            }
            xmlRoot.appendChild(xmlElement);
        }
        this->write_XMLFile(workoutPath,&xmlDoc,stepFile);
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
    QSortFilterProxyModel *metaProxy =  new QSortFilterProxyModel();
    metaProxy->setSourceModel(workouts_meta);
    metaProxy->setFilterRegExp("\\b"+workID+"\\b");
    metaProxy->setFilterKeyColumn(1);
    metaProxy->removeRow(0,QModelIndex());

    QSortFilterProxyModel *stepProxy = new QSortFilterProxyModel();
    stepProxy->setSourceModel(workouts_steps);
    stepProxy->setFilterRegExp("\\b"+workID+"\\b");
    stepProxy->removeRows(0,stepProxy->rowCount(),QModelIndex());

    if(isdelete) this->set_workoutIds();
    this->write_standard_workouts();
}
