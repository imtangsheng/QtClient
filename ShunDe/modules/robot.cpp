#include "robot.h"
#include <QDesktopServices>
#include <QFileDialog>
#include <QMenu>
#include <QMessageBox>
#include <QScreen>
#include <QToolTip>
#include "AppOS.h"
#include "function/worker_inspection_thread.h"
#include "modules/sqlite.h"

//switch (getRobotType())
//{
//case RobotType_default:
//    //break;
//case RobotType_HikVision_Camera:{
//    break;}
//case RobotType_SelfCamera_launchdigital_thermal:{
//    break;}
//default:
//    break;
//}

QString getAbsoluteFilePath(int sPicFileName) {
    qDebug()<<"getAbsoluteFilePath"<<sPicFileName;
    // 获取当前日期和时间
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 创建年月目录
    QString yearMonthDir = currentDateTime.toString("yyyy-MM");
    QString absPath = QString("D:/PicturePath/%1").arg(yearMonthDir);
    QDir dir(absPath);
    if (!dir.exists()) {
        dir.mkpath(absPath);
    }

    // 生成文件名
    QString fileNameFormat = "yyyy-MM-dd HH-mm-ss.zzz";
    QString fileName = currentDateTime.toString(fileNameFormat);

    // 拼接完整路径
    QString fullFilePath = QString("%1/%2.jpg").arg(absPath, fileName);

    return fullFilePath;
}

Robot::Robot(QWidget *parent) : QWidget(parent),
                                ui(new Ui::Robot)
{
    ui->setupUi(this);
    //    inspection.setParent(this);
    QMenu *toolMenu = new QMenu(this);
    // QMenu menu(this);
    toolMenu->addAction("Test", this, [this](){
        qDebug() << "showContextMenu(const QPoint &pos)" << this->isFullScreen();
        test();
    });

    ui->toolButton_widget_cameraChannel_isShow->setMenu(toolMenu);

    //ui->label_inspection_current_task_point_current_action_progress_value->setText(QString("%1 %").arg(100));

    SQL = SQLite::getInstance();
    SQL->initDb("test.db");
    QSqlError error;
    error = SQL->init_EventCenter();
    if (error.isValid()) {
        qWarning() << "Failed to initialize EventCenter MasterWindow:" << error.text();
    }
    error = SQL->init_inspectionTasks();
    if (error.isValid()) {
        qWarning() << "Failed to initialize inspection tasks:" << error.text();
    }
    error = SQL->init_inspectionCheckpoints();
    if (error.isValid()) {// 初始化失败
        qWarning() << "Failed to initialize inspection Checkpoints:" << error.text();
    }

}

Robot::~Robot()
{
    qDebug() << "Robot::~Robot() 0";
    if(worker_inspection_thread->isRunning()) worker_inspection_thread->quit();

    worker_inspection_thread->deleteLater();
    delete client;
    delete data;
    delete ui;

    qDebug() << "Robot::~Robot() 1";
}

void Robot::test()
{
    qDebug() << "Robot::test()";
    hikVisionCamera.start();
}

