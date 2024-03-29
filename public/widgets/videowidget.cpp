#include "videowidget.h"

#include <QKeyEvent>
#include <QMouseEvent>

VideoWidget::VideoWidget(QWidget *parent) : QVideoWidget{parent}
{
    setSizePolicy(QSizePolicy::Ignored,QSizePolicy::Ignored);
    setStyleSheet("background-color: red;");

//    QPalette p = palette();
//    p.setColor(QPalette::Window, Qt::red);
//    setPalette(p);

#ifndef Q_OS_ANDROIN // QTBUG-95723
    setAttribute(Qt::WA_OpaquePaintEvent);
#endif
    init();
}

void VideoWidget::init()
{
    player.setAudioOutput(&audioOutput);
    player.setVideoOutput(this);
}

void VideoWidget::quit()
{
    if(player.isPlaying()){
        player.stop();
    }
}

void VideoWidget::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) && isFullScreen()) {
        setFullScreen(false);
        event->accept();
    } else if (event->key() == Qt::Key_Enter && event->modifiers() & Qt::Key_Alt) {
        setFullScreen(!isFullScreen());
        event->accept();
    } else {
        QVideoWidget::keyPressEvent(event);
    }
}

void VideoWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    setFullScreen(!isFullScreen());
    event->accept();
}

void VideoWidget::mousePressEvent(QMouseEvent *event)
{
    qDebug()<<"VideoWidget::mousePressEvent(QMouseEvent *"<<event->button();
    emit mousePress();
    return QVideoWidget::mousePressEvent(event);
}

void VideoWidget::enterEvent(QEnterEvent *event)
{
    qDebug()<<"VideoWidget::enterEvent(QEnterEvent *event)"<<event->button();
    emit mouseEnterEvent();
    return QVideoWidget::enterEvent(event);
}

void VideoWidget::leaveEvent(QEvent *event)
{
    qDebug()<<"VideoWidget::leaveEvent(QEvent *event)"<<event->type();
    emit mouseLeaveEvent();
    return QVideoWidget::leaveEvent(event);
}

bool VideoWidget::eventFilter(QObject *watched, QEvent *event)
{
    qDebug()<<"bool VideoWidget::eventFilter(QObject *watched, QEvent *event)";
    return QVideoWidget::eventFilter(watched, event);
}
