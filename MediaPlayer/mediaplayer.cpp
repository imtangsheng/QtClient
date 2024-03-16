#include "mediaplayer.h"

#include "ui_MediaWidget.h"

namespace Ui {
class MediaWidget;
}

//! [0] 定义插件接入
class MediaWidget : public QWidget
{
public:
    MediaWidget(QWidget *parent = nullptr);

public:
    Ui::MediaWidget *ui;
    void init();
    QWidget *getHomeTiler();

private:
    ~MediaWidget()
    {

    }

};
MediaWidget::MediaWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaWidget)
{
    ui->setupUi(this);
}

void MediaWidget::init()
{

}

QWidget *MediaWidget::getHomeTiler()
{
//    return ui->widget_homeTitler;
    return ui->groupBox_video;
}




MediaPlayer::MediaPlayer(QObject *parent) :
    QGenericPlugin(parent),
    widget(new MediaWidget)
{

}


void MediaPlayer::init()
{
//    ui->setupUi()
    qDebug()<<"创建的插件MediaPlayer::init()";
}

QWidget *MediaPlayer::getHomeTiler()
{
//    return ui->groupBox_video;
    return widget->getHomeTiler();
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


