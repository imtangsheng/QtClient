#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include "ui/camerawidget.h"
#include "modules/robot.h"

struct Device {
    int id = -1;
    DeviceType type = DeviceType_Other;
    Robot *robot = new Robot();
    QString ipAddress = "";
    bool isOnline = false;
    QTcpSocket *client = nullptr;
};


namespace Ui {
class HomeWindow;
}

class HomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();
    void init();
    void quit();
    void RelayoutCameraWidget();

    QTcpServer server;
    QMap<int, Device> deviceMap; // 使用QMap
    bool startTcpServerListen(const QString &ipAddress = "0.0.0.0",const quint16 &port = 12345);

public slots:
    bool CameraWidgetPlay(const QUrl &source);

    void MouseButtonPressCameraWidget(int index);

private slots:
    void on_toolButton_cameraChannel02_clicked();
    void on_toolButton_cameraChannel01_clicked();

    void on_toolButton_DeviceControlWidget_show_clicked();

    void on_toolButton_DeviceControlWidget_notShow_clicked();

    void on_toolButton_DeviceControlWidgetSetting_show_clicked();

    void on_toolButton_DeviceControlWidgetSetting_notShow_clicked();

    void on_test_clicked();
    void on_toolButton_robot_send_cmd_clicked();

    void on_toolButton_robot_move_forward_pressed();
    void on_toolButton_robot_move_forward_released();
    void on_toolButton_robot_move_backward_pressed();
    void on_toolButton_robot_move_backward_released();

    void on_toolButton_robot_speed_add_clicked();

    void on_toolButton_robot_speed_subtract_clicked();

    void on_toolButton_robot_state_set_clicked();

    void on_toolButton_robot_charge_set_clicked();


    void on_toolButton_robot_camera_set_PAN_TITL_clicked();

    void on_toolButton_robot_map_set_clicked();

    void on_toolButton_robot_move_goto_clicked();

    void on_toolButton_robot_move_goto_cancel_clicked();

    void on_toolButton_robot_time_set_clicked();


private:
    Ui::HomeWindow *ui;
    int cameraWidgetsNum = 4; // 自定义的cameraWidgets列表的大小
    QList<CameraWidget*> cameraWidgets;
    int currentItemCameraWidget = -1;
    int lastItemCameraWidget = -1;

    void RelayoutPTZControlWidget();

    int deviceId = 0;
};

#endif // HOMEWINDOW_H
