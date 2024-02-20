#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "ui/VideoPlayback.h"

#include "public/AppData.h"
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QIcon>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    init();


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init()
{
    qDebug()<<"MainWindow::init"<<EXE_CONFIG["version"];
//    VideoPlayback *ui_VideoPlayback = new VideoPlayback;

////    QIcon icon_videoPlayback = QIcon::fromTheme("media-playback-start");//系统主题不能用
//    QIcon icon_videoPlayback(":/asset/video_playback/Movie_Active.svg");
//    ui->tabWidget_mainWindow->addTab(ui_VideoPlayback,icon_videoPlayback,"视频回放");
}


void MainWindow::on_pushButton_video_playback_clicked()
{
    qDebug()<<"on_pushButton_video_playback_clicked()";
//    qDebug()<<m_tabWidget_mainWindow.contains("video_playback");
//    qDebug()<<m_tabWidget_mainWindow["video_playback"]<<m_tabWidget_mainWindow.contains("video_playback");
//    qDebug()<<m_tabWidget_mainWindow.contains("video_playback");
    if (m_tabWidget_mainWindow["video_playback"].isNull()){
        m_tabWidget_mainWindow["video_playback"] = ui->tabWidget_mainWindow->count();
        VideoPlayback *ui_VideoPlayback = new VideoPlayback;
        //    QIcon icon_videoPlayback = QIcon::fromTheme("media-playback-start");//系统主题不能用
        QIcon icon_videoPlayback(":/asset/video_playback/Movie_Active.svg");
        ui->tabWidget_mainWindow->insertTab(m_tabWidget_mainWindow["video_playback"].toInt(),ui_VideoPlayback,icon_videoPlayback,"视频回放");
        qDebug()<<m_tabWidget_mainWindow["video_playback"];
    }
    ui->tabWidget_mainWindow->setCurrentIndex(m_tabWidget_mainWindow["video_playback"].toInt());

}

