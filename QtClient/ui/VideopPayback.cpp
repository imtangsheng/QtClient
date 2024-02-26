#include "VideoPlayback.h"
#include "ui_VideoPlayback.h"

#include "../public/AppData.h"

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
    initSettingPlayback();
    init_filesView();
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

void VideoPlayback::updataMoreWidgetGeometry()
{
    qDebug()<<"showVideoPlaybacckOperationWidget()"<<ui->dockWidget_videoPlayback->geometry();
    QPoint videoPos = videoWidget->mapFromGlobal(QPoint(0,0));
    qDebug()<<"videoWidget:"<<videoWidget->x()<<videoWidget->y()<<videoWidget->width()<<videoWidget->height();
    qDebug()<<"this:"<<this->x()<<this->y()<<this->width()<<this->height();
    qDebug()<<"parentWidget:"<<this->parentWidget()->x()<<this->parentWidget()->y()<<this->parentWidget()->width()<<this->parentWidget()->height();
    qDebug()<<"videoPos:"<<videoPos.x()<<videoPos.y();
    ui->dockWidget_videoPlayback->setGeometry(0, 30,ui->dockWidget_videoPlayback->width(),videoWidget->height());

//    ui->dockWidget_videoPlayback->setGeometry(videoWidget->width() - videoPos.x() - ui->dockWidget_videoPlayback->width(), - videoPos.y(),ui->dockWidget_videoPlayback->width(),videoWidget->height());
//        ui->dockWidget_videoPlayback->setGeometry(videoWidget->width() - videoPos.x() - ui->dockWidget_videoPlayback->width(), -videoPos.y(),ui->dockWidget_videoPlayback->width(),videoWidget->height());

    qDebug()<<ui->dockWidget_videoPlayback->geometry();

}

QWidget *VideoPlayback::getVideoPaly()
{
    return ui->widget_videoPlay;
}
#include <QFileDialog>
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

void VideoPlayback::on_pushButton_operation_clicked()
{
    qDebug()<<"on_pushButton_operation_clicked()";
    if(ui->dockWidget_videoPlayback->isVisible()){
        qDebug()<<"dockWidget_videoPlayback->setVisible(false);";
//        ui->dockWidget_videoPlayback->lower();
        ui->dockWidget_videoPlayback->hide();
        ui->dockWidget_videoPlayback->setVisible(false);
    }else {
        qDebug()<<"showVideoPlaybacckOperationWidget()1";
        updataMoreWidgetGeometry();
//        videoWidget->lower();
//        ui->dockWidget_videoPlayback->raise();
        ui->dockWidget_videoPlayback->show();
    }


}

void VideoPlayback::on_pushButton_openFile_clicked()
{
    // 创建打开文件对话框
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile); // 设置对话框模式为选择已存在的文件

    QString filePath = fileDialog.getOpenFileName(this, "Open File"); // 获取选择的文件路径
    if (!filePath.isEmpty()) {
        // 在这里处理打开文件的逻辑
        qDebug() << "Selected file: " << filePath;
        m_player->setSource(QUrl::fromLocalFile(filePath));
    }

}

void VideoPlayback::on_comboBox_videoFiles_currentIndexChanged(int index)
{
    qDebug()<<"on_comboBox_videoFiles_currentIndexChanged(int index)"<<index;
}

void VideoPlayback::on_comboBox_videoFiles_editTextChanged(const QString &arg1)
{
    qDebug()<<"on_comboBox_videoFiles_editTextChanged(const QString &arg1)"<<arg1;
}

void VideoPlayback::on_pushButton_settings_clicked()
{
    qDebug()<<"on_pushButton_settings_clicked()";
    on_pushButton_more_clicked();
    ui->tabWidget_videoPlayMore->setCurrentWidget(ui->tab_videoSetting);
}

void VideoPlayback::on_pushButton_more_clicked()
{
    qDebug()<<"on_pushButton_more_clicked()";
    if(!ui->dockWidget_videoPlayback->isHidden()){
        qDebug()<<"ui->dockWidget_videoPlayback->setVisible(false);";
        ui->dockWidget_videoPlayback->lower();
        ui->dockWidget_videoPlayback->hide();
//        ui->dockWidget_videoPlayback->setVisible(false);
    }else{
        qDebug()<<"ui->dockWidget_videoPlayback->setVisible(true);";
//        ui->dockWidget_videoPlayback->setParent(this->parentWidget()); //设置了才可以显示
        videoWidget->setWindowFlags(Qt::FramelessWindowHint);
        videoWidget->lower();
        ui->dockWidget_videoPlayback->setFloating(true);
        ui->dockWidget_videoPlayback->raise(); //刷新后被重置了,调用前需要使用设置一次setCentralWidget(ui->tabWidget_mainWindow);，后面就可以不用了，但是不知道为什么第一次会不行
//
        updataMoreWidgetGeometry();
        ui->dockWidget_videoPlayback->show();
//        ui->dockWidget_videoPlayback->setVisible(true);
    }
}

