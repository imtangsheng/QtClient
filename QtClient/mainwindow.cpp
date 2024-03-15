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
    connect(ui_TitleBar,&TitleBar::posChange,this,&MainWindow::geometryChanged);
//    ui_header = ui_WidgetHeader->getUi();
    setMenuWidget(ui_TitleBar);
//    ui->statusBar->addWidget();
//    addDockWidget(Qt::BottomDockWidgetArea,ui->WidgetStatus);
    ui->statusBar->addPermanentWidget(ui->widget_statusBarTitle,1);//拉伸系数(stretch factor)设置最大占满，默认0是在在右侧
//    ui->statusBar->showMessage();
    connect(ui->statusBar,&QStatusBar::showMessage,this,&MainWindow::showMessage);
    connect(ui->statusBar,&QStatusBar::clearMessage,this,&MainWindow::clearMessage);

    _Awake();
}

void MainWindow::showUI()
{
    qDebug() << "MainWindow::show() 当前登录用户："<<CurrentUser;
    showMessage("当前登录用户：" +CurrentUser);
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

    delete SUB_MAIN;
//    deleteLater();//直接使用会奔溃
    qDebug() << "MainWindow::closeEvent(QCloseEvent *event) END";
}

MainWindow::~MainWindow()
{
    qDebug() << "MainWindow::~MainWindow() delete this";
    delete ui;
    //    qApp->quit();
}

void MainWindow::_Awake()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
//    setWindowFlags( Qt::FramelessWindowHint);
//    setWindowFlags(Qt::CustomizeWindowHint);
//    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);
    ui->WidgetStatus->setVisible(false);
    ui->toolButton_WidgetStatus_isStaysOnTopHint->setVisible(false);

}

/*
 * Qt::Widget	0x00000000	这是 的默认类型。如果这种类型的小组件有父级，则为子小组件，如果没有父级，则为独立窗口。参见 Qt：：Window 和 Qt：：SubWindow。
 * Qt::Window 指示小组件是一个窗口，通常具有窗口系统框架和标题栏，而不管小组件是否具有父项。请注意，如果小部件没有父级，则无法取消设置此标志。
 * Qt::FramelessWindowHint	0x00000800	生成无边框窗口。
 * Qt::CustomizeWindowHint	0x02000000	关闭默认窗口标题提示。
 * Qt::WindowStaysOnTopHint	0x00040000	通知窗口系统该窗口应位于所有其他窗口的顶部。请注意，在 X11 上的某些窗口管理器上，您还必须传递 Qt：：X11BypassWindowManagerHint 才能使此标志正常工作。
Qt::CustomizeWindowHint 【可以缩放】，【不能拖动】，【最上面会有白边】，最大化会消失不能双击放大
Qt::Widget | Qt::CustomizeWindowHint
Qt::Window | Qt::CustomizeWindowHint
Qt::FramelessWindowHint 无边框和标题，【只有右下角可以缩放】，【不能拖动】，不能双击放大
Qt::Window | Qt::FramelessWindowHint
*/
#include <QJsonArray>
#include <QTimer>
#include <QWindow>
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
//    ui->dockWidget_test->setFloating(true);
//    addDockWidget(Qt::BottomDockWidgetArea,ui->dockWidget_test);
    ui->WidgetStatus->raise();
//    ui->dockWidget_test->setTitleBarWidget(ui_TitleBar);
    //    SUB_MAIN->show();
//    addDockWidget(Qt::TopDockWidgetArea,SUB_MAIN->ui->dockWidget);
    //    setCorner(Qt::BottomLeftCorner,Qt::BottomDockWidgetArea);

    qDebug() << "MainWindow::test  mapTabIndexToWindow"<<mapTabIndexToWindow;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    qDebug() << "MainWindow::mousePressEvent(QMouseEvent *"<<event->button();
    showMessage("MainWindow::mousePressEvent(QMouseEvent *");
    return QMainWindow::mousePressEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    qDebug() << "MainWindow::resizeEvent(QResizeEvent"<<event->size();
    QMainWindow::resizeEvent(event);
    geometryChanged(this->pos());

}

void MainWindow::paintEvent(QPaintEvent *event)
{
    qDebug() << "MainWindow::paintEvent(QPaintEvent *"<<event->type();

    QMainWindow::paintEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
    qDebug() << "MainWindow::changeEvent(QEvent *"<<event->type();
    QMainWindow::changeEvent(event);
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

void MainWindow::geometryChanged(const QPoint& pos)
{
    qDebug() << "MainWindow::geometryChanged(const QPoint *"<<pos;
    if(ui->WidgetStatus->isFloating())    {
        QPoint posW = mapFromGlobal(QPoint(0, 0)); //负数
        qDebug() << "dockWidget::resizeEvent(QResizeEvent"<<posW.toPointF();
        ui->WidgetStatus->raise();
        ui->WidgetStatus->setGeometry(-posW.x(),
                                      -posW.y() + height() -ui->WidgetStatus->height() ,
                                         width(),
                                      ui->WidgetStatus->height());
    }

}

void MainWindow::showMessage(const QString &text, int timeout)
{
    ui->label_statusBarMsg->setText(text);
    if(timeout>0){
        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, &MainWindow::clearMessage);
        timer->start(timeout);
    }
}

