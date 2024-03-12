#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#include "ui/TitleBar.h"

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

    void init();
    void test();

    void showUI();
//signals:
//    void testSignal();

protected:
    void mousePressEvent(QMouseEvent *event) override;
private slots:
    void on_Button_videoPlayback_clicked();

    void on_Button_dataView_clicked();

private:
    TitleBar *ui_TitleBar;
    QWidget *ui_foot;
    Ui::MainWindow *ui;

    void closeEvent(QCloseEvent *event) override;

    QJsonObject jsonSettingsVariable;

    int currentTabNum;
    void addTabWidget(int window);
    void TabCloseRequested(int index);
    void TabCurrentChanged(int index);

};
#endif // MAINWINDOW_H
