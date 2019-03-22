#ifndef FOODHISTORY_POPUP_H
#define FOODHISTORY_POPUP_H

#include <QDialog>
#include <foodplanner.h>

namespace Ui {
class foodhistory_popup;
}

class foodhistory_popup : public QDialog
{
    Q_OBJECT

public:
    explicit foodhistory_popup(QWidget *parent = nullptr,foodplanner *pFood = nullptr);
    ~foodhistory_popup();

private:
    Ui::foodhistory_popup *ui;

    foodplanner *foodplan;
};

#endif // FOODHISTORY_POPUP_H
