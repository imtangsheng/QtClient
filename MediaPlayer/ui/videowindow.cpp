#include "videowindow.h"
#include "AppOS.h"

#include <QWindow>

VideoWindow::VideoWindow(QWidget *parent) : QMainWindow(parent),
                                            ui(new Ui::VideoWindow)
{
    ui->setupUi(this);
    init();
}

VideoWindow::~VideoWindow()
{

    delete ui;
    qDebug() << "VideoWindow::~VideoWindow()释放";
}
/*
第一次显示VideoWindow::showEvent(QShowEvent * 0x722f1fec40 "VideoWindow" QList("VideoWindow/AppJson", "VideoWindow/geometry", "VideoWindow/isFloating")
VideoWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status) QMediaPlayer::LoadingMedia
VideoWindow::sourceChanged(const QUrl & QUrl("rtsp://admin@192.168.1.99:554/Streaming/Channels/2")
TitleBar::showEvent(QShowEvent  QShowEvent(Show, 0x722f1feff0)
MainWindow::resizeEvent(QResizeEvent QSize(1920, 1080)
MainWindow::geometryChanged(const QPoint * QPoint(-1919,32)
qt.multimedia.ffmpeg.mediadataholder: AVStream duration -9223372036854775808 is invalid. Taking it from the metadata
qt.multimedia.ffmpeg.mediadataholder: AVStream duration -9223372036854775808 is invalid. Taking it from the metadata
VideoWindow::durationChanged(qint64 duration) 0
VideoWindow::tracksChanged()
VideoWindow::hasVideoChanged(bool videoAvailable) true
VideoWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status) QMediaPlayer::LoadedMedia
VideoWindow::playbackStateChanged(QMediaPlayer::PlaybackState newState) QMediaPlayer::PlayingState
VideoWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status) QMediaPlayer::BufferingMedia
qt.multimedia.ffmpeg.demuxer: Failed to seek, pos 0
VideoWindow::positionChange(qint64 progress) 1200
*/
void VideoWindow::init()
{
    qDebug() << "VideoWindow::init()";
    /*[0] 处理视频播放界面：信号、设置*/
    connect(ui->video, &VideoWidget::mouseEnterEvent, this, &VideoWindow::mouseEnterVideo);
    connect(ui->video, &VideoWidget::mouseLeaveEvent, this, &VideoWindow::mouseLeaveVideo);
    connect(ui->video, &VideoWidget::mousePress, this, [=]()
            { mouseIsSelected(true); });

    ui->video->init();
    player = &ui->video->player;
    /*[0-1] 处理视频播放界面：信号连接*/
    //![1]
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &VideoWindow::mediaStatusChanged);
    connect(player, &QMediaPlayer::sourceChanged, this, &VideoWindow::sourceChanged);//![2]
    connect(player, &QMediaPlayer::durationChanged, this, &VideoWindow::durationChanged);//![3]
    connect(player, &QMediaPlayer::tracksChanged, this, &VideoWindow::tracksChanged);//![4]
    //![10]
    connect(player, &QMediaPlayer::hasVideoChanged, this, &VideoWindow::hasVideoChanged);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &VideoWindow::playbackStateChanged);//![11]
    //![end]
    connect(player, &QMediaPlayer::bufferProgressChanged, this, &VideoWindow::bufferProgressChanged);
    connect(player, &QMediaPlayer::errorOccurred, this, &VideoWindow::errorOccurred);

    /*[1]处理视频浮动窗口设置界面：信号、设置*/
    connect(ui->WidgetMore,&QDockWidget::topLevelChanged,this,&VideoWindow::WidgetMoreIsFloating);
    ui->WidgetMore->setTitleBarWidget(ui->WidgetMoreTitleBar);
//    ui->WidgetMore->setWidget(ui->tabWidget_Video);
    //    setContentsMargins(0, 0, 0, 0);
    // ui->WidgetMore->setWindowFlags(Qt::CustomizeWindowHint); //不可设置标题
    //    ui->WidgetMore->hide();
    //    ui->WidgetPlayerControls->hide();
    //    ui->PlayerControls->setAttribute(Qt::WA_TranslucentBackground);
    //    ui->PlayerControls->setWindowOpacity(0.6);
    //[2]处理视频控制界面信号显示信号
    connect(ui->toolButton_microphoneMute, &ToolButton::hovered, this, [this](){
        qDebug() << "ToolButton hovered";
        ui->horizontalSlider_volume->setVisible(true);
    });

