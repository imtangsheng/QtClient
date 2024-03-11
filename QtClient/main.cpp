#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>


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

    if(getExeConfigJson()){
        qDebug()<<"当前软件开发版本："<<EXE_CONFIG["version"].toString();
    }
    FirstShowWidget first;
    MainWindow w;
//    QEventLoop loop;
//    QObject::connect(&first,&FirstShowWidget::loginSuccess,&w,&MainWindow::show);
//    loop.exec(); // 等待finished信号
    // 信号发射后继续执行
//    w.show();
    if(first.startAutoLogin()){
        w.show();
        first.close();
    }else{
        QObject::connect(&first,&FirstShowWidget::loginSuccess,&w,&MainWindow::show);
        first.show();
    }

    SUB_MAIN = new SubMain;
    return a.exec();
}
