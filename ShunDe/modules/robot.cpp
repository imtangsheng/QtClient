#include "robot.h"
#include <QMenu>
#include <QMessageBox>
#include <QScreen>
#include <QToolTip>
#include "AppOS.h"
#include "function/worker_inspection_thread.h"


Robot::Robot(QWidget *parent) : QWidget(parent),
                                ui(new Ui::Robot)
{
    ui->setupUi(this);
//    inspection.setParent(this);
    QMenu * toolMenu = new QMenu(this);
    //QMenu menu(this);
    toolMenu->addAction("Full Screen",this,[this](){
        qDebug()<<"showContextMenu(const QPoint &pos)"<<this->isFullScreen();
    });

    ui->toolButton_widget_cameraChannel_isShow->setMenu(toolMenu);
}

Robot::~Robot()
{
    delete client;
    delete data;
    delete ui;
    delete worker_inspection_thread;
    qDebug() << "Robot::~Robot()";
}

void Robot::init()
{
    config = AppJson[this->objectName()].toObject()[i2s(id)].toObject();

    if (config.contains("camera"))
    {
        QJsonObject camera = config["camera"].toObject();


        ui->lineEdit_channel01_video_name->setText(camera["video_name"].toString());
        ui->lineEdit_channel01_video_username->setText(camera["video_username"].toString());
        ui->lineEdit_channel01_video_password->setText(camera["video_password"].toString());
        ui->lineEdit_channel01_video_ip->setText(camera["video_ip"].toString());
        ui->comboBox_channel01_video_stream->setCurrentText(camera["video_stream"].toString());

        ui->lineEdit_channel02_thermal_name->setText(camera["thermal_name"].toString());
        ui->lineEdit_channel02_thermal_username->setText(camera["thermal_username"].toString());
        ui->lineEdit_channel02_thermal_password->setText(camera["thermal_password"].toString());
        ui->lineEdit_channel02_thermal_ip->setText(camera["thermal_ip"].toString());
        ui->comboBox_channel02_thermal_stream->setCurrentText(camera["thermal_stream"].toString());

        //名称显示
        ui->label_channel01_video_name->setText(camera["video_name"].toString());
        ui->label_channel02_thermal_name->setText(camera["thermal_name"].toString());

    }else{
        ui->pushButton_robot->setText("机器人设备 配置不存在"+i2s(id));
    }

    if (config.contains("inspection")){
        inspection.config = config["inspection"].toObject();
    }

    name = config["name"].toString("机器人设备"+i2s(id));
    updateRobotNameShow(name);

    ui->pushButton_robot->setIcon(QIcon(":/asset/Robot/Robot.svg"));
    connect(inspection.ui->toolButton_point_position_get,&QToolButton::clicked,this,[=](){
        qDebug() << "toolButton_point_position_get,&QToolButton::clicked"<<pose;
        inspection.ui->doubleSpinBox_poiont_position->setValue(pose/1000);//mm->m
    });

    start();

    qDebug() << "void Robot::init()"<<config;
}

void Robot::start()
{

    worker_inspection_thread = new WorkerInspectionThread(this);
    //qDebug() << "worker_inspection_thread:"<<QThread::currentThreadId();
    connect(&inspection,&Inspection::updata_task_run_time,worker_inspection_thread,&WorkerInspectionThread::updata_task_run_time);

    inspection.start();
}

void Robot::clientOffline()
{
    ui->pushButton_robot->setIcon(QIcon(":/asset/Robot/Robot_Offline.svg"));
    ui->pushButton_robot->setText("设备离线");
    robotStatus = RobotRunningStatus_Null;

    ui->toolButton_robot_status->setToolTip("离线");
    ui->toolButton_robot_status->setIcon(QIcon(":/asset/Robot/Robot_warning.svg"));
}

