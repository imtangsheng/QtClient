#include "HomeWindow.h"
#include "ui_HomeWindow.h"
#include <QMessageBox>
#include <QTimer>
#include <QToolTip>
#include "AppOS.h"
#include "ui/MasterWindow.h"

#include "modules/robot.h"

HomeWindow *homeWindow;
QMap<int, Device> DeviceMap;

HomeWindow::HomeWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::HomeWindow)
{
    ui->setupUi(this);
}

HomeWindow::~HomeWindow()
{
    delete ui;
    /*DeviceMap是在栈上分配的，它会在其作用域结束时自动释放，你不需要手动释放内存*/
//    // 释放DeviceMap中的每个Device对象
    for (auto it = DeviceMap.begin(); it != DeviceMap.end(); ++it) {
        delete it.value().robot;
    }
//    // 清空DeviceMap中的元素
//    DeviceMap.clear();
    qDebug() << "HomeWindow::~HomeWindow()";
}

void HomeWindow::start()
{
    config = AppJson["HomeWindow"].toObject();
    QJsonObject devices = config["devices"].toObject();

    foreach (QString key, devices.keys())
    {
        QJsonObject device = devices[key].toObject();
        addNewRobotDevice(key.toInt());
        //初始化
        DeviceMap[key.toInt()].type = DeviceType(device["type"].toInt());
        DeviceMap[key.toInt()].robot->clientOfflineEvent();
    }
    ui->comboBox_device_add_type->addItem("1云台I代", DeviceType_Other);
    ui->comboBox_device_add_type->addItem("2云台II", DeviceType_Robot);
    ui->comboBox_device_add_type->addItem("3云台I代", DeviceType_Robot_test);

    //待增加服务器监听模块
    //    for (DeviceType type : DeviceType) {

    //    }
    qDebug() << "void HomeWindow::init() config[\"devices\"].isNull() " << config["devices"].isNull() << devices.isEmpty();
    qDebug() << "void HomeWindow::init() config[\"ipAddress\"] " << config["ipAddress"].toString("0.0.0.0")<<config["port"].toInt(12345);

    DeviceId = 0;

    RelayoutCameraWidget();
    //    cameraWidgets.resize(cameraWidgetsNum);
    RelayoutPTZControlWidget();
    startTcpServerListen("0.0.0.0", 12345);
    qDebug() << "void HomeWindow::init()";
    // 接收到机器人数据
}

void HomeWindow::quit()
{
    QJsonObject devices = config["devices"].toObject();
    foreach (QString key, devices.keys())
    {
        QJsonObject device = devices[key].toObject();
        int id = key.toInt();
        DeviceMap[id].robot->quit();
        //        deviceMap[id].id = key.toInt();
        //        deviceMap[id].robot = new Robot();
        //        deviceMap[id].robot->config = device["config"].toObject();
        //        deviceMap[id].robot->init();//配置读取，摄像头数据 通道号等
        //        deviceMap[id].type = DeviceType(device["type"].toInt());
        device["type"] = DeviceMap[id].type;
        device["config"] = DeviceMap[id].robot->config;
        devices[key] = device;
    }
    config["devices"] = devices;

    AppJson["HomeWindow"] = config;
    qDebug() << "void HomeWindow::quit()";
}

void HomeWindow::RelayoutCameraWidget()
{
    // 清空布局
    QLayoutItem *item;

    while ((item = ui->LayoutPreview->takeAt(0)) != nullptr)
    {
        qDebug() << "void HomeWindow::RelayoutCameraWidget() 11";
        ui->LayoutPreview->removeItem(item);
    }
    qDebug() << "初始布局" << cameraWidgets.size() << cameraWidgetsNum;
    while (cameraWidgets.size() < cameraWidgetsNum)
    {
        CameraWidget *cameraWidget = new CameraWidget(ui->WidgetPreview);
        cameraWidgets.append(cameraWidget);
        connect(cameraWidget, &CameraWidget::mousePress, this, [=]
                {
            lastItemCameraWidget = currentItemCameraWidget;
            currentItemCameraWidget = cameraWidgets.indexOf(cameraWidget);
            MouseButtonPressCameraWidget(currentItemCameraWidget); });
        //qDebug() << "初始布局 void HomeWindow::RelayoutCameraWidget() 22:" << cameraWidgets.size();
    }

    int numColumns = 2; // 每行的列数
    for (int i = 0; i < cameraWidgetsNum; i++)
    {
        //qDebug() << "void HomeWindow::RelayoutCameraWidget() 33:" << i << i / numColumns << i % numColumns;
        //        CameraWidget* widget = cameraWidgets[i];
        ui->LayoutPreview->addWidget(cameraWidgets[i], i / numColumns, i % numColumns);
    }
}

