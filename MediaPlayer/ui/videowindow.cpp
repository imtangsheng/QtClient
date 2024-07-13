#include "videowindow.h"
#include "AppOS.h"

#include <QWindow>
#include <QFileDialog>
#include <QToolTip>
#include <QDir>
#include <QDir>
#include <QCalendarWidget>

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
第一次显示VideoWindow::showEvent(QShowEvent * 0x722f1fec40 "VideoWindow" QList("VideoWindow/MediaPlayerJson", "VideoWindow/geometry", "VideoWindow/isFloating")
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
    connect(player, &QMediaPlayer::sourceChanged, this, &VideoWindow::sourceChanged);     //![2]
    connect(player, &QMediaPlayer::durationChanged, this, &VideoWindow::durationChanged); //![3]
    connect(player, &QMediaPlayer::tracksChanged, this, &VideoWindow::tracksChanged);     //![4]
    //![10]
    connect(player, &QMediaPlayer::hasVideoChanged, this, &VideoWindow::hasVideoChanged);
    connect(player, &QMediaPlayer::playbackStateChanged, this, &VideoWindow::playbackStateChanged); //![11]
    //![end]
    connect(player, &QMediaPlayer::bufferProgressChanged, this, &VideoWindow::bufferProgressChanged);
    connect(player, &QMediaPlayer::errorOccurred, this, &VideoWindow::errorOccurred);

    /*[1]处理视频浮动窗口设置界面：信号、设置*/
    connect(ui->WidgetMore, &QDockWidget::topLevelChanged, this, &VideoWindow::WidgetMoreIsFloating);
    ui->WidgetMore->setTitleBarWidget(ui->WidgetMoreTitleBar);
    //    ui->WidgetMore->setWidget(ui->tabWidget_Video);
    //    setContentsMargins(0, 0, 0, 0);
    // ui->WidgetMore->setWindowFlags(Qt::CustomizeWindowHint); //不可设置标题
    //    ui->WidgetMore->hide();
    //    ui->WidgetPlayerControls->hide();
    //    ui->PlayerControls->setAttribute(Qt::WA_TranslucentBackground);
    //    ui->PlayerControls->setWindowOpacity(0.6);
    //[2]处理视频控制界面信号显示信号
    connect(ui->toolButton_microphoneMute, &ToolButton::hovered, this, [this]()
            {
        qDebug() << "ToolButton hovered";
        ui->horizontalSlider_volume->setVisible(true); });

    //    connect(ui->horizontalSlider_volume, &Slider::mouseEnterEvent, this, [this](){

    //    });
    connect(ui->horizontalSlider_volume, &Slider::mouseLeaveEvent, this, [=]
            {
         qDebug() << "ui->horizontalSlider_volume, &Slider::mouseLeaveEvent";
        ui->horizontalSlider_volume->setVisible(false); });

    ui->toolButton_videoPause->setVisible(false);
    ui->horizontalSlider_volume->setVisible(false);

    /*视频文件日期设置界面*/
    ui->dateEdit_videofiles->setDate(QDate::currentDate());
    ui->dateEdit_videofiles->calendarWidget()->setWeekdayTextFormat(Qt::Saturday,QTextCharFormat());
    ui->dateEdit_videofiles->calendarWidget()->setWeekdayTextFormat(Qt::Sunday,QTextCharFormat());
    ui->dateEdit_videofiles->calendarWidget()->setDateTextFormat(QDate(),QTextCharFormat());

    /*[end]处理设置文件，配置读取初始化*/
    MediaPlayerSettings.beginGroup(objectName());

    if (MediaPlayerSettings.value("isFloating", false).toBool())
    {
        ui->WidgetMore->setFloating(true);
    }
    const auto geometry = MediaPlayerSettings.value("geometry", QByteArray()).toByteArray(); // QByteArray 类型
    if (!geometry.isEmpty())
    {
        ui->WidgetMore->restoreGeometry(geometry);
    }
    MediaPlayerJson = MediaPlayerSettings.value("AppJson", QJsonObject()).toJsonObject();
    playHistoryJson = MediaPlayerSettings.value("PlayHistory", QJsonObject()).toJsonObject();
    MediaPlayerSettings.endGroup();

    if(playHistoryJson.contains("currentFilePath")){
        // 检查文件路径是否存在
        QString filesPath = playHistoryJson["currentFilePath"].toString();
        QFileInfo fileInfo(filesPath);
        if (!fileInfo.exists() || !fileInfo.isDir()) {
            // 如果文件路径不存在或不是目录,则输出错误提示并返回
            playHistoryJson.remove("currentFilePath");
            qDebug() << "Error: File path does not exist or is not a directory."<<filesPath;
        }else{
            set_videoFilesListLocal_and_tableWidget_playerList_byFilePath(filesPath);
        }
    }
}

