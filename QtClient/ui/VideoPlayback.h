#ifndef VIDEOPLAYBACK_H
#define VIDEOPLAYBACK_H

#include <QWidget>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QRect>
#include <QResizeEvent>

namespace Ui {
class VideoPlayback;
}

class VideoPlayback : public QWidget
{
    Q_OBJECT

public:
    explicit VideoPlayback(QWidget *parent = nullptr);
    ~VideoPlayback();

    void init();

    QVideoWidget *videoWidget;
    QAudioOutput *audioOutput;

    void mediaStateChanged(QMediaPlayer::MediaStatus mediaState);
    void playerStateChanged();
    void hasVideoChanged(bool videoAuailable);

    enum MediaControls:int {
        Play,
        Pause,
        Stop
    };

    QWidget *getVideoPaly();

private slots:
    void on_pushButton_play_clicked();
    void on_pushButton_pause_clicked();
    void on_pushButton_stop_clicked();

    void on_pushButton_test_clicked();


    void on_horizontalSlider_playbackRate_valueChanged(int value);

    void on_horizontalSlider_playbackVolume_valueChanged(int value);

    void on_pushButton_fullScreen_clicked();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public:
//private:
    Ui::VideoPlayback *ui;

    QRect *m_playlistInfo;

    QMediaPlayer *m_player;

    bool mediaControls(int control);

};

#endif // VIDEOPLAYBACK_H
