#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#include "ui/TitleBar.h"
#include "public/AppSystem.h"
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

    void _Awake();//初始化运行一次
    void init();
    void test();
    void _Start();//
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

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
//    void geometryChanged(const QRect &newGeometry, const QRect &oldGeometry) override;

private slots:
    void jump_ShowMainTabWidget(int index,QString name);

    void on_Button_videoPlayback_clicked();

    void on_Button_dataView_clicked();

    void on_toolButton_WidgetStatus_isFloatable_clicked();

    void on_toolButton_WidgetStatus_isStaysOnTopHint_clicked();

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
    void addTabWidget(TabWindow window);
    void TabCloseRequested(int index);
    void TabCurrentChanged(int index);

    bool isFloatableWidgetStatus = false;
    void geometryChanged(const QPoint& pos = QPoint(0, 0));

    void showMessage(const QString &text, int timeout = 0);
    void clearMessage();

    bool isOnTopWidgetStatus = false;
};
#endif // MAINWINDOW_H
