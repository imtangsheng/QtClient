#include "ShunDe.h"
#include "AppOS.h"

#include "ui/HomeWindow.h"
#include "ui/MasterWindow.h"

QSettings AppSettings("config/ShunDe.ini",QSettings::IniFormat);
QJsonObject AppJson;

ShunDe::ShunDe(QObject *parent) : QObject(parent)
{
    init();
}

ShunDe::~ShunDe()
{
    qDebug() << "ShunDe::~ShunDe() 开始";
    //delete masterWindow;
    //delete homeWindow;
    delete widgets;
}
void ShunDe::init()
{
    widgets = new HomeWidgets;

    ObjectName = widgets->objectName();
    WindowIcon = QIcon(":/asset/titler/ShunDe.svg");
    WindowTitle = tr("主页预览");
    connect(widgets, &HomeWidgets::signals_show_widget_by_name, this, &ShunDe::jumpTabWidget);

    AppSettings.beginGroup(ObjectName);
    AppJson = AppSettings.value("AppJson", QJsonObject()).toJsonObject();
    AppSettings.endGroup();

    masterWindow = MasterWindow::getInstance(widgets);
    homeWindow = new HomeWindow(widgets); //HomeWindow::start() 中使用了masterWindow对象

//    masterWindow->start();
//    homeWindow->start();

    qDebug()<<"创建的插件ShunDe::init()";
}

void ShunDe::quit()
{

    masterWindow->quit();
    homeWindow->quit();
    widgets->quit();

    AppSettings.beginGroup(ObjectName);
    AppSettings.setValue("AppJson", AppJson);
    AppSettings.endGroup();
    AppSettings.sync();
    qDebug()<<"ShunDe::quit()";

}

void ShunDe::jumpTabWidget(const QString& name)
{
    emit signalShowMainWidget(id,name);
}

QObject *ShunDe::instance()
{
    return this;
}

bool ShunDe::Start()
{
    qDebug()<<"ShunDe::Start()";
    return true;
}

QString ShunDe::getObjectNane() const
{
    return ObjectName;
}

QWidget *ShunDe::getWidgetByName(QString name, int *type)
{
    if(name == HomeMain_TabWidgetName){
        if(type != nullptr){
            *type = 11;
        }
        
        return homeWindow;
    }
    return widgets->findChild<QWidget *>(name);
}

bool ShunDe::widgetReturnAfterRemoved(QWidget *widget)
{
    qDebug()<<"ShunDe::widgetReturnAfterRemoved()";
    if(widget){
        widget->setParent(widgets);
        return true;
    }
    return false;
}

