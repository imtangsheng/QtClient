#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>
#include <QJsonDocument>

#include "public/AppSystem.h"
QJsonObject EXE_CONFIG;
//QSettings APP_SETTINGS;
QSettings APP_SETTINGS(PATH_APP_SETTINGS,QSettings::IniFormat); //无编码配置，已经移除，使用UTF-8

#include "ui/SubMain.h"
SubMain *SUB_MAIN;

#include "FirstShowWidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
//    a.setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);//让QWidget子对象不再共享父窗口,各自独立
#ifdef QT_NO_DEBUG
    qInstallMessageHandler(logToFile);
#endif
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QtClient_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            qDebug()<<"当前显示语言Languages："<<baseName;
            break;
        }
    }

    //初始化配置 EXE_CONFIG
    qDebug()<<"当前软件版本号："<<APP_VERSION<<"当前工作目录："<<QDir::currentPath();
    QFile file(PATH_EXE_CONFIG);
    if(!file.open(QIODevice::ReadOnly)) {
        qFatal("Config file not found");
        QMessageBox::critical(nullptr, "Error", "Config file not found");
        return -1;
    }
    EXE_CONFIG = QJsonDocument::fromJson(file.readAll()).object();
    if(EXE_CONFIG.isEmpty()) {
        qFatal("Config file is not valid JSON");
        QMessageBox::critical(nullptr, "Error", "Config file is not valid JSON");
        file.close();
        return -1;
    }
    file.close();
    qDebug()<<"当前软件开发版本："<<EXE_CONFIG["version"].toString();

    FirstShowWidget first;
    first.show();

    MainWindow w;
    w.show();

    SUB_MAIN = new SubMain;
    return a.exec();
}
