#include "mediaplayer.h"

MediaPlayer::MediaPlayer(QObject *parent) : QGenericPlugin(parent)
{

}


void MediaPlayer::init()
{
//    ui->setupUi()
//    id = 10;
//    QPluginMetaData metaData = QPluginMetaData::data
    qDebug()<<this->metaObject()->className();
    qDebug()<<this->staticMetaObject.metaType();
    qDebug()<<"创建的插件MediaPlayer::init()"<<id;


}

QWidget *MediaPlayer::getHomeTiler()
{
    return widgets.ui->groupBox_video;
}

/*
 * name：一个字符串，表示要创建的插件的名称。
 * spec：一个字符串，表示插件的规范或配置。
*/
QObject *MediaPlayer::create(const QString &name, const QString &spec)
{
    qDebug()<<"MediaPlayer::create(const QString &"<<name<<spec;
//    static_assert(false, "You need to implement this function");//用于指定要检查的条件。如果条件为假，则会触发编译错误
    return this;
}

MediaPlayer::~MediaPlayer()
{
    qDebug()<<"MediaPlayer::~MediaPlayer()释放";
}