void Robot::quit()
{
    inspection.quit();
    config["inspection"] = inspection.config;
    QJsonObject devices = AppJson[this->objectName()].toObject();
    devices[i2s(id)] = config;
    AppJson[this->objectName()] = devices;

    if(worker_inspection_thread->isRunning()){
        qDebug() << "void Robot::quit() worker_inspection_thread->isRunning() "<<worker_inspection_thread->isRunning();
        worker_inspection_thread->quit();//会请求线程退出,但不会立即退出
        //worker_inspection_thread->wait();//会阻塞当前的程序,直到目标线程退出。
        worker_inspection_thread->terminate();
        //QThread::terminate() 会立即终止线程的执行。这是一个强制性的退出方式,可能会导致资源泄漏或其他问题

    }
    //worker_inspection_thread->deleteLater();
    //deleteLater();//自动释放
    qDebug() << "void Robot::quit()";
}

bool Robot::sendMessage(const QString &message)
{
    if (!client)
    {
        qWarning() << "客户端sendMessage deviceId 不存在" << client;
        QMessageBox::warning(this, "警告", "客户端不存在");
        return false;
    }
    qDebug() << "客户端sendMessage" << client->isValid() << client->isOpen();
    if (!client->isValid())
    {
        qWarning() << "无法启动服务器：" << client->isOpen() << client->error();
        QMessageBox::warning(this, "警告", "无法启动服务器");
        return false;
    }

    client->write(message.toUtf8());
    // 将QString转换为GBK编码的QByteArray
    //    QByteArray byteArray = QString("this is 机器人").toLocal8Bit();
    //    qDebug() << "接收到客户端数据：" <<message.toUtf8()<<byteArray;
    //    client->write(byteArray);
    return true;
}

bool Robot::sendMessage(const QByteArray &message)
{
    //qDebug() << "客户端sendMessage"<<client;使用先初始化
    if (!client)
    {
        qWarning() << "客户端sendMessage deviceId 不存在";
        QMessageBox::warning(this, "警告", "客户端不存在");
        return false;
    }
    qDebug() << "客户端sendMessage" << client->isValid() << client->isOpen();
    if (!client->isValid())
    {
        qWarning() << "无法启动服务器：" << client->isOpen() << client->error();
        QMessageBox::warning(this, "警告", "无法启动服务器");
        return false;
    }

    client->write(message);
    return true;
}

bool Robot::isCmdState()
{
    if (robotStatus == RobotRunningStatus_Manual)
    {
        return true;
    }
    return false;
}

bool Robot::isCmdCharging()
{
    if (robotStatus == RobotRunningStatus_ChargeGo || robotStatus == RobotRunningStatus_Charging)
    {
        return true;
    }
    return false;
}

