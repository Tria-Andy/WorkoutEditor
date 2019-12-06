#ifndef DIALOG_MAP_H
#define DIALOG_MAP_H

#include <QDialog>
#include <QtCore>
#include <QLabel>
#include <QFileDialog>
#include "settings.h"
#include "standardworkouts.h"


namespace Ui {
class Dialog_map;
}

class Dialog_map : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_map(QWidget *parent = nullptr, QString workID = nullptr,QString mapPix = nullptr);
    ~Dialog_map();

private slots:
    void on_toolButton_close_clicked();
    void on_toolButton_map_clicked();

    void on_toolButton_save_clicked();

private:
    Ui::Dialog_map *ui;
    QString mapImage,mapPath,workoutID;
    void openFile();
    void setImage(QString);
};

#endif // DIALOG_MAP_H