void VideoWindow::startShow()
{
    qDebug() << "VideoWindow::startShow()" << ui->comboBox_updatePlayerList->count() << ui->comboBox_playerPath->count();
    /*[1]视频播放列表配置初始化读取*/
    //    ui->tableWidget_playerList->resizeColumnsToContents();
    //    playerListUpdate();
    QJsonObject playerList = MediaPlayerJson["playerList"].toObject();
    ui->comboBox_playerPath->clear();
    ui->comboBox_updatePlayerList->clear();

    foreach (QString key, playerList.keys())
    {
        qDebug() << "VideoWindow::playerListUpdate(): key-value" << key << playerList[key].toString();
        // 会发送currentTextChanged信号，报错index out of range
        ui->comboBox_playerPath->addItem(key, playerList[key].toString());
        ui->comboBox_updatePlayerList->addItem(key, playerList[key].toString());
    }

    /*[2]视频播放配置初始化读取*/
    //    AppJson["playPageStep"]
    if (MediaPlayerJson.contains("playPageStep"))
    {
        ui->lineEdit_playPageStep->setText(i2s(MediaPlayerJson["playPageStep"].toInt(10*1000) / 1000));
    }
    ui->lineEdit_playPageStep->setValidator(new QIntValidator(ui->lineEdit_playPageStep));

    ui->checkBox_autoPlay->setChecked(MediaPlayerJson["AutoPlay"].toBool(false));

    if (MediaPlayerJson.contains("playSource"))
    {
        ui->lineEdit_playSource->setText(MediaPlayerJson["playSource"].toString());
        player->setSource(QUrl(MediaPlayerJson["playSource"].toString()));
        if (MediaPlayerJson["AutoPlay"].toBool(false))
        {
            player->play();
        }
    }

    if (MediaPlayerJson.contains("fileExtensions"))
    {
        ui->lineEdit_fileExtensions->setText(MediaPlayerJson["fileExtensions"].toString(".mp4|.MP4"));
    }

    if (MediaPlayerJson.contains("volume"))
    {
        ui->horizontalSlider_volume->setValue(MediaPlayerJson["volume"].toInt(100));
    }

    ui->toolButton_listNext->setVisible(false);
    ui->toolButton_listPrevious->setVisible(false);
    ui->toolButton_AutoShow->setVisible(false);
    ui->toolButton_fixedWidget->setVisible(false);
}

void VideoWindow::quit()
{
    MediaPlayerSettings.beginGroup(objectName());
    MediaPlayerSettings.setValue("isFloating", ui->WidgetMore->isFloating());
    MediaPlayerSettings.setValue("geometry", ui->WidgetMore->saveGeometry());
    MediaPlayerSettings.setValue("AppJson", MediaPlayerJson);
    MediaPlayerSettings.setValue("PlayHistory", playHistoryJson);
    MediaPlayerSettings.endGroup();
    ui->video->quit();
    ui->WidgetMore->close();
    qDebug() << "VideoWindow::quit()";
}