//    connect(ui->horizontalSlider_volume, &Slider::mouseEnterEvent, this, [this](){

//    });
    connect(ui->horizontalSlider_volume, &Slider::mouseLeaveEvent, this, [=]{
         qDebug() << "ui->horizontalSlider_volume, &Slider::mouseLeaveEvent";
        ui->horizontalSlider_volume->setVisible(false);
    });


    /*[end]处理设置文件，配置读取初始化*/
    AppSettings.beginGroup(objectName());

    if(AppSettings.value("isFloating",false).toBool()){ui->WidgetMore->setFloating(true);}
    const auto geometry = AppSettings.value("geometry",QByteArray()).toByteArray(); // QByteArray 类型
    if (!geometry.isEmpty()){ui->WidgetMore->restoreGeometry(geometry);}
    AppJson = AppSettings.value("AppJson", QJsonObject()).toJsonObject();

    AppSettings.endGroup();


}

void VideoWindow::startShow()
{
    ui->toolButton_videoPause->setVisible(false);
    ui->horizontalSlider_volume->setVisible(false);
    //其他测试项目
    //    ui->video->player.setSource(QUrl::fromLocalFile("G:/data/雪花啤酒/test.mp4"));
    //    ui->video->player.setSource(QUrl("rtsp://admin:dacang80@192.168.1.99:554/Streaming/Channels/1"));
    //    ui->video->player.play();
//    source = QUrl("rtsp://admin:dacang80@192.168.1.99:554/Streaming/Channels/2");
    source = QUrl("G:/data/雪花啤酒/test.mp4");
    player->setSource(source);
    player->play();

}

void VideoWindow::quit()
{
    AppSettings.beginGroup(objectName());
    AppSettings.setValue("isFloating", ui->WidgetMore->isFloating());
    AppSettings.setValue("geometry", ui->WidgetMore->saveGeometry());
    AppSettings.setValue("AppJson", AppJson);
    AppSettings.endGroup();
    ui->video->quit();
    ui->WidgetMore->close();
    qDebug() << "VideoWindow::quit()";
}

void VideoWindow::mouseEnterVideo()
{
    qDebug() << "VideoWindow::mouseEnterVideo()";
}

void VideoWindow::mouseLeaveVideo()
{
    qDebug() << "VideoWindow::mouseLeaveVideo()";
}

void VideoWindow::mouseIsSelected(bool selected)
{
    if (selected)
    {
        ui->Widget->setContentsMargins(1, 1, 1, 1);
    }
    else
    {
        ui->Widget->setContentsMargins(0, 0, 0, 0);
    }
}

void VideoWindow::WidgetMoreIsFloating(bool isFloating)
{
    if(isFloating){
//        ui->WidgetMore->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window | Qt::CustomizeWindowHint);// | ui->WidgetMore->windowFlags());
        ui->WidgetMore->setWindowFlags(Qt::WindowStaysOnTopHint | ui->WidgetMore->windowFlags());
        ui->WidgetMore->setWindowOpacity(0.7);
        if(ui->WidgetMore->isHidden()){
            ui->WidgetMore->show();
            qDebug() << "VideoWindow::WidgetMoreIsFloating(bool isFloating) show();";
        }
//        ui->WidgetMore->setAllowedAreas(Qt::AllToolBarAreas);
        //        QWindow * pWin = ui->WidgetMore->windowHandle();
        //        pWin->setFlag(Qt::WindowStaysOnTopHint,true);
//        ui->WidgetMore->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
//        ui->WidgetMore->setTitleBarWidget(ui->WidgetMoreTitleBar);
        ui->WidgetMore->setContentsMargins(-20, -20, -20, -20);
    }
}

bool VideoWindow::PlayExecuteCmd(int command)
{
    switch (command) {
    case QMediaPlayer::PlayingState:

        break;
    default:
        break;
    }
    return true;
}

void VideoWindow::showEvent(QShowEvent *event)
{
    qDebug() << "第一次显示VideoWindow::showEvent(QShowEvent *" <<event<<objectName()<<AppSettings.allKeys();
    startShow();


}



