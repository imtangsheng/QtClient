#ifndef ROBOT_H
#define ROBOT_H

#include <QWidget>
#include <QNetworkAccessManager>
#include <QtNetwork>
#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonObject>
#include "ui_robot.h"

#define Robot_CMD_Header 0x5A // 命令 头

#define Robot_CMD_Camera_Up "5A 02 01"         // 相机云台 上
#define Robot_CMD_Camera_Down "5A 02 02"       // 相机云台 下
#define Robot_CMD_Camera_Left "5A 02 03"       // 相机云台 左
#define Robot_CMD_Camera_Right "5A 02 04"      // 相机云台 右
#define Robot_CMD_Camera_Left_Up "5A 02 05"    // 相机云台 左上
#define Robot_CMD_Camera_Left_Down "5A 02 06"  // 相机云台 左下
#define Robot_CMD_Camera_Right_Up "5A 02 07"   // 相机云台 右上
#define Robot_CMD_Camera_Right_Down "5A 02 08" // 相机云台 右下

#define Robot_CMD_Camera_Light "5A 02 20" // 相机云台 灯光

#define Robot_CMD_Move_Forward "5A 02 09"  // 手动有效  前进
#define Robot_CMD_Move_Backward "5A 02 0A" // 手动有效  后退
#define Robot_CMD_Speed_Add "5A 02 0B"     // 加档
#define Robot_CMD_Speed_Subtrat "5A 02 0C" // 减档
#define Robot_CMD_State_Cmd "5A 02 0D"     // 手动模式 tcp命令模式
#define Robot_CMD_State_Auto "5A 02 0E"    // 自动模式，不能移动
#define Robot_CMD_State_Charge "5A 02 0F"  // 回充电

#define Robot_CMD_Move_Stop "5A 02 14" // 对应手动模式的前进后退按钮弹起操作

#define Robot_CMD_Audio_On "5A 04 01"  // 打开喊话
#define Robot_CMD_Audio_Off "5A 04 00" // 关闭喊话

#define Robot_CMD_Charge_Off "5A 09 01" // 退出充电

typedef struct __attribute__((packed))
{
    uint8_t head;               /* 数据包头,每包数据均从0x55开始 */
    uint8_t networkIndicator;   /* 网络判断,固定为1 */
    uint8_t areaIndicator;      /* 区域判断,固定为0 */
    uint8_t robotNumber;        /* 机器人编号 */
    int32_t temperature;        /* 温度,单位0.1°C */
    uint8_t humidity;           /* 湿度 */
    uint8_t CO;                 /* 一氧化碳 CO */
    uint8_t H2S;                /* 硫化氢 H2S */
    uint8_t O2;                 /* 氧气 O2*/
    uint8_t CH4;                /* 甲烷 CH4 */
    uint32_t smoke;             /* 烟感 */
    uint8_t batteryLevel;       /* 电量百分比 */
    int32_t pose;               /* 机器人当前坐标 */
    uint8_t gear;               /* 速度挡位 */
    uint8_t reserved1;          /* 保留,不使用 */
    uint8_t state;              /* 运行状态 */
    uint8_t turnaroundSignal;   /* 掉头信号 */
    uint8_t indicatorColor;     /* 指示灯颜色 */
    uint8_t patrolPointReached; /* 巡逻点到位标志 */
    // 28bytes
    uint8_t self_test;    // 机器人自检状态
    uint8_t reserved2;    /* 保留,不使用 */
    uint32_t laser1;      // 激光1数据 前
    uint32_t laser2;      // 激光2数据 前
    uint32_t ultrasonic1; // 超声波1 前
    uint32_t laser3;      // 激光3数据 后
    uint32_t laser4;      // 激光4数据 后
    uint32_t ultrasonic2; // 超声波2 后
    // 54 bytes
    uint8_t reserved3;
    int8_t camera_pan; // [55]自有云台的偏航角 水平角度 0-360° *放大10数传输
    uint8_t reserved4;
    int8_t camera_tilt;     // [57]自有云台的俯仰角 垂直角度 -20-90° *放大10数传输
    uint32_t reserved5;     // [58 59 60 61] 保留,不使用
    uint16_t speed_current; // []当前速度
    // 共计 64bytes
} RobotRecvPacket;

typedef union
{
    RobotRecvPacket robotData;
    uint8_t RobotRecvPacketBuffer[64];
} RobotDataPacket;

/*定义机器人运行状态 0 - 6 */
enum RobotRunningStatus
{
    RobotRunningStatus_Null = 0,     // 机器人运行状态 丢失
    RobotRunningStatus_Manual = 1,   // 机器人运行状态 手动
    RobotRunningStatus_Auto = 2,     // 机器人运行状态 自动
    RobotRunningStatus_ChargeGo = 3, // 机器人运行状态 去往充电点
    RobotRunningStatus_Charging = 4, // 机器人运行状态 充电中
    RobotRunningStatus_Init = 6,     // 机器人运行状态 初始化
};
enum DeviceType
{
    DeviceType_Other,
    DeviceType_Robot,
    DeviceType_Robot_test,

};

struct RobotDevice
{
    int id = -1;
    DeviceType type = DeviceType_Other;
    QString ipAddress = "";
    bool isOnline = false;
    QTcpSocket *client = nullptr;
};

namespace Ui
{
    class Robot;
}

class Robot : public QWidget
{
    Q_OBJECT

public:
    explicit Robot(QWidget *parent = nullptr);
    ~Robot();

    QJsonObject config;
    Ui::Robot *ui;
    void init();
    void start();
    void clientOffline();
    void quit();

    bool sendMessage(const QString &message);
    bool sendMessage(const QByteArray &message);

    bool isCmdState();
    bool isCmdCharging();


    int id = -1;
    QTcpSocket *client;
    RobotRecvPacket *data;

    int pose;
    int robotBatteryLevel = -1;
    RobotRunningStatus robotStatus = RobotRunningStatus_Null;
    void updateDataShow();
    int camera_pan;
    int camera_tilt;
    bool updateCameraPose_Pan_Tilt(int pan, int tilt);

    QMap<int, RobotRecvPacket *> robotDataMap; // 使用QMap

private:
    // QString ipAddress = "127.0.0.1"; // 服务器IP地址
    // quint16 port = 12345; // 服务器端口号
signals:
    void setCameraWidgetPlay(const int &id, const QUrl &source);

private slots:
    void on_toolButton_widget_cameraChannel_isShow_clicked();
    void on_toolButton_channel01_video_play_clicked();
    void on_toolButton_channel02_thermal_play_clicked();

    void on_toolButton_robto_config_save_clicked();
    void on_pushButton_robot_clicked();
    void on_toolButton_robot_batteryLevel_clicked();
    void on_toolButton_robot_status_clicked();
};

#endif // ROBOT_H