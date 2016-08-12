#ifndef DIALOG_LAPEDITOR_H
#define DIALOG_LAPEDITOR_H

#include <QDialog>

namespace Ui {
class Dialog_lapeditor;
}

class Dialog_lapeditor : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_lapeditor(QWidget *parent = 0);
    ~Dialog_lapeditor();

private:
    Ui::Dialog_lapeditor *ui;
};

#endif // DIALOG_LAPEDITOR_H
