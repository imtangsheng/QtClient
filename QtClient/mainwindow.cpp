#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/VideoPlayback.h"
#include "ui/DataView.h"
#include "public/AppSystem.h"
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QIcon>

#define VARNAME(var) #var

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui_TitleBar = new TitleBar(this);
//    ui_header = ui_WidgetHeader->getUi();
    setMenuWidget(ui_TitleBar);

}

void MainWindow::showUI()
{
    qDebug() << "MainWindow::show() 当前登录用户："<<CurrentUser;
    init();
    test();
    show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow::closeEvent(QCloseEvent *event)" << event;
    //    qApp->quit(); //重载主程序点击退出事件，软件退出
    APP_SETTINGS.beginGroup("MainWindow");
    APP_SETTINGS.setValue("geometry", saveGeometry());
    APP_SETTINGS.setValue("jsonSettingsVariable",jsonSettingsVariable);
//    APP_SETTINGS.setValue("jsonSettingsVariable",QJsonDocument::fromVariant(jsonSettingsVariable).toJson());
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
#include <QJsonArray>
void MainWindow::init()
{
    qDebug() << "MainWindow::init 使用版本号：" << EXE_CONFIG["version"];
//    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    setWindowFlags( Qt::FramelessWindowHint);
//    setContentsMargins(-9,-9,-9,-9);
    //    this->setContentsMargins(2,2,2,2); //设置QWidget内部内容的边距
    ui->tabWidget_mainWindow->setAttribute(Qt::WA_TranslucentBackground); //窗体透明

    // 连接删除请求的槽函数
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabCloseRequested, this, &MainWindow::TabCloseRequested);
    connect(ui->tabWidget_mainWindow, &QTabWidget::currentChanged, this, &MainWindow::TabCurrentChanged);

    APP_SETTINGS.beginGroup("MainWindow");
    const auto geometry = APP_SETTINGS.value("geometry", QByteArray()).toByteArray(); // QByteArray 类型
    if (geometry.isEmpty())
        setGeometry(200, 200, 800, 600);
    else
        restoreGeometry(geometry);


    jsonSettingsVariable = APP_SETTINGS.value("jsonSettingsVariable",QJsonObject()).toJsonObject();
    //    const auto list2 = APP_SETTINGS.value("list",QVariant::fromValue(QList<int>{}));
    //    QList<int> list4 = APP_SETTINGS.value("list").value<QList<int>>();
    APP_SETTINGS.endGroup();

    qDebug()<<"jsonSettingsVariable 格式的配置："<<jsonSettingsVariable;
//    foreach (const QJsonValue &value, jsonSettingsVariable["TabWindow"].toArray()) {
//        //        qDebug()<<tab<< jsonSettingsVariable["TabWindow"].toArray();
//        qDebug()<<"TabWindow:"<<value.toInt();
//    }
//    foreach (TabWindow tab, jsonSettingsVariable["TabWindow"].toVariant().value<QList<TabWindow>>()) {
////        qDebug()<<tab<< jsonSettingsVariable["TabWindow"].toArray();
//    }

//    QJsonObject object = QJsonDocument::fromJson("{\"a\":{\"1\":[1,2,3]},\"b\":{\"2\":1}}").object();
//    qDebug()<<"QJsonObject object:"<<object;
//    QJsonObject &ref = object; jsonSettingsVariable["TabWindow"]
//    QJsonObject *ref2 = &object;
//    ref["key"] = "value";
//    qDebug()<<"QJsonObject object:"<<object<<ref;
//    *ref2 = {{"property1", 1},{"property2", 2}};
//    qDebug()<<"QJsonObject object:"<<object<<ref<<*ref2;
//    jsonTest = &object;
//    *jsonTest = {{"property1", 2},{"property2", 4}};
//    qDebug()<<"QJsonObject object:"<<object<<ref<<*ref2;

}