void Robot::init()
{
    config = AppJson[this->objectName()].toObject()[i2s(id)].toObject();

    setRobotType(RobotType(config["robotType"].toInt(0)));
    if (config.contains("camera"))
    {
        QJsonObject camera = config["camera"].toObject();

        hikVisionCamera.camera = camera;
        hikVisionCamera.loginInfo.sDeviceAddress = camera["video_ip"].toString();
        hikVisionCamera.loginInfo.sPassword = camera["video_password"].toString();

        ui->comboBox_robot_type->setCurrentIndex(config["robotType"].toInt(0));

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

        // 名称显示
        ui->label_channel01_video_name->setText(camera["video_name"].toString());
        ui->label_channel02_thermal_name->setText(camera["thermal_name"].toString());
    }
    else
    {
        ui->pushButton_robot->setText("机器人设备 配置不存在" + i2s(id));
    }

    if (config.contains("inspection"))
    {
        inspection.config = config["inspection"].toObject();
    }

    name = config["name"].toString("机器人设备" + i2s(id));
    updateRobotNameShow(name);

    ui->pushButton_robot->setIcon(QIcon(":/asset/Robot/Robot.svg"));
    connect(inspection.ui->toolButton_point_position_get, &QToolButton::clicked, this, [=]()
            {
                qDebug() << "toolButton_point_position_get,&QToolButton::clicked" << pose;
                inspection.ui->doubleSpinBox_poiont_position->setValue(pose / 1000); // mm->m
            });

    connect(inspection.ui->toolButton_vision_PTZPOS_get, &QToolButton::clicked, this, [=](){
        switch (getRobotType())
        {
        case RobotType_default:
            //break;
        case RobotType_HikVision_Camera:{
            QJsonObject action = hikVisionCamera.PTZPOS_get(inspection.ui->comboBox_vision_PTZPOS_lChannel->currentIndex()+1);
            qDebug() << "toolButton_vision_PTZPOS_get,&QToolButton::clicked"<<action;
            if(action["success"].toBool()){
                inspection.ui->doubleSpinBox_vision_PTZPOS_wPanPos->setValue(action["wPanPos"].toInt() / 10.0);
                inspection.ui->doubleSpinBox_vision_PTZPOS_wTiltPos->setValue(action["wTiltPos"].toInt() / 10.0);
                inspection.ui->doubleSpinBox_vision_PTZPOS_wZoomPos->setValue(action["wZoomPos"].toInt() / 10.0);
            }
            break;}
        case RobotType_SelfCamera_launchdigital_thermal:
        {
            // 自研发云台没有倍率
            inspection.ui->doubleSpinBox_vision_PTZPOS_wPanPos->setValue(camera_pan / 10.0);
            inspection.ui->doubleSpinBox_vision_PTZPOS_wTiltPos->setValue(camera_tilt / 10.0);
            break;
        }
        default:
            break;
        }


    });

    //*机器人设置*//
    ui->spinBox_vision_wait_default->setValue(config["vision_wait_default"].toInt(vision_wait_default));
    ui->spinBox_vision_default_PTZPreset_dwPresetIndex->setValue(config["vision_default_PTZPreset_dwPresetIndex"].toInt(vision_default_PTZPreset_dwPresetIndex));
    ui->doubleSpinBox_vision_default_PTZPOS_wPanPos->setValue(config["vision_default_PTZPOS_wPanPos"].toDouble(vision_default_PTZPOS_wPanPos));
    ui->doubleSpinBox_vision_default_PTZPOS_wTiltPos->setValue(config["vision_default_PTZPOS_wTiltPos"].toDouble(vision_default_PTZPOS_wTiltPos));

    connect(&inspection,&Inspection::run_action_operation,this,&Robot::run_action_operation);
    start();

    //qDebug() << "void Robot::init()" << config;
}

void Robot::start()
{

    worker_inspection_thread = new WorkerInspectionThread(this);
    // qDebug() << "worker_inspection_thread:"<<QThread::currentThreadId();
    connect(&inspection, &Inspection::updata_task_run_time, worker_inspection_thread, &WorkerInspectionThread::updata_task_run_time);

    inspection.start();
    hikVisionCamera.start();
}

void Robot::clientOnlineEvent()
{
    ui->pushButton_robot->setIcon(QIcon(":/asset/Robot/Robot.svg"));
    ui->pushButton_robot->setText(name);
    ui->toolButton_robot_status->setToolTip(name+"在线");
    ui->toolButton_robot_status->setIcon(QIcon(":/asset/Robot/Robot.svg"));
}