void Robot::updateDataShow()
{
    //qDebug() << "HomeWindow::updateRobotDataShow(int )" << sizeof(data);
    //    RobotRecvPacket packet = *data;
    //    qDebug() << "head:               0x" << QString::number(packet.head, 16).toUpper();
    //    qDebug() << "networkIndicator:   " << packet.networkIndicator;
    //    qDebug() << "areaIndicator:      " << packet.areaIndicator;
    //    qDebug() << "robotNumber:        " << packet.robotNumber;
    //    qDebug() << "temperature:        " << packet.temperature * 0.1 << "°C";
    //    qDebug() << "humidity:           " << packet.humidity;
    //    qDebug() << "CO:                 " << packet.CO;
    //    qDebug() << "H2S:                " << packet.H2S;
    //    qDebug() << "O2:                 " << packet.O2;
    //    qDebug() << "CH4:                " << packet.CH4;
    //    qDebug() << "smoke:              " << packet.smoke;
    //    qDebug() << "batteryLevel:       " << packet.batteryLevel << "%";
    //    qDebug() << "pose:               " << packet.pose;
    //    qDebug() << "gear:               " << packet.gear;
    //    qDebug() << "reserved1:          " << packet.reserved1;
    //    qDebug() << "state:              " << packet.state;
    //    qDebug() << "turnaroundSignal:   " << packet.turnaroundSignal;
    //    qDebug() << "indicatorColor:     " << packet.indicatorColor;
    //    qDebug() << "patrolPointReached:" << packet.patrolPointReached;
    //    qDebug() << "self_test:          " << packet.self_test;
    //    qDebug() << "reserved2:          " << packet.reserved2;
    //    qDebug() << "laser1:             " << packet.laser1;
    //    qDebug() << "laser2:             " << packet.laser2;
    //    qDebug() << "ultrasonic1:        " << packet.ultrasonic1;
    //    qDebug() << "laser3:             " << packet.laser3;
    //    qDebug() << "laser4:             " << packet.laser4;
    //    qDebug() << "ultrasonic2:        " << packet.ultrasonic2;
    //    qDebug() << "camera_pan:         " << packet.camera_pan / 10.0 << "°";
    //    qDebug() << "camera_tilt:        " << packet.camera_tilt / 10.0 << "°";
    //    qDebug() << "reserved5:          " << packet.reserved5;
    //    qDebug() << "speed_current:      " << packet.speed_current;

    // 更新传感器
    //    ui->label_robot_temperature->setText(i2s(data->temperature));
    //    ui->label_robot_humidity->setText(i2s(data->humidity));
    //    ui->label_robot_CO->setText(i2s(data->CO));
    //    ui->label_robot_H2S->setText(i2s(data->H2S));
    //    ui->label_robot_O2->setText(i2s(data->O2));
    //    ui->label_robot_CH4->setText(i2s(data->CH4));
    //    ui->label_robot_smoke->setText(i2s(data->smoke));
    // 更新机器人状态
    if (robotStatus != RobotRunningStatus(data->state))
    {
        robotStatus = RobotRunningStatus(data->state);
        switch (robotStatus)
        {
        case RobotRunningStatus_Null:
            ui->toolButton_robot_status->setToolTip("离线");
            ui->toolButton_robot_status->setIcon(QIcon(":/asset/Robot/Robot_Offline.svg"));
            break;
        case RobotRunningStatus_Manual:
            ui->toolButton_robot_status->setToolTip("手动");
            ui->toolButton_robot_status->setIcon(QIcon(":/asset/Robot/Robot_cmd.svg"));
            break;
        case RobotRunningStatus_Auto:
            ui->toolButton_robot_status->setToolTip("自动");
            ui->toolButton_robot_status->setIcon(QIcon(":/asset/Robot/Robot_Auto.svg"));
            break;
        case RobotRunningStatus_ChargeGo:
            ui->toolButton_robot_status->setToolTip("返回充电桩");
            ui->toolButton_robot_status->setIcon(QIcon(":/asset/Robot/Robot_BattereGo.svg"));
            break;
        case RobotRunningStatus_Charging:
            ui->toolButton_robot_status->setToolTip("充电中");
            ui->toolButton_robot_status->setIcon(QIcon(":/asset/Robot/Robot_Battering.svg"));
            break;
        case RobotRunningStatus_Init:
            ui->toolButton_robot_status->setToolTip("返回参考点");
            ui->toolButton_robot_status->setIcon(QIcon(":/asset/Robot/Robot.svg"));
            break;
        default:
            ui->toolButton_robot_status->setIcon(QIcon(""));
            break;
        }
    }
    // 更新机器人电量显示
    if (robotBatteryLevel != data->batteryLevel)
    {
        robotBatteryLevel = data->batteryLevel;
        ui->toolButton_robot_batteryLevel->setToolTip(i2s(robotBatteryLevel) + "%");
        switch (robotBatteryLevel)
        {
        case 0 ... 25:
            ui->toolButton_robot_batteryLevel->setIcon(QIcon(":/asset/Robot/Battery_Level_1.svg"));
            break;
        case 26 ... 50:
            ui->toolButton_robot_batteryLevel->setIcon(QIcon(":/asset/Robot/Battery_Level_2.svg"));
            break;
        case 51 ... 75:
            ui->toolButton_robot_batteryLevel->setIcon(QIcon(":/asset/Robot/Battery_Level_3.svg"));
            break;
        case 76 ... 100:
            ui->toolButton_robot_batteryLevel->setIcon(QIcon(":/asset/Robot/Battery_Level_4.svg"));
            break;
        default:
            ui->toolButton_robot_batteryLevel->setIcon(QIcon());
            break;
        }
    }

    if (pose != data->pose)
    {
        pose = data->pose;
        ui->label_robot_pose->setText(i2s(pose) + "mm");
    }
    // 更新角度
    if (camera_pan != data->camera_pan || camera_tilt != data->camera_tilt)
    {
        camera_pan = data->camera_pan;
        camera_tilt = data->camera_tilt;
        std::thread t(&Robot::updateCameraPose_Pan_Tilt, this, camera_pan, camera_tilt);
        t.detach();
    }
    // updateCameraPose_Pan_Tilt(data->camera_pan, data->camera_tilt);
}

