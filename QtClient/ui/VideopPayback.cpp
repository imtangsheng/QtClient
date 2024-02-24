#include "VideoPlayback.h"
#include "ui_VideoPlayback.h"

//#include "public/AppData.h"

VideoPlayback::VideoPlayback(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoPlayback)
{
    ui->setupUi(this);
    this->installEventFilter(this);
//    this->resizeEvent();
//    connect(this,&QWidget::resizeEvent,this,&VideoPlayback::resizeEvent1);
    init();
}

VideoPlayback::~VideoPlayback()
{
    delete ui;
    qDebug()<<"~VideoPlayback()";
}

void VideoPlayback::init()
{
    qDebug()<<"VideoPlayback::init()";
    m_player = new QMediaPlayer;
    connect(m_player,&QMediaPlayer::mediaStatusChanged,this,&VideoPlayback::mediaStateChanged);
    connect(m_player,&QMediaPlayer::playbackStateChanged,this,&VideoPlayback::playerStateChanged);
    connect(m_player,&QMediaPlayer::hasVideoChanged,this,&VideoPlayback::hasVideoChanged);


    videoWidget = new QVideoWidget;
    videoWidget->installEventFilter(this);
    videoWidget->setStyleSheet("background:gray;");
    videoWidget->setParent(this);
    videoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);
//    connect(videoWidget,&QVideoWidget::customContextMenuRequested(
//    QPalette palette;
////    palette.setColor(QPalette::Window,Qt::red);
//    QPixmap backgroundImage(":/asset/logo.ico");
////    palette.setBrush(QPalette::Background,backgroundImage);
//    palette.setBrush(QPalette::Base, backgroundImage);
//    videoWidget->setAutoFillBackground(true);
//    videoWidget->setPalette(palette);

    ui->layout_videoplay->addWidget(videoWidget,1);
//    ui->layout_videoplay->addWidget(ui->tabWidget,2);
    videoWidget->hide();
    audioOutput = new QAudioOutput;

    m_player->setVideoOutput(videoWidget);
    m_player->setAudioOutput(audioOutput);

    m_player->setSource(QUrl::fromLocalFile("test.mp4"));


//    m_player->play();



//    videoWidget->show();
    qDebug()<<"play_clicked"<<m_player->isAvailable()<<m_player->hasVideo();
    hasVideoChanged(m_player->hasVideo());
//    ui->pushButton_pause->hide();
//    ui->frame->setVisible(false);


    m_playlistInfo =new QRect(50,50,100,150);

}

void VideoPlayback::mediaStateChanged(QMediaPlayer::MediaStatus mediaState)
{
    qDebug()<<mediaState;
}


void VideoPlayback::playerStateChanged()
{
//    qDebug()<<m_player->PlayingState;
//    qDebug()<<m_player->playbackState();

    switch (m_player->playbackState())
    {
    case QMediaPlayer::StoppedState:
        qDebug()<<"Player"<<m_player->StoppedState;
//        videoWidget->setVisible(false);
        if(!videoWidget->isHidden()){
            videoWidget->hide();
        }
        if(ui->label_videoBackground->isHidden()){
            ui->label_videoBackground->show();
        }
        if(ui->pushButton_play->isHidden()){
            ui->pushButton_pause->hide();
            ui->pushButton_play->show();
        }
        break;
    case QMediaPlayer::PlayingState:
        qDebug()<<"Player"<<m_player->PlayingState;
        ui->pushButton_play->hide();
        ui->pushButton_pause->show();
        if(videoWidget->isHidden()){
            videoWidget->show();
        }
        if(!ui->label_videoBackground->isHidden()){
            ui->label_videoBackground->hide();
        }
        break;
    case QMediaPlayer::PausedState:
        qDebug()<<"Player"<<m_player->PausedState;
        ui->pushButton_play->show();
        ui->pushButton_pause->hide();
        break;
    default:
        qDebug()<<"QMediaPlayer未知状态";
        break;
    }
}

void VideoPlayback::hasVideoChanged(bool videoAuailable)
{
    qDebug()<<"videoAuailable:"<<videoAuailable;
    if(videoAuailable){
        ui->pushButton_play->setIcon(QIcon(":/asset/media/VideoPlay.svg"));
//        ui->pushButton_pause->setIcon(QIcon(":/asset/media/VideoPause.svg"));
//        ui->pushButton_pause->show();
    }else{
        ui->pushButton_play->setIcon(QIcon(":/asset/media/VideoPlay_notAvailable.svg"));
//        ui->pushButton_pause->setIcon(QIcon(":/asset/media/VideoPause_notAvailable.svg"));
        ui->pushButton_pause->hide();

    }
}

