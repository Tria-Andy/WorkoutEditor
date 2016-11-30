#include "logger.h"
#include <QApplication>
#include <QDebug>
#include <QDateTime>
#include <QTextStream>
#include <QMessageLogger>
#include "settings.h"


void logMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
}


logger::logger(bool logging)
{
    enableLog = logging;
    //qDebug() << "Function Name: " << Q_FUNC_INFO;

    QString settingFile = QApplication::applicationDirPath() + "/WorkoutEditor.ini";
    QSettings *mysettings = new QSettings(settingFile,QSettings::IniFormat);

    mysettings->beginGroup("Logging");
        bool enableLog = mysettings->value("enablelog").toBool();
    mysettings->endGroup();

    if(enableLog)
    {
        qInstallMessageHandler(logMessageOutput);
    }
}


