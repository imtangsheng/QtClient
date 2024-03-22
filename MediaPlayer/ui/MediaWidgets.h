#ifndef MEDIAWIDGETS_H
#define MEDIAWIDGETS_H

#include <QAudioOutput>
#include <QFileSystemModel>
#include <QMediaPlayer>
#include <QSettings>
#include <QVideoWidget>
#include <QWidget>

#include "ui_MediaWidgets.h"


namespace Ui {
class MediaWidgets;
}

class MediaWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit MediaWidgets(QWidget *parent = nullptr);
    ~MediaWidgets();

    Ui::MediaWidgets *ui;

    void init();

    void quit();

    void setVideoSourceAndPlay(QString source);

    //Signals from player
    void hasVideoChanged(bool videoAvailable);
    void mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void playbackStateChanged(QMediaPlayer::PlaybackState newState);
    void sourceChanged(const QUrl &media);
    void errorOccurred(QMediaPlayer::Error error, const QString &errorString);

    void sliderMovedForPlayer(int value);

signals:
    void homeMune_jump_TabWidget();

private slots:
    void on_Button_videoPlayback_clicked();

    void on_pushButton_test_clicked();

    void on_toolButton_play_clicked();

    void on_toolButton_stop_clicked();

    void on_toolButton_previous_clicked();

    void on_toolButton_next_clicked();

    void on_toolButton_pause_clicked();

    void on_horizontalSlider_volume_valueChanged(int value);

private:
    QString currentPlayFile;
    QUrl url;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    enum MediaControls : int
    {
        Play,
        Pause,
        Stop
    };

    bool playerControls(int control);

    QStringList fileExtensions;
    QFileSystemModel *m_fileModel;

protected:
    void closeEvent(QCloseEvent *event) override;

    QSettings* config;//("config/config.ini",QSettings::IniFormat); //无编码配置，已经移除，使用UTF-8
};

#endif // MEDIAWIDGETS_H