bool HomeWindow::addNewRobotDevice(int id)
{
    if(DeviceMap.contains(id)){
        return false;
    }
    DeviceMap[id].id = id;
    if(!DeviceMap[id].robot){
        DeviceMap[id].robot = new Robot(this);
    }
    DeviceMap[id].robot->id = id;
    DeviceMap[id].robot->init(); // 配置读取，摄像头数据 通道号等
    ui->LayoutDevice->addWidget(DeviceMap[id].robot->ui->widgetMenu);//机器人设备菜单
    ui->Layout_Robot_Inspection->addWidget(DeviceMap[id].robot->inspection.ui->widget_inspection);//机器人巡检设置
    ui->Layout_Robot_Data->addWidget(DeviceMap[id].robot->ui->widget_robot_data);//机器人设备菜单
    masterWindow->ui->horizontalLayout_DeviceConfigSettings->insertWidget(id,DeviceMap[id].robot->ui->widgetConfig,Qt::LeftToRight);//机器人参数设置ui
    //masterWindow->ui->horizontalLayout_DeviceInspection->insertWidget(id,DeviceMap[id].robot->inspection.ui->widget_inspection,Qt::LeftToRight);//机器人巡检设置ui
    connect(DeviceMap[id].robot, &Robot::setCameraWidgetPlay, this, &HomeWindow::CameraWidgetPlay);
    ui->comboBox_device_add_id->addItem(i2s(id), id);

    ui->label_map->add_robot_icon(id,DeviceMap[id].robot->name,DeviceMap[id].robot->ui->toolButton_robot_map);
    connect(ui->label_map,&MapLabel::move_robot_icon,this,[=](const int& robot_id,const QPoint& pos){
        qDebug()<<"导航robot_id"<<robot_id<<" pos:"<<pos;
        DeviceMap[robot_id].robot->ui->toolButton_robot_map->move(pos);
        qDebug()<<"导航robot_id"<<DeviceMap[robot_id].robot->getPoseFromPicturePos(pos);
        DeviceMap[robot_id].robot->moveTo(DeviceMap[robot_id].robot->getPoseFromPicturePos(pos));
    });
    return true;
}

bool HomeWindow::startTcpServerListen(const QString &ipAddress, const quint16 &port)
{
    qDebug() << "启动服务器：" << ipAddress << port;
    if (!server.listen(QHostAddress(ipAddress), port))
    {
        qWarning() << "无法启动服务器：" << server.errorString();
        return false;
    }
    qDebug() << "服务器已启动，监听地址：" << ipAddress << "，端口号：" << port;
    // 处理新的客户端连接
    QObject::connect(&server, &QTcpServer::newConnection, this, [=]()
                     {

        // 接受新的客户端连接
        QTcpSocket *clientSocket = server.nextPendingConnection();
        // 处理客户端连接
        qDebug() << "新的客户端连接：" << clientSocket->peerAddress().toString() << ":" << clientSocket->peerPort();
        // 接收一次数据
        future = QtConcurrent::run(&HomeWindow::ProcessNewConnection,this,clientSocket);

        // 客户端断开连接
        QObject::connect(clientSocket, &QTcpSocket::disconnected, [clientSocket]() {
            qDebug() << "客户端断开连接：" << clientSocket->peerAddress().toString() << ":" << clientSocket->peerPort();
            clientSocket->deleteLater();
        }); });
    return true;
}

