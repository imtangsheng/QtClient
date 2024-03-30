#include "AppSystem.h"

QString PATH_EXE_CONFIG = "config.json";
QString PATH_APP_SETTINGS = "./config/config.ini";
QString PATH_LOG = "log/";
QString CurrentUser = "None";

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

QJsonObject AppJson;
bool getExeConfigJson()
{
    //初始化配置 EXE_CONFIG
    qDebug()<<"当前软件版本号："<<APP_VERSION<<"当前工作目录："<<QDir::currentPath();
    QFile file(PATH_EXE_CONFIG);
    if(!file.open(QIODevice::ReadOnly)) {
        qErrnoWarning("Config file not found");
        //        qFatal("Config file not found"); // 软件会崩溃
        file.close();
        return -1;
    }
    AppJson = QJsonDocument::fromJson(file.readAll()).object();
    file.close();
    if(AppJson.isEmpty()) {
        qErrnoWarning("Config file is not valid JSON");
//        QMessageBox::critical(nullptr, "Error", "Config file is not valid JSON");
        return -1;
    }
    return true;

}

IsSucce ReadJsonData(QJsonObject &jsonData, const QString &filePath)
{
    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray jsonDataByteArray = file.readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonDataByteArray);
        jsonData = jsonDoc.object();
        return true;
    }
    return false;
}

IsSucce SavaJsonData(QJsonObject &jsonData, const QString &filePath)
{
    QJsonDocument jsonDoc(jsonData);
    QByteArray jsonDataByteArray = jsonDoc.toJson();

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(jsonDataByteArray);
        file.close();
        return true;
    }
    return false;
}
