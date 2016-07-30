#ifndef DAY_POPUP_H
#define DAY_POPUP_H

#include <QtGui>
#include <QDialog>
#include <QTimeEdit>
#include <QTextBrowser>
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QPushButton>
#include "dialog_edit.h"
#include "schedule.h"
#include "settings.h"

namespace Ui {
class day_popup;
}

class day_popup : public QDialog
{
    Q_OBJECT

public:
    explicit day_popup(QWidget *parent = 0, const QDate w_date = QDate(), schedule *p_sched = 0,settings *p_settings = 0);
    ~day_popup();

private slots:
    void on_pushButton_edit_clicked();
    void on_pushButton_close_clicked();

private:
    Ui::day_popup *ui;
    schedule *workSched;
    settings *pop_settings;
    const QDate *workout_date;
    QString weekPhase;
    void show_workouts();

};

#endif // DAY_POPUP_H
