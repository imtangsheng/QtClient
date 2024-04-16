#include "ShunDe.h"
#include "AppOS.h"

QSettings AppSettings("config/ShunDe.ini",QSettings::IniFormat);
QJsonObject AppJson;



ShunDe::ShunDe(QObject *parent) : QObject(parent)
{
    init();
}

ShunDe::~ShunDe()
{
    qDebug()<<"创建的插件ShunDe::~ShunDe()释放";
}

void ShunDe::init()
{
    widgets = new HomeWidgets;

    ObjectName = widgets->objectName();
    WindowIcon = QIcon(":/asset/titler/ShunDe.svg");
    WindowTitle = tr("主页预览");
    connect(widgets, &HomeWidgets::homeMune_jump_TabWidget, this, &ShunDe::jumpTabWidget);

    AppSettings.beginGroup(ObjectName);
    AppJson = AppSettings.value("AppJson", QJsonObject()).toJsonObject();
    AppSettings.endGroup();

    window = new HomeWindow(widgets);

    qDebug()<<"创建的插件ShunDe::init()";
}

void ShunDe::quit()
{
    window->quit();
    widgets->quit();

    AppSettings.beginGroup(ObjectName);
    AppSettings.setValue("AppJson", AppJson);
    AppSettings.endGroup();
    AppSettings.sync();
    qDebug()<<"ShunDe::quit()";

}

void ShunDe::jumpTabWidget()
{
    emit signalShowMainWidget(id,HomeMain_TabWidgetName);
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
        
        return window;
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

