#ifndef DIALOG_EXPORT_H
#define DIALOG_EXPORT_H

#include <QDialog>
#include <QStandardItemModel>
#include <QFile>
#include "settings.h"

namespace Ui {
class Dialog_export;
}

class Dialog_export : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_export(QWidget *parent = 0,QStandardItemModel *w_model = 0, settings *p_settings = 0);
    ~Dialog_export();

private slots:
    void on_pushButton_close_clicked();

    void on_pushButton_export_clicked();

    void on_radioButton_day_clicked();

    void on_radioButton_week_clicked();

    void on_comboBox_week_export_currentIndexChanged(const QString &arg1);

    void on_dateEdit_export_dateChanged(const QDate &date);

    void on_comboBox_time_export_currentIndexChanged(const QString &time);

private:
    Ui::Dialog_export *ui;
    settings *export_settings;
    QDate *current_date;
    QStandardItemModel *workout_model;
    QList<QStandardItem*> workout_time;
    QStringList weeks;
    QString fileName, fileContent;

    int export_mode;

    void workout_export();
    void write_file(QString,QString);
    void set_filecontent(QModelIndex);
    void set_exportselection(bool,bool);
    void set_infolabel(int);
    void get_exportinfo(QString,QString,bool);
    void set_comboBox_time();
};

#endif // DIALOG_EXPORT_H
