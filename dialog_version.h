#ifndef DIALOG_VERSION_H
#define DIALOG_VERSION_H

#include <QDialog>

namespace Ui {
class Dialog_version;
}

class Dialog_version : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_version(QWidget *parent = 0);
    ~Dialog_version();

private:
    Ui::Dialog_version *ui;
};

#endif // DIALOG_VERSION_H
