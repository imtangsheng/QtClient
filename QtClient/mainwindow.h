#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

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

signals:
    void onDestorySignal();


protected:
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void changeEvent(QEvent *event) override;
//    void geometryChanged(const QRect &newGeometry, const QRect &oldGeometry) override;

private slots:
    void on_Button_videoPlayback_clicked();

    void on_Button_dataView_clicked();

    void on_toolButton_WidgetStatus_isFloatable_clicked();

    void on_toolButton_WidgetStatus_isStaysOnTopHint_clicked();

private:
    TitleBar *ui_TitleBar;
    Ui::MainWindow *ui;
    QWidget * wtest;
    QList<PluginInterface*> pluginInterface;
    bool loadPlugin();

    void closeEvent(QCloseEvent *event) override;

    QJsonObject jsonMainConfig;

    int currentTabNum;
    int tabCloseLastShowNum = 0;
    QMap<int,TabWindow> mapTabIndexToWindow;
    void addTabWidget(TabWindow window);
    void TabCloseRequested(int index);
    void TabCurrentChanged(int index);
    void TabRemove(int index);

    bool isFloatableWidgetStatus = false;
    void geometryChanged(const QPoint& pos = QPoint(0, 0));

    void showMessage(const QString &text, int timeout = 0);
    void clearMessage();

    bool isOnTopWidgetStatus = false;
};
#endif // MAINWINDOW_H
