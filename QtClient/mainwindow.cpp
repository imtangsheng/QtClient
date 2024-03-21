#include <QJsonArray>
#include <QTimer>
#include <QWindow>
#include <QMessageBox>
#include <QDir>
#include <QFile>
#include <QIcon>
#include <QTabBar>
#include <QPluginLoader>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "ui/VideoPlayback.h"
#include "ui/DataView.h"
#include "ui/SubMain.h"
SubMain *SUB_MAIN;

#define VARNAME(var) #var

inline QString i2s(int num) {
    return QString::number(num);
}

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
    _Awake();
}

void MainWindow::showUI()
{
    qDebug() << "MainWindow::show() 当前登录用户："<<CurrentUser;
    showMessage("当前登录用户：" +CurrentUser);

    init();
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
void MainWindow::_Awake()
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
//    setWindowFlags( Qt::FramelessWindowHint);
//    setWindowFlags(Qt::CustomizeWindowHint);
//    setWindowFlags(Qt::Window | Qt::CustomizeWindowHint);

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
                                              foreach (PluginInterface* plug, pluginList) {
//            ui->layout_tabMain->addWidget(plug->getHomeTiler());
            ui->layout_tabMain->addWidget(plug->getWidgetByName(HomeMenu_WidgetName));
//            ui->tabMainTest->addWidget(plug->getHomeTiler());
//            ui->tabMainTest->addWidget(SUB_MAIN->ui->widget_test);

        }
//        ui->tab_main->layout()->addItem(ui->verticalSpacer_tabMain);
    }
    qDebug() << "MainWindow::init() end";
}

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
    foreach (const QString key, jsonMainConfig["TabWindow"].toObject().keys()) {
        QJsonObject objKey = jsonMainConfig["TabWindow"].toObject()[key].toObject();
        switch (objKey["type"].toInt()) {
        case TabWindowType_Self:
            addTabWidget(TabWindow(objKey["window"].toInt()));
            break;
        case TabWindowType_Plugin:
            //判断加载的插件中是否有这个插件名称object name
            qDebug()<<"pluginNameToListIndex"<<pluginGetListIndexFromName<<objKey;
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
    qDebug()<<"ui->tabWidget_mainWindow->count()格式的配置："<<ui->tabWidget_mainWindow->count();
    for(int index = 1; index < ui->tabWidget_mainWindow->count();index++){
        //初始化Tab标签的数据和其他配置
//        int index = jsonMainConfig["TabWindow"].toObject()[key].toObject()["index"].toInt();
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
//    qDebug() << "MainWindow::paintEvent(QPaintEvent *"<<event->type();

    QMainWindow::paintEvent(event);
}

void MainWindow::changeEvent(QEvent *event)
{
//    qDebug() << "MainWindow::changeEvent(QEvent *"<<event->type();
    QMainWindow::changeEvent(event);
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
        ui->tabWidget_mainWindow->setCurrentIndex(ui->tabWidget_mainWindow->count() - 1);
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
        ui->tabWidget_mainWindow->setCurrentIndex(ui->tabWidget_mainWindow->count() - 1);
    }
}


void MainWindow::addTabWidget(TabWindow window)
{
    QJsonObject tabJson = jsonMainConfig["TabWindow"].toObject();
    QWidget * addWidget;
    QIcon icon;
    QString name;
    switch (window) {
    case TabWindow_VideoPlayback:{
        VideoPlayback* ui_VideoPlayback = new VideoPlayback(SUB_MAIN);
        addWidget = ui_VideoPlayback->getVideoPaly();
        icon = QIcon(":/asset/Home/playback.svg");
        name = tr("视频回放");
        break;} //以限定变量的作用域
    case TabWindow_DataView:{
        DataView *ui_DataView = new DataView(SUB_MAIN);
        addWidget = ui_DataView->getDataView();
        icon = QIcon(":/asset/Home/DataView.svg");
        name = tr("数据图表");
        break;}
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
    qDebug()<<"加载插件EchoWindow::loadPlugin():"<<pluginsDir;
    const QStringList entries = pluginsDir.entryList(QDir::Files);
    pluginList.clear();
    for (const QString &fileName : entries) {
        qDebug()<<"加载插件Window::loadPlugin():"<<entries<<fileName;
        QPluginLoader pluginLoader(pluginsDir.absoluteFilePath(fileName),this);
        QObject *plugin = pluginLoader.instance();
        if (plugin) {
            PluginInterface* inter = qobject_cast<PluginInterface*>(plugin);
            if(inter){
                qDebug()<<"加载插件Window::"<<pluginList.count()<<inter->id;
                qDebug() << "MainWindow::init()"<<"插件加载界面";
                inter->id = pluginList.count();
                pluginList.append(inter);
                qDebug()<<"加载插件Window::loadPlugin() name:"<<pluginLoader.metaData().value("MetaData").toObject().value("Name").toString();
                //信号连接字符方法，可以使用建立一个全局唯一的信号类，建立一个返回该类的方法从而使用指针方法发送信号
                connect(plugin,SIGNAL(signalShowMainWidget(int,QString)),this,SLOT(jump_ShowMainTabWidget(int,QString)));
                //记录插件对应的lilst下标
                qDebug()<<"加载插件Window::"<<pluginList.count()<<inter->id<<inter->ObjectName<<inter->getObjectNane();
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
    ui->tabWidget_mainWindow->insertTab(indexTab,
                addWidget,
                                        pluginList.at(index)->WindowIcon,
                                        pluginList.at(index)->WindowTitle);

    tabJson[i2s(indexTab)] = objPlugin;
    jsonMainConfig["TabWindow"] = tabJson;
    return true;
}
