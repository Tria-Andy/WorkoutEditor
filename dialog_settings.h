#ifndef DIALOG_SETTINGS_H
#define DIALOG_SETTINGS_H

#include <QtCore>
#include <QDialog>
#include <QListWidget>
#include <QStandardItemModel>
#include "settings.h"

namespace Ui {
class Dialog_settings;
}

class Dialog_settings : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog_settings(QWidget *parent = 0);
    ~Dialog_settings();

private slots:


    void on_pushButton_cancel_clicked();

    void on_comboBox_selInfo_currentTextChanged(const QString &arg1);

    void on_listWidget_selection_itemDoubleClicked(QListWidgetItem *item);

    void on_pushButton_up_clicked();

    void on_pushButton_down_clicked();

    void on_pushButton_add_clicked();

    void on_pushButton_delete_clicked();

    void on_pushButton_edit_clicked();

    void on_comboBox_thresSport_currentTextChanged(const QString &arg1);

    void on_pushButton_save_clicked();

    void on_lineEdit_thresPace_textEdited(const QString &arg1);

    void on_lineEdit_thresPower_textEdited(const QString &arg1);

    void on_lineEdit_hfThres_textEdited(const QString &arg1);

    void on_lineEdit_hfmax_textEdited(const QString &arg1);

    void on_dateEdit_saisonStart_dateChanged(const QDate &date);

    void on_pushButton_color_clicked();

private:
    Ui::Dialog_settings *ui;
    QStandardItemModel *level_model,*hf_model;
    QStringList sportList,model_header,paceList,hfList;
    double *powerlist;
    void set_listEntries(QString);
    void save_settingsChanges();
    void set_thresholdView(QString);
    void set_thresholdModel(QStringList);
    void set_hfmodel();
    void enableSavebutton();
    void writeChangedValues();
};

#endif // DIALOG_SETTINGS_H
