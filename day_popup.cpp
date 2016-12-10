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

#include "day_popup.h"
#include "ui_day_popup.h"

day_popup::day_popup(QWidget *parent, const QDate w_date, schedule *p_sched) :
    QDialog(parent),
    ui(new Ui::day_popup)
{
    ui->setupUi(this);
    workout_date = &w_date;
    workSched = p_sched;
    weekPhase = workSched->get_weekPhase(w_date);
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->show_workouts();
}

day_popup::~day_popup()
{
    delete ui;
}

void day_popup::show_workouts()
{
    QModelIndex index;
    QList<QStandardItem*> list = workSched->workout_schedule->findItems(workout_date->toString("dd.MM.yyyy"),Qt::MatchExactly,1);
    int col_count = (list.count()*3)-1;
    if(list.count() == 0)
    {
        ui->pushButton_edit->setEnabled(false);
        return;
    }
    if(list.count() == 1) this->setFixedWidth(400);
    if(list.count() > 1 ) this->setFixedWidth(300*list.count());
    index = workSched->workout_schedule->indexFromItem(list.at(0));
    ui->label_weekinfo->setText(workout_date->toString("dd.MM.yyyy") + " - Phase: " + weekPhase + " - Workouts: " + QString::number(list.count()) );

    QStringList work_list;
    work_list << "Workout:" << "Sport:" << "Code:" << "Title:" << "Duration:" << "Distance:" << "Stress:" << "Pace:";
    int fontsize = 10;
    int clabel,cvalue;

    QTextCursor cursor = ui->textBrowser_work->textCursor();
    cursor.beginEditBlock();

    QTextTableFormat tableFormat;
    tableFormat.setAlignment(Qt::AlignAbsolute);
    tableFormat.setBackground(Qt::white);
    tableFormat.setCellPadding(1);
    tableFormat.setCellSpacing(1);
    tableFormat.setBorder(0);
    QVector<QTextLength> constraints;
        constraints << QTextLength(QTextLength::FixedLength,70)
                    << QTextLength(QTextLength::FixedLength,200)
                    << QTextLength(QTextLength::FixedLength,20)
                    << QTextLength(QTextLength::FixedLength,70)
                    << QTextLength(QTextLength::FixedLength,200)
                    << QTextLength(QTextLength::FixedLength,20)
                    << QTextLength(QTextLength::FixedLength,70)
                    << QTextLength(QTextLength::FixedLength,200);

    tableFormat.setColumnWidthConstraints(constraints);

    QTextTable *table = cursor.insertTable(work_list.count(),col_count,tableFormat);
    QTextCharFormat format = cursor.charFormat();
    format.setFontPointSize(fontsize);

    QTextCharFormat boldFormat = format;
    boldFormat.setFontWeight(QFont::Bold);

    QTextTableCell cell;
    QTextCursor cellcursor;

    for(int i = 0; i < list.count();++i)
    {
        index = workSched->workout_schedule->indexFromItem(list.at(i));

        for(int x = 0; x < work_list.count(); ++x)
        {
            clabel = i*2;
            cvalue = clabel+1;
            cell = table->cellAt(x,clabel+i);
            cellcursor = cell.firstCursorPosition();
            cellcursor.insertText(work_list.at(x));

            cell = table->cellAt(x,cvalue+i);
            cellcursor = cell.firstCursorPosition();
            if(x == 7)
            {
                cellcursor.insertText(settings::get_workout_pace(workSched->workout_schedule->item(index.row(),7)->text().toDouble(),QTime::fromString(workSched->workout_schedule->item(index.row(),6)->text(),"hh:mm:ss"),workSched->workout_schedule->item(index.row(),3)->text(),true));
            }
            else
            {
                cellcursor.insertText(workSched->workout_schedule->item(index.row(),x+2)->text());
            }
        }
    }
    table->insertRows(table->rows(),1);
    cursor.endEditBlock();

}

void day_popup::on_pushButton_edit_clicked()
{
     accept();
}

void day_popup::on_pushButton_close_clicked()
{
     reject();
}