bool Robot::updateCameraPose_Pan_Tilt(int pan, int tilt)
{
    // 创建XML数据
    QByteArray xmlData = ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                          "<TextOverlay >"
                          "<id>1</id>"
                          "<enabled>true</enabled>"
                          "<displayText>"
                          "P" +
                          i2s(pan) + ":" + "T" + i2s(tilt) +
                          "</displayText>"
                          "</TextOverlay>")
                             .toUtf8();

    // 发送POST请求，并将XML数据作为请求的主体数据
    qDebug() << "xmlData:" << xmlData;
    QJsonObject camera = config["camera"].toObject();
    QUrl api_path = QUrl("http://" + camera["video_ip"].toString() + "/ISAPI/System/Video/inputs/channels/1/overlays/text/1");
    // 设置Digest Authorization认证参数
    QString username = camera["video_username"].toString();
    QString password = camera["video_password"].toString();

    QNetworkAccessManager manager;
    // 创建一个QNetworkRequest对象，并设置请求的URL
    QNetworkRequest request(api_path);
    QNetworkReply *reply = manager.put(request, xmlData);
    // 等待请求完成
    QEventLoop loop;
    // 处理认证
    QObject::connect(&manager, &QNetworkAccessManager::authenticationRequired, [&](QNetworkReply *reply, QAuthenticator *authenticator)
                     {
        authenticator->setUser(username);
        authenticator->setPassword(password);
        qDebug() << "Response:处理认证"<<reply; });
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    // 检查请求是否成功
    if (reply->error() == QNetworkReply::NoError)
    {
        // 读取响应数据
        QByteArray response = reply->readAll();
        qDebug() << "Response:" << response;
    }
    else
    {
        // 处理请求错误
        qDebug() << "Error:" << reply->errorString();
    }
    // 释放资源
    reply->deleteLater();
    return true;
}

bool Robot::moveTo(int32_t pose)
{

    QByteArray byteArray;
    byteArray.append(Robot_CMD_Header);
    byteArray.append(0x0A);
    byteArray.append(0x02);
    byteArray.append(0x01);
    byteArray.append(reinterpret_cast<const char *>(&pose), sizeof(pose));
    //    byteArray.append(0xff & arg1);
    //    byteArray.append((0xff00 & arg1) >> 8);
    //    byteArray.append((0xff0000 & arg1) >> 16);
    //    byteArray.append((0xff000000 & arg1) >> 24);
    sendMessage(byteArray);
}

void Robot::update_inspection_data_show()
{
    ui->label_inspection_current_task_value->setText(inspection_data.current_task_name);

    ui->label_inspection_current_task_point_current_value->setText(inspection_data.current_task_name);
    ui->label_inspection_current_task_point_next_value->setText(inspection_data.current_task_name);

    ui->label_inspection_current_task_point_current_action_value->setText(inspection_data.current_task_name);
    ui->label_inspection_current_task_point_current_action_progress_value->setText(inspection_data.current_task_name);


    ui->label_inspection_info_current_task_finish_points_value->setText(inspection_data.current_task_name);
    ui->label_inspection_info_current_task_not_finish_points_value->setText(inspection_data.current_task_name);

    ui->label_inspection_task_next_value->setText(inspection_data.current_task_name);
    ui->label_inspection_task_next_time_value->setText(inspection_data.current_task_name);

}

