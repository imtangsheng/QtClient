#include "dataview.h"
#include "AppOS.h"

QSettings AppSettings("config/DataView.ini",QSettings::IniFormat);
QJsonObject AppJson;

DataView::DataView(QObject *parent) : QObject(parent)
{
    widgets = new DataWidgets;
    snowBeerWindow = new SnowBeerWindow(widgets);
    init();
}

DataView::~DataView()
{
    qDebug()<<"创建的插件DataView::~DataView()释放";
}

void DataView::init()
{
    ObjectName = widgets->objectName();
    WindowIcon = QIcon(":/asset/titler/DataView.svg");
    WindowTitle = tr("数据看板");
    connect(widgets, &DataWidgets::homeMune_jump_TabWidget, this, &DataView::jumpTabWidget);
    qDebug()<<"创建的插件DataView::init()";
}

void DataView::quit()
{
    snowBeerWindow->quit();
    widgets->quit();
    AppSettings.sync();

    qDebug()<<"DataView::quit()";

}

void DataView::jumpTabWidget()
{
    emit signalShowMainWidget(id,HomeMain_TabWidgetName);
}

QObject *DataView::instance()
{
    return this;
}

bool DataView::Start()
{
    qDebug()<<"DataView::Start()";
    return true;
}

QString DataView::getObjectNane() const
{
    return ObjectName;
}

QWidget *DataView::getWidgetByName(QString name, int *type)
{
    if(name == HomeMain_TabWidgetName){
        if(type != nullptr){
            *type = PluginType_Video;
        }
        
        return snowBeerWindow;
    }
    return widgets->findChild<QWidget *>(name);
}

bool DataView::widgetReturnAfterRemoved(QWidget *widget)
{
    qDebug()<<"DataView::widgetReturnAfterRemoved()";
    if(widget){
        widget->setParent(widgets);
        return true;
    }
    return false;
}

