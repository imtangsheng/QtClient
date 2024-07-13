#include "mediaplayer.h"
#include "AppOS.h"

QSettings MediaPlayerSettings("config/MediaPlayer.ini", QSettings::IniFormat);
QJsonObject MediaPlayerJson;

MediaPlayer::MediaPlayer(QObject *parent) : QObject(parent)
{
    widgets = new MediaWidgets;
    videoWindow = new VideoWindow(widgets);
    init();
}

void MediaPlayer::init()
{
    ObjectName = widgets->objectName();
    WindowIcon = QIcon(":/asset/titler/MediaPlayer.svg");
    WindowTitle = tr("视频回放");
    connect(widgets, &MediaWidgets::homeMune_jump_TabWidget, this, &MediaPlayer::jumpTabWidget);
    qDebug() << "创建的插件MediaPlayer::init()" << id << ObjectName << widgets->objectName();
}

QObject *MediaPlayer::instance()
{
    return this;
}

bool MediaPlayer::Start()
{
    qDebug() << "bool MediaPlayer::Start()" << ObjectName;
    //    AppSettings.setPath(QSettings::IniFormat, QSettings::UserScope,"config/"+ObjectName+".ini");
    return true;
}

QString MediaPlayer::getObjectNane() const
{
    return ObjectName;
}

QWidget *MediaPlayer::getWidgetByName(QString name, int *type)
{
    if (name == HomeMain_TabWidgetName)
    {
        if(type != nullptr){
            *type = PluginType_Video;
        }

        return videoWindow;
    }
    return widgets->findChild<QWidget *>(name);
}

bool MediaPlayer::widgetReturnAfterRemoved(QWidget *widget)
{
    qDebug() << "MediaPlayer::widgetReturnAfterRemoved(QWidget *widget)";
    if (widget)
    {
        // 使用setParent方法将子QWidget重新设置为父QWidget的子对象
        widget->setParent(widgets);
        //        widgets.layout()->addWidget(widget);//方法会奔溃
        return true;
    }
    return false;
}

void MediaPlayer::quit()
{
    qDebug() << "MediaPlayer::quit()";
    widgets->quit(); // 先保存，再删除，
    videoWindow->quit();
    MediaPlayerSettings.sync(); // 未保存的写入永久存储，并加载在此期间由其他应用程序的更改的任何设置。通常会在析构和事件循环中调用。
}

void MediaPlayer::jumpTabWidget()
{
    emit signalShowMainWidget(id, HomeMain_TabWidgetName);
}

/*
 * name：一个字符串，表示要创建的插件的名称。
 * spec：一个字符串，表示插件的规范或配置。
 */
// QObject *MediaPlayer::create(const QString &name, const QString &spec)
//{
//     qDebug()<<"MediaPlayer::create(const QString &"<<name<<spec;
////    static_assert(false, "You need to implement this function");//用于指定要检查的条件。如果条件为假，则会触发编译错误
//    return this;
//}

MediaPlayer::~MediaPlayer()
{
    //    delete widgets;
    //    delete videoWindow;
    qDebug() << "MediaPlayer::~MediaPlayer()";
}