int32_t Robot::getPoseFromPicturePos(const QPoint &pos)
{
    int32_t x = pos.x();
    int32_t y = pos.y();
    return  y * 100; // 将 x 和 y 打包成 int32_t 类型
}

QPoint Robot::getPicturePosFromPose(const int32_t &pose)
{

    return QPoint(0, pose/100);
}

void Robot::updateRobotNameShow(const QString &name)
{

    ui->lineEdit_robot_name->setText(name);
    ui->pushButton_robot->setText(name);
    ui->label_robot_name->setText(name);
    ui->pushButton_robot_gas_isShow->setText(name);
    ui->toolButton_robot_map->setText(name);

    inspection.ui->pushButton_robot_name->setText(name);


}


void Robot::on_toolButton_widget_cameraChannel_isShow_clicked()
{
    if (ui->widget_cameraChannel->isVisible())
    {
        ui->widget_cameraChannel->setVisible(false);
        ui->toolButton_widget_cameraChannel_isShow->setIcon(QIcon(":/asset/Robot/Robot_menu_show.svg"));
    }
    else
    {
        ui->widget_cameraChannel->setVisible(true);
        ui->toolButton_widget_cameraChannel_isShow->setIcon(QIcon(":/asset/Robot/Robot_menu_hide.svg"));
    }
}

void Robot::on_toolButton_channel01_video_play_clicked()
{

    QJsonObject camera = config["camera"].toObject();
    QUrl source = QUrl("rtsp://" +
                       camera["video_username"].toString() + ":" +
                       camera["video_password"].toString() + "@" +
                       camera["video_ip"].toString() + ":554/Streaming/Channels/" +
                       camera["video_stream"].toString());
    emit setCameraWidgetPlay(id, source);
}

void Robot::on_toolButton_channel02_thermal_play_clicked()
{
    QJsonObject camera = config["camera"].toObject();
    //0 是通道号
    //主码流"rtspUrlMain": "rtsp://192.168.1.19:554/0/main", 子码流 "rtspUrlSub": "rtsp://192.168.1.19:554/0/sub"
    QUrl source = QUrl("rtsp://" +
                       camera["thermal_username"].toString() + ":" +
                       camera["thermal_password"].toString() + "@" +
                       camera["thermal_ip"].toString() + "/0/" +
                       camera["thermal_stream"].toString() );
    emit setCameraWidgetPlay(id, source);
}

void Robot::on_toolButton_robto_config_save_clicked()
{

    QJsonObject camera = config["camera"].toObject();
    config["name"] = ui->lineEdit_robot_name->text();

    camera["video_name"] = ui->lineEdit_channel01_video_name->text();
    camera["video_username"] = ui->lineEdit_channel01_video_username->text();
    camera["video_password"] = ui->lineEdit_channel01_video_password->text();
    camera["video_ip"] = ui->lineEdit_channel01_video_ip->text();
    camera["video_stream"] = ui->comboBox_channel01_video_stream->currentText();

    camera["thermal_name"] = ui->lineEdit_channel02_thermal_name->text();
    camera["thermal_username"] = ui->lineEdit_channel02_thermal_username->text();
    camera["thermal_password"] = ui->lineEdit_channel02_thermal_password->text();
    camera["thermal_ip"] = ui->lineEdit_channel02_thermal_ip->text();
    camera["thermal_stream"] = ui->comboBox_channel02_thermal_stream->currentText();

    config["camera"] = camera;
    ui->widgetSetting->close();
    //更新名称
    //init()
    name = config["name"].toString();
    updateRobotNameShow(name);

    ui->label_channel01_video_name->setText(camera["video_name"].toString());
    ui->label_channel02_thermal_name->setText(camera["thermal_name"].toString());
}