bool VideoWindow::startPlay(const QString &file)
{
    qDebug() << "VideoWindow::startPlay(const QString &" << file<<currentPlaySource;
    //[1]!保存历史记录
    QJsonObject playJson = playHistoryJson[currentPlaySource].toObject();
    qDebug() << "保存历史记录:"<<player->position() <<player->duration();
    if (player->position() < player->duration() && player->position() > 0){
        playJson["position"] = player->position();
    }else{
        playJson["position"] = 0;
    }
    playHistoryJson[currentPlaySource] = playJson;
    qDebug() << playJson;
    //[2]
    currentPlaySource = file;
    if (currentPlaySource.startsWith("http://") || currentPlaySource.startsWith("https://"))
    {
        player->setSource(QUrl(currentPlaySource)); // 网络流
    }
    else if (currentPlaySource.startsWith("rtsp:"))
    {
        player->setSource(QUrl(currentPlaySource)); // rtsp:视频流协议
        ui->horizontalSlider_position->setVisible(false);
        ui->label_videoProgress->setVisible(false);
        disconnect(player, &QMediaPlayer::positionChanged, this, &VideoWindow::positionChange); // rtsp直播流需要取消
        disconnect(ui->horizontalSlider_position, &QSlider::sliderMoved, player, &QMediaPlayer::setPosition);
    }
    else
    {
        // 本地文件
        player->setSource(QUrl::fromLocalFile(currentPlaySource));
    }
    ui->lineEdit_playSource->setText(currentPlaySource);
    if (MediaPlayerJson["AutoPlay"].toBool(false))
    {
        player->play();
    }
    return true;
}

void VideoWindow::getVideoFilesInfoByDir(const QString &directory)
{
    QDir dir(directory);
    //dir.setFilter(QDir::Files);
    dir.setSorting(QDir::Time | QDir::Reversed);
    QFileInfoList files = dir.entryInfoList(QDir::Files) ;
    QStringList videoExtensionList = fileExtensions.split("|");

    for(const QFileInfo& file: std::as_const(files)){
        for(const QString& ext : videoExtensionList){
            if(file.suffix().toLower() == ext.mid(1)){
                VideoInfo info;
                info.filePath =file.absoluteFilePath();
                info.fileName = file.baseName();
                info.dateTime = file.lastModified();
                info.date = file.lastModified().date();
                //时长
                if (playHistoryJson.contains(file.absoluteFilePath())){
                    info.duration = playHistoryJson[file.absoluteFilePath()].toObject()["duration"].toInteger() / 1000;
                }
                videoFilesListLocal.append(info);
                break;
            }
        }
    }

    foreach (QFileInfo dirInfo, dir.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot))
    {
        //qDebug() << "fileInfo dirInfo:" << dirInfo; // QFileInfo(G:\data\)
        getVideoFilesInfoByDir(dirInfo.absoluteFilePath());
    }

}

void VideoWindow::update_CalendarWidget_videoDate_show()
{
    ui->dateEdit_videofiles->calendarWidget()->setDateTextFormat(QDate(),QTextCharFormat());
    QTextCharFormat format;
    format.setForeground(Qt::red);
    for(const VideoInfo& info : videoFilesListLocal) {
        if(info.dateTime.isValid())
            ui->dateEdit_videofiles->calendarWidget()->setDateTextFormat(info.dateTime.date(),format);
    }
}

void VideoWindow::setItem_tableWidget_playerList(const int &row, const VideoInfo &info)
{
    ui->tableWidget_playerList->setItem(row,TableWidgetItem_Column_FileName,new QTableWidgetItem(info.fileName));
    ui->tableWidget_playerList->setItem(row,TableWidgetItem_Column_DateTime,new QTableWidgetItem(info.dateTime.toString("yyyy/MM/dd-hh:mm:ss")));
    ui->tableWidget_playerList->setItem(row,TableWidgetItem_Column_Time,new QTableWidgetItem(QTime(0, 0).addMSecs(info.duration).toString("hh:mm:ss")));

}