void VideoPlayback::on_pushButton_playVideoFile_clicked()
{
    qDebug()<<"on_pushButton_playVideoFile_clicked()";
    ui->treeView_videoFiles->setRootIndex(m_fileModel_videoPlayback->index(ui->lineEdit_rootPath->text()));
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

void VideoPlayback::initSettingPlayback()
{
    qDebug()<<"VideoPlayback::init()";
    /**视频播放界面相关属性初始化**/
    m_player = new QMediaPlayer;
    connect(m_player,&QMediaPlayer::mediaStatusChanged,this,&VideoPlayback::mediaStateChanged);
    connect(m_player,&QMediaPlayer::playbackStateChanged,this,&VideoPlayback::playerStateChanged);
    connect(m_player,&QMediaPlayer::hasVideoChanged,this,&VideoPlayback::hasVideoChanged);

    videoWidget = new QVideoWidget;
    videoWidget->installEventFilter(this);
    videoWidget->setStyleSheet("background:gray;");
    videoWidget->setParent(this);
    videoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);
    videoWidget->setWindowFlags(Qt::FramelessWindowHint);
    //    connect(videoWidget,&QVideoWidget::customContextMenuRequested(
    //    QPalette palette;
    ////    palette.setColor(QPalette::Window,Qt::red);
    //    QPixmap backgroundImage(":/asset/logo.ico");
    ////    palette.setBrush(QPalette::Background,backgroundImage);
    //    palette.setBrush(QPalette::Base, backgroundImage);
    //    videoWidget->setAutoFillBackground(true);
    //    videoWidget->setPalette(palette);

    ui->layout_videoplay->addWidget(videoWidget,1);
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
    qDebug()<<"showVideoPlaybacckOperationWidget()";
    /**浮动窗口初始化**/
//    ui->dockWidget_videoPlayback->setParent(this->parentWidget()); //设置了才可以显示
    ui->dockWidget_videoPlayback->setParent(nullptr);
//    ui->dockWidget_videoPlayback->raise(); //设置了才可以显示在其界面上
//    ui->dockWidget_videoPlayback->setParent(this);//不可以非主窗口
    ui->dockWidget_videoPlayback->setWindowOpacity(0.8);
//    QPoint videoPos = videoWidget->mapFromGlobal(QPoint(0,0));
//    qDebug()<<"videoWidget:"<<videoWidget->x()<<videoWidget->y()<<videoWidget->width()<<videoWidget->height();
//    qDebug()<<"this:"<<this->x()<<this->y()<<this->width()<<this->height();
//    qDebug()<<"parentWidget:"<<this->parentWidget()->x()<<this->parentWidget()->y()<<this->parentWidget()->width()<<this->parentWidget()->height();
//    qDebug()<<"videoPos:"<<videoPos.x()<<videoPos.y();
    ui->dockWidget_videoPlayback->setAttribute(Qt::WA_TranslucentBackground);
//    ui->dockWidget_videoPlayback->setGeometry(0, 0,ui->dockWidget_videoPlayback->width(),videoWidget->height());
//    ui->dockWidget_videoPlayback->setVisible(true);
//    ui->dockWidget_videoPlayback->raise(); //刷新后被重置了 需要设置一下这个 setCentralWidget
//    ui->dockWidget_videoPlayback->setVisible(false);
    ui->dockWidget_videoPlayback->hide();


}

