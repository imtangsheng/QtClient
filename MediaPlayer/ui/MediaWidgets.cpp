#include "MediaWidgets.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

//QSettings Config("config/config.ini",QSettings::IniFormat); //无编码配置，已经移除，使用UTF-8

MediaWidgets::MediaWidgets(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaWidgets)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    init();
}

void MediaWidgets::closeEvent(QCloseEvent *event)
{
    qDebug()<<"MediaWidgets::closeEvent(QCloseEvent *"<<event;

}

MediaWidgets::~MediaWidgets()
{

    // 创建一个事件循环
//    QEventLoop loop; 需要app没有在销毁前
//    // 连接停止信号和事件循环的退出槽
//    QObject::connect(player, &QMediaPlayer::playbackStateChanged, [&loop](QMediaPlayer::PlaybackState state){
//        if (state == QMediaPlayer::StoppedState) {
//            loop.quit();
//        }
//    });

    // 等待停止操作完成
//    loop.quit();
//    loop.exec();
    delete ui;
    qDebug()<<"MediaWidgets::~MediaWidgets() 释放 成功"<<objectName();
}

void MediaWidgets::init()
{
    config =  new QSettings("config/"+objectName()+".ini",QSettings::IniFormat); //无编码配置，已经移除，使用UTF-8
    config->beginGroup("MediaWidgets");
    currentPlayFile  = config->value("currentPlayFile","").toString();
    fileExtensions = config->value("fileExtensions").toStringList();
    //字符数组为空时会判断为有一个成员
    if(fileExtensions.isEmpty() || (fileExtensions.size() == 1 && fileExtensions.at(0).isEmpty())){

        fileExtensions  = QString(".mp4,.avi").split(",");
    }
    config->endGroup();

    player = new QMediaPlayer(this);
    //声音输出
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);
    //视频输出
    player->setVideoOutput(ui->widget_video);
    if(!currentPlayFile.isEmpty()){
        setVideoSourceAndPlay(currentPlayFile);

    }
    //

    //信号连接
    connect(player, &QMediaPlayer::hasVideoChanged, this, &MediaWidgets::hasVideoChanged);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MediaWidgets::mediaStatusChanged);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &MediaWidgets::playbackStateChanged);
    connect(player, &QMediaPlayer::sourceChanged, this, &MediaWidgets::sourceChanged);
    connect(player, &QMediaPlayer::errorOccurred, this, &MediaWidgets::errorOccurred);

    // 连接slider信号到槽函数
    connect(ui->horizontalSlider_position, &QSlider::valueChanged, player, &QMediaPlayer::setPosition);
    connect(ui->horizontalSlider_position, &QSlider::sliderMoved, this, &MediaWidgets::sliderMovedForPlayer);

    qDebug()<<"MediaWidgets::init()"<<currentPlayFile;
    qDebug()<<"MediaWidgets::saveConfig()"<<currentPlayFile<<fileExtensions;
}

void MediaWidgets::quit()
{
    //1.~MediaWidgets()不可以在这里设置改变,会直接跳过不执行，怀疑是先一步与析构函数释放全局变量
    //2.使用变量不能声明，使用改使用指针。同上，会触发三个无窗口句柄，无效的窗口句柄 但是会执行。
    //3.解决方案：在父析构中调用保存配置的操作，且不使用全局变量
    config->beginGroup("MediaWidgets");
    config->setValue("currentPlayFile",currentPlayFile);
    config->setValue("fileExtensions",fileExtensions);
    config->endGroup();
    // 异步保存设置,在播放视频时退出会保存无效的窗口句柄这个错
    config->sync();
    qDebug()<<"MediaWidgets::saveConfig()"<<currentPlayFile<<fileExtensions;
//    player->stop();
    if(player->isPlaying()){
        qDebug()<<"MediaWidgets::player 播放中，先停止";
        player->pause();
        player->stop();
        //QObject::killTimer: Timers cannot be stopped from another thread
        emit player->destroyed();

    }
    player = nullptr;
    player->deleteLater();
    qDebug()<<"MediaWidgets::~MediaWidgets() player释放 成功"<<objectName();
}