void Robot::clientOfflineEvent()
{
    robotStatus = RobotRunningStatus_Null;
    client = nullptr;

    ui->pushButton_robot->setIcon(QIcon(":/asset/Robot/Robot_Offline.svg"));
    ui->pushButton_robot->setText("设备离线");

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

    if (worker_inspection_thread->isRunning())
    {
        qDebug() << "void Robot::quit() worker_inspection_thread->isRunning() " << worker_inspection_thread->isRunning();
        worker_inspection_thread->quit(); // 会请求线程退出,但不会立即退出
        // worker_inspection_thread->wait();//会阻塞当前的程序,直到目标线程退出。
        worker_inspection_thread->terminate();
        // QThread::terminate() 会立即终止线程的执行。这是一个强制性的退出方式,可能会导致资源泄漏或其他问题
    }
    // worker_inspection_thread->deleteLater();
    // deleteLater();//自动释放
    hikVisionCamera.quit();
    qDebug() << "void Robot::quit()";
}

bool Robot::clientSendMessage(const QByteArray &data)
{
    if (client)
    {
        client->write(data);
        return true;
    }
    return false;
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
    // qDebug() << "客户端sendMessage"<<client;使用先初始化
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

RobotType Robot::getRobotType()
{
    QReadLocker locker(&m_rwLock);
    return robotType;
}

void Robot::setRobotType(RobotType type)
{
    QWriteLocker locker(&m_rwLock);
    robotType = type;
}

void Robot::updateDataShow()
{
    // qDebug() << "HomeWindow::updateRobotDataShow(int )" << sizeof(data);
    //     RobotRecvPacket packet = *data;
    //     qDebug() << "head:               0x" << QString::number(packet.head, 16).toUpper();
    //     qDebug() << "networkIndicator:   " << packet.networkIndicator;
    //     qDebug() << "areaIndicator:      " << packet.areaIndicator;
    //     qDebug() << "robotNumber:        " << packet.robotNumber;
    //     qDebug() << "temperature:        " << packet.temperature * 0.1 << "°C";
    //     qDebug() << "humidity:           " << packet.humidity;
    //     qDebug() << "CO:                 " << packet.CO;
    //     qDebug() << "H2S:                " << packet.H2S;
    //     qDebug() << "O2:                 " << packet.O2;
    //     qDebug() << "CH4:                " << packet.CH4;
    //     qDebug() << "smoke:              " << packet.smoke;
    //     qDebug() << "batteryLevel:       " << packet.batteryLevel << "%";
    //     qDebug() << "pose:               " << packet.pose;
    //     qDebug() << "gear:               " << packet.gear;
    //     qDebug() << "reserved1:          " << packet.reserved1;
    //     qDebug() << "state:              " << packet.state;
    //     qDebug() << "turnaroundSignal:   " << packet.turnaroundSignal;
    //     qDebug() << "indicatorColor:     " << packet.indicatorColor;
    //     qDebug() << "patrolPointReached:" << packet.patrolPointReached;
    //     qDebug() << "self_test:          " << packet.self_test;
    //     qDebug() << "reserved2:          " << packet.reserved2;
    //     qDebug() << "laser1:             " << packet.laser1;
    //     qDebug() << "laser2:             " << packet.laser2;
    //     qDebug() << "ultrasonic1:        " << packet.ultrasonic1;
    //     qDebug() << "laser3:             " << packet.laser3;
    //     qDebug() << "laser4:             " << packet.laser4;
    //     qDebug() << "ultrasonic2:        " << packet.ultrasonic2;
    //     qDebug() << "camera_pan:         " << packet.camera_pan / 10.0 << "°";
    //     qDebug() << "camera_tilt:        " << packet.camera_tilt / 10.0 << "°";
    //     qDebug() << "reserved5:          " << packet.reserved5;
    //     qDebug() << "speed_current:      " << packet.speed_current;

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

        ui->toolButton_robot_map->move(getPicturePosFromPose(pose));
    }
    // 更新角度
    switch (getRobotType())
    {
    case RobotType_default:
        //break;
    case RobotType_HikVision_Camera:
        break;
    case RobotType_SelfCamera_launchdigital_thermal:
    {
        // 自研发云台没有显示角度的功能，使用海康接口方法，显示在画面中
        if (camera_pan != data->camera_pan || camera_tilt != data->camera_tilt)
        {
            camera_pan = data->camera_pan;
            camera_tilt = data->camera_tilt;
            std::thread t(&HikVisionCamera::updateCameraPose_Pan_Tilt, &hikVisionCamera, camera_pan, camera_tilt);
            t.detach();
        }
        break;
    }
    default:
        break;
    }
}