int HomeWindow::ProcessNewConnection(QTcpSocket *socket)
{
    qDebug() << "HomeWindow::ProcessNewConnection(QTcpSocket *" << socket;
    // 接收一次数据 第一次等待10s接收信息
    if (socket->waitForReadyRead(10000))
    {

        QByteArray headerData = socket->readAll();
        int value = (int)headerData.at(0) << 8 | (int)headerData.at(1);
        qDebug() << "新的客户端连接：value" << headerData.at(0) << (int)headerData.at(0) << (int)headerData.at(1) << value;
        if (headerData.at(0) == 0x55 && headerData.at(1) == 0x01)
        {
            // 机器人
            int clientId = (int)headerData.at(2) << 8 | (int)headerData.at(3);
            qDebug() << "新的客户端连接：currenId" << clientId;
            //自动添加新设备，初始化，如果已经存在，只需要更新连接socket
            addNewRobotDevice(clientId);
            DeviceMap[clientId].ipAddress = socket->peerAddress().toString() + ":" + socket->peerPort();
            DeviceMap[clientId].isOnline = true;
            DeviceMap[clientId].robot->client = socket;
            DeviceMap[clientId].robot->clientOnlineEvent();

            // 接收客户端发送的数据
            QObject::connect(socket, &QTcpSocket::readyRead, this, [=]()
                             {
                QByteArray bytes = socket->readAll();
                //只处理64字节的数据，对粘包，丢包不进行处理
                if(bytes.length()==64){
                    //qDebug() << "接收到客户端数据："<< QString(bytes.at(55))<<(int)bytes.at(56)<<(int)bytes.at(57)<<(int)bytes.at(58);
                    QReadWriteLock lock;
                    lock.lockForWrite();
                    // 访问共享资源的读写操作
                    DeviceMap[clientId].robot->data = (RobotRecvPacket *) bytes.data();
                    lock.unlock();

                    DeviceMap[clientId].robot->updateDataShow();
                }else{
                    qDebug() << "接收到客户端异常长度数据：" <<bytes.length();//<< bytes.toHex();
                }

            }); // 接收机器人客户端发送的数据

            QObject::connect(socket, &QTcpSocket::disconnected,this, [=]() {
                DeviceMap[clientId].isOnline = false;
                DeviceMap[clientId].robot->clientOfflineEvent();

                //DeviceMap[clientId].robot->quit();

                socket->deleteLater();
            });
        }
    }
    else
    {
    }

    return 0;
    // 客户端断开连接
    //    QObject::connect(socket, &QTcpSocket::disconnected, [socket]() {
    //        qDebug() << "客户端断开连接：" << socket->peerAddress().toString() << ":" << socket->peerPort();
    //        socket->deleteLater();
    //    });
}

bool HomeWindow::CameraWidgetPlay(const int &id, const QUrl &source)
{
    DeviceId = id;
    if (currentItemCameraWidget == -1)
    {
        return false;
    }
    //存在同源奔溃后无法重新连接的问题，暂时未测
    // QUrl source = QUrl("rtsp://admin:dacang80@192.168.1.38:554/Streaming/Channels/101");
    cameraWidgets.at(currentItemCameraWidget)->player.setSource(source);
    cameraWidgets.at(currentItemCameraWidget)->player.play();
    // cameraWidgets.at(currentItemCameraWidget)->player.error();
    return true;
}

void HomeWindow::MouseButtonPressCameraWidget(int index)
{
    qDebug() << "HomeWindow::MouseButtonPressCameraWidget(int " << index;
    // border：指定边框的样式。2px：指定边框的宽度为2像素。solid：指定边框的样式为实线。red：指定边框的颜色为红色
    if (lastItemCameraWidget != -1)
    {
        qDebug() << "HomeWindow::MouseButtonPressCameraWidget(int lastItemCameraWidget:" << lastItemCameraWidget;
        cameraWidgets.at(lastItemCameraWidget)->setContentsMargins(0, 0, 0, 0); // 显示边框
    }
    cameraWidgets.at(index)->setContentsMargins(1, 1, 1, 1); // 显示边框
}

void HomeWindow::on_toolButton_DeviceControlWidget_show_clicked()
{
    // 1 表示AWidget 2 表示BWidget: 1-show
    ui->DeviceControlWidget->setVisible(true);
    // 1-hide 2-show
    ui->toolButton_DeviceControlWidget_show->setVisible(false);
    ui->toolButton_DeviceControlWidget_notShow->setVisible(true);
    ui->toolButton_DeviceControlWidgetSetting_show->setVisible(true);
    ui->toolButton_DeviceControlWidgetSetting_notShow->setVisible(false);
}