void MediaWidgets::setVideoSourceAndPlay(QString source)
{
    if (source.startsWith("http://") || source.startsWith("https://")) {
        // 网络流
        url = QUrl(source);
    } else {
        // 本地文件
        url = QUrl::fromLocalFile(source);
    }
    player->setSource(url);
    playerControls(MediaControls::Play);
}

void MediaWidgets::hasVideoChanged(bool videoAvailable)
{
    qDebug()<<"MediaWidgets::hasVideoChanged(bool "<<videoAvailable;
}

void MediaWidgets::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug()<<"MediaWidgets::mediaStatusChanged(QMediaPlayer::MediaStatus "<<status;
}

void MediaWidgets::playbackStateChanged(QMediaPlayer::PlaybackState newState)
{
    qDebug()<<"MediaWidgets::playbackStateChanged(QMediaPlayer::PlaybackState "<<newState;
}

void MediaWidgets::sourceChanged(const QUrl &media)
{
    qDebug()<<"MediaWidgets::sourceChanged(const QUrl "<<&media<<currentPlayFile<<player->duration();
    ui->horizontalSlider_position->setRange(0,player->duration());// 设置滑动条的范围为媒体的总时长
}

void MediaWidgets::errorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    qDebug()<<"MediaWidgets::errorOccurred(QMediaPlayer::Error "<<error<<errorString;
}
#include <QToolTip>
void MediaWidgets::sliderMovedForPlayer(int value)
{
    // 槽函数，显示播放时间
//    qint64 duration = player->duration();
//    qint64 position = value * duration / ui->horizontalSlider_position->maximum();
    int minutes = value / 60000; // 每分钟60秒，每秒1000毫秒
    int seconds = (value / 1000) % 60;

    QToolTip::showText(
        ui->horizontalSlider_position->mapToGlobal(QPoint(value, 0)),
        QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')),
        ui->horizontalSlider_position);
}

bool MediaWidgets::playerControls(int control)
{
    switch (control)
    {
    case MediaControls::Play:
        player->play();
        break;
    case MediaControls::Pause:
        player->pause();
        break;
    case MediaControls::Stop:
        player->stop();
        break;
    default:
        break;
    }

    return true;
}




void MediaWidgets::on_Button_videoPlayback_clicked()
{
    qDebug()<<"MediaWidgets::on_Button_videoPlayback_clicked()";
    emit homeMune_jump_TabWidget();
}


void MediaWidgets::on_pushButton_test_clicked()
{
    // 创建打开文件对话框
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile); // 设置对话框模式为选择已存在的文件

    QString filePath = fileDialog.getOpenFileName(this, "Open File", nullptr, "Mp4 Files (*.mp4)"); // 获取选择的文件路径
    if (!filePath.isEmpty())
    {
        // 在这里处理打开文件的逻辑
        qDebug() << "Selected file: " << filePath;
        currentPlayFile = filePath;
        player->setSource(QUrl::fromLocalFile(filePath));
    }
}


void MediaWidgets::on_toolButton_play_clicked()
{
    qDebug() << "play_clicked" << player->isAvailable() << player->hasVideo();
    if (player->isAvailable() && player->hasVideo())
    {
        qDebug() << "play_clicked video file ";
        player->play();
    } else if(!currentPlayFile.isEmpty()){
        setVideoSourceAndPlay(currentPlayFile);
    }
    else
    {
        qDebug() << "play_clicked not video file ";
        QMessageBox::critical(this, "Error", "video file not available");
    }
}

void MediaWidgets::on_toolButton_stop_clicked()
{
    playerControls(MediaControls::Stop);
}

void MediaWidgets::on_toolButton_pause_clicked()
{
    playerControls(MediaControls::Pause);
}

void MediaWidgets::on_toolButton_previous_clicked()
{

}


void MediaWidgets::on_toolButton_next_clicked()
{

}

void MediaWidgets::on_horizontalSlider_volume_valueChanged(int value)
{
    qDebug() << "MediaWidgets::on_horizontalSlider_volume_valueChanged:" << value / 10 << value / 10.0;
    audioOutput->setVolume(value);
}

