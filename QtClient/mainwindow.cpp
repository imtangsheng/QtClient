#include <QJsonArray>
#include <QTimer>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QTabBar>
#include <QPluginLoader>
#include "public/AppSystem.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

#define TIMEMS QTime::currentTime().toString("hh:mm:ss zzz")

#include "ui/SubMain.h"

#include "modules/sqlite.h"
#include "modules/httpserver.h"

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
//    connect(ui->statusBar,&QStatusBar::showMessage,this,&MainWindow::showMessage);
//    connect(ui->statusBar,&QStatusBar::clearMessage,this,&MainWindow::clearMessage);
    SUB_MAIN = new SubMain;
    gSql = SQLite::instance(nullptr,"test.db");
    gHttpServer = HttpServer::instance(this);
    _Awake();

}

void MainWindow::showUI()
{
    qDebug() << "MainWindow::show() 当前登录用户："<<CurrentUser;
    showMessage("当前登录用户：" +CurrentUser);

    init();
    _Start();

    ui->dateTimeEdit_events_time_begin->setDateTime(QDateTime::currentDateTime().date().startOfDay());
    ui->dateTimeEdit_events_time_end->setDateTime(QDateTime::currentDateTime().date().endOfDay());

    show();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug() << "MainWindow::closeEvent(QCloseEvent *event)" << event;
    //    qApp->quit(); //重载主程序点击退出事件，软件退出
    AppSettings.beginGroup("MainWindow");
    AppSettings.setValue("geometry", saveGeometry());
    AppSettings.setValue("jsonSettingsVariable",jsonMainConfig);
//    APP_SETTINGS.setValue("jsonSettingsVariable",QJsonDocument::fromVariant(jsonSettingsVariable).toJson());
    AppSettings.endGroup();
    emit quit();
    delete SUB_MAIN;
    qDebug() << "MainWindow::closeEvent(QCloseEvent *event) END";
}

