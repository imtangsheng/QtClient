#include "mediaplayer.h"

MediaPlayer::MediaPlayer(QObject *parent) : QObject(parent)
{
    widgets = new MediaWidgets(nullptr);
    init();
}

void MediaPlayer::init()
{
    ObjectName = widgets->objectName();
    WindowIcon = QIcon(":/asset/titler/MediaPlayer.svg");
    WindowTitle = tr("视频回放");
    connect(widgets,&MediaWidgets::homeMune_jump_TabWidget,this,&MediaPlayer::jumpTabWidget);
    qDebug()<<"创建的插件MediaPlayer::init()"<<id<<ObjectName<<widgets->objectName();

}

QString MediaPlayer::getObjectNane()
{
    return ObjectName;
}

QWidget *MediaPlayer::getWidgetByName(QString name)
{
    return widgets->findChild<QWidget*>(name);
}

bool MediaPlayer::widgetReturnAfterRemoved(QWidget *widget)
{
    qDebug()<<"MediaPlayer::widgetReturnAfterRemoved(QWidget *widget)";
    if(widget){
        // 使用setParent方法将子QWidget重新设置为父QWidget的子对象
        widget->setParent(widgets);
//        widgets.layout()->addWidget(widget);//方法会奔溃
        return true;
    }
    return false;
}

QWidget *MediaPlayer::getHomeTiler()
{
    return widgets->ui->HomeMenu;
}

void MediaPlayer::quit()
{
    qDebug()<<"MediaPlayer::quit()保存，再删除";
    widgets->quit(); //先保存，再删除，
}

void MediaPlayer::jumpTabWidget()
{
    emit signalShowMainWidget(id,HomeMain_TabWidgetName);
}

/*
 * name：一个字符串，表示要创建的插件的名称。
 * spec：一个字符串，表示插件的规范或配置。
*/
//QObject *MediaPlayer::create(const QString &name, const QString &spec)
//{
//    qDebug()<<"MediaPlayer::create(const QString &"<<name<<spec;
////    static_assert(false, "You need to implement this function");//用于指定要检查的条件。如果条件为假，则会触发编译错误
//    return this;
//}

MediaPlayer::~MediaPlayer()
{
    delete widgets;
    qDebug()<<"MediaPlayer::~MediaPlayer()";
}


