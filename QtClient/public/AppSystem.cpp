#include "AppSystem.h"

QString PATH_EXE_CONFIG = "config.json";
QString PATH_APP_SETTINGS = "./config/config.ini";
QString PATH_LOG = "log/";


void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QString logMessage;
    QDateTime dateTime = QDateTime::currentDateTime();
    switch (type) {
    case QtDebugMsg:
        logMessage = QString("[%1] Debug: %2:%3 - %4").arg(dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"),context.file).arg(context.line).arg(msg);
        break;
    case QtInfoMsg:
        logMessage = QString("[%1] Debug: %2:%3 - %4").arg(dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"),context.function).arg(context.line).arg(msg);
        break;
    case QtWarningMsg:
        logMessage = QString("[%1] Warning: %2").arg(dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"),msg);
        break;
    case QtCriticalMsg:
        logMessage = QString("[%1] Critical: %2").arg(dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"),msg);
        break;
    case QtFatalMsg:
        logMessage = QString("[%1] Fatal: %2").arg(dateTime.toString("yyyy-MM-dd hh:mm:ss.zzz"),msg);
        break;
    }
    QString fileName = PATH_LOG ;
    QDir dir(fileName);
    if(!dir.exists())
    {
        dir.mkpath(fileName);
    }
    fileName += dateTime.toString("yyyy-MM-dd")+"-log.txt";
    QFile logFile(fileName);
    if (logFile.open(QIODevice::WriteOnly | QIODevice::Append)) {
        QTextStream logStream(&logFile);
        logStream << logMessage << Qt::endl;
        logFile.close();
    }

}

