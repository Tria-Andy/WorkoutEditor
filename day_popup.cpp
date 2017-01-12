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
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->show_workouts(w_date,p_sched);
}

day_popup::~day_popup()
{
    delete ui;
}

void day_popup::show_workouts(QDate w_date,schedule *schedP)
{
    QString workoutDate = w_date.toString("dd.MM.yyyy");
    QSortFilterProxyModel *scheduleProxy = new QSortFilterProxyModel();
    scheduleProxy->setSourceModel(schedP->workout_schedule);
    scheduleProxy->setFilterRegExp("\\b"+workoutDate+"\\b");
    scheduleProxy->setFilterKeyColumn(1);
    int workCount = scheduleProxy->rowCount();
    int col_count = (workCount*3)-1;

    if(workCount == 0)
    {
        ui->toolButton_edit->setEnabled(false);
        return;
    }
    if(workCount == 1) this->setFixedWidth(400);
    if(workCount > 1 ) this->setFixedWidth(300*workCount);

    ui->label_weekinfo->setText(workoutDate + " - Phase: " + schedP->get_weekPhase(w_date) + " - Workouts: " + QString::number(workCount) );

    QStringList work_list;
    work_list << "Workout:" << "Sport:" << "Code:" << "Title:" << "Duration:" << "Distance:" << "Stress:" << "Pace:";
    int worklistCount = work_list.count();
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

    QTextTable *table = cursor.insertTable(worklistCount,col_count,tableFormat);
    QTextCharFormat format = cursor.charFormat();
    format.setFontPointSize(fontsize);

    QTextCharFormat boldFormat = format;
    boldFormat.setFontWeight(QFont::Bold);

    QTextTableCell cell;
    QTextCursor cellcursor;

    for(int i = 0; i < workCount;++i)
    {
        for(int x = 0; x < worklistCount; ++x)
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
                cellcursor.insertText(this->get_workout_pace(scheduleProxy->data(scheduleProxy->index(i,7)).toDouble(),
                                                             QTime::fromString(scheduleProxy->data(scheduleProxy->index(i,6)).toString(),"hh:mm:ss"),
                                                             scheduleProxy->data(scheduleProxy->index(i,3)).toString(),true));
            }
            else
            {
                cellcursor.insertText(scheduleProxy->data(scheduleProxy->index(i,x+2)).toString());
            }
        }
    }
    table->insertRows(table->rows(),1);
    cursor.endEditBlock();
}

void day_popup::on_toolButton_close_clicked()
{
    reject();
}

void day_popup::on_toolButton_edit_clicked()
{
    accept();
}
