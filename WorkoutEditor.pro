#-------------------------------------------------
#
# Project created by QtCreator 2016-01-11T09:48:56
#
#-------------------------------------------------

QT       += core gui
QT       += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = WorkoutEditor
TEMPLATE = app

#Application version
VERSION_MAJOR = 1
VERSION_MINOR = 2
VERSION_BUILD = 7
DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"

#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}
VERSION_PE_HEADER = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

SOURCES += main.cpp\
        mainwindow.cpp \
    settings.cpp \
    dialog_add.cpp \
    dialog_edit.cpp \
    dialog_export.cpp \
    dialog_workouts.cpp \
    activity.cpp \
    dialog_stresscalc.cpp \
    qcustomplot.cpp \
    week_popup.cpp \
    day_popup.cpp \
    dialog_settings.cpp \
    dialog_pacecalc.cpp \
    standardworkouts.cpp \
    dialog_addweek.cpp \
    year_popup.cpp \
    schedule.cpp \
    dialog_week_copy.cpp \
    dialog_version.cpp \
    jsonhandler.cpp \
    logger.cpp \
    stress_popup.cpp \
    calculation.cpp \
    xmlhandler.cpp \
    dialog_workcreator.cpp \
    filereader.cpp

HEADERS  += mainwindow.h \
    calendar_delegate.h \
    settings.h \
    dialog_add.h \
    dialog_edit.h \
    dialog_export.h \
    dialog_workouts.h \
    activity.h \
    del_spinbox_int.h \
    dialog_stresscalc.h \
    qcustomplot.h \
    week_popup.h \
    day_popup.h \
    dialog_settings.h \
    dialog_pacecalc.h \
    standardworkouts.h \
    dialog_addweek.h \
    week_delegate.h \
    summery_delegate.h \
    year_popup.h \
    schedule.h \
    dialog_week_copy.h \
    dialog_version.h \
    del_intselect.h \
    jsonhandler.h \
    logger.h \
    del_level.h \
    del_addweek.h \
    stress_popup.h \
    calculation.h \
    xmlhandler.h \
    dialog_workcreator.h \
    del_workcreator.h \
    del_treeview.h \
    del_avgselect.h \
    filereader.h

FORMS    += mainwindow.ui \
    dialog_add.ui \
    dialog_edit.ui \
    dialog_export.ui \
    dialog_workouts.ui \
    dialog_stresscalc.ui \
    week_popup.ui \
    day_popup.ui \
    dialog_settings.ui \
    dialog_pacecalc.ui \
    dialog_addweek.ui \
    year_popup.ui \
    dialog_week_copy.ui \
    dialog_version.ui \
    dialog_lapeditor.ui \
    stress_popup.ui \
    dialog_workcreator.ui

RESOURCES += \
    resources.qrc

#Add Icon
win32 {
    RC_FILE = WorkoutEditor.rc
}
