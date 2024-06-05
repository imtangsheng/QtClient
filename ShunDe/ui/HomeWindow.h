#ifndef HOMEWINDOW_H
#define HOMEWINDOW_H

#include <QJsonObject>
#include <QMainWindow>
#include <QTcpServer>
#include "modules/camerawidget.h"
#include <QtConcurrent/QtConcurrent>

namespace Ui
{
    class HomeWindow;
}

class HomeWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HomeWindow(QWidget *parent = nullptr);
    ~HomeWindow();
    void start();
    void quit();
    void RelayoutCameraWidget();

    QTcpServer server;
    int SelectedId = -1;

    bool addNewRobotDevice(int id);

    QFuture<int> future;
    bool startTcpServerListen(const QString &ipAddress = "0.0.0.0", const quint16 &port = 12345);
    int ProcessNewConnection(QTcpSocket *socket);

public slots:
    bool CameraWidgetPlay(const int &id, const int &channel,const QUrl &source);

    void MouseButtonPressCameraWidget(int index);

    void UpdateSelectedId(const int &id);

private slots:
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

    void on_toolButton_device_management_isShow_clicked();

    void on_toolButton_device_add_clicked();

    void on_toolButton_device_update_clicked();

    void on_toolButton_device_delete_clicked();

    void on_comboBox_device_manage_id_currentTextChanged(const QString &arg1);

    void on_toolButton_map_show_clicked();

    /*云台控制指令*/
    void on_toolButton_camera_robot_up_pressed();
    void on_toolButton_camera_robot_up_released();
    void on_toolButton_camera_robot_down_pressed();
    void on_toolButton_camera_robot_down_released();
    void on_toolButton_camera_robot_left_pressed();
    void on_toolButton_camera_robot_left_released();
    void on_toolButton_camera_robot_right_pressed();
    void on_toolButton_camera_robot_right_released();
    void on_toolButton_camera_robot_left_up_pressed();
    void on_toolButton_camera_robot_left_up_released();
    void on_toolButton_camera_robot_right_up_pressed();
    void on_toolButton_camera_robot_right_up_released();
    void on_toolButton_camera_robot_left_down_pressed();
    void on_toolButton_camera_robot_left_down_released();
    void on_toolButton_camera_robot_right_down_pressed();
    void on_toolButton_camera_robot_right_down_released();

    void on_toolButton__camera_PTZControl_LIGHT_PWRON_clicked();
    void on_toolButton__camera_PTZControl_WIPER_PWRON_clicked();
    void on_toolButton_camera_robot_auto_clicked();

    void on_toolButton_camera_preview_cancel_clicked();

private:
    QJsonObject config;
    Ui::HomeWindow *ui;
    int cameraWidgetsNum = 4; // 自定义的cameraWidgets列表的大小
    QList<CameraWidget *> cameraWidgets;
    int selectedCameralChannel = 1;
    int currentItemCameraWidget = -1;
    int lastItemCameraWidget = -1;

    void RelayoutPTZControlWidget();
};

extern HomeWindow *homeWindow;
#endif // HOMEWINDOW_H