void VideoWindow::set_tableWidget_playerList_byDate(const QDate &date)
{
    //视频列表更新数据
    map_tableWidgetIndex_to_videoFilesListLocalIndex.clear();
    ui->tableWidget_playerList->clearContents();
    ui->tableWidget_playerList->setRowCount(0);
    for (int var = 0; var < videoFilesListLocal.size(); ++var) {
        const VideoInfo& info = videoFilesListLocal.at(var);
        if(info.dateTime.date() == date){
            int row = ui->tableWidget_playerList->rowCount(); // 添加一行到表格中
            ui->tableWidget_playerList->insertRow(row);
            setItem_tableWidget_playerList(row,info);
            map_tableWidgetIndex_to_videoFilesListLocalIndex[row] = var;
        }
    }
    // 自适应大小
    ui->tableWidget_playerList->resizeColumnsToContents();
}

void VideoWindow::set_videoFilesListLocal_and_tableWidget_playerList_byFilePath(const QString &filesPath)
{
    ui->lineEdit_pathPlayerList->setText(filesPath);

    //数据清空
    videoFilesListLocal.clear();
    getVideoFilesInfoByDir(filesPath);
    update_CalendarWidget_videoDate_show();

    //视频列表更新数据
    map_tableWidgetIndex_to_videoFilesListLocalIndex.clear();
    ui->tableWidget_playerList->clearContents();
    ui->tableWidget_playerList->setRowCount(videoFilesListLocal.size());

    for (int var = 0; var < videoFilesListLocal.size(); ++var) {
        map_tableWidgetIndex_to_videoFilesListLocalIndex[var] = var;
        setItem_tableWidget_playerList(var,videoFilesListLocal.at(var));
    }
    // 自适应大小
    ui->tableWidget_playerList->resizeColumnsToContents();
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
    if (selected){
        ui->Widget->setContentsMargins(1, 1, 1, 1);//显示边框
    }else{
        ui->Widget->setContentsMargins(0, 0, 0, 0);
    }
}

void VideoWindow::WidgetMoreIsFloating(bool isFloating)
{
    if (isFloating)
    {
        //        ui->WidgetMore->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Window | Qt::CustomizeWindowHint);// | ui->WidgetMore->windowFlags());
        ui->WidgetMore->setWindowFlags(Qt::WindowStaysOnTopHint | ui->WidgetMore->windowFlags());
        ui->WidgetMore->setWindowOpacity(0.7);
        if (ui->WidgetMore->isHidden())
        {
            ui->WidgetMore->show();
            qDebug() << "VideoWindow::WidgetMoreIsFloating(bool isFloating) show();";
        }
        //        ui->WidgetMore->setAllowedAreas(Qt::AllToolBarAreas);
        //        QWindow * pWin = ui->WidgetMore->windowHandle();
        //        pWin->setFlag(Qt::WindowStaysOnTopHint,true);
        //        ui->WidgetMore->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
        //        ui->WidgetMore->setTitleBarWidget(ui->WidgetMoreTitleBar);
        //ui->WidgetMore->setContentsMargins(-20, -20, -20, -20);
    }
}

bool VideoWindow::PlayExecuteCmd(int command)
{
    switch (command)
    {
    case QMediaPlayer::PlayingState:

        break;
    default:
        break;
    }
    return true;
}

void VideoWindow::showEvent(QShowEvent *event)
{
    qDebug() << "第一次显示VideoWindow::showEvent(QShowEvent *" << event << objectName() << MediaPlayerSettings.allKeys();
    startShow();
}

