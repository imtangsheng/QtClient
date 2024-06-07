#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H

#include <QJsonObject>
#include <QMainWindow>
#include "ui_videowindow.h"

#define TableWidgetItem_Column_FileName 0
#define TableWidgetItem_Column_DateTime 1
#define TableWidgetItem_Column_Time 2
//#define TableWidgetItem_Column_Size 3

struct VideoInfo
{
    QString filePath;
    QString fileName;
    QDateTime dateTime;
    QDate date;
    qint64 duration = 0;
};

namespace Ui
{
    class VideoWindow;
}

class VideoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoWindow(QWidget *parent = nullptr);
    ~VideoWindow();

    void init();
    void startShow();
    void quit();
    bool startPlay(const QString &file);
    /**视频文件操作方法**/
    QList<VideoInfo> videoFilesListLocal;
    void getVideoFilesInfoByDir(const QString& directory);
    void update_CalendarWidget_videoDate_show();
    void setItem_tableWidget_playerList(const int& row,const VideoInfo& info);
    void set_tableWidget_playerList_byDate(const QDate& date);
    QMap<int,int> map_tableWidgetIndex_to_videoFilesListLocalIndex;
    void set_videoFilesListLocal_and_tableWidget_playerList_byFilePath(const QString &filesPath);

public slots:
    void mouseEnterVideo();
    void mouseLeaveVideo();
    void mouseIsSelected(bool selected = true);

    void WidgetMoreIsFloating(bool isFloating);
    bool PlayExecuteCmd(int command);

protected:
    void showEvent(QShowEvent *event) override; // 在控件第一次显示时以及每次从隐藏状态恢复到显示状态时都会被触发

protected slots:
    /*视频播放信号连接*/
    //[!1-LoadingMedia !6-LoadedMedia !8-BufferingMedia -BufferedMedia -EndOfMedia]
    /*
     * enum QMediaPlayer::MediaStatus定义媒体播放器的当前媒体的状态。
        QMediaPlayer::NoMedia	0	没有当前媒体。玩家在
        QMediaPlayer::LoadingMedia	1	正在加载当前介质。玩家可以处于任何状态。
        QMediaPlayer::LoadedMedia	2	当前介质已加载。玩家在
        QMediaPlayer::StalledMedia	3	由于缓冲不足或其他一些临时中断，当前媒体的播放已停止。玩家PlayingState or PausedState.
        QMediaPlayer::BufferingMedia	4	播放器正在缓冲数据，但已缓冲足够的数据，以便在不久的将来继续播放。玩家PlayingState or PausedState.
        QMediaPlayer::BufferedMedia	5	播放器已完全缓冲当前媒体。玩家在 PlayingState or PausedState.
        QMediaPlayer::EndOfMedia	6	播放已到达当前媒体的末尾。玩家在 StoppedState.
        QMediaPlayer::InvalidMedia	7	无法播放当前媒体。玩家在 StoppedState.
    */
    void mediaStatusChanged(QMediaPlayer::MediaStatus status); // 媒体状态变化
    //[2!]
    void sourceChanged(const QUrl &media); // 播放源变化
    //[3!]
    void durationChanged(qint64 duration); // 总时长变化
    //[4!]
    void tracksChanged(); // 曲目变化
    //[5!] Signals from player
    void hasVideoChanged(bool videoAvailable); // 表示视觉内容的可用性已更改为
    //[7!]
    /*  enum QMediaPlayer::PlaybackState        定义媒体播放器的当前状态。
        QMediaPlayer::StoppedState	0	媒体播放器未播放内容，播放将从当前曲目的开头开始。
        QMediaPlayer::PlayingState	1	媒体播放器当前正在播放内容。这表示与属性相同。
        QMediaPlayer::PausedState	2	媒体播放器已暂停播放，当前曲目的播放将从播放器暂停的位置恢复。
    */
    void playbackStateChanged(QMediaPlayer::PlaybackState newState); // 播放状态变化
    //[9!]
    void positionChange(qint64 progress); // 播放位置变化
    //[end]
    void bufferProgressChanged(float filled);                                  // 缓存变化，0-1
    void errorOccurred(QMediaPlayer::Error error, const QString &errorString); // 错误

    /*视频播放列表方法*/
    void playerList_update();

private slots:
    void on_Button_moreWidget_isFloatable_clicked();

    void on_toolButton_videoPlay_clicked();
    void on_toolButton_videoPause_clicked();
    void on_toolButton_videoStop_clicked();
    void on_toolButton_fullScreen_clicked();
    void on_toolButton_moreSetting_clicked();
    void on_horizontalSlider_position_valueChanged(int value);
    void on_horizontalSlider_position_sliderMoved(int position);
    void on_toolButton_microphoneMute_clicked();
    void on_horizontalSlider_volume_valueChanged(int value);
    void on_toolButton_fastBback_clicked();
    void on_toolButton_fastForward_clicked();
    void on_comboBox_rate_currentIndexChanged(int index);

    void on_toolButton_openFiles_clicked();
    void on_pushButton_playerPath_update_clicked();

    //void on_comboBox_playerPath_currentIndexChanged(int index);
    void on_comboBox_playerPath_currentTextChanged(const QString &arg1);

    void on_pushButton_playerPath_add_clicked();
    void on_pushButton_playerPath_delete_clicked();
    void on_pushButton_setEditPlayerPath_clicked();
    void on_pushButton_setPlayPageStep_clicked();
    void on_pushButton_setPlaySource_clicked();
    void on_checkBox_autoPlay_stateChanged(int arg1);
    void on_pushButton_setFileExtensions_clicked();

    void on_tableWidget_playerList_itemDoubleClicked(QTableWidgetItem *item);
    void on_tableWidget_playerList_doubleClicked(const QModelIndex &index);

    void on_comboBox_updatePlayerList_activated(int index);
    void on_dateEdit_videofiles_userDateChanged(const QDate &date);

    void on_pushButton_videofiles_data_set_clicked();

private:
    Ui::VideoWindow *ui;
    //    std::unique_ptr<QMediaPlayer> player;//std::unique_ptr不能直接指向已经存在的对象
    // 视频播放
    QMediaPlayer *player;
    QUrl source;
    qint64 m_duration;
    void updateDurationInfo(qint64 currentInfo); // 更新视频时长信息
    // 视频播放文件列表
    QString currentPlaySource;
    // 定义视频文件列表方法
    QString fileExtensions = ".mp4|.MP4|.avi|.mkv";
    QJsonObject playHistoryJson;
};

#endif // VIDEOWINDOW_H