MainWindow::~MainWindow()
{
    delete ui;
    //    qApp->quit();
    qDebug() << "MainWindow::~MainWindow() delete this";
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
Qt::WindowMinimizeButtonHint 可以任务栏最大最小化
*/
void MainWindow::_Awake()
{
    // 定义文件路径
    QString faviconFilePath = "favicon.png";
    // 判断文件是否存在
    if (QFile(faviconFilePath).exists()) {
        // 文件存在，加载并显示图像
        setWindowIcon(QIcon(faviconFilePath));
    }

    //setWindowFlags(windowFlags() | Qt::FramelessWindowHint);//可以任务栏图标点击最小化,但是不可以触发调节大小
    //setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    // 启用窗口可调整大小
//    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::FramelessWindowHint);
    //setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
//    setWindowFlags( Qt::FramelessWindowHint);//可以调节大小,但是不可以触发任务栏图标点击最小化
    setWindowFlags(windowFlags() | Qt::CustomizeWindowHint);//不知道为啥会没有顶部白线，有nativeEvent 处理后

    ui->toolButton_WidgetStatus_isStaysOnTopHint->setVisible(false);
//    ui->toolButton_WidgetStatus_isStaysOnTopHint->setParent(ui->WidgetStatus); //无用，还是会不显示
    ui->WidgetStatus->setVisible(false);

    /* MainWindow::ui->tabWidget_mainWindow 初始化加载 */
    // 连接删除请求的槽函数
    connect(ui->tabWidget_mainWindow, &QTabWidget::tabCloseRequested, this, &MainWindow::TabCloseRequested);
    connect(ui->tabWidget_mainWindow, &QTabWidget::currentChanged, this, &MainWindow::TabCurrentChanged);

}

void MainWindow::init()
{
    qDebug() << "MainWindow::init()";
    if(pluginLoad()){
        qDebug() << "MainWindow::插件数量"<<pluginList.count();
        foreach (PluginInterface* plugin, pluginList) {
            plugin->Start();
            ui->layout_tabMain->addWidget(plugin->getWidgetByName(HomeMenu_WidgetName));

        }
//        ui->tab_main->layout()->addItem(ui->verticalSpacer_tabMain);
    }
    qDebug() << "MainWindow::init() end";
}

void MainWindow::_Start()
{
    qDebug() << "MainWindow::_Start() 使用版本号：" << AppJson["version"];
    AppSettings.beginGroup("MainWindow");
    const auto geometry = AppSettings.value("geometry", QByteArray()).toByteArray(); // QByteArray 类型
    if (geometry.isEmpty())
        setGeometry(200, 200, 800, 600);
    else
        restoreGeometry(geometry);

    jsonMainConfig = AppSettings.value("jsonSettingsVariable",QJsonObject()).toJsonObject();
    //    const auto list2 = APP_SETTINGS.value("list",QVariant::fromValue(QList<int>{}));
    //    QList<int> list4 = APP_SETTINGS.value("list").value<QList<int>>();
    AppSettings.endGroup();
    //qDebug()<<"jsonSettingsVariable 格式的配置："<<jsonMainConfig;
    foreach (const QString key, jsonMainConfig["TabWindow"].toObject().keys()) {
        QJsonObject objKey = jsonMainConfig["TabWindow"].toObject()[key].toObject();
        switch (objKey["type"].toInt()) {
        case TabWindowType_Self:
            addTabWidget(objKey["window"].toInt());
            break;
        case TabWindowType_Plugin:
            //判断加载的插件中是否有这个插件名称object name
            //qDebug()<<"pluginNameToListIndex"<<pluginGetListIndexFromName<<objKey;
            if(pluginGetListIndexFromName.contains(objKey["object"].toString())){
                pluginTabInsertMainWindow(pluginGetListIndexFromName[objKey["object"].toString()],objKey["name"].toString());
            }else{
                qWarning()<<objKey["name"].toString()<<"名为的插件不存在";
            }
            break;
        default:
            break;
        }
    }

    ui->tabWidget_mainWindow->setTabsClosable(true);
    // 要设置第1个选项卡不可关闭
    ui->tabWidget_mainWindow->tabBar()->setTabButton(0,QTabBar::RightSide,nullptr);
    //qDebug()<<"ui->tabWidget_mainWindow->count()格式的配置："<<ui->tabWidget_mainWindow->count();
    for(int index = 1; index < ui->tabWidget_mainWindow->count();index++){
        //初始化Tab标签的数据和其他配置
//        int index = jsonMainConfig["TabWindow"].toObject()[key].toObject()["index"].toInt();
        ui->tabWidget_mainWindow->tabBar()->tabButton(index,QTabBar::RightSide)->setVisible(false);
    }
    ui->tabWidget_mainWindow->setCurrentIndex(jsonMainConfig["TabCurrent"].toInt());

    /*事件中心显示操作*/
    QSqlError error = gSql->init_EventCenter();
    if(error.isValid()){
        // 初始化失败
        qWarning() << "Failed to initialize Event Center MainWindow:" << error.text();
        //return ;
    }else {
        ui->tableView_events->setModel(gSql->EventCenter_Model);
        ui->tableView_events->resizeColumnsToContents();
        ui->tableView_events->setSortingEnabled(true);
        ui->tableView_events->setItemDelegateForColumn(ItemDelegateEventLevel,new EventLevelDelegate);
        ui->tableView_events->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        ui->tableView_events->horizontalHeader()->setSectionResizeMode(5,QHeaderView::Stretch);//details 5
    }
    /*http服务事件中心显示操作记录*/
    gHttpServer->route_add_sql();
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


}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
//    qDebug() << "MainWindow::mousePressEvent(QMouseEvent *"<<event->button();
    return QMainWindow::mousePressEvent(event);
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
//    qDebug() << "MainWindow::resizeEvent(QResizeEvent"<<event->size();
    QMainWindow::resizeEvent(event);
    geometryChanged(this->pos());

}

