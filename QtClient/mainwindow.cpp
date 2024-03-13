#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/VideoPlayback.h"
#include "ui/DataView.h"
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QTabBar>

#include "ui/SubMain.h"
SubMain *SUB_MAIN;

#define VARNAME(var) #var

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui_TitleBar = new TitleBar(this);
//    ui_header = ui_WidgetHeader->getUi();
    setMenuWidget(ui_TitleBar);
    _Awake();
}

void MainWindow::showUI()
{
    qDebug() << "MainWindow::show() 当前登录用户："<<CurrentUser;
    SUB_MAIN = new SubMain;
    _Start();
    show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow::closeEvent(QCloseEvent *event)" << event;
    //    qApp->quit(); //重载主程序点击退出事件，软件退出
    APP_SETTINGS.beginGroup("MainWindow");
    APP_SETTINGS.setValue("geometry", saveGeometry());
    APP_SETTINGS.setValue("jsonSettingsVariable",jsonMainConfig);
//    APP_SETTINGS.setValue("jsonSettingsVariable",QJsonDocument::fromVariant(jsonSettingsVariable).toJson());
    APP_SETTINGS.endGroup();
    qDebug() << "MainWindow::closeEvent(QCloseEvent *event) END";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow::~MainWindow() delete this";
    delete ui;
    delete SUB_MAIN;
    //    qApp->quit();
}

void MainWindow::_Awake()
{
    //    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    setWindowFlags( Qt::FramelessWindowHint);

}

/*
Qt::CustomizeWindowHint 【可以缩放】，【不能拖动】，【最上面会有白边】，最大化会消失不能双击放大
Qt::Widget | Qt::CustomizeWindowHint
Qt::Window | Qt::CustomizeWindowHint
Qt::FramelessWindowHint 无边框和标题，【只有右下角可以缩放】，【不能拖动】，不能双击放大
Qt::Window | Qt::FramelessWindowHint
*/
#include <QJsonArray>
void MainWindow::_Start()
{
    qDebug() << "MainWindow::_Start() 使用版本号：" << EXE_CONFIG["version"];
    APP_SETTINGS.beginGroup("MainWindow");
    const auto geometry = APP_SETTINGS.value("geometry", QByteArray()).toByteArray(); // QByteArray 类型
    if (geometry.isEmpty())
        setGeometry(200, 200, 800, 600);
    else
        restoreGeometry(geometry);

    jsonMainConfig = APP_SETTINGS.value("jsonSettingsVariable",QJsonObject()).toJsonObject();
    //    const auto list2 = APP_SETTINGS.value("list",QVariant::fromValue(QList<int>{}));
    //    QList<int> list4 = APP_SETTINGS.value("list").value<QList<int>>();
    APP_SETTINGS.endGroup();
    qDebug()<<"jsonSettingsVariable 格式的配置："<<jsonMainConfig;
    /* MainWindow::ui->tabWidget_mainWindow 初始化加载 */
    // 连接删除请求的槽函数
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabCloseRequested, this, &MainWindow::TabCloseRequested);
    connect(ui->tabWidget_mainWindow, &QTabWidget::currentChanged, this, &MainWindow::TabCurrentChanged);
    foreach (const QString key, jsonMainConfig["TabWindow"].toObject().keys()) {
        //初始化加载页面
        addTabWidget(TabWindow(key.toInt()));
    }
    ui->tabWidget_mainWindow->setTabsClosable(true);
    // 要设置第1个选项卡不可关闭
    ui->tabWidget_mainWindow->tabBar()->setTabButton(0,QTabBar::RightSide,nullptr);
    foreach (const QString key, jsonMainConfig["TabWindow"].toObject().keys()) {
        //初始化Tab标签的数据和其他配置
        int index = jsonMainConfig["TabWindow"].toObject()[key].toObject()["index"].toInt();
        ui->tabWidget_mainWindow->tabBar()->tabButton(index,QTabBar::RightSide)->setVisible(false);
    }
    ui->tabWidget_mainWindow->setCurrentIndex(jsonMainConfig["TabCurrent"].toInt());

    test();
}

void MainWindow::test()
{
    qDebug() << "MainWindow::test";
    ui->tabWidget_mainWindow->setAttribute(Qt::WA_TranslucentBackground); //窗体透明
    qDebug() << "MainWindow::test  mapTabIndexToWindow"<<mapTabIndexToWindow;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "MainWindow::mousePressEvent(QMouseEvent *"<<event->button();
    return QMainWindow::mousePressEvent(event);
}

void MainWindow::on_Button_videoPlayback_clicked()
{
    qDebug() << "on_pushButton_video_playback_clicked()";
    // 判断是否存在指定名称的Tab页
    QWidget* existingTab = ui->tabWidget_mainWindow->findChild<QWidget*>(TabWindowMap()[TabWindow_VideoPlayback]);
    if(existingTab){
        qDebug() << "MainWindow::on_Button_videoPlayback_clicked()) TabWindowMap*"<<existingTab->objectName();
        ui->tabWidget_mainWindow->setCurrentWidget(existingTab);
    }else{
        addTabWidget(TabWindow_VideoPlayback);
        ui->tabWidget_mainWindow->setCurrentIndex(jsonMainConfig["TabWindow"].toObject()[QString::number(TabWindow_VideoPlayback)].toObject()["index"].toInt());
    }
}

