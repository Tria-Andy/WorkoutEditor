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
    void on_toolButton_edit_clicked();
    void on_treeWidget_foodhistory_itemClicked(QTreeWidgetItem *item, int column);
    void on_spinBox_sport_valueChanged(int arg1);
    void on_spinBox_food_valueChanged(int arg1);

private:
    Ui::foodhistory_popup *ui;
    foodplanner *foodplan;
    QMap<QPair<QDate,int>,QMap<QDate,QList<QVariant>>> *foodHistory;

    QVector<double> weekList,metaRate,calConversion,calSport,calFood,calDiff,weight;
    int weekCount;
    bool updateHistory;
    double athleteWeight;
    QStringList weekLabels;
    QStringList *historyHeader;

    void set_foodHistoryTree();
    void reset_editValues();
    void set_plotValues(int,bool);
    void set_graph(int,double);
};
#endif // FOODHISTORY_POPUP_H