/*
[1]VideoWindow::errorOccurred(QMediaPlayer::Error error, const QString &errorString) QMediaPlayer::ResourceError "Could not open file"
[2]VideoWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status) QMediaPlayer::InvalidMedia
*/
void VideoWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    qDebug() << "VideoWindow::mediaStatusChanged(QMediaPlayer::MediaStatus status)" << status;
    switch (status)
    {
    case QMediaPlayer::NoMedia: // 0	The is no current media. The player is in the StoppedState.
        break;
    case QMediaPlayer::LoadingMedia: //	1	正在加载当前介质。玩家可以处于任何状态。
        break;
    case QMediaPlayer::LoadedMedia: //	2	当前介质已加载。 StoppedState.
        if (!ui->toolButton_videoPlay->isEnabled())
        {ui->toolButton_videoPlay->setEnabled(true);}
        //表示媒体已经加载完成，可以进行播放操作默认0
        player->setPosition(playHistoryJson[currentPlaySource].toObject()["position"].toInteger());
        break;
    case QMediaPlayer::StalledMedia: //	3	由于缓冲不足或其他一些临时中断，当前媒体的播放已停止。The player is in the PlayingState or PausedState.
        break;
    case QMediaPlayer::BufferingMedia: //	4	播放器正在缓冲数据，但已缓冲足够的数据，以便在不久的将来继续播放。The player is in the PlayingState or PausedState.
        break;
    case QMediaPlayer::BufferedMedia: //	5	播放器已完全缓冲当前媒体。The player is in the PlayingState or PausedState.
        break;
    case QMediaPlayer::EndOfMedia: //	6	播放已到达当前媒体的末尾。The player is in the StoppedState.
        break;
    case QMediaPlayer::InvalidMedia: //	7	无法播放当前媒体。The player is in the StoppedState.
        break;
    default:
        // Defines the status of a media player's current media.
        break;
    }
}

void VideoWindow::sourceChanged(const QUrl &media)
{
    qDebug() << "VideoWindow::sourceChanged(const QUrl &" << media;
}

void VideoWindow::durationChanged(qint64 duration)
{
    qDebug() << "VideoWindow::durationChanged(qint64 duration)" << duration; // 文件时长毫秒
    if (duration > 0)
    {
        ui->horizontalSlider_position->setVisible(true);
        ui->label_videoProgress->setVisible(true);
        connect(player, &QMediaPlayer::positionChanged, this, &VideoWindow::positionChange); // rtsp直播流需要取消
        connect(ui->horizontalSlider_position, &QSlider::sliderMoved, player, &QMediaPlayer::setPosition);

        m_duration = duration / 1000;
        ui->horizontalSlider_position->setMaximum(duration);
        ui->toolButton_fastBback->setEnabled(true);
        ui->toolButton_fastForward->setEnabled(true);

        // 保存读取时长&读取进度
        QJsonObject durationJson = playHistoryJson[currentPlaySource].toObject();
        durationJson["duration"] = duration;
        playHistoryJson[currentPlaySource] = durationJson;

    }
    else
    {
        ui->horizontalSlider_position->setVisible(false);
        ui->label_videoProgress->setVisible(false);
        disconnect(player, &QMediaPlayer::positionChanged, this, &VideoWindow::positionChange); // rtsp直播流需要取消
        disconnect(ui->horizontalSlider_position, &QSlider::sliderMoved, player, &QMediaPlayer::setPosition);
        m_duration = 0;
        ui->toolButton_fastBback->setEnabled(false);
        ui->toolButton_fastForward->setEnabled(false);
    }
}

void VideoWindow::tracksChanged()
{
    qDebug() << "VideoWindow::tracksChanged()";
    qDebug() <<"本地文件:"<<playHistoryJson[currentPlaySource];
    qDebug() <<"本地文件:"<<playHistoryJson[currentPlaySource].toObject().contains("position")<<playHistoryJson[currentPlaySource].toObject()["position"].toInteger();
    if (playHistoryJson[currentPlaySource].toObject().contains("position"))
    {
        qDebug() <<"本地文件setPosition:"<<playHistoryJson[currentPlaySource].toObject()["position"].toInteger();
        player->setPosition(playHistoryJson[currentPlaySource].toObject()["position"].toInteger());
    }


}

void VideoWindow::hasVideoChanged(bool videoAvailable)
{
    qDebug() << "VideoWindow::hasVideoChanged(bool videoAvailable)" << videoAvailable;
    // 播放、暂停、停止、全屏
    ui->toolButton_videoPlay->setEnabled(videoAvailable);
    ui->toolButton_videoPause->setEnabled(videoAvailable);
    ui->toolButton_videoStop->setEnabled(videoAvailable);
    ui->toolButton_fullScreen->setEnabled(videoAvailable);
}

