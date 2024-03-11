#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/VideoPlayback.h"
#include "ui/DataView.h"
#include "public/AppSystem.h"
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QIcon>



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui_TitleBar = new TitleBar(this);
//    ui_header = ui_WidgetHeader->getUi();
    setMenuWidget(ui_TitleBar);

    test();
    init();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow::closeEvent(QCloseEvent *event)" << event;
    //    qApp->quit(); //重载主程序点击退出事件，软件退出
    APP_SETTINGS.beginGroup("MainWindow");
    APP_SETTINGS.setValue("geometry", saveGeometry());
    APP_SETTINGS.endGroup();
}

MainWindow::~MainWindow()
{
    qDebug() << "~MainWindow()";
    delete ui;
    //    qApp->quit();
}

/*
Qt::CustomizeWindowHint 【可以缩放】，【不能拖动】，【最上面会有白边】，最大化会消失不能双击放大
Qt::Widget | Qt::CustomizeWindowHint
Qt::Window | Qt::CustomizeWindowHint
Qt::FramelessWindowHint 无边框和标题，【只有右下角可以缩放】，【不能拖动】，不能双击放大
Qt::Window | Qt::FramelessWindowHint
*/
void MainWindow::init()
{
    qDebug() << "MainWindow::init" << EXE_CONFIG["version"];
//    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    setWindowFlags( Qt::FramelessWindowHint);
//    setContentsMargins(-9,-9,-9,-9);
    //    this->setContentsMargins(2,2,2,2); //设置QWidget内部内容的边距
//    setAttribute(Qt::WA_TranslucentBackground); //窗体透明


    //    ui->tabWidget_main->setStyleSheet(" background-color:transparent;background-color: rgb(0, 0, 255);QTabWidget::pane{border:none}QTabWidget::pane { border: 0; }");
    //    ui->tabWidget_main->setF
    //    ui->tabWidget_main->setStyleSheet("QTabWidget::pane { border: 0; }");
//        this->setStyleSheet("QMainWindow::separator {width: 1px; border: none;} ;\n background-color: rgb(170, 0, 0);");

//    setWindowFlags(windowFlags() | Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
//    setMenuWidget(ui->widget_header_titleBar);
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

    APP_SETTINGS.beginGroup("MainWindow");
    const auto geometry = APP_SETTINGS.value("geometry", QByteArray()).toByteArray(); // QByteArray 类型
    if (geometry.isEmpty())
        setGeometry(200, 200, 800, 600);
    else
        restoreGeometry(geometry);
    APP_SETTINGS.endGroup();
}

void MainWindow::test()
{
    qDebug() << "MainWindow::test";
    //    setCentralWidget(ui->tabWidget_mainWindow);
    // #include <QThread>
    // 下载结束后延迟1秒
    //    QThread::msleep(5000);
    addTabWidget(TabWindow_VideoPlayback);

    addTabWidget(TabWindow_DataView);
    ui->tabWidget_mainWindow->setCurrentIndex(m_tabWidget_mainWindow["DataView"].toInt());
    //    ui->tabWidget_mainWindow->setCurrentIndex(m_tabWidget_mainWindow["VideoPlayback"].toInt());
    //    QCoreApplication::processEvents(); // 处理界面事件

    //    setCentralWidget(ui->tabWidget_mainWindow); //添加主界面，会导致重合，两个界面布局重叠。取消thsi父窗口设置可以解决
    //    setCentralWidget(ui->centralwidget);

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "MainWindow::mousePressEvent(QMouseEvent *"<<event->button();
    return QMainWindow::mousePressEvent(event);
}

void MainWindow::on_Button_videoPlayback_clicked()
{
    qDebug() << "on_pushButton_video_playback_clicked()";
    //    qDebug()<<m_tabWidget_mainWindow.contains("video_playback");
    //    qDebug()<<m_tabWidget_mainWindow["video_playback"]<<m_tabWidget_mainWindow.contains("video_playback");
    //    qDebug()<<m_tabWidget_mainWindow.contains("video_playback");
    addTabWidget(TabWindow_VideoPlayback);
    int index = m_tabWidget_mainWindow["VideoPlayback"].toInt();
    //    ui->tabWidget_mainWindow->isTabVisible(index);
    if (!ui->tabWidget_mainWindow->isTabVisible(index))
    {
        ui->tabWidget_mainWindow->setTabVisible(index, true);
    }
    ui->tabWidget_mainWindow->setCurrentIndex(index);
    qDebug() << m_tabWidget_mainWindow["VideoPlayback"];
}