bool Robot::moveTo(int32_t pose, int timeout)
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
    for (int i = 0; i < timeout; i++)
    {
        if (clientSendMessage(byteArray))
            return true;
        QThread::sleep(1);
    }

    return false;
}

bool Robot::control(int command, bool stop)
{
    bool success = false;
    switch (command) {
    case 0:
        success = stop ? 1:0;
        break;
    default:
        success = stop ? 1:0;
        break;
    }
    return success;
}

void Robot::start_inspection_task_and_data_show()
{
    // 数据初始化
    inspection_data.warnings = 0;
    inspection_data.completed = 0;
    // 更新任务名称
    update_inspection_data_show(InspectionUpdata_task_current);

    inspection_data.task_current_state = tr("进行中");
    update_inspection_data_show(InspectionUpdata_task_current_state);

    update_inspection_data_show(InspectionUpdata_task_next);
}

void Robot::start_inspection_task_point_and_data_show()
{
    //inspection_data.warnings = 0;
    inspection_data.current_task_point_content_strList = QStringList();
}

void Robot::update_inspection_data_show(InspectionUpdataType type)
{
    switch (type)
    {
    case InspectionUpdata_task_current:
        // 待同时更新下个任务点
        ui->label_inspection_current_task_value->setText(inspection_data.current_task_name);
        break;
    case InspectionUpdata_task_current_state:
        ui->label_inspection_task_current_state_value->setText(inspection_data.task_current_state);
        break;
    case InspectionUpdata_task_current_point:
        ui->label_inspection_current_task_point_current_value->setText(inspection_data.current_task_point_name);
        ui->label_inspection_current_task_point_next_value->setText(inspection_data.current_task_point_next_name);
        break;
    case InspectionUpdata_task_current_point_action:
        ui->label_inspection_current_task_point_current_action_value->setText(inspection_data.current_task_point_current_action);
        break;
    case InspectionUpdata_task_current_point_next:
        // 已经放弃
        ui->label_inspection_current_task_point_next_value->setText(inspection_data.current_task_name);
        break;
    case InspectionUpdata_task_current_poiont_progress:
        //ui->label_inspection_current_task_point_current_action_progress_value->setText(inspection_data.current_task_point_current_progress);
        break;
    case InspectionUpdata_task_current_completion_progress:
        ui->label_inspection_info_current_task_finish_points_value->setText(tr("%1个").arg(inspection_data.completed));
        ui->label_inspection_info_current_task_not_finish_points_value->setText(tr("%1个").arg(inspection_data.not_completed));
        ui->label_inspection_current_task_point_current_action_progress_value->setText(tr("%1 %").arg(100*(inspection_data.totalPoints - inspection_data.not_completed) / inspection_data.totalPoints));
        break;
    case InspectionUpdata_task_next:
        // 待同时更新下个任务点
        ui->label_inspection_task_next_value->setText(inspection_data.task_next_name);
        ui->label_inspection_task_next_time_value->setText(inspection_data.task_next_time);
        break;
    default:
        break;
    }
}

void Robot::end_inspection_task_and_data_show()
{
    if (inspection_data.warnings == 0)
    {
        inspection_data.task_current_state = tr("结束");
    }
    else
    {
        inspection_data.task_current_state = tr("异常%1个").arg(inspection_data.warnings);
    }
    update_inspection_data_show(InspectionUpdata_task_current_state);
}

