#ifndef CAMERAWIDGET_H
#define CAMERAWIDGET_H

#include <QWidget>
#include "ui_camerawidget.h"
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPainter>
#include <QtMultimedia>

namespace Ui {
class CameraWidget;
}

class CameraWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWidget(QWidget *parent = nullptr);
    ~CameraWidget();
    void init();
    void quit();

    QMediaPlayer player;
    QAudioOutput audioOutput;

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;
    void paintEvent(QPaintEvent * event) override;

signals:
    void mouseEnterEvent();
    void mouseLeaveEvent();
    void mousePress();

private:
    Ui::CameraWidget *ui;
    QWidget *parent1;
};

#endif // CAMERAWIDGET_H