void MainWindow::on_Button_dataView_clicked()
{
    qDebug() << "on_pushButton_dataview_clicked";
    addTabWidget(TabWindow_DataView);
    int index = m_tabWidget_mainWindow["DataView"].toInt();
    //    ui->tabWidget_mainWindow->isTabVisible(index);
    if (!ui->tabWidget_mainWindow->isTabVisible(index))
    {
        ui->tabWidget_mainWindow->setTabVisible(index, true);
    }
    ui->tabWidget_mainWindow->setCurrentIndex(index);
    qDebug() << m_tabWidget_mainWindow["DataView"];
}

void MainWindow::addTabWidget(TabWindow window)
{
    switch (window)
    {
    case TabWindow_VideoPlayback:
        if (m_tabWidget_mainWindow["VideoPlayback"].isNull())
        {
            m_tabWidget_mainWindow["VideoPlayback"] = ui->tabWidget_mainWindow->count();
            VideoPlayback *ui_VideoPlayback = new VideoPlayback();
            //    QIcon icon_videoPlayback = QIcon::fromTheme("media-playback-start");//系统主题不能用
            QIcon icon_videoPlayback(":/asset/video_playback/Movie_Active.svg");
            ui->tabWidget_mainWindow->insertTab(m_tabWidget_mainWindow["VideoPlayback"].toInt(), ui_VideoPlayback->getVideoPaly(), icon_videoPlayback, "视频回放");
            qDebug() << m_tabWidget_mainWindow["VideoPlayback"];
        }
        break;
    case TabWindow_DataView:
        if (m_tabWidget_mainWindow["DataView"].isNull())
        {
            DataView *ui_DataView = new DataView();
            QIcon icon_dataView(":/asset/dataview/DataAnalysis.svg");
            m_tabWidget_mainWindow["DataView"] = ui->tabWidget_mainWindow->insertTab(ui->tabWidget_mainWindow->count(), ui_DataView->getDataView(), icon_dataView, "数据图表");

            // 设置允许该标签可以被删除
            ui->tabWidget_mainWindow->setTabEnabled(m_tabWidget_mainWindow["DataView"].toInt(), true);
            //            ui->tabWidget_mainWindow->setTabsClosable(true);

            qDebug() << m_tabWidget_mainWindow["DataView"];
        }
        break;
    default:
        break;
    }
}

void MainWindow::TabCloseRequested(int index)
{
    qDebug() << "TabCloseRequested(int index):" << index;
    //    ui->tabWidget_mainWindow->isVisible();
    if (index == 0)
    {
        return;
    }
    ui->tabWidget_mainWindow->setTabVisible(index, false);
}

#include <QMouseEvent>
#include <QTabBar>
#include <QToolBar>
#include <QToolBar>
void MainWindow::TabCurrentChanged(int index)
{
    qDebug() << "TabCurrentChanged(int index):" << index;
    //    QTabBar *tabBar = ui->tabWidget_mainWindow->tabBar();
    //    tabBar->setTabsClosable(false);
    //    ui->tabWidget_mainWindow->setTabBar(tabBar);
}

#include<QDockWidget>

void MainWindow::on_pushButton_test_clicked()
{
    qDebug() << "MainWindow::on_pushButton_test_clicked()";
    QDockWidget *dockWidget = new QDockWidget(nullptr);
//    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
//    dockWidget->setFloating(true);
//    dockWidget->setWindowFlags(Qt::FramelessWindowHint);
//    dockWidget->setTitleBarWidget(ui->horizontalLayout);
    QWidget *titleWidget = new QWidget(this);
//    titleWidget->setLayout(ui->horizontalLayout);

//    setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
//    titleBar = new TitleBar(this);

    dockWidget->setTitleBarWidget(titleWidget);

    QWidget *contentWidget = new QWidget(dockWidget);
    QVBoxLayout *layout = new QVBoxLayout(contentWidget);
    QLabel *label = new QLabel("This is a dock widget content", contentWidget);
    layout->addWidget(label);

    dockWidget->setWidget(contentWidget);

//    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);
    dockWidget->show();
//    hide();


}