void VideoWindow::playbackStateChanged(QMediaPlayer::PlaybackState newState)
{
    qDebug() << "VideoWindow::playbackStateChanged(QMediaPlayer::PlaybackState newState)" << newState;
    switch (newState)
    {
    case QMediaPlayer::StoppedState: //	0	The media player is not playing content, playback will begin from the start of the current track.
        ui->toolButton_videoPlay->setVisible(true);
        ui->toolButton_videoPause->setVisible(false);
        break;
    case QMediaPlayer::PlayingState: //	1	The media player is currently playing content. This indicates the same as the playing property.
        ui->toolButton_videoPlay->setVisible(false);
        ui->toolButton_videoPause->setVisible(true);
        break;
    case QMediaPlayer::PausedState: //	2	The media player has paused playback, playback of the current track will resume from the position the player was paused at.
        ui->toolButton_videoPlay->setVisible(true);
        ui->toolButton_videoPause->setVisible(false);
        break;
    default:
        break;
    }
}

void VideoWindow::positionChange(qint64 progress)
{
    // qDebug() << "VideoWindow::positionChange(qint64 progress)"<<progress;
    if (!ui->horizontalSlider_position->isSliderDown())
    {
        ui->horizontalSlider_position->setValue(progress);
    }
}

void VideoWindow::bufferProgressChanged(float filled)
{
    qDebug() << "VideoWindow::bufferProgressChanged(float filled)" << filled;
}

void VideoWindow::errorOccurred(QMediaPlayer::Error error, const QString &errorString)
{
    qDebug() << "VideoWindow::errorOccurred(QMediaPlayer::Error error, const QString &errorString)" << error << errorString;
}

void VideoWindow::playerList_update()
{
    qDebug() << "VideoWindow::playerListUpdate()";
    QJsonObject playerList = MediaPlayerJson["playerList"].toObject();
    //    disconnect(ui->comboBox_playList)
    ui->comboBox_updatePlayerList->clear();
    foreach (QString key, playerList.keys())
    {
        qDebug() << "VideoWindow::playerListUpdate(): key-value" << key << playerList[key].toString();
        //        ui->comboBox_playerPath->addItem(key,playerList[key]);
        ui->comboBox_updatePlayerList->addItem(key, playerList[key].toString());
    }
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
    // ui->WidgetMore->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    //     if(ui->WidgetMore->isFloating()){
    //         ui->WidgetMore->setFloating(false);
    //     }else {
    //         ui->WidgetMore->setFloating(true);
    //     }
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


void VideoWindow::on_toolButton_moreSetting_clicked()
{
    if (ui->WidgetMore->isVisible())
    {
        ui->WidgetMore->setVisible(false);
    }
    else
    {
        ui->WidgetMore->setVisible(true);
    }
}

void VideoWindow::on_horizontalSlider_position_valueChanged(int value)
{
    if (ui->horizontalSlider_position->isSliderDown())
    {
        qDebug() << "VideoWindow::on_horizontalSlider_position_valueChanged(int " << value;
    }
    updateDurationInfo(value / 1000);
}

void VideoWindow::on_horizontalSlider_position_sliderMoved(int position)
{
    qDebug() << "VideoWindow::on_horizontalSlider_position_sliderMoved(int " << position;
    int minutes = position / 60000; // 每分钟60秒，每秒1000毫秒
    int seconds = (position / 1000) % 60;
    QPoint sliderPos = ui->horizontalSlider_position->mapToGlobal(QPoint(0, 0)); // 获取进度条的全局位置
    QPoint cursorPos = QCursor::pos();                                           // 获取当前鼠标的全局位置
    qDebug() << sliderPos << cursorPos;
    QToolTip::showText(
        QPoint(cursorPos.x(), sliderPos.y() - 42),
        QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')),
        ui->horizontalSlider_position);
}

void VideoWindow::on_toolButton_microphoneMute_clicked()
{
    if (player->audioOutput()->isMuted())
    {
        player->audioOutput()->setMuted(false);
        // 取消静音图标
        ui->toolButton_microphoneMute->setIcon(QIcon(":/asset/playerControls/MicrophoneMute.svg"));
    }
    else
    {
        player->audioOutput()->setMuted(true);
        // 静音图标
        ui->toolButton_microphoneMute->setIcon(QIcon(":/asset/playerControls/Microphone.svg"));
    }
}

void VideoWindow::on_horizontalSlider_volume_valueChanged(int value)
{
    player->audioOutput()->setVolume(value);
    MediaPlayerJson["volume"] = value;
}

void VideoWindow::on_toolButton_fastBback_clicked()
{
    if (player->position() < MediaPlayerJson["playPageStep"].toInteger(10 * 1000))
    {
        player->setPosition(0);
    }
    else
    {
        player->setPosition(player->position() - MediaPlayerJson["playPageStep"].toInteger(10 * 1000));
    }
}

void VideoWindow::on_toolButton_fastForward_clicked()
{
    if (player->position() < (player->duration() - MediaPlayerJson["playPageStep"].toInteger(10 * 1000)))
    {
        player->setPosition(player->position() + MediaPlayerJson["playPageStep"].toInteger(10 * 1000));
    }
}

void VideoWindow::on_comboBox_rate_currentIndexChanged(int index)
{
    qDebug() << "VideoWindow::on_comboBox_rate_currentIndexChanged(int " << index << (index < 3 ? 0.5 * (index + 1) : std::pow(2, index - 2));
    //    qreal playbackRate = index >3 ? 0.5*(index+1) : 2^(index-2);
    player->setPlaybackRate(index < 3 ? 0.5 * (index + 1) : std::pow(2, index - 2));
}

void VideoWindow::on_toolButton_openFiles_clicked()
{
    // 创建打开文件对话框
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile); // 设置对话框模式为选择已存在的文件

    QString filePath = fileDialog.getOpenFileName(this, "Open File", nullptr, "Mp4 Files (*.mp4)"); // 获取选择的文件路径
    if (!filePath.isEmpty())
    {
        // 在这里处理打开文件的逻辑
        qDebug() << "Selected file: " << filePath << QDir(filePath).absolutePath();
        player->setSource(QUrl::fromLocalFile(filePath));

        set_videoFilesListLocal_and_tableWidget_playerList_byFilePath(QDir(filePath).absolutePath());
    }
}

