#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QVideoWidget>
#include <QMediaPlayer>
#include <QAudioOutput>

class VideoWidget : public QVideoWidget
{
    Q_OBJECT
public:
    explicit VideoWidget(QWidget *parent = nullptr);
    void init();
    void quit();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

    bool eventFilter(QObject *watched, QEvent *event) override;

signals:

    void mouseEnterEvent();
    void mouseLeaveEvent();

public:
    QMediaPlayer player;
    QAudioOutput audioOutput;
    
};

#endif // VIDEOWIDGET_H
