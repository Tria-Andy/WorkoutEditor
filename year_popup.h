#ifndef YEAR_POPUP_H
#define YEAR_POPUP_H

#include <QDialog>
#include <QStandardItemModel>
#include "schedule.h"
#include "settings.h"

namespace Ui {
class year_popup;
}

class year_popup : public QDialog
{
    Q_OBJECT

public:
    explicit year_popup(QWidget *parent = 0, QString pInfo = 0,int position = 0,schedule *p_sched = 0, QString pPhase = 0,int pIndex = 0,settings *p_settings = 0);
    ~year_popup();

private slots:
    void on_pushButton_close_clicked();

    void on_comboBox_select_currentIndexChanged(int index);

private:
    Ui::year_popup *ui;

    QStringList partInfo,phaseList,selectList;
    QString phase;
    int col,phaseindex,widthFactor,selectAxis;
    settings *pop_settings;
    schedule *workSched;

    void set_plot();
};

#endif // YEAR_POPUP_H