void MainWindow::test()
{
    qDebug() << "MainWindow::test";
    foreach (const QString key, jsonSettingsVariable["TabWindow"].toObject().keys()) {
        qDebug()<<"QJsonObject object key:"<<key;
        qDebug()<<"QJsonObject object:"<<jsonSettingsVariable["TabWindow"].toObject()[key].toObject();
        //初始化加载页面
        addTabWidget(key.toInt());
    }
    ui->tabWidget_mainWindow->setCurrentIndex(jsonSettingsVariable["TabCurrent"].toInt());

}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "MainWindow::mousePressEvent(QMouseEvent *"<<event->button();
    return QMainWindow::mousePressEvent(event);
}

void MainWindow::on_Button_videoPlayback_clicked()
{
    qDebug() << "on_pushButton_video_playback_clicked()";
//    for(int i=0;i<ui->tabWidget_mainWindow->count();i++){
//        QWidget* tab = ui->tabWidget_mainWindow->widget(i);
//        qDebug() << "tabWidget_mainWindow->widget on_pushButton_video_playback_clicked()"<<tab->objectName();
//    }
    // 判断是否存在指定名称的Tab页
    QWidget* existingTab = ui->tabWidget_mainWindow->findChild<QWidget*>(TabWindowMap()[TabWindow_VideoPlayback]);
    if(existingTab){
        ui->tabWidget_mainWindow->setCurrentWidget(existingTab);
    }else{
        addTabWidget(TabWindow_VideoPlayback);
        ui->tabWidget_mainWindow->setCurrentIndex(jsonSettingsVariable["TabWindow"].toObject()[QString::number(TabWindow_VideoPlayback)].toObject()["index"].toInt());
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
        ui->tabWidget_mainWindow->setCurrentIndex(jsonSettingsVariable["TabWindow"].toObject()[QString::number(TabWindow_DataView)].toObject()["index"].toInt());
    }
}


void MainWindow::addTabWidget(int window)
{
    QJsonObject tabJson = jsonSettingsVariable["TabWindow"].toObject();
//    TabWindow tabW = static_cast<TabWindow>(window);
    if(TabWindow_VideoPlayback == window){
        VideoPlayback *ui_VideoPlayback = new VideoPlayback();
        QWidget *tabVideoPlayback = ui_VideoPlayback->getVideoPaly();
        if(tabVideoPlayback->objectName() == TabWindowMap()[TabWindow_VideoPlayback]){
            //    QIcon icon_videoPlayback = QIcon::fromTheme("media-playback-start");//系统主题不能用
            QIcon icon(":/asset/Home/playback.svg");
            QJsonObject objVideoPlayback = tabJson[QString::number(TabWindow_VideoPlayback)].toObject();
            if(objVideoPlayback["index"].isNull()){
                objVideoPlayback["index"] = ui->tabWidget_mainWindow->count();
            }
            ui->tabWidget_mainWindow->insertTab(objVideoPlayback["index"].toInt(), tabVideoPlayback, icon, tr("视频回放"));
            tabJson[QString::number(TabWindow_VideoPlayback)] = objVideoPlayback;

        }
    }else if (TabWindow_DataView==window){
        DataView *ui_DataView = new DataView();
        QWidget *tabDataView= ui_DataView->getDataView();
        if(tabDataView->objectName() == TabWindowMap()[TabWindow_DataView]){
            QIcon icon(":/asset/Home/DataView.svg");
            QJsonObject objDataView = tabJson[QString::number(TabWindow_DataView)].toObject();
            if(objDataView["index"].isNull()){
                objDataView["index"] = ui->tabWidget_mainWindow->count();
            }
            ui->tabWidget_mainWindow->insertTab(objDataView["index"].toInt(), tabDataView, icon, tr("数据图表"));
            tabJson[QString::number(TabWindow_DataView)] = objDataView;
        }
    }else{
        qDebug()<<"window 没有定义对应的ui"<<window;
    }

    jsonSettingsVariable["TabWindow"] = tabJson;
    qDebug()<<"addTabWidget(int window):"<<tabJson<<jsonSettingsVariable;
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
    jsonSettingsVariable["TabCurrent"] = index;
    //    QTabBar *tabBar = ui->tabWidget_mainWindow->tabBar();
    //    tabBar->setTabsClosable(false);
    //    ui->tabWidget_mainWindow->setTabBar(tabBar);
}