void Robot::on_pushButton_robot_clicked()
{
    ui->widgetSetting->setWindowFlags(Qt::Dialog);
    ui->widgetSetting->setWindowTitle(config["camera"].toObject()["name"].toString(objectName()));
    ui->widgetSetting->setWindowIcon(QIcon(":/asset/Robot/Robot.svg"));
    ui->widgetSetting->setAttribute(Qt::WA_ShowModal, true);
    ui->widgetSetting->setParent(this);
    // 获取主屏幕
    //    QScreen* screen = QGuiApplication::primaryScreen();
    //    QRect screenGeometry = screen->geometry();

    //    // 将对话框移动到屏幕中心
    //    int x = screenGeometry.center().x() - ui->widgetSetting->width() / 2;
    //    int y = screenGeometry.center().y() - ui->widgetSetting->height() / 2;
    //    ui->widgetSetting->move(x, y);

    //    // 获取所有屏幕
    //    QList<QScreen*> screens = QGuiApplication::screens();
    //    // 查找包含对话框的屏幕
    //    qDebug()<<screens.size();
    //    QScreen* targetScreen = screens.at(1);
    //    // 如果找到了目标屏幕，则将对话框移动到该屏幕的中心

    // 获取当前鼠标的位置
    QPoint cursorPos = QCursor::pos();

    // 查找包含鼠标位置的屏幕
    QScreen *targetScreen = nullptr;
    for (QScreen *screen : QGuiApplication::screens())
    {
        if (screen->geometry().contains(cursorPos))
        {
            targetScreen = screen;
            break;
        }
    }
    if (targetScreen)
    {
        QRect targetGeometry = targetScreen->geometry();
        int x = targetGeometry.center().x() - ui->widgetSetting->width() / 2;
        int y = targetGeometry.center().y() - ui->widgetSetting->height() / 2;
        ui->widgetSetting->move(x, y);
    }
    ui->lineEdit_robot_id->setText(i2s(id));
    ui->widgetSetting->show();
}

void Robot::on_toolButton_robot_batteryLevel_clicked()
{
    // 充电中点击就取消充电 返回充电桩就取消该指令
    switch (robotStatus)
    {
    case RobotRunningStatus_ChargeGo:
    case RobotRunningStatus_Charging:
        sendMessage(QByteArray::fromHex(Robot_CMD_Charge_Off));
        break;
    case RobotRunningStatus_Manual:
    case RobotRunningStatus_Auto:
        sendMessage(QByteArray::fromHex(Robot_CMD_State_Charge));
    default:
        QToolTip::showText(ui->toolButton_robot_batteryLevel->mapToGlobal(QPoint(0, 0)),
                           "目前机器人状态不支持该指令，需要处于充电或者未充电模式",
                           ui->toolButton_robot_batteryLevel);
        break;
    }
}

void Robot::on_toolButton_robot_status_clicked()
{
    // 充电中点击就取消充电 返回充电桩就取消该指令
    switch (robotStatus)
    {
    case RobotRunningStatus_Manual:
        sendMessage(QByteArray::fromHex(Robot_CMD_State_Auto));
        break;
    case RobotRunningStatus_Auto:
        sendMessage(QByteArray::fromHex(Robot_CMD_State_Cmd));
    case RobotRunningStatus_ChargeGo:
    case RobotRunningStatus_Charging:
    default:
        QToolTip::showText(ui->toolButton_robot_status->mapToGlobal(QPoint(0, 0)),
                           "目前机器人状态不支持该指令，需要处于手动或者自动模式",
                           ui->toolButton_robot_status);
        break;
    }
}



void Robot::on_pushButton_start_inspection_task_clicked()
{
    worker_inspection_thread->current_task = inspection.config["tasks"].toObject()[inspection.ui->comboBox_task_name->currentText()].toObject();
    //thread_task.start();
    worker_inspection_thread->start();
    //worker_inspection_thread->run();
    //QMetaObject::invokeMethod(worker_inspection_thread, "start", Qt::QueuedConnection);
    qDebug() << "on_pushButton_start_inspection_task_clicked():"<<QThread::currentThreadId();

}

void Robot::on_pushButton_robot_gas_isShow_clicked()
{
    ui->widget_gas_show->setVisible(!ui->widget_gas_show->isVisible());
}


void Robot::on_toolButton_robot_map_clicked()
{
    qDebug() <<"Robot::on_toolButton_robot_map_clicked()";
}

