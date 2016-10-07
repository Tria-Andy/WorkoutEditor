#include "dialog_version.h"
#include "ui_dialog_version.h"
#include "settings.h"

Dialog_version::Dialog_version(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog_version)
{
    ui->setupUi(this);

    ui->textBrowser_info->setText("<center>"
                                  "<h2>WorkoutEditor</h2>"
                                  "<h2>for GoldenCheetah</h2>"
                                  "<h3>Version "+settings::get_version()+ " Build "+settings::get_builddate()+"</h3><br>"
                                  "Planing year and week workout schedule and export it to GC.<br>"
                                  "Edit existing workouts out of GC.<br>"
                                  "<br>Developed by Andreas Hunner"
                                  "</center>"
                                  );
}

Dialog_version::~Dialog_version()
{
    delete ui;
}

