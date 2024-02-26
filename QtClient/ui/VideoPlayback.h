#ifndef VIDEOPLAYBACK_H
#define VIDEOPLAYBACK_H

#include <QWidget>
#include <QVideoWidget>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QRect>
#include <QResizeEvent>
#include <QFileSystemModel>

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

    void updataMoreWidgetGeometry();

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

    void on_pushButton_operation_clicked();

    void on_pushButton_openFile_clicked();

    void on_comboBox_videoFiles_currentIndexChanged(int index);

    void on_comboBox_videoFiles_editTextChanged(const QString &arg1);



    void on_pushButton_settings_clicked();

    void on_pushButton_more_clicked();

    void on_pushButton_playVideoFile_clicked();

protected:
    bool eventFilter(QObject *object, QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

public:
private:
    void initSettingPlayback();
    Ui::VideoPlayback *ui;
    QRect *m_playlistInfo;
    QMediaPlayer *m_player;
    bool mediaControls(int control);

    void init_filesView();
    QFileSystemModel *m_fileModel_videoPlayback;
    QString m_currentFilePathDir;
    void fileModelSelection(QModelIndex index);
    void fileBrowserDoubleClicked(QModelIndex index);


};

#endif // VIDEOPLAYBACK_H
