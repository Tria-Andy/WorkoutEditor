#ifndef FOODMACRO_POPUP_H
#define FOODMACRO_POPUP_H

#include <QDialog>
#include <foodplanner.h>

namespace Ui {
class foodmacro_popup;
}

class foodmacro_popup : public QDialog
{
    Q_OBJECT

public:
    explicit foodmacro_popup(QWidget *parent = nullptr,foodplanner *pFood = nullptr,const QDate startDate = QDate());
    ~foodmacro_popup();

private slots:
    void on_toolButton_close_clicked();

private:
    Ui::foodmacro_popup *ui;
    foodplanner *foodplan;
    QDate startDay;
    int dayCount;
    QList<QDateTime> weekDates;
    QVector<double> carbValues,proteinValues, fatValues, fiberValues,sugarValues,xValues;
    QVector<double> carbMarco,proteinMacro, fatMacro, fiberMacro,sugarMacro;
    void set_plotValues(QDate);
    void set_graph(QDate);
};

#endif // FOODMACRO_POPUP_H