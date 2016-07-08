#ifndef DIALOG_PREFERENCE_H
#define DIALOG_PREFERENCE_H

#include <QDialog>

namespace Ui {
class Dialog_preference;
}

class Dialog_preference : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_preference(QWidget *parent = 0);
    ~Dialog_preference();

private:
    Ui::Dialog_preference *ui;
};

#endif // DIALOG_PREFERENCE_H