void MainWindow::paintEvent(QPaintEvent *event)
{
//    qDebug() << "MainWindow::paintEvent(QPaintEvent *"<<event->type();

    QMainWindow::paintEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
//    qDebug() << "MainWindow::changeEvent(QEvent *"<<event->type();
    QMainWindow::changeEvent(event);
}

#ifdef Q_OS_WIN
#include "windows.h"
#include "windowsx.h"
#endif
bool MainWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    //qDebug() << "MainWindow::nativeEvent(const QByteArray &"<<eventType<<message<<*result;
    // 检查事件类型是否为窗口大小变化事件
    //windows_generic_MSG表示通用的Windows消息，包括诸如鼠标、键盘、窗口大小调整等各种类型的消息
    //windows_dispatcher_MSG表示分发器消息，用于在Qt的事件循环中分发和处理Windows消息
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG")
    {
        MSG* msg = static_cast<MSG*>(message);
        //qDebug()<<TIMEMS<<"nativeEvent"<<msg->wParam<<msg->message;
        if(msg->message == WM_NCCALCSIZE) {
            //WM_NCCALCSIZE是一个Windows消息，用于通知窗口系统计算非客户区（non-client area）的大小和位置。非客户区包括窗口的边框、标题栏、系统菜单和窗口装饰等。
            *result = 0;return true;
        } else if (msg->message == WM_NCHITTEST || msg->message == WM_NCMOUSEMOVE)
        {
            //qDebug()<<TIMEMS<<"nativeEvent 鼠标在窗口移动的事件"<<msg->wParam<<msg->message<<WM_NCMOUSEMOVE;
            //WM_NCMOUSEMOVE是一个Windows消息，用于通知窗口鼠标在窗口的非客户区（非工作区）移动的事件。
            //WM_NCHITTEST：当鼠标在窗口的非客户区（非工作区）移动时，系统会发送这个消息给窗口。该消息用于确定鼠标指针在非客户区的哪个部分。例如拖动窗口或调整窗口大小。
            //WM_MOUSEMOVE：当鼠标在窗口内移动时，系统会发送这个消息给窗口。该消息用于通知窗口鼠标的当前位置。例如更新鼠标指针的样式或响应鼠标移动事件。
            //WM_NCHITTEST是一个Windows消息，用于确定鼠标在窗口的非客户区（non-client area）的哪个部分进行了点击或者悬停。
            // 判断鼠标位置是否在窗口边界
            const int width = this->width();
            const int height = this->height();
            const QPoint localPos = mapFromGlobal(QPoint(GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam)));//#include "windowsx.h"
            if (localPos.x() < borderSize && localPos.y() < borderSize) {*result = HTTOPLEFT;}
            else if (localPos.x() > width - borderSize && localPos.y() < borderSize) {*result = HTTOPRIGHT;}
            else if (localPos.x() < borderSize && localPos.y() > height - borderSize) {*result = HTBOTTOMLEFT;}
            else if (localPos.x() > width - borderSize && localPos.y() > height - borderSize) {*result = HTBOTTOMRIGHT;}
            else if (localPos.x() < borderSize) {*result = HTLEFT;}
            else if (localPos.x() > width - borderSize) {*result = HTRIGHT;}
            else if (localPos.y() < borderSize) {*result = HTTOP;}
            else if (localPos.y() > height - borderSize) {*result = HTBOTTOM;}

            //qDebug()<<TIMEMS<<"*result:"<<*result;
            if (*result != 0) {return true;}

        }/*else if (msg->message == WM_LBUTTONDOWN) {
            // WM_LBUTTONDOWN（消息代码：0x0201）：当用户在任务栏上的应用程序按钮上按下鼠标左键时，系统会发送这个消息给应用程序的主窗口。
            qDebug() << "Clicked on taskbar";
            // 在这里执行你想要的操作
        }*/

    }
#endif //win平台结束

