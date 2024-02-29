#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>

//#include "ui_SubWindow.h"

//extern Ui::SubWindow *ui_SubWindow;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void init();
    void test();
signals:
    void testSignal();

private slots:
    void on_pushButton_video_playback_clicked();

    void on_pushButton_dataView_clicked();

private:
    Ui::MainWindow *ui;

    void closeEvent(QCloseEvent *event) override;

    QJsonObject m_tabWidget_mainWindow;

    enum TabWindow {
        TabWindow_VideoPlayback,
        TabWindow_DataView
    };
    void addTabWidget(TabWindow window);
    void TabCloseRequested(int index);
    void TabCurrentChanged(int index);
};
#endif // MAINWINDOW_H