bool Robot::run_action_operation(PointAction operation, QJsonObject action)
{
    QString display;
    bool success = false;
    switch (operation)
    {
    case PointAction::PointAction_Time:{
        QThread::sleep(action["sleep"].toInt());
        display = "时间:" + i2s(action["sleep"].toInt());
        success = true;
        break;}
    case PointAction::PointAction_Vision_PTZControl:{
        display = "设备基本控制";
        switch (getRobotType())
        {
        case RobotType_default:
            //break;
        case RobotType_HikVision_Camera:{
            success = hikVisionCamera.PTZControl(action["lChannel"].toInt(1),action["dwPTZCommand"].toInt(),action["dwStop"].toInt(1));
            break;}
        case RobotType_SelfCamera_launchdigital_thermal:{
            success = control(action["dwPTZCommand"].toInt(),action["dwStop"].toInt(1));
            break;}
        default:
            break;
        }
        qDebug() << action["lChannel"].toInt();
        qDebug() << Qt::CheckState(action["dwStop"].toInt());
        qDebug() << action["dwPTZCommand"].toInt();

        break;}
    case PointAction::PointAction_Vision_PTZPreset:{
        display = "云台预置点功能:" + i2s(action["dwPresetIndex"].toInt());
        qDebug() << action["lChannel"].toInt();
        qDebug() << action["dwPresetIndex"].toInt();
        qDebug() << action["dwPTZPresetCmd"].toInt();
        success = hikVisionCamera.PTZPreset(action["lChannel"].toInt(1),action["dwPresetIndex"].toInt(),action["dwPTZPresetCmd"].toInt());
        break;}
    case PointAction::PointAction_Vision_PTZPreset_Capture:{
        display = "云台预置点拍照:" + i2s(action["dwPresetIndex"].toInt());
        //1-就位 停30秒 2-拍照 停30秒 3-默认位置
        if(run_action_operation(PointAction_Vision_PTZPreset,action)){
            QThread::sleep(config["vision_wait_default"].toInt(vision_wait_default));
            success = run_action_operation(PointAction_Vision_CaptureJPEGPicture,action);
            QThread::sleep(config["vision_wait_default"].toInt(vision_wait_default));
            hikVisionCamera.PTZPreset(action["lChannel"].toInt(1),config["vision_default_PTZPreset_dwPresetIndex"].toInt(vision_default_PTZPreset_dwPresetIndex),action["dwPTZPresetCmd"].toInt());
        }
        break;}
    case PointAction::PointAction_Vision_PTZPOS:{
        display = "云台设置PTZ参数";
//        qDebug() << action["lChannel"].toInt();
//        qDebug() << action["wAction"].toInt();
//        qDebug() << action["wPanPos"].toDouble();
//        qDebug() << action["wTiltPos"].toDouble();
//        qDebug() << action["wZoomPos"].toDouble();
        switch (getRobotType())
        {
        case RobotType_default:
            //break;
        case RobotType_HikVision_Camera:{
            success = hikVisionCamera.PTZPOS_set(action["lChannel"].toInt(1),action["wAction"].toInt(),action["wPanPos"].toDouble()*10,action["wTiltPos"].toDouble()*10,action["wZoomPos"].toDouble()*10);
            break;}
        case RobotType_SelfCamera_launchdigital_thermal:{
            QByteArray byteArray;
            byteArray.append(Robot_CMD_Header);
            byteArray.append(Robot_CMD_SET_PTZPOS);
            //    byteArray.append(0x01);
            int16_t pan = action["wPanPos"].toDouble() * 10;
            int16_t titl = action["wTiltPos"].toDouble() * 10;
            byteArray.append(reinterpret_cast<const char *>(&pan), sizeof(pan));
            byteArray.append(reinterpret_cast<const char *>(&titl), sizeof(titl));
            success = clientSendMessage(byteArray);
            break;}
        default:
            break;
        }

        break;}
    case PointAction::PointAction_Vision_PTZPOS_Capture:{
        //1-就位 停30秒 2-拍照 停30秒 3-默认位置
        if(run_action_operation(PointAction_Vision_PTZPOS,action)){
            QThread::sleep(config["vision_wait_default"].toInt(vision_wait_default));
            success = run_action_operation(PointAction_Vision_CaptureJPEGPicture,action);
            QThread::sleep(config["vision_wait_default"].toInt(vision_wait_default));
            QJsonObject default_action = action;
            default_action["wPanPos"] = config["vision_default_PTZPOS_wPanPos"].toDouble(vision_default_PTZPOS_wPanPos) * 10;
            default_action["wTiltPos"] = config["vision_default_PTZPOS_wTiltPos"].toDouble(vision_default_PTZPOS_wTiltPos) * 10;
            run_action_operation(PointAction_Vision_PTZPOS,action);
        }
        break;}
    case PointAction::PointAction_Vision_CaptureJPEGPicture:{
        display = "云台抓图:" + i2s(action["lChannel"].toInt());
        QString fileName = getAbsoluteFilePath(action["sPicFileName"].toInt());
        RobotType type = getRobotType();
        if(type == RobotType_SelfCamera_launchdigital_thermal && action["lChannel"].toInt(1) == 2){
            //其他方法，热成像的摄像头
            success = false;
        }else{
            success = hikVisionCamera.CaptureJPEGPicture(action["lChannel"].toInt(),fileName);
        }
        if(success) inspection_data.current_task_point_content_strList.append(fileName);
        break;}
    case PointAction::PointAction_Vision_Realtime_Thermometry:{
        display = "热成像测温";
        qDebug() << action["sPicFileName"].toInt();
        QString fileName = getAbsoluteFilePath(action["sPicFileName"].toInt());
        success = hikVisionCamera.Realtime_Thermometry(fileName);
        if(success) inspection_data.current_task_point_content_strList.append(fileName);
        break;}
    case PointAction::PointAction_Robot_Control:{
        display = "机器人控制";
        qDebug() << action["robot_cmd"].toString().toUtf8();
        success = clientSendMessage(action["robot_cmd"].toString().toUtf8());
        break;}
    case PointAction::PointAction_Vision_Other:{
        display = "其他";
        break;}
    default:
        break;
    }
    qDebug() << "run_action_operation:" << display;
    return success;
}