void HomeWindow::on_toolButton_DeviceControlWidget_notShow_clicked()
{
    ui->DeviceControlWidget->setVisible(false);
    // 1-hide 2-hide
    ui->toolButton_DeviceControlWidget_show->setVisible(true);
    ui->toolButton_DeviceControlWidget_notShow->setVisible(false);
    ui->toolButton_DeviceControlWidgetSetting_show->setVisible(false);
    ui->toolButton_DeviceControlWidgetSetting_notShow->setVisible(false);
}

void HomeWindow::on_toolButton_DeviceControlWidgetSetting_show_clicked()
{
    ui->DeviceControlWidgetSetting->setVisible(true);
    // 1-show 2-show 全显示，只保留一个隐藏
    ui->toolButton_DeviceControlWidget_show->setVisible(false);
    ui->toolButton_DeviceControlWidget_notShow->setVisible(false);
    ui->toolButton_DeviceControlWidgetSetting_show->setVisible(false);
    ui->toolButton_DeviceControlWidgetSetting_notShow->setVisible(true);
}

void HomeWindow::on_toolButton_DeviceControlWidgetSetting_notShow_clicked()
{
    ui->DeviceControlWidgetSetting->setVisible(false);
    // 1-show 2-hide
    ui->toolButton_DeviceControlWidget_show->setVisible(false);
    ui->toolButton_DeviceControlWidget_notShow->setVisible(true);
    ui->toolButton_DeviceControlWidgetSetting_show->setVisible(true);
    ui->toolButton_DeviceControlWidgetSetting_notShow->setVisible(false);
}

void HomeWindow::RelayoutPTZControlWidget()
{
    ui->widget_PTZControl_PAN_AUTO->colorBrushHtml = "#607d8b";
    // 屏蔽圆形区域，但是有锯齿
    // ui->widget_PTZControl->setMask(QRegion(ui->widget_PTZControl->rect().adjusted(1,1,-1,-1), QRegion::Ellipse));
    // 重置父窗口状态，可以修改布局位置
    ui->widget_PTZControl_PAN_AUTO->setParent(nullptr);
    ui->widget_PTZControl_PAN_AUTO->setParent(ui->widget_PTZControl);

    int width = ui->toolButton_camera_robot_auto->width() * sqrt(2);
    int height = ui->toolButton_camera_robot_auto->height() * sqrt(2);
    qDebug() << "修改布局位置" << width << height << ui->widget_PTZControl_PAN_AUTO->width() << ui->widget_PTZControl_PAN_AUTO->height();
    ui->widget_PTZControl_PAN_AUTO->setGeometry(
        (ui->widget_PTZControl->width() - width) / 2 - 1,
        (ui->widget_PTZControl->height() - height) / 2 - 1,
        width + 4,
        height + 4);
    qDebug() << "修改布局位置" << width << height << ui->widget_PTZControl_PAN_AUTO->width() << ui->widget_PTZControl_PAN_AUTO->height();
    ui->widget_PTZControl_PAN_AUTO->stackUnder(ui->widget_PTZControl);
}

void HomeWindow::on_test_clicked()
{
}

void HomeWindow::on_toolButton_robot_send_cmd_clicked()
{
    qDebug() << "HomeWindow::on_toolButton_robot_send_cmd_clicked()";
    //    QByteArray byteArray;
    //    byteArray.append(0x5A);
    //    byteArray.append(0x02);
    //    byteArray.append(0x01);
    //    robot.sendMessage(currentDeviceId,byteArray);

    //    QByteArray byteArray2(QByteArray::fromHex("5A 02 01"));
    //    robot.sendMessage(currentDeviceId,byteArray2);
    QString cmd = ui->lineEdit_robot_cmd->text();
    //    QString cmd = "5a 02 01";
    //    QByteArray cmdByteArray;
    //    QStringList hexValues = cmd.split(" ");
    //    for (const QString& hexValue : hexValues) {
    //        bool ok;
    //        int value = hexValue.toInt(&ok, 16);
    //        if (ok) {
    //            cmdByteArray.append(static_cast<char>(value));
    //        }
    //    }
    DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(cmd.toUtf8()));
}

void HomeWindow::on_toolButton_robot_move_forward_pressed()
{
    qDebug() << "HomeWindow::on_toolButton_robot_move_forward_pressed()";
    // robot.sendMessage(currentDeviceId,QByteArray::fromRawData("\x5A\x02\x09", 3));
    DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Move_Forward));
}

void HomeWindow::on_toolButton_robot_move_forward_released()
{
    DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Move_Stop));
}