void VideoWindow::on_comboBox_playerPath_currentTextChanged(const QString &arg1)
{
    qDebug() << "void VideoWindow::on_comboBox_playerPath_currentTextChanged(const QString &" << arg1;
    ui->lineEdit_playerPath->setText(MediaPlayerJson["playerList"].toObject()[arg1].toString());
}

void VideoWindow::on_pushButton_setEditPlayerPath_clicked()
{
    qDebug() << "VideoWindow::on_lineEdit_playerPath_selectionChanged()";
    // 打开文件对话框，选择目录
    QString directory = QFileDialog::getExistingDirectory(this, "选择播放文件目录");
    if (!directory.isEmpty())
    {
        // 阻止信号的发射
        //        ui->lineEdit_playerPath->blockSignals(true);
        ui->lineEdit_playerPath->setText(directory);
        //        // 恢复信号的发射
        //        ui->lineEdit_playerPath->blockSignals(false);
    }
}

void VideoWindow::on_pushButton_playerPath_update_clicked()
{
    QJsonObject playerList = MediaPlayerJson["playerList"].toObject();
    QString key = ui->comboBox_playerPath->currentText();
    if (!playerList.contains(key))
    {
        QToolTip::showText(ui->pushButton_playerPath_update->mapToGlobal(QPoint(0, 0)),
                           "要更改的键值不存在，请先增加键值",
                           ui->pushButton_playerPath_update);
        return;
    }
    playerList.insert(key, ui->lineEdit_playerPath->text());
    MediaPlayerJson["playerList"] = playerList;
    playerList_update();
}

