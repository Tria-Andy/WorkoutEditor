#include "dialog_version.h"
#include "ui_dialog_version.h"

Dialog_version::Dialog_version(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_version)
{
    ui->setupUi(this);

    ui->textBrowser_info->setText("<center>"
                                  "<h2>WorkoutEditor</h2>"
                                  "<h2>for GoldenCheetah</h2>"
                                  "<h3>Version 1.0.0 RC1 Build 20160828</h3><br>"
                                  "WorkoutEditor is a planing tool to set up the year and week schedule and export it to GC.<br>"
                                  "With the Editor can edit existing workouts out of GC.<br>"
                                  "<br>Developed by Andreas Hunner"
                                  "</center>"
                                  );
}

Dialog_version::~Dialog_version()
{
    delete ui;
}