QWidget *VideoPlayback::getVideoPaly()
{
    return ui->widget_videoPlay;
}
#include <QMessageBox>
void VideoPlayback::on_pushButton_play_clicked()
{
    qDebug()<<"play_clicked"<<m_player->isAvailable()<<m_player->hasVideo();
    if(m_player->isAvailable() && m_player->hasVideo() ){
        qDebug()<<"play_clicked video file ";
        mediaControls(MediaControls::Play);
    }else {
        qDebug()<<"play_clicked not video file ";
        QMessageBox::critical(this, "Error", "video file not available");
    }

}

void VideoPlayback::on_pushButton_pause_clicked()
{
    qDebug()<<"pause_clicked";
    mediaControls(MediaControls::Pause);

}

void VideoPlayback::on_pushButton_stop_clicked()
{
    mediaControls(MediaControls::Stop);
}



void VideoPlayback::on_horizontalSlider_playbackRate_valueChanged(int value)
{
    qDebug()<<"playbackRate_valueChanged:"<<value/10<<value/10.0;
    ui->label_playbackRate->setText(QString::number(value/10.0)+"x");
    m_player->setPlaybackRate(value/10.0);
}

void VideoPlayback::on_horizontalSlider_playbackVolume_valueChanged(int value)
{
    qDebug()<<"playbackVolume_valueChanged:"<<value/10<<value/10.0;
    audioOutput->setVolume(value);
}

void VideoPlayback::on_pushButton_fullScreen_clicked()
{
    qDebug()<<"on_pushButton_fullScreen_clicked:"<<m_player->isPlaying()<<m_player->hasVideo();
    if(m_player->isPlaying()){
        videoWidget->setFullScreen(true);
    }


}

bool VideoPlayback::eventFilter(QObject *object, QEvent *event)
{
//    qDebug()<<"eventFilter:"<<object<<event->type();

    if(object == videoWidget){
        switch (event->type()) {
        case QEvent::MouseButtonPress:
            if(videoWidget->isFullScreen()){
                videoWidget->setFullScreen(false);
                return true;
            break;
        case QEvent::Resize:
            qDebug()<<"eventFilter resize:"<<videoWidget->geometry();
            break;

        default:
            break;
        }
//        qDebug()<<"object:"<<object<<videoWidget;
        return true;

        }
    }

    return QObject::eventFilter(object,event);
}

void VideoPlayback::resizeEvent(QResizeEvent *event)
{
    qDebug()<<"resizeEvent:"<<event;
    QWidget::resizeEvent(event);
}

bool VideoPlayback::mediaControls(int control)
{
    switch (control) {
    case MediaControls::Play:
        m_player->play();
        break;
    case MediaControls::Pause:
        m_player->pause();
        break;
    case MediaControls::Stop:
        m_player->stop();
        break;
    default:
        break;
    }

    return true;
}

void VideoPlayback::on_pushButton_test_clicked()
{
    qDebug()<<"test";
    if(ui->frame->isVisible()){
        ui->frame->lower();
        ui->dockWidget->hide();
        ui->frame->setVisible(false);
    }else {
        qDebug()<<"test2";
        videoWidget->lower();

//        ui->widget_videoPlay->layout()->addWidget(ui->frame);
        ui->frame->setVisible(true);
        ui->frame->show();
        ui->frame->raise();
        ui->dockWidget->show();
        ui->dockWidget->setFloating(true);
        ui->frame->setWindowFlags(ui->frame->windowFlags()|Qt::WindowStaysOnTopHint);
        ui->frame->setWindowFlags(ui->frame->windowFlags()|Qt::Dialog|Qt::FramelessWindowHint);
//        ui->frame->setWindowFlags(Qt::FramelessWindowHint);
        ui->frame->setAttribute(Qt::WA_TranslucentBackground);
        ui->frame->setWindowOpacity(0.8);
        ui->frame->activateWindow();
        QPoint videoPos = videoWidget->mapFromGlobal(QPoint(0,0));

        qDebug()<<"test3"<<videoWidget->x()<<videoWidget->y()<<videoWidget->width()<<videoWidget->height();
//        ui->frame->setGeometry(this->parentWidget()->x()+this->parentWidget()->width()-ui->frame->width(),this->parentWidget()->y()+58,ui->frame->width(),videoWidget->height());
        ui->frame->setGeometry(videoWidget->width() - videoPos.x() - ui->frame->width(),-videoPos.y(),ui->frame->width(),videoWidget->height());
        qDebug()<<"test4"<<this->parentWidget()->x()<<this->parentWidget()->y()<<this->parentWidget()->width()<<this->parentWidget()->height();
        qDebug()<<"test3"<<this->x()<<this->y()<<this->width()<<this->height();

        qDebug()<<"testx:"<<videoPos.x()<<videoPos.y();
        qDebug()<<ui->frame->geometry();
        ui->frame->raise();
        ui->frame->show();
        ui->frame->setParent(this->parentWidget());
        m_playlistInfo->setRight(this->videoWidget->x());
        m_playlistInfo->setTop(this->videoWidget->y());

    }

//    m_player->setSource(QUrl::fromLocalFile("test.mp4"));
}