void VideoWindow::on_pushButton_playerPath_add_clicked()
{
    QJsonObject playerList = MediaPlayerJson["playerList"].toObject();
    QString key = ui->comboBox_playerPath->currentText();
    if (playerList.contains(key))
    {
        QToolTip::showText(ui->pushButton_playerPath_add->mapToGlobal(QPoint(0, 0)),
                           "要增加的键值已存在，请使用修改功能",
                           ui->pushButton_playerPath_add);
        return;
    }
    if (ui->comboBox_playerPath->findText(key) == -1)
    {
        QToolTip::showText(ui->pushButton_playerPath_add->mapToGlobal(QPoint(0, 0)),
                           "要增加的键值尚未保存，请修改完键值后输入Enter确认添加",
                           ui->pushButton_playerPath_add);
        return;
    }
    playerList.insert(key, ui->lineEdit_playerPath->text());
    MediaPlayerJson["playerList"] = playerList;
    playerList_update();
}

void VideoWindow::on_pushButton_playerPath_delete_clicked()
{
    QJsonObject playerList = MediaPlayerJson["playerList"].toObject();
    QString key = ui->comboBox_playerPath->currentText();
    if (!playerList.contains(key))
    {
        QToolTip::showText(ui->pushButton_playerPath_update->mapToGlobal(QPoint(0, 0)),
                           "要删除的键值不存在，请确认该键值已经生效",
                           ui->pushButton_playerPath_update);
        return;
    }
    playerList.remove(key);
    ui->comboBox_playerPath->removeItem(ui->comboBox_playerPath->currentIndex());
    MediaPlayerJson["playerList"] = playerList;
    playerList_update();
}

void VideoWindow::on_pushButton_setPlayPageStep_clicked()
{
    MediaPlayerJson["playPageStep"] = ui->lineEdit_playPageStep->text().toInt() * 1000;
}

void VideoWindow::on_pushButton_setPlaySource_clicked()
{
    MediaPlayerJson["playSource"] = ui->lineEdit_playSource->text();
    player->setSource(QUrl(MediaPlayerJson["playSource"].toString()));
}

void VideoWindow::on_checkBox_autoPlay_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked){MediaPlayerJson["AutoPlay"] = true;}
    else{MediaPlayerJson["AutoPlay"] = false;}
}

void VideoWindow::on_pushButton_setFileExtensions_clicked()
{
    MediaPlayerJson["fileExtensions"] = ui->lineEdit_fileExtensions->text();
}

void VideoWindow::on_tableWidget_playerList_itemDoubleClicked(QTableWidgetItem *item)
{
    qDebug() << "VideoWindow::on_tableWidget_playerList_itemDoubleClicked(QTableWidgetItem *" << item->text();
}

void VideoWindow::on_tableWidget_playerList_doubleClicked(const QModelIndex &index)
{
    qDebug() << "VideoWindow::on_tableWidget_playerList_doubleClicked(const QModelIndex &" << index.row();
    startPlay(videoFilesListLocal.at(map_tableWidgetIndex_to_videoFilesListLocalIndex[index.row()]).filePath);
}


void VideoWindow::on_comboBox_updatePlayerList_activated(int index)
{
    //activated() 信号只会在用户手动选择了下拉列表中的某个项目时才会被触发,而 currentIndexChanged() 信号则会在任何情况下都会被触发,即使是程序内部设置了当前索引。
    qDebug() << "on_comboBox_updatePlayerList_activated(int "<<index;
    QString fileDir = ui->comboBox_updatePlayerList->currentData().toString();
    playHistoryJson["currentFilePath"] = fileDir;
    set_videoFilesListLocal_and_tableWidget_playerList_byFilePath(fileDir);

}

void VideoWindow::on_dateEdit_videofiles_userDateChanged(const QDate &date)
{
    qDebug() << "VideoWindow::on_dateEdit_videofiles_userDateChanged(const QDate &date)"<<date;
    set_tableWidget_playerList_byDate(date);
    //取消选中 日期颜色恢复
    ui->dateEdit_videofiles->calendarWidget()->setSelectedDate(QDate());
}

void VideoWindow::on_pushButton_videofiles_data_set_clicked()
{
    set_tableWidget_playerList_byDate(ui->dateEdit_videofiles->date());
}

