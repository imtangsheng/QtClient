#include "mainwindow.h"
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QMessageBox>

#include "public/AppSystem.h"

//QSettings APP_SETTINGS;
QSettings APP_SETTINGS(PATH_APP_SETTINGS,QSettings::IniFormat); //无编码配置，已经移除，使用UTF-8

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
    MainWindow w;    
    std::unique_ptr<FirstShowWidget> first(new FirstShowWidget());
//    QObject::connect(first.get(),&FirstShowWidget::loginSuccess,&w,&MainWindow::showUI);
//    first->show();
    if(first->startAutoLogin()){
        //对于dialog/window还需要先关闭窗口 对指针类使用
//        first->deleteLater(); //只能使用指针方式，才会调用析构函数...使用会造成其他界面退出时奔溃
        first.reset(nullptr);//正确的释放方式，采用自动退出机制
        w.showUI();
    }else{
        QObject::connect(first.get(),&FirstShowWidget::loginSuccess,&w,&MainWindow::showUI);
        first->show();
    }

    return a.exec();
}