void VideoWindow:: mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "VideoWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status)"<<status;
    switch (status) {
    case QMediaPlayer::NoMedia://0	The is no current media. The player is in the StoppedState.
        break;
    case QMediaPlayer::LoadingMedia://	1	正在加载当前介质。玩家可以处于任何状态。
        break;
    case QMediaPlayer::LoadedMedia://	2	当前介质已加载。 StoppedState.
        if(!ui->toolButton_videoPlay->isEnabled()){
            ui->toolButton_videoPlay->setEnabled(true);
        }
        break;
    case QMediaPlayer::StalledMedia://	3	由于缓冲不足或其他一些临时中断，当前媒体的播放已停止。The player is in the PlayingState or PausedState.
        break;
    case QMediaPlayer::BufferingMedia://	4	播放器正在缓冲数据，但已缓冲足够的数据，以便在不久的将来继续播放。The player is in the PlayingState or PausedState.
        break;
    case QMediaPlayer::BufferedMedia://	5	播放器已完全缓冲当前媒体。The player is in the PlayingState or PausedState.
        break;
    case QMediaPlayer::EndOfMedia://	6	播放已到达当前媒体的末尾。The player is in the StoppedState.
        break;
    case QMediaPlayer::InvalidMedia://	7	无法播放当前媒体。The player is in the StoppedState.
        break;
    default:
        //Defines the status of a media player's current media.
        break;
    }
}

void VideoWindow::sourceChanged(const QUrl &media)
{
    qDebug() << "VideoWindow::sourceChanged(const QUrl &"<<media;
}


void VideoWindow::durationChanged(qint64 duration)
{
    qDebug() << "VideoWindow::durationChanged(qint64 duration)"<<duration;//文件时长毫秒
    if(duration>0){
        ui->horizontalSlider_position->setVisible(true);
        ui->label_videoProgress->setVisible(true);
        connect(player, &QMediaPlayer::positionChanged, this, &VideoWindow::positionChange); //rstp直播流需要取消
        connect(ui->horizontalSlider_position,&QSlider::sliderMoved,player,&QMediaPlayer::setPosition);

        m_duration = duration / 1000;
        ui->horizontalSlider_position->setMaximum(duration);
    }else{
        ui->horizontalSlider_position->setVisible(false);
        ui->label_videoProgress->setVisible(false);
        disconnect(player, &QMediaPlayer::positionChanged, this, &VideoWindow::positionChange); //rstp直播流需要取消
        disconnect(ui->horizontalSlider_position,&QSlider::sliderMoved,player,&QMediaPlayer::setPosition);
        m_duration = 0;
    }
}

void VideoWindow::tracksChanged()
{
    qDebug() << "VideoWindow::tracksChanged()";
}


void VideoWindow::hasVideoChanged(bool videoAvailable)
{
    qDebug() << "VideoWindow::hasVideoChanged(bool videoAvailable)"<<videoAvailable;
    //播放、暂停、停止、全屏
    ui->toolButton_videoPlay->setEnabled(videoAvailable);
    ui->toolButton_videoPause->setEnabled(videoAvailable);
    ui->toolButton_videoStop->setEnabled(videoAvailable);
    ui->toolButton_fullScreen->setEnabled(videoAvailable);
}

void VideoWindow::playbackStateChanged(QMediaPlayer::PlaybackState newState)
{
    qDebug() << "VideoWindow::playbackStateChanged(QMediaPlayer::PlaybackState newState)"<<newState;
    switch (newState) {
    case QMediaPlayer::StoppedState://	0	The media player is not playing content, playback will begin from the start of the current track.
        ui->toolButton_videoPlay->setVisible(true);
        ui->toolButton_videoPause->setVisible(false);
        break;
    case QMediaPlayer::PlayingState://	1	The media player is currently playing content. This indicates the same as the playing property.
        ui->toolButton_videoPlay->setVisible(false);
        ui->toolButton_videoPause->setVisible(true);
        break;
    case QMediaPlayer::PausedState://	2	The media player has paused playback, playback of the current track will resume from the position the player was paused at.
        ui->toolButton_videoPlay->setVisible(true);
        ui->toolButton_videoPause->setVisible(false);
        break;
    default:
        break;
    }

}


void VideoWindow::positionChange(qint64 progress)
{
    //qDebug() << "VideoWindow::positionChange(qint64 progress)"<<progress;
    if (!ui->horizontalSlider_position->isSliderDown())
    {
        ui->horizontalSlider_position->setValue(progress);
    }
}


void VideoWindow::bufferProgressChanged(float filled)
{
    qDebug() << "VideoWindow::bufferProgressChanged(float filled)"<<filled;
}



