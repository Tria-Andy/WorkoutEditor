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
VERSION_MINOR = 6
VERSION_BUILD = 0
DEV_BUILD = 1
DEFINES += "VERSION_MAJOR=$$VERSION_MAJOR"\
       "VERSION_MINOR=$$VERSION_MINOR"\
       "VERSION_BUILD=$$VERSION_BUILD"\
       "DEV_BUILD=$$DEV_BUILD"\

#Target version
VERSION = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}
VERSION_PE_HEADER = $${VERSION_MAJOR}.$${VERSION_MINOR}.$${VERSION_BUILD}

SOURCES += main.cpp\
    dialog_nutrition.cpp \
        mainwindow.cpp \
    settings.cpp \
    activity.cpp \
    dialog_stresscalc.cpp \
    qcustomplot.cpp \
    day_popup.cpp \
    dialog_settings.cpp \
    dialog_pacecalc.cpp \
    standardworkouts.cpp \
    dialog_addweek.cpp \
    schedule.cpp \
    dialog_week_copy.cpp \
    dialog_version.cpp \
    jsonhandler.cpp \
    logger.cpp \
    calculation.cpp \
    xmlhandler.cpp \
    dialog_workcreator.cpp \
    foodplanner.cpp \
    dialog_map.cpp \
    foodmacro_popup.cpp

HEADERS  += mainwindow.h \
    dialog_nutrition.h \
    settings.h \
    activity.h \
    dialog_stresscalc.h \
    qcustomplot.h \
    day_popup.h \
    dialog_settings.h \
    dialog_pacecalc.h \
    standardworkouts.h \
    dialog_addweek.h \
    schedule.h \
    dialog_week_copy.h \
    dialog_version.h \
    jsonhandler.h \
    logger.h \
    del_level.h \
    calculation.h \
    xmlhandler.h \
    dialog_workcreator.h \
    del_mousehover.h \
    foodplanner.h \
    dialog_map.h \
    foodmacro_popup.h

FORMS    += mainwindow.ui \
    dialog_nutrition.ui \
    dialog_stresscalc.ui \
    day_popup.ui \
    dialog_settings.ui \
    dialog_pacecalc.ui \
    dialog_week_copy.ui \
    dialog_version.ui \
    dialog_workcreator.ui \
    dialog_addweek.ui \
    dialog_map.ui \
    foodmacro_popup.ui

RESOURCES += \
    resources.qrc

#Add Icon
win32 {
    RC_FILE = WorkoutEditor.rc
}

DISTFILES += \
    editormodel.qmodel