#ifdef Q_OS_MACOS
    if(evnetType == "NSevent"){}
#endif

#ifdef Q_OS_LINUX
    if(eventType == "xcb_generic_event_t") {}
#endif
    return QMainWindow::nativeEvent(eventType, message, result);// 如果不需要处理该事件，调用基类的nativeEvent函数
}

void MainWindow::show_message(const int &level, const QString &message)
{
    switch (level) {
    case 0:
        showMessage(message,5000);
        break;
    default:
        showMessage(message);
        break;
    }
}

void MainWindow::jump_ShowMainTabWidget(int index, QString name)
{
    qDebug() << "MainWindow::jump_ShowMainTabWidget(int "<<index<<name;
    QWidget* existingTab = ui->tabWidget_mainWindow->findChild<QWidget*>(name);
    if(existingTab){
        ui->tabWidget_mainWindow->setCurrentWidget(existingTab);
    }else{
        qDebug() << "MainWindow::jump_ShowMainTabWidget(int "<<pluginList.count()<<pluginList.size();
        pluginTabInsertMainWindow(index,name);
        ui->tabWidget_mainWindow->setCurrentIndex(ui->tabWidget_mainWindow->count() - 1);
    }
}

//void MainWindow::on_Button_dataView_clicked()
//{
//    qDebug() << "on_pushButton_dataview_clicked";
//    QWidget* existingTab = ui->tabWidget_mainWindow->findChild<QWidget*>(TabWindowMap()[TabWindow_DataView]);
//    if(existingTab){
//        ui->tabWidget_mainWindow->setCurrentWidget(existingTab);
//    }else{
//        addTabWidget(TabWindow_DataView);
//        ui->tabWidget_mainWindow->setCurrentIndex(ui->tabWidget_mainWindow->count() - 1);
//    }
//}


void MainWindow::addTabWidget(int window)
{
    QJsonObject tabJson = jsonMainConfig["TabWindow"].toObject();
    QWidget * addWidget;
    QIcon icon;
    QString name;
    switch (window) {
//    case TabWindow_DataView:{
//        DataView *ui_DataView = new DataView(SUB_MAIN);
//        addWidget = ui_DataView->getDataView();
//        icon = QIcon(":/asset/Home/DataView.svg");
//        name = tr("数据图表");
//        break;}
    default:
        qWarning()<<"window 没有定义对应的ui"<<window;
        return;
    }
    int indexTab = ui->tabWidget_mainWindow->count();
    QJsonObject obj = tabJson[i2s(indexTab)].toObject();
    obj["type"] = TabWindowType_Self;
    obj["window"] = window;
    ui->tabWidget_mainWindow->insertTab(indexTab, addWidget, icon,name);
    tabJson[i2s(indexTab)] = obj;
    jsonMainConfig["TabWindow"] = tabJson;
    qDebug()<<"addTabWidget(int window):"<<tabJson<<jsonMainConfig;
}

void MainWindow::TabCloseRequested(int index)
{
    qDebug() << "TabCloseRequested(int index):" << index;
    QJsonObject tabJson = jsonMainConfig["TabWindow"].toObject();
    QJsonObject newTabJson;
    foreach (const QString &key,tabJson.keys()){
        if(key.toInt()<index){
            newTabJson[key] = tabJson[key];
        }else if(key.toInt()>index){
            newTabJson[i2s(key.toInt()-1)] = tabJson[key];
        } else if(key.toInt() == index){
            QWidget *tabWindow = ui->tabWidget_mainWindow->widget(index);
            ui->tabWidget_mainWindow->removeTab(index);
            switch(tabJson[key].toObject()["type"].toInt()){
            case TabWindowType_Self:
                delete tabWindow;
                break;
            case TabWindowType_Plugin:
                pluginList.at(pluginGetListIndexFromName[tabJson[key].toObject()["object"].toString()])->widgetReturnAfterRemoved(tabWindow);
                qWarning() << "TabCloseRequested(int index) 步骤 不存在1:";
                break;
            default:
                delete tabWindow;
                break;
            }
        }else{
            qWarning() << "TabCloseRequested(int index) 步骤 不存在:";
        }
    }
    jsonMainConfig["TabWindow"] = newTabJson;;
    qDebug() << "TabCloseRequested(int index) 步骤4:" << tabJson<<newTabJson;;

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
//        button->setStyleSheet("background-color: transparent;image:;");
    }
    QWidget *buttonCurrent = ui->tabWidget_mainWindow->tabBar()->tabButton(index,QTabBar::RightSide);
    if(buttonCurrent){
//        qDebug() << "TabCurrentChanged(int index):当前选中 ture"<<index;
        buttonCurrent->setVisible(true);
//        buttonCurrent->setStyleSheet("image: url(:/asset/Home/Close.svg);");
        tabCloseLastShowNum = index;
    }

}