void VideoPlayback::init_filesView()
{
    qDebug()<<"init_filesView()";
    // 目录树模型
    m_fileModel_videoPlayback = new QFileSystemModel;

    m_fileModel_videoPlayback->setRootPath(QDir::rootPath());
//    m_fileModel_videoPlayback->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    QStringList filters;
    filters << "*.mp4" << "*.avi";
    m_fileModel_videoPlayback->setNameFilters(filters);
    m_fileModel_videoPlayback->setNameFilterDisables(false);
    // 目录树视图

    ui->treeView_videoFiles->setModel(m_fileModel_videoPlayback);

    ui->treeView_videoFiles->setWindowTitle("Directories video");
    // 设置文件浏览视图的根目录

    ui->lineEdit_rootPath->setText(EXE_CONFIG["pathVideoFiles"].toString());
    ui->treeView_videoFiles->setRootIndex(m_fileModel_videoPlayback->index(ui->lineEdit_rootPath->text()));
//    ui->treeView_videoFiles->setRootIndex(m_fileModel_videoPlayback->index("G:/data/雪花啤酒"));

//    QModelIndexList indexes = m_fileModel_videoPlayback->match(
//        m_fileModel_videoPlayback->index(ui->lineEdit_rootPath->text()),
//        Qt::DisplayRole, filters, 1, Qt::MatchFilterSuffix);

//    if(indexes.isEmpty()) {
//        // 过滤后没有匹配项
//        return;
//    }

//    m_fileModel_videoPlayback->setRootIndex(indexes.first());


    // 当前目录变化信号
    connect(ui->treeView_videoFiles->selectionModel(),&QItemSelectionModel::currentChanged,this,&VideoPlayback::fileModelSelection);
    connect(ui->treeView_videoFiles, &QTreeView::doubleClicked,this, &VideoPlayback::fileBrowserDoubleClicked);
    //    connect(ui->treeView_files->selectionModel(), &QTreeView::doubleClicked,this, &VideoPlayback::fileBrowserDoubleClicked);


}

void VideoPlayback::fileModelSelection(QModelIndex index)
{
    qDebug()<<"fileModelSelection(QModelIndex index)"<<index;
    qDebug()<<index.data(QFileSystemModel::FilePathRole).toString();
    m_currentFilePathDir = index.data(QFileSystemModel::FilePathRole).toString();
}

void VideoPlayback::fileBrowserDoubleClicked(QModelIndex index)
{
    qDebug()<<"fileBrowserDoubleClicked(QModelIndex index)"<<index;
    qDebug()<<index.data(QFileSystemModel::FilePathRole).toString();
    m_currentFilePathDir = index.data(QFileSystemModel::FilePathRole).toString();

    m_player->setSource(QUrl::fromLocalFile(m_currentFilePathDir));

}

void VideoPlayback::on_pushButton_test_clicked()
{
    qDebug()<<"test";
    if(ui->frame->isVisible()){
        ui->frame->lower();
        ui->dockWidget_videoPlayback->hide();
        ui->frame->setVisible(false);
    }else {
        qDebug()<<"test2";
        videoWidget->lower();
        ui->frame->show();
        qDebug()<<"show1()";
//        ui->widget_videoPlay->layout()->addWidget(ui->frame);
        ui->frame->setVisible(true);
        ui->dockWidget_videoPlayback->show();
        //ui->dockWidget->setFloating(true);
        ui->frame->setWindowFlags(ui->frame->windowFlags()|Qt::WindowStaysOnTopHint);
//        ui->frame->setWindowFlags(ui->frame->windowFlags()|Qt::Dialog|Qt::FramelessWindowHint);
//        ui->frame->setWindowFlags(Qt::FramelessWindowHint);
        ui->frame->setAttribute(Qt::WA_TranslucentBackground);
        ui->frame->setWindowOpacity(0.8);
        ui->frame->activateWindow();
        QPoint videoPos = videoWidget->mapFromGlobal(QPoint(0,0));

        qDebug()<<"test3"<<videoWidget->x()<<videoWidget->y()<<videoWidget->width()<<videoWidget->height();
        ui->frame->setGeometry(0,0,ui->frame->width(),videoWidget->height());
//        ui->frame->setGeometry(this->parentWidget()->x()+this->parentWidget()->width()-ui->frame->width(),this->parentWidget()->y()+58,ui->frame->width(),videoWidget->height());
//        ui->frame->setGeometry(videoWidget->width() - videoPos.x() - ui->frame->width(),-videoPos.y(),ui->frame->width(),videoWidget->height());
        qDebug()<<"test4"<<this->parentWidget()->x()<<this->parentWidget()->y()<<this->parentWidget()->width()<<this->parentWidget()->height();
        qDebug()<<"test3"<<this->x()<<this->y()<<this->width()<<this->height();

        qDebug()<<"testx:"<<videoPos.x()<<videoPos.y();
        qDebug()<<ui->frame->geometry();
        ui->frame->raise();
        ui->frame->show();
        qDebug()<<"show end()";
        ui->frame->setParent(this->parentWidget());
//        m_playlistInfo->setRight(this->videoWidget->x());
//        m_playlistInfo->setTop(this->videoWidget->y());

    }

//    m_player->setSource(QUrl::fromLocalFile("test.mp4"));
}
