#ifndef DIALOG_NUTRITION_H
#define DIALOG_NUTRITION_H

#include <QDialog>
#include "foodplanner.h"

namespace Ui {
class Dialog_nutrition;
}

class Dialog_nutrition : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_nutrition(QWidget *parent = nullptr,foodplanner *pFood = nullptr);
    ~Dialog_nutrition();

private slots:
    void on_toolButton_close_clicked();

private:
    Ui::Dialog_nutrition *ui;
};

#endif // DIALOG_NUTRITION_H