void MainWindow::geometryChanged(const QPoint& pos)
{
    //qDebug() << "MainWindow::geometryChanged(const QPoint *"<<pos;
    if(ui->WidgetStatus->isFloating())    {
        QPoint posW = mapFromGlobal(QPoint(0, 0)); //负数
        qDebug() << "dockWidget::resizeEvent(QResizeEvent"<<posW.toPointF();
        ui->WidgetStatus->raise();
        ui->WidgetStatus->setGeometry(-posW.x(),-posW.y() + height() -ui->WidgetStatus->height(),width(),ui->WidgetStatus->height());
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

#include <QMouseEvent>
#include <QResizeEvent>
#include <QWindow>
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

/*
 * This happens automatically on application termination, so you shouldn't normally need to call this function.
 * If other instances of QPluginLoader are using the same plugin, the call will fail, and unloading will only happen when every instance has called unload().
 * Don't try to delete the root component. Instead rely on that unload() will automatically delete it when needed.
*/
bool MainWindow::pluginLoad()
{
    QDir pluginsDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_WIN)
//    if (pluginsDir.dirName().toLower() == "debug" || pluginsDir.dirName().toLower() == "release")
//        pluginsDir.cdUp();
#elif defined(Q_OS_MAC)
#endif
    pluginsDir.cd("plugins");
    //qDebug()<<"加载插件EchoWindow::loadPlugin():"<<pluginsDir;
    const QStringList entries = pluginsDir.entryList(QDir::Files);
    pluginList.clear();
    for (const QString &fileName : entries) {
        //qDebug()<<"加载插件Window::loadPlugin():"<<entries<<fileName;
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName),this);
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            PluginInterface* inter = qobject_cast<PluginInterface*>(plugin);
            if(inter){
//                qDebug()<<"加载插件Window::"<<pluginList.count()<<inter->id;
                inter->id = pluginList.count();
                pluginList.append(inter);
//                qDebug()<<"加载插件Window::loadPlugin() name:"<<pluginLoader.metaData().value("MetaData").toObject().value("Name").toString();
                //信号连接字符方法，可以使用建立一个全局唯一的信号类，建立一个返回该类的方法从而使用指针方法发送信号
                //1.连接信号和槽函数时使用括号，例如 SIGNAL(quit()) 和 SLOT(quit())，这表示你正在使用字符串来指示信号和槽函数。
                //2.不使用括号：从 Qt 5 开始，你也可以在连接信号和槽函数时不使用括号。这意味着你直接使用函数指针来指示信号和槽函数。这种方式更加直观和类型安全，因为它在编译时进行了检查。
                connect(this,SIGNAL(quit()),plugin,SLOT(quit()));
                connect(plugin,SIGNAL(signalShowMainWidget(int,QString)),this,SLOT(jump_ShowMainTabWidget(int,QString)));
                //新增信息通知方法
                connect(plugin,SIGNAL(signalShowMessage(int,QString)),this,SLOT(show_message(int,QString)));
//                connect(this,SIGNAL(quit),plugin,SLOT(quit));//不会触发该信号
                //记录插件对应的lilst下标
                //qDebug()<<"加载插件Window::"<<pluginList.count()<<inter->id<<inter->ObjectName<<inter->getObjectNane();
                pluginGetListIndexFromName.insert(inter->getObjectNane(),inter->id);
            }
//            pluginLoader.unload();//主动释放会把加载的插件也释放掉
        }
    }
    return pluginList.isEmpty() ? false : true;