void MainWindow::clearMessage()
{
    ui->label_statusBarMsg->setText("");
}

void MainWindow::on_toolButton_WidgetStatus_isFloatable_clicked()
{
    qDebug() << "MainWindow::on_toolButton_WidgetStatus_isFloatable_clicked()"<<ui->WidgetStatus->isFloating();
    if(isFloatableWidgetStatus){
//        qDebug() << "MainWindow::on_toolButton_WidgetStatus_isFloatable_clicked()11"<<ui->WidgetStatus->isFloating();
        isFloatableWidgetStatus = false;
        ui->WidgetStatus->setFloating(false);

        ui->toolButton_WidgetStatus_isFloatable->setToolTip("浮动");
        ui->toolButton_WidgetStatus_isFloatable->setIcon(QIcon(":/asset/Home/double-arrow-top.svg"));

        ui->statusBar->addPermanentWidget(ui->widget_statusBarTitle,1);
        ui->WidgetStatus->setVisible(false);
        ui->toolButton_WidgetStatus_isStaysOnTopHint->setVisible(false);//置顶前提是要先显示

    }else{
        isFloatableWidgetStatus = true;
//        qDebug() << "MainWindow::on_toolButton_WidgetStatus_isFloatable_clicked()22"<<ui->WidgetStatus->isFloating()<<ui->WidgetStatus->windowFlags();
        ui->statusBar->removeWidget(ui->widget_statusBarTitle);
        ui->WidgetStatus->setFloating(true);

        ui->toolButton_WidgetStatus_isFloatable->setToolTip("取消浮动");
        ui->toolButton_WidgetStatus_isFloatable->setIcon(QIcon(":/asset/Home/double-arrow-down.svg"));

//        ui->WidgetStatus->setWindowFlags(Qt::Window);
        ui->WidgetStatus->setTitleBarWidget(ui->widget_statusBarTitle);
//        ui->WidgetStatus->setTitleBarWidget(ui->label_statusBarMsg);
        geometryChanged();
        ui->WidgetStatus->setVisible(true);
        ui->toolButton_WidgetStatus_isStaysOnTopHint->setVisible(true);//置顶前提是要先显示

    }
//    qDebug() << "MainWindow::on_toolButton_isStaysOnTopHint_clicked()置顶"<<ui->statusBar->geometry()<<ui->widget_statusBarTitle->geometry();
}


void MainWindow::on_toolButton_WidgetStatus_isStaysOnTopHint_clicked()
{
    qDebug() << "MainWindow::on_toolButton_WidgetStatus_isStaysOnTopHint_clicked()";
    Qt::WindowFlags flags = ui->WidgetStatus->windowFlags();
    bool isOnTop = flags.testFlags(Qt::WindowStaysOnTopHint);
    qDebug()<<flags<<isOnTop;
    if(isOnTopWidgetStatus){
        isOnTopWidgetStatus = false;

        ui->toolButton_WidgetStatus_isStaysOnTopHint->setToolTip("置顶");
        ui->toolButton_WidgetStatus_isStaysOnTopHint->setIcon(QIcon(":/asset/Home/on_top.svg"));

//        qDebug() << "MainWindow::on_toolButton_isStaysOnTopHint_clicked()取消置顶";
        QWindow* pWin = ui->WidgetStatus->windowHandle();
        pWin->setFlag(Qt::WindowStaysOnTopHint,false);
//        ui->WidgetStatus->setWindowFlag(Qt::WindowStaysOnTopHint,false);
        qDebug() << "MainWindow::on_toolButton_isStaysOnTopHint_clicked()"<<ui->WidgetStatus->isHidden();
//        ui->WidgetStatus->show();//会发生闪动，瞬间消失再显示
//        pWin->setFlag(Qt::WindowStaysOnTopHint,false);
//        pWin->setFlags(Qt::Widget | Qt::WindowStaysOnTopHint);
    } else {
        // 置顶
        isOnTopWidgetStatus = true;

        ui->toolButton_WidgetStatus_isStaysOnTopHint->setToolTip("取消置顶");
        ui->toolButton_WidgetStatus_isStaysOnTopHint->setIcon(QIcon(":/asset/Home/on_top_cancel.svg"));

        QWindow* pWin = ui->WidgetStatus->windowHandle();
//        ui->WidgetStatus->setWindowFlag(Qt::WindowStaysOnTopHint,true);
        qDebug() << "MainWindow::on_toolButton_isStaysOnTopHint_clicked()"<<ui->WidgetStatus->isHidden();
//        ui->WidgetStatus->show();
        pWin->setFlag(Qt::WindowStaysOnTopHint,true);
    }
//    qDebug() << "MainWindow::on_toolButton_isStaysOnTopHint_clicked()置顶"<<ui->statusBar->geometry()<<ui->widget_statusBarTitle->geometry();
}
