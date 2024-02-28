#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include "ui/VideoPlayback.h"
#include "ui/DataView.h"
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
    qDebug()<<"~MainWindow()";
}

void MainWindow::init()
{
    qDebug()<<"MainWindow::init"<<EXE_CONFIG["version"];


//    addTabWidget(TabWindow_VideoPlayback);
//    addTabWidget(TabWindow_DataView);
//    ui->tabWidget_mainWindow->setCurrentIndex(m_tabWidget_mainWindow["DataView"].toInt());
    //    ui->tabWidget_mainWindow->setFocusPolicy(Qt::TabFocus);
//    connect(this,&MainWindow::testSignal,this,&MainWindow::test);
//    emit testSignal();
//    ui->tabWidget_mainWindow->setTabsClosable(true);
    // 连接删除请求的槽函数
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabCloseRequested, this, &MainWindow::TabCloseRequested);
    connect(ui->tabWidget_mainWindow, &QTabWidget::currentChanged, this, &MainWindow::TabCurrentChanged);
    test();
}

void MainWindow::test()
{
    qDebug()<<"MainWindow::test000000";
//    setCentralWidget(ui->tabWidget_mainWindow);
    //#include <QThread>
    // 下载结束后延迟1秒
    //    QThread::msleep(5000);
    addTabWidget(TabWindow_VideoPlayback);

    addTabWidget(TabWindow_DataView);
    ui->tabWidget_mainWindow->setCurrentIndex(m_tabWidget_mainWindow["DataView"].toInt());
//    ui->tabWidget_mainWindow->setCurrentIndex(m_tabWidget_mainWindow["VideoPlayback"].toInt());
//    QCoreApplication::processEvents(); // 处理界面事件

    setCentralWidget(ui->tabWidget_mainWindow);

    qDebug()<<"MainWindow::test111111";
}




void MainWindow::on_pushButton_video_playback_clicked()
{
    qDebug()<<"on_pushButton_video_playback_clicked()";
//    qDebug()<<m_tabWidget_mainWindow.contains("video_playback");
//    qDebug()<<m_tabWidget_mainWindow["video_playback"]<<m_tabWidget_mainWindow.contains("video_playback");
//    qDebug()<<m_tabWidget_mainWindow.contains("video_playback");
    addTabWidget(TabWindow_VideoPlayback);
    int index = m_tabWidget_mainWindow["VideoPlayback"].toInt();
//    ui->tabWidget_mainWindow->isTabVisible(index);
    if(!ui->tabWidget_mainWindow->isTabVisible(index)){
        ui->tabWidget_mainWindow->setTabVisible(index,true);
    }
    ui->tabWidget_mainWindow->setCurrentIndex(index);
    qDebug()<<m_tabWidget_mainWindow["VideoPlayback"];
}

void MainWindow::on_pushButton_dataView_clicked()
{
    qDebug()<<"on_pushButton_dataview_clicked";
    addTabWidget(TabWindow_DataView);
    int index = m_tabWidget_mainWindow["DataView"].toInt();
    //    ui->tabWidget_mainWindow->isTabVisible(index);
    if(!ui->tabWidget_mainWindow->isTabVisible(index)){
        ui->tabWidget_mainWindow->setTabVisible(index,true);
    }
    ui->tabWidget_mainWindow->setCurrentIndex(index);
    qDebug()<<m_tabWidget_mainWindow["DataView"];
}

void MainWindow::addTabWidget(TabWindow window)
{
    switch (window)
    {
    case TabWindow_VideoPlayback:
        if (m_tabWidget_mainWindow["VideoPlayback"].isNull()){
            m_tabWidget_mainWindow["VideoPlayback"] = ui->tabWidget_mainWindow->count();
            VideoPlayback *ui_VideoPlayback = new VideoPlayback(this);
            //    QIcon icon_videoPlayback = QIcon::fromTheme("media-playback-start");//系统主题不能用
            QIcon icon_videoPlayback(":/asset/video_playback/Movie_Active.svg");
            ui->tabWidget_mainWindow->insertTab(m_tabWidget_mainWindow["VideoPlayback"].toInt(),ui_VideoPlayback->getVideoPaly(),icon_videoPlayback,"视频回放");
            qDebug()<<m_tabWidget_mainWindow["VideoPlayback"];
        }
        break;
    case TabWindow_DataView:
        if (m_tabWidget_mainWindow["DataView"].isNull()){
            DataView *ui_DataView = new DataView(this);
            QIcon icon_dataView(":/asset/dataview/DataAnalysis.svg");
            m_tabWidget_mainWindow["DataView"] = ui->tabWidget_mainWindow->insertTab(ui->tabWidget_mainWindow->count(),ui_DataView->getDataView(),icon_dataView,"数据图表");

            // 设置允许该标签可以被删除
            ui->tabWidget_mainWindow->setTabEnabled(m_tabWidget_mainWindow["DataView"].toInt(), true);
//            ui->tabWidget_mainWindow->setTabsClosable(true);

            qDebug()<<m_tabWidget_mainWindow["DataView"];
        }
        break;
    default:
        break;
    }
}

void MainWindow::TabCloseRequested(int index)
{
    qDebug()<<"TabCloseRequested(int index):"<<index;
//    ui->tabWidget_mainWindow->isVisible();
    if(index==0){return;}
    ui->tabWidget_mainWindow->setTabVisible(index,false);

}

#include<QTabBar>
void MainWindow::TabCurrentChanged(int index)
{
    qDebug()<<"TabCurrentChanged(int index):"<<index;
    QTabBar *tabBar = ui->tabWidget_mainWindow->tabBar();
//    tabBar->setTabsClosable(false);
//    ui->tabWidget_mainWindow->setTabBar(tabBar);

}