void HomeWindow::on_toolButton_robot_move_backward_pressed()
{
    DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Move_Backward));
}

void HomeWindow::on_toolButton_robot_move_backward_released()
{
    DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Move_Stop));
}

void HomeWindow::on_toolButton_robot_speed_add_clicked()
{
    DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Speed_Add));
}

void HomeWindow::on_toolButton_robot_speed_subtract_clicked()
{
    DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Speed_Subtrat));
}

void HomeWindow::on_toolButton_robot_state_set_clicked()
{
    if (DeviceMap[DeviceId].robot->isCmdState())
    {
        // 手动模式切换自动
        if (DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_State_Auto)))
        {
            ui->toolButton_robot_state_set->setText("手动模式");
        }
    }
    else
    {
        if (DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_State_Cmd)))
        {
            ui->toolButton_robot_state_set->setText("自动模式");
        }
    }
}

void HomeWindow::on_toolButton_robot_charge_set_clicked()
{
    if (!DeviceMap[DeviceId].robot->isCmdCharging())
    {
        // 开启充电
        if (DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_State_Charge)))
        {
            ui->toolButton_robot_charge_set->setText("取消充电");
            ui->toolButton_robot_charge_set->setStyleSheet("background-color: red;");
        }
    }
    else
    {
        if (DeviceMap[DeviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Charge_Off)))
        {
            ui->toolButton_robot_charge_set->setText("开启充电");
            ui->toolButton_robot_charge_set->setStyleSheet("");
        }
    }
}

void HomeWindow::on_toolButton_robot_camera_set_PAN_TITL_clicked()
{
    QByteArray byteArray;
    byteArray.append(Robot_CMD_Header);
    byteArray.append(0x22);
    //    byteArray.append(0x01);
    int16_t pan = ui->lineEdit_camera_PAN->text().toDouble() * 10;
    int16_t titl = ui->lineEdit_camera_TITL->text().toDouble() * 10;
    byteArray.append(reinterpret_cast<const char *>(&pan), sizeof(pan));
    byteArray.append(reinterpret_cast<const char *>(&titl), sizeof(titl));

    DeviceMap[DeviceId].robot->sendMessage(byteArray);
}

void HomeWindow::on_toolButton_robot_map_set_clicked()
{
    QByteArray byteArray;
    byteArray.append(Robot_CMD_Header);
    byteArray.append(0x0B);
    int32_t origin = ui->lineEdit_robot_map_origin->text().toInt();
    int32_t end = ui->lineEdit_robot_map_end->text().toInt();
    byteArray.append(reinterpret_cast<const char *>(&origin), sizeof(origin));
    byteArray.append(reinterpret_cast<const char *>(&end), sizeof(end));

    DeviceMap[DeviceId].robot->sendMessage(byteArray);
}

void HomeWindow::on_toolButton_robot_move_goto_clicked()
{
    QByteArray byteArray;
    byteArray.append(Robot_CMD_Header);
    byteArray.append(0x0A);
    byteArray.append(0x02);
    byteArray.append(0x01);

    int32_t arg1 = ui->lineEdit_robot_move_pose->text().toInt();
    byteArray.append(reinterpret_cast<const char *>(&arg1), sizeof(arg1));
    //    byteArray.append(0xff & arg1);
    //    byteArray.append((0xff00 & arg1) >> 8);
    //    byteArray.append((0xff0000 & arg1) >> 16);
    //    byteArray.append((0xff000000 & arg1) >> 24);
    DeviceMap[DeviceId].robot->sendMessage(byteArray);
}

void HomeWindow::on_toolButton_robot_move_goto_cancel_clicked()
{
    QByteArray byteArray;
    byteArray.append(Robot_CMD_Header);
    byteArray.append(0x0A);
    byteArray.append(0x02);
    byteArray.append(0x00 & 0xff);
    DeviceMap[DeviceId].robot->sendMessage(byteArray);
}