int32_t Robot::getPoseFromPicturePos(const QPoint &pos)
{
    return pos.x();
    //int32_t x = pos.x();
    //int32_t y = pos.y();
    //return y * 100; // 将 x 和 y 打包成 int32_t 类型
}

QPoint Robot::getPicturePosFromPose(const int32_t &pose)
{

    //return QPoint(0, pose / 100);
    return QPoint(pose,0);
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
    emit setCameraWidgetPlay(id, 1,source);
}

void Robot::on_toolButton_channel02_thermal_play_clicked()
{
    QJsonObject camera = config["camera"].toObject();
    // 0 是通道号
    // 主码流"rtspUrlMain": "rtsp://192.168.1.19:554/0/main", 子码流 "rtspUrlSub": "rtsp://192.168.1.19:554/0/sub"
    QUrl source = QUrl("rtsp://" +
                       camera["thermal_username"].toString() + ":" +
                       camera["thermal_password"].toString() + "@" +
                       camera["thermal_ip"].toString() + "/0/" +
                       camera["thermal_stream"].toString());
    emit setCameraWidgetPlay(id, 2,source);
}

void Robot::on_toolButton_robto_config_save_clicked()
{

    QJsonObject camera = config["camera"].toObject();
    config["name"] = ui->lineEdit_robot_name->text();
    config["robotType"] = ui->comboBox_robot_type->currentIndex();
    setRobotType(RobotType(config["robotType"].toInt(0)));

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
    hikVisionCamera.camera = camera;

    ui->widgetSetting->close();
    // 更新名称
    // init()
    name = config["name"].toString();
    updateRobotNameShow(name);

    ui->label_channel01_video_name->setText(camera["video_name"].toString());
    ui->label_channel02_thermal_name->setText(camera["thermal_name"].toString());
}

