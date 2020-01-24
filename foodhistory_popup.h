#ifndef FOODHISTORY_POPUP_H
#define FOODHISTORY_POPUP_H

#include <QDialog>
#include <foodplanner.h>

namespace Ui {
class foodhistory_popup;
}

class foodhistory_popup : public QDialog, public calculation
{
    Q_OBJECT

public:
    explicit foodhistory_popup(QWidget *parent = nullptr,foodplanner *pFood = nullptr);
    ~foodhistory_popup();

private slots:
    void on_toolButton_close_clicked();
    void on_comboBox_weekCount_currentIndexChanged(int index);
    void on_toolButton_reset_clicked();

private:
    Ui::foodhistory_popup *ui;
    foodplanner *foodplan;
    QMap<QPair<int,int>,QMap<QDate,QList<QVariant>>> *foodHistory;

    QVector<double> weekList,metaRate,calConversion,calSport,calFood,calDiff,weight;
    int weekCount,dialogResult;
    double athleteWeight;
    QStringList weekLabels;

    void set_foodHistoryTree();

    void set_plotValues(int,int,bool);
    void set_graph(int,double);
};
#endif // FOODHISTORY_POPUP_H