void HomeWindow::on_toolButton_robot_time_set_clicked()
{

    int32_t arg1 = ui->timeEdit_robot_time_begin->time().msecsSinceStartOfDay() / 1000;
    int32_t arg2 = QTime::currentTime().msecsSinceStartOfDay() / 1000;
    int32_t arg3 = ui->timeEdit_robot_time_end->time().msecsSinceStartOfDay() / 1000;
    qDebug() << "时间：" << arg1 << arg2 << arg3;
    if (arg3 < arg1)
    {
        QMessageBox msgBox(this);
        msgBox.setText("开始时间大于等于结束时间，请重新设置");
        // 3秒后自动关闭
        QTimer::singleShot(3000, &msgBox, &QMessageBox::close);
        qWarning() << "开始时间大于等于结束时间";
        return;
    }
    QByteArray byteArray;
    byteArray.append(Robot_CMD_Header);
    byteArray.append(0x0C);
    byteArray.append(reinterpret_cast<const char *>(&arg1), sizeof(arg1));
    byteArray.append(reinterpret_cast<const char *>(&arg2), sizeof(arg2));
    byteArray.append(reinterpret_cast<const char *>(&arg3), sizeof(arg3));
    DeviceMap[DeviceId].robot->sendMessage(byteArray);
}

void HomeWindow::on_toolButton_device_management_isShow_clicked()
{
    ui->widget_device_add->setVisible(!ui->widget_device_add->isVisible());
}

void HomeWindow::on_toolButton_device_add_clicked()
{
    QJsonObject devices = config["devices"].toObject();
    QString key = ui->comboBox_device_add_id->currentText();
    if (devices.contains(key))
    {
        QToolTip::showText(ui->toolButton_device_add->mapToGlobal(QPoint(0, 0)),
                           "要增加的键值已存在，请使用修改功能",
                           ui->toolButton_device_add);
        return;
    }

    int id = key.toInt();
    DeviceMap[id].id = 0;
    DeviceMap[id].robot = new Robot();
    // deviceMap[id].robot->config = device["config"].toObject();
    DeviceMap[id].robot->init(); // 配置读取，摄像头数据 通道号等
    DeviceMap[id].type = DeviceType(DeviceType_Robot_test);

    QJsonObject device;
    device["type"] = ui->comboBox_device_add_type->currentText().toInt();
    devices[i2s(id)] = device;
    config["devices"] = devices;

    ui->LayoutDevice->addWidget(DeviceMap[id].robot->ui->widgetMenu);
    connect(DeviceMap[id].robot, &Robot::setCameraWidgetPlay, this, &HomeWindow::CameraWidgetPlay);
}

void HomeWindow::on_toolButton_device_update_clicked()
{
    QJsonObject devices = config["devices"].toObject();
    QString key = ui->comboBox_device_add_id->currentText();
    if (!devices.contains(key))
    {
        QToolTip::showText(ui->toolButton_device_update->mapToGlobal(QPoint(0, 0)),
                           "要修改的键值不存在，请使用增加功能",
                           ui->toolButton_device_update);
        return;
    }

    int id = key.toInt();
    QJsonObject device;
    device["type"] = ui->comboBox_device_add_type->currentData().toInt();
    devices[i2s(id)] = device;
    config["devices"] = devices;
}

void HomeWindow::on_toolButton_device_delete_clicked()
{
    QJsonObject devices = config["devices"].toObject();
    QString key = ui->comboBox_device_add_id->currentText();
    if (!devices.contains(key))
    {
        QToolTip::showText(ui->toolButton_device_delete->mapToGlobal(QPoint(0, 0)),
                           "要删除的键值不存在",
                           ui->toolButton_device_delete);
        return;
    }

    int id = key.toInt();
    ui->LayoutDevice->removeWidget(DeviceMap[id].robot->ui->widgetMenu);
    disconnect(DeviceMap[id].robot, &Robot::setCameraWidgetPlay, this, &HomeWindow::CameraWidgetPlay);

    devices.remove(i2s(id));

    DeviceMap[id].robot->config = QJsonObject();
    DeviceMap[id].robot->quit();
    DeviceMap.remove(id);

    config["devices"] = devices;
}

void HomeWindow::on_comboBox_device_add_id_currentTextChanged(const QString &arg1)
{
    QJsonObject devices = config["devices"].toObject();
    if (devices.contains(arg1))
    {
        int type = devices[arg1].toObject()["type"].toInt(-1);
        if (type != -1)
        {
            int index = ui->comboBox_device_add_type->findData(type);
            if (index != -1)
            {
                ui->comboBox_device_add_type->setCurrentIndex(index);
            }
        }
    }
}


void HomeWindow::on_toolButton_map_show_clicked()
{
    ui->window_map->setVisible(!ui->window_map->isVisible());
}



