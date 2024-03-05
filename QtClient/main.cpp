#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QJsonDocument>

#include "public/AppData.h"
QJsonObject EXE_CONFIG;
//QSettings APP_SETTINGS;
QSettings APP_SETTINGS("./config/config.ini",QSettings::IniFormat);

#include "ui/SubMain.h"

SubMain *SUB_MAIN;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QtClient_" + QLocale(locale).name();
        qDebug()<<"当前软件uiLanguages："<<baseName;
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            qDebug()<<"当前软件Languages"<<&translator;
            break;
        }
    }
    //初始化配置 EXE_CONFIG
    qDebug()<<"当前软件版本号："<<APP_VERSION<<"当前工作目录："<<QDir::currentPath();
    QFile file("config.json");
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(nullptr, "Error", "Config file not found");
        return -1;
    }
    EXE_CONFIG = QJsonDocument::fromJson(file.readAll()).object();
    if(EXE_CONFIG.isEmpty()) {
        QMessageBox::critical(nullptr, "Error", "Config file is not valid JSON");
        file.close();
        return -1;
    }
    file.close();
    qDebug()<<"当前软件版本："<<EXE_CONFIG["version"].toString();

    MainWindow w;
    w.show();
    SUB_MAIN = new SubMain;
    return a.exec();
}