void VideoWindow::errorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    qDebug() << "VideoWindow::errorOccurred(QMediaPlayer::Error error, const QString &errorString)"<<error<<errorString;
}

void VideoWindow::on_Button_moreWidget_isFloatable_clicked()
{
//    if (ui->dockWidgetContents_WidgetMore->isHidden())
//    {
//        ui->dockWidgetContents_WidgetMore->show();
//    }
//    else
//    {
//        ui->dockWidgetContents_WidgetMore->hide();
//    }

    if (ui->WidgetMore_tabWidget_Video->isVisible())
    {
        ui->WidgetMore_tabWidget_Video->setVisible(false);
    }
    else
    {
        ui->WidgetMore_tabWidget_Video->setVisible(true);
    }
//ui->WidgetMore->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
//    if(ui->WidgetMore->isFloating()){
//        ui->WidgetMore->setFloating(false);
//    }else {
//        ui->WidgetMore->setFloating(true);
//    }
}

void VideoWindow::on_toolButton_videoPlay_clicked()
{
    player->play();
}


void VideoWindow::on_toolButton_videoPause_clicked()
{
    player->pause();
}


void VideoWindow::on_toolButton_videoStop_clicked()
{
    player->stop();
}


void VideoWindow::on_toolButton_fullScreen_clicked()
{
    ui->video->setFullScreen(true);
}

void VideoWindow::updateDurationInfo(qint64 currentInfo)
{
    QString sStr;
    if (currentInfo || m_duration)
    {
        QTime currentTime((currentInfo / 3600) % 60, (currentInfo / 60) % 60, currentInfo % 60,
                          (currentInfo * 1000) % 1000);
        QTime totalTime((m_duration / 3600) % 60, (m_duration / 60) % 60, m_duration % 60,
                        (m_duration * 1000) % 1000);

        QString format = "mm:ss";
        if (m_duration > 3600)
            format = "hh:mm:ss";
        sStr = currentTime.toString(format) + "/" + totalTime.toString(format);
    }
    ui->label_videoProgress->setText(sStr);
}

#include <QToolTip>
void VideoWindow::sliderMovedForPlayer(int value)
{
    int minutes = value / 60000; // 每分钟60秒，每秒1000毫秒
    int seconds = (value / 1000) % 60;

    QToolTip::showText(
        ui->horizontalSlider_position->mapToGlobal(QPoint(value, 0)),
        QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')),
        ui->horizontalSlider_position);
}


void VideoWindow::on_toolButton_moreSetting_clicked()
{
    if(ui->WidgetMore->isVisible()){
        ui->WidgetMore->setVisible(false);
    }else{
        ui->WidgetMore->setVisible(true);
    }
}


void VideoWindow::on_horizontalSlider_position_valueChanged(int value)
{
    if (ui->horizontalSlider_position->isSliderDown()){
        qDebug() << "VideoWindow::on_horizontalSlider_position_valueChanged(int "<<value;

    }
    updateDurationInfo(value / 1000);

}


void VideoWindow::on_horizontalSlider_position_sliderMoved(int position)
{
    qDebug() << "VideoWindow::on_horizontalSlider_position_sliderMoved(int "<<position;
    int minutes = position / 60000; // 每分钟60秒，每秒1000毫秒
    int seconds = (position / 1000) % 60;
    QPoint sliderPos = ui->horizontalSlider_position->mapToGlobal(QPoint(0, 0)); // 获取进度条的全局位置
    QPoint cursorPos = QCursor::pos(); // 获取当前鼠标的全局位置
    qDebug() <<sliderPos<<cursorPos;
    QToolTip::showText(
        QPoint(cursorPos.x(),sliderPos.y() - 42),
        QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')),
        ui->horizontalSlider_position);
}

void VideoWindow::on_toolButton_microphoneMute_clicked()
{
    if(player->audioOutput()->isMuted()){
        player->audioOutput()->setMuted(false);
        //取消静音图标
        ui->toolButton_microphoneMute->setIcon(QIcon(":/asset/playerControls/MicrophoneMute.svg"));
    }else{
        player->audioOutput()->setMuted(true);
        //静音图标
        ui->toolButton_microphoneMute->setIcon(QIcon(":/asset/playerControls/Microphone.svg"));

    }
}

void VideoWindow::on_horizontalSlider_volume_valueChanged(int value)
{
    player->audioOutput()->setVolume(value);
}

