#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include <QCommandLineParser>
#include <QSystemSemaphore>
#include <QSharedMemory>
#include "mainwindow.h"
#include "public/AppSystem.h"
#include <QDir>

QString PATH_APP_SETTINGS = "./config/config.ini";//别处定义会报错：Empty filename passed to function
QSettings AppSettings(PATH_APP_SETTINGS,QSettings::IniFormat); //无编码配置，已经移除，使用UTF-8

#include "FirstShowWidget.h"

int main(int argc, char *argv[])
{
    // 获取应用程序所在目录
    QString appDir = QCoreApplication::applicationDirPath();
    //遍历 libs 目录下所有的子目录
    QDir libsDir(QDir(appDir).filePath("libs"));
    QStringList subLibsDirs = libsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for(const QString& subdir: subLibsDirs){
        QString libDir = QDir(libsDir.absoluteFilePath(subdir)).filePath("lib");
        //添加 lib 目录到搜索路径
        QCoreApplication::addLibraryPath(libDir);
    }

    QApplication app(argc, argv);
    // 确保只运行一次
    QSystemSemaphore sema("RobotStudioClient",1,QSystemSemaphore::Open);
    sema.acquire();// 在临界区操作共享内存   SharedMemory
    QSharedMemory mem("DcRobotSystemObject");// 全局对象名
    if (!mem.create(1))// 如果全局对象已存在则退出
    {
        QMessageBox::information(0,"提示","软件已开启!!");
        sema.release();// 如果是 Unix 系统，会自动释放。
        return 0;
    }
    sema.release();// 临界区

    QCommandLineParser parser;
    parser.setApplicationDescription("My Application");
    parser.addHelpOption();

    QCommandLineOption nameOption("name", "Specify a name", "name","tang");
    parser.addOption(nameOption);

    // 解析命令行参数 --name "tang"
    parser.process(app);
    QString name = parser.value(nameOption);
#ifdef QT_NO_DEBUG
    qInstallMessageHandler(logToFile);
#endif
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QtClient_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            app.installTranslator(&translator);
            qDebug()<<"当前显示语言Languages："<<baseName;
            break;
        }
    }

    if(getExeConfigJson()){
        qDebug()<<"当前软件开发版本："<<AppJson["version"].toString();
    }
    MainWindow w;
    FirstShowWidget first;
    if(first.startAutoLogin()){
        w.showUI();
    }else{
        QObject::connect(&first,&FirstShowWidget::loginSuccess,&w,&MainWindow::showUI);
        first.show();
    }
    qDebug() << "APP the Name is:" << name;
    return app.exec();
}
