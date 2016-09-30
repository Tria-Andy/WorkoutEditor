#-------------------------------------------------
#
# Project created by QtCreator 2016-01-11T09:48:56
#
#-------------------------------------------------

QT       += charts
QT       += core gui
QT       += xml

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets printsupport

TARGET = WorkoutEditor
TEMPLATE = app


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
    dialog_inteditor.cpp \
    day_popup.cpp \
    dialog_settings.cpp \
    dialog_pacecalc.cpp \
    standardworkouts.cpp \
    dialog_addweek.cpp \
    year_popup.cpp \
    schedule.cpp \
    dialog_week_copy.cpp \
    dialog_version.cpp \
    dialog_lapeditor.cpp

HEADERS  += mainwindow.h \
    calendar_delegate.h \
    settings.h \
    dialog_add.h \
    dialog_edit.h \
    dialog_export.h \
    dialog_workouts.h \
    activity.h \
    del_spinbox_int.h \
    del_spinbox_double.h \
    dialog_stresscalc.h \
    qcustomplot.h \
    week_popup.h \
    dialog_inteditor.h \
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
    dialog_lapeditor.h \
    del_swimlap.h \
    del_intview.h

FORMS    += mainwindow.ui \
    dialog_add.ui \
    dialog_edit.ui \
    dialog_export.ui \
    dialog_workouts.ui \
    dialog_stresscalc.ui \
    week_popup.ui \
    dialog_inteditor.ui \
    day_popup.ui \
    dialog_settings.ui \
    dialog_pacecalc.ui \
    dialog_addweek.ui \
    year_popup.ui \
    dialog_week_copy.ui \
    dialog_version.ui \
    dialog_lapeditor.ui

RESOURCES += \
    resources.qrc
