#ifndef MEDIAWIDGETS_H
#define MEDIAWIDGETS_H

#include <QAudioOutput>
#include <QFileSystemModel>
#include <QMediaPlayer>
#include <QSettings>
#include <QVideoWidget>
#include <QWidget>

#include "ui_MediaWidgets.h"

namespace Ui
{
class HomeMainWidget;
}

class MediaWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit MediaWidgets(QWidget *parent = nullptr);
    ~MediaWidgets();

    Ui::HomeMainWidget *ui;
    void init();
    void quit();

signals:
    void homeMune_jump_TabWidget();

private slots:
    void on_Button_videoPlayback_clicked();

private:

protected:

protected slots:


};

#endif // MEDIAWIDGETS_H