void MainWindow::on_Button_dataView_clicked()
{
    qDebug() << "on_pushButton_dataview_clicked";
    QWidget* existingTab = ui->tabWidget_mainWindow->findChild<QWidget*>(TabWindowMap()[TabWindow_DataView]);
    if(existingTab){
        ui->tabWidget_mainWindow->setCurrentWidget(existingTab);
    }else{
        addTabWidget(TabWindow_DataView);
        ui->tabWidget_mainWindow->setCurrentIndex(jsonMainConfig["TabWindow"].toObject()[QString::number(TabWindow_DataView)].toObject()["index"].toInt());
    }
}


void MainWindow::addTabWidget(TabWindow window)
{
    QJsonObject tabJson = jsonMainConfig["TabWindow"].toObject();
    if(TabWindow_VideoPlayback == window){
//        std::unique_ptr<VideoPlayback> ui_VideoPlayback = std::make_unique<VideoPlayback>(this);
        VideoPlayback* ui_VideoPlayback = new VideoPlayback(SUB_MAIN);
        QWidget* tabVideoPlayback = ui_VideoPlayback->getVideoPaly();
        if(tabVideoPlayback->objectName() == TabWindowMap()[TabWindow_VideoPlayback]){
            //    QIcon icon_videoPlayback = QIcon::fromTheme("media-playback-start");//系统主题不能用
            QIcon icon(":/asset/Home/playback.svg");
            QJsonObject objVideoPlayback = tabJson[QString::number(TabWindow_VideoPlayback)].toObject();
            if(objVideoPlayback["index"].isNull()){
                objVideoPlayback["index"] = ui->tabWidget_mainWindow->count();
            }
            qDebug() << "MainWindow::addTabWidget(int window) *"<<objVideoPlayback["index"].toInt()<<ui->tabWidget_mainWindow->count();
            ui->tabWidget_mainWindow->insertTab(objVideoPlayback["index"].toInt(), tabVideoPlayback, icon, tr("视频回放"));
            mapTabIndexToWindow[objVideoPlayback["index"].toInt()] = TabWindow_VideoPlayback;
            tabJson[QString::number(TabWindow_VideoPlayback)] = objVideoPlayback;
        }
    }else if (TabWindow_DataView==window){
        DataView *ui_DataView = new DataView(SUB_MAIN);
        QWidget *tabDataView= ui_DataView->getDataView();
        if(tabDataView->objectName() == TabWindowMap()[TabWindow_DataView]){
            QIcon icon(":/asset/Home/DataView.svg");
            QJsonObject objDataView = tabJson[QString::number(TabWindow_DataView)].toObject();
            if(objDataView["index"].isNull()){
                objDataView["index"] = ui->tabWidget_mainWindow->count();
            }
            ui->tabWidget_mainWindow->insertTab(objDataView["index"].toInt(), tabDataView, icon, tr("数据图表"));
            mapTabIndexToWindow[objDataView["index"].toInt()] = TabWindow_DataView;
            tabJson[QString::number(TabWindow_DataView)] = objDataView;
        }
    }else{
        qDebug()<<"window 没有定义对应的ui"<<window;
    }

    jsonMainConfig["TabWindow"] = tabJson;
    qDebug()<<"addTabWidget(int window):"<<tabJson<<jsonMainConfig;
}

void MainWindow::TabCloseRequested(int index)
{
    qDebug() << "TabCloseRequested(int index):" << index;
    QJsonObject tabJson = jsonMainConfig["TabWindow"].toObject();
    QMap<int,TabWindow> newTabIndexToWindow;

    if(mapTabIndexToWindow.contains(index)){
        foreach (QString key, tabJson.keys()) {
            int objectIndex = tabJson[key].toObject()["index"].toInt();
            if(objectIndex==index){
                tabJson.remove(key);
                QWidget *tabWindow = ui->tabWidget_mainWindow->widget(index);
                ui->tabWidget_mainWindow->removeTab(index);
                delete tabWindow;
//                tabWindow->deleteLater();
//                tabWindow->destroyed();
            }else if(objectIndex>index){
                QJsonObject tabIndex = tabJson[key].toObject();
                tabIndex["index"] = tabIndex["index"].toInt() - 1;
                tabJson[key] = tabIndex;
                newTabIndexToWindow[tabIndex["index"].toInt()] = TabWindow(key.toInt());

            }else{
                newTabIndexToWindow[objectIndex] = TabWindow(key.toInt());
            }
        }
    }else{
        qWarning()<<"mapTabIndexToWindow.contains(index) no key"<<index<<mapTabIndexToWindow;
    }

    jsonMainConfig["TabWindow"] = tabJson;
    mapTabIndexToWindow = newTabIndexToWindow;
    qDebug() << "TabCloseRequested(int index) 步骤4:" << tabJson<<mapTabIndexToWindow;

}

void MainWindow::TabCurrentChanged(int index)
{
    qDebug() << "TabCurrentChanged(int index):当前选中" << index;
        jsonMainConfig["TabCurrent"] = index;
    //    QTabBar *tabBar = ui->tabWidget_mainWindow->tabBar();
    //    tabBar->setTabsClosable(false);
    //    ui->tabWidget_mainWindow->setTabBar(tabBar);

//    ui->tabWidget_mainWindow->tabBar()->tabButton(index,QTabBar::RightSide)->setVisible(false);
    QWidget *button = ui->tabWidget_mainWindow->tabBar()->tabButton(tabCloseLastShowNum,QTabBar::RightSide);
    if(button){
//        qDebug() << "TabCurrentChanged(int index):当前选中tabCloseLastShowNum false"<<tabCloseLastShowNum;
        button->setVisible(false);
    }
    button = ui->tabWidget_mainWindow->tabBar()->tabButton(index,QTabBar::RightSide);
    if(button){
//        qDebug() << "TabCurrentChanged(int index):当前选中 ture"<<index;
        button->setVisible(true);
        tabCloseLastShowNum = index;
    }

}

