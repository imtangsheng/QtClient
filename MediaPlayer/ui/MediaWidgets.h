#ifndef MEDIAWIDGETS_H
#define MEDIAWIDGETS_H

#include <QMediaPlayer>
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

signals:
    void homeMune_jump_TabWidget();

private slots:
    void on_Button_videoPlayback_clicked();

private:
    QMediaPlayer *m_player;
};

#endif // MEDIAWIDGETS_H
