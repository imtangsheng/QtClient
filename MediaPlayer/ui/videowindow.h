#ifndef VIDEOWINDOW_H
#define VIDEOWINDOW_H


#include <QMainWindow>
#include "ui_videowindow.h"

namespace Ui {
class VideoWindow;
}

class VideoWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit VideoWindow(QWidget *parent = nullptr);
    ~VideoWindow();

    void init();
    void quit();

public slots:
    void mouseEnterVideo();
    void mouseLeaveVideo();

protected:

    void showEvent(QShowEvent *event) override; //在控件第一次显示时以及每次从隐藏状态恢复到显示状态时都会被触发

private slots:
    void on_Button_moreFloatable_clicked();


private:
    Ui::VideoWindow *ui;
};

#endif // VIDEOWINDOW_H
