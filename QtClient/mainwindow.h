#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include "ui/TitleBar.h"
#include <QMap>
#include "PluginInterface.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void Awake_();//初始化运行一次
    void init_();
    void test();
    void Start_();//
//    void _Update();//
//    void _FixedUpdate();//
//    void _LateUpdata();//
//    void _OnEnable();
//    void _OnDestory();
//    void _Close();

    void showUI();
    /*定义Tab窗口的类型*/
    enum TabWindowType
    {
        TabWindowType_Self = 1,
        TabWindowType_Plugin = 2,
    };

signals:
    void onDestorySignal();
    void quit();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
//    void geometryChanged(const QRect &newGeometry, const QRect &oldGeometry) override;
    const int borderSize = 6; //调整窗口大小内边距 修改为 6px
    bool nativeEvent(const QByteArray &eventType,void *message, qintptr *result) override; //qt5使用loog

private slots:
    void show_message(const int& level = -1,const QString& message = "");

    void jump_ShowMainTabWidget(int index,QString name);

    void on_toolButton_WidgetStatus_isFloatable_clicked();

    void on_toolButton_WidgetStatus_isStaysOnTopHint_clicked();

    /*事件中心显示操作*/
    void on_toolButton_events_query_time_clicked();

    void on_toolButton_events_query_value_clicked();

private:
    QJsonObject jsonMainConfig;
    TitleBar *ui_TitleBar;
    Ui::MainWindow *ui;
    QList<PluginInterface*> pluginList;
    bool pluginLoad();
    bool pluginTabInsertMainWindow(int index,QString name);
    QMap<QString,int> pluginGetListIndexFromName;

    void closeEvent(QCloseEvent *event) override;

    int currentTabNum;
    int tabCloseLastShowNum = 0;
    void addTabWidget(int window);
    void TabCloseRequested(int index);
    void TabCurrentChanged(int index);

    bool isFloatableWidgetStatus = false;
    void geometryChanged(const QPoint& pos = QPoint(0, 0));

    void showMessage(const QString &text, int timeout = 0);
    void clearMessage();

    bool isOnTopWidgetStatus = false;
};
#endif // MAINWINDOW_H