void Robot::on_pushButton_robot_clicked()
{
    ui->widgetSetting->setWindowFlags(Qt::Dialog);
    ui->widgetSetting->setWindowTitle(config["name"].toString(objectName()));
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


void Robot::on_toolButton_inspection_task_start_clicked()
{
    QString task_name = inspection.ui->comboBox_task_name->currentText();
    worker_inspection_thread->current_task = inspection.config["tasks"].toObject()[task_name].toObject();
    worker_inspection_thread->current_task_name = task_name;
    // thread_task.start();
    worker_inspection_thread->start();
    // worker_inspection_thread->run();
    // QMetaObject::invokeMethod(worker_inspection_thread, "start", Qt::QueuedConnection);
    qDebug() << "on_pushButton_start_inspection_task_clicked():" << QThread::currentThreadId();
}

void Robot::on_toolButton_inspection_task_cancel_clicked()
{
    if (worker_inspection_thread->isRunning()) {
        // 显示确认弹窗
        QMessageBox confirmBox(QMessageBox::Question, "取消检查", "确定要取消当前检查吗?", QMessageBox::Yes | QMessageBox::No, this);
        int result = confirmBox.exec();

        if (result == QMessageBox::Yes) {
            // 用户确认取消检查
            //worker_inspection_thread->quit();
            //worker_inspection_thread->wait();//等待退出
    //        worker_inspection_thread->exit();
            worker_inspection_thread->terminate();
            inspection_data.task_current_state = tr("强制取消");
            update_inspection_data_show(InspectionUpdata_task_current_state);
        }
    }
}


void Robot::on_toolButton_robot_map_clicked()
{
    qDebug() << "Robot::on_toolButton_robot_map_clicked()";
}

void Robot::on_pushButton_robot_gas_isShow_clicked()
{
    ui->widget_gas_show->setVisible(!ui->widget_gas_show->isVisible());
}


void Robot::on_toolButton__gas_alarm_set_clicked()
{

}

void Robot::on_toolButton_vision_default_set_clicked()
{
    config["vision_wait_default"] = ui->spinBox_vision_wait_default->value();
    config["vision_default_PTZPreset_dwPresetIndex"] = ui->spinBox_vision_default_PTZPreset_dwPresetIndex->value();
    config["vision_default_PTZPOS_wPanPos"] = ui->doubleSpinBox_vision_default_PTZPOS_wPanPos->value();
    config["vision_default_PTZPOS_wTiltPos"] = ui->doubleSpinBox_vision_default_PTZPOS_wTiltPos->value();
}

void Robot::on_pushButton_scripts_filePath_get_clicked()
{
    // 获取当前应用程序的目录
    QString currentDir = QDir::current().absolutePath();
    // 构建 scripts/ 目录的路径
    QString scriptsDir = QDir(currentDir).filePath("scripts");
    // 打开文件选择对话框
    QString selectedFile = QFileDialog::getOpenFileName(this,"选择 Python 脚本",scriptsDir,"Python 脚本 (*.py)");
    if (selectedFile.isEmpty())return;

    ui->lineEdit_scripts_filePath->setText(selectedFile);

}


void Robot::on_pushButton_scripts_filePath_run_clicked()
{
    QString filePath = ui->lineEdit_scripts_filePath->text();
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::critical(nullptr, "错误", QString("文件 '%1' 不存在。").arg(filePath));
        return;
    }
    QUrl url = QUrl::fromLocalFile(filePath);
    if (!QDesktopServices::openUrl(url)) {
        QMessageBox::critical(nullptr, "错误", QString("无法打开文件 '%1'。是否设置打开默认文件的应用程序。").arg(filePath));
        return;
    }

}