//    if(pluginInterface.isEmpty()){return false;}
    //    else{return true;}
}

bool MainWindow::pluginTabInsertMainWindow(int index,QString name)
{
    qDebug()<<"MainWindow::pluginTabInsertMainWindow(int "<<index<<name;
    if( index < 0  || index >= pluginList.count()){
        qDebug() << "pluginInterface下标：" << index << " 不存在";
        return false;
    }
    QWidget * addWidget = pluginList.at(index)->getWidgetByName(name);
    if(!addWidget){
        qDebug() << "pluginInterface界面Widget：" << name << " 不存在";
        return false;
    }
    QJsonObject tabJson = jsonMainConfig["TabWindow"].toObject();
    int indexTab = ui->tabWidget_mainWindow->count();
    QJsonObject objPlugin = tabJson[i2s(indexTab)].toObject();
    objPlugin["type"] = TabWindowType_Plugin;
    objPlugin["name"] = name;
    objPlugin["object"] = pluginList.at(index)->ObjectName;
    
    pluginList.at(index)->indexTabBar = indexTab;
    //ui->tabWidget_mainWindow->insertTab(indexTab,addWidget,pluginList.at(index)->WindowIcon,pluginList.at(index)->WindowTitle);
    ui->tabWidget_mainWindow->insertTab(indexTab,addWidget,addWidget->windowIcon(),addWidget->windowTitle());
    tabJson[i2s(indexTab)] = objPlugin;
    jsonMainConfig["TabWindow"] = tabJson;
    return true;
}

/*事件中心显示操作*/
void MainWindow::on_toolButton_events_query_time_clicked()
{
    qDebug()<<"MainWindow::on_toolButton_events_query_time_clicked()";
    // 设置筛选条件
    gSql->EventCenter_Model->setFilter(QString("time >= '%1' AND time <= '%2'")
                                             .arg(ui->dateTimeEdit_events_time_begin->dateTime().toString(Qt::ISODate))
                                             .arg(ui->dateTimeEdit_events_time_end->dateTime().toString(Qt::ISODate)));
    // 提交筛选条件
    if (!gSql->EventCenter_Model->select()) {
        // 筛选失败，输出错误信息
        qWarning() << "筛选失败，错误信息:" << gSql->EventCenter_Model->lastError().text();
                                                   showMessage("筛选失败，错误信息:"+gSql->EventCenter_Model->lastError().text());
    }
}


void MainWindow::on_toolButton_events_query_value_clicked()
{
    // 获取当前选中单元格的索引
    QModelIndex currentIndex = ui->tableView_events->currentIndex();
    if (!currentIndex.isValid()) {
        // 没有选中单元格,返回 false
        return;
    }
    // 获取选中单元格所在的行和列
    int selectedRow = currentIndex.row();
    int selectedColumn = currentIndex.column();

    // 根据选中单元格的列序号,获取对应的数据
    QString selectedData = gSql->EventCenter_Model->data(gSql->EventCenter_Model->index(selectedRow, selectedColumn)).toString();

    qDebug()<<"test:"<<selectedRow << selectedColumn<<selectedData;
    // 提交筛选条件
    if (!gSql->filter_EventCenter(selectedColumn,selectedData)) {
        // 筛选失败，输出错误信息
        qWarning() << "筛选失败，错误信息:" << gSql->EventCenter_Model->lastError().text();
        showMessage("筛选失败，错误信息:"+gSql->EventCenter_Model->lastError().text());
    }

}

