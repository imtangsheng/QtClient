#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QJsonObject>

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
private slots:
    void on_pushButton_video_playback_clicked();

private:
    Ui::MainWindow *ui;
    QJsonObject m_tabWidget_mainWindow;
};
#endif // MAINWINDOW_H
