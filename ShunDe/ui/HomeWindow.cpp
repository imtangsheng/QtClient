#include "HomeWindow.h"
#include "ui_HomeWindow.h"

#include <QMessageBox>
#include <QTimer>
#include "AppOS.h"

HomeWindow::HomeWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HomeWindow)
{
    ui->setupUi(this);
    init();
}

HomeWindow::~HomeWindow()
{
    delete ui;
    qDebug()<<"HomeWindow::~HomeWindow()";
}

void HomeWindow::init()
{
    deviceId = 0;

    //for(int i=0;i<1;i++){
    //deviceMap[deviceId].robot = new Robot(this);
        deviceMap[deviceId].robot->init();

        ui->LayoutDevice->addWidget(deviceMap[deviceId].robot->ui->widgetMenu);
        connect(deviceMap[deviceId].robot,&Robot::setCameraWidgetPlay,this,&HomeWindow::CameraWidgetPlay);
    //}

    RelayoutCameraWidget();
//    cameraWidgets.resize(cameraWidgetsNum);
    RelayoutPTZControlWidget();

    //startTcpServerListen("0.0.0.0", 12345);
    qDebug()<<"void HomeWindow::init()";
    //接收到机器人数据
}

void HomeWindow::quit()
{

}

void HomeWindow::RelayoutCameraWidget()
{
    // 清空布局
    QLayoutItem* item;

    while ((item = ui->LayoutPreview->takeAt(0)) != nullptr) {
        qDebug()<<"void HomeWindow::RelayoutCameraWidget() 11";
        ui->LayoutPreview->removeItem(item);
    }
    qDebug()<<"初始布局"<<cameraWidgets.size() << cameraWidgetsNum;
    while(cameraWidgets.size() < cameraWidgetsNum){
        CameraWidget* cameraWidget = new CameraWidget(ui->WidgetPreview);
        cameraWidgets.append(cameraWidget);
        connect(cameraWidget,&CameraWidget::mousePress,this,[=]{
            lastItemCameraWidget = currentItemCameraWidget;
            currentItemCameraWidget = cameraWidgets.indexOf(cameraWidget);
            MouseButtonPressCameraWidget(currentItemCameraWidget);
        });
        qDebug()<<"初始布局 void HomeWindow::RelayoutCameraWidget() 22:"<<cameraWidgets.size();
    }

    int numColumns = 2; // 每行的列数
    for(int i = 0;i<cameraWidgetsNum;i++){
        qDebug()<<"void HomeWindow::RelayoutCameraWidget() 33:"<<i <<i / numColumns << i % numColumns ;
//        CameraWidget* widget = cameraWidgets[i];
        ui->LayoutPreview->addWidget(cameraWidgets[i],i / numColumns,i % numColumns);
    }

}

bool HomeWindow::startTcpServerListen(const QString &ipAddress, const quint16 &port)
{
    qDebug() << "启动服务器：" << ipAddress<<port;
        if(!server.listen(QHostAddress(ipAddress),port)){
        qWarning() << "无法启动服务器：" << server.errorString();
            return false;
    }
    qDebug() << "服务器已启动，监听地址：" << ipAddress << "，端口号：" << port;
    // 处理新的客户端连接
    QObject::connect(&server, &QTcpServer::newConnection,this, [=]() {
        // 接受新的客户端连接
        QTcpSocket *clientSocket = server.nextPendingConnection();

        // 处理客户端连接
        qDebug() << "新的客户端连接：" << clientSocket->peerAddress().toString() << ":" << clientSocket->peerPort();
            // 接收一次数据
            if (clientSocket->waitForReadyRead()) {
            QByteArray headerData = clientSocket->readAll();
            int value = (int)headerData.at(0) << 8 | (int)headerData.at(1);
            qDebug() << "新的客户端连接：value" <<headerData.at(0)<< (int)headerData.at(0)<< (int)headerData.at(1)<<value;
            if(headerData.at(0) == 0x55 && headerData.at(1) == 0x01){
                //机器人
                int clientId = (int)headerData.at(2) << 8 | (int)headerData.at(3);
                qDebug() << "新的客户端连接：currenId" << clientId;
                deviceMap[clientId].id = clientId;
                deviceMap[clientId].type = DeviceType_Robot;
                deviceMap[clientId].ipAddress = ipAddress +":"+ port;
                deviceMap[clientId].isOnline = true;
                deviceMap[clientId].client = clientSocket;
                deviceMap[clientId].robot->client = clientSocket;
                // 接收客户端发送的数据
                QObject::connect(clientSocket, &QTcpSocket::readyRead, this,[=]() {
                    QByteArray bytes = clientSocket->readAll();
                    qDebug() << "接收到客户端数据：" << bytes;
                    //
                    deviceMap[clientId].robot->data = (RobotRecvPacket *) bytes.data();
                    qDebug()<<"RobotRecvPacket"<<deviceMap[clientId].robot->data;
                    qDebug()<<"head数据头"<<deviceMap[clientId].robot->data->head;
                    qDebug()<<"networkIndicator网络判断"<<deviceMap[clientId].robot->data->networkIndicator;


                    deviceMap[clientId].robot->updateDataShow();
                });// 接收机器人客户端发送的数据
            }

        }// 处理新的客户端连接 end

        // 客户端断开连接
        QObject::connect(clientSocket, &QTcpSocket::disconnected, [clientSocket]() {
            qDebug() << "客户端断开连接：" << clientSocket->peerAddress().toString() << ":" << clientSocket->peerPort();
                                                                                                           clientSocket->deleteLater();
        });
    });
    return true;
}

bool HomeWindow::CameraWidgetPlay(const QUrl &source)
{
    if(currentItemCameraWidget == -1){
        return false;
    }

    //QUrl source = QUrl("rtsp://admin:dacang80@192.168.1.38:554/Streaming/Channels/101");
    cameraWidgets.at(currentItemCameraWidget)->player.setSource(source);
    cameraWidgets.at(currentItemCameraWidget)->player.play();
    //cameraWidgets.at(currentItemCameraWidget)->player.error();
    return true;
}

void HomeWindow::MouseButtonPressCameraWidget(int index)
{
    qDebug()<<"HomeWindow::MouseButtonPressCameraWidget(int "<<index;
    //border：指定边框的样式。2px：指定边框的宽度为2像素。solid：指定边框的样式为实线。red：指定边框的颜色为红色
    if(lastItemCameraWidget != -1){
        qDebug()<<"HomeWindow::MouseButtonPressCameraWidget(int lastItemCameraWidget:"<<lastItemCameraWidget;
        cameraWidgets.at(lastItemCameraWidget)->setContentsMargins(0,0,0,0);//显示边框
    }
    cameraWidgets.at(index)->setContentsMargins(1,1,1,1);//显示边框
}


void HomeWindow::on_toolButton_cameraChannel02_clicked()
{
    if(currentItemCameraWidget == -1){
        return;
    }

    QUrl source = QUrl("rtsp://admin:dacang80@192.168.1.38:554/Streaming/Channels/201");

    cameraWidgets.at(currentItemCameraWidget)->player.setSource(source);
    cameraWidgets.at(currentItemCameraWidget)->player.play();
}


void HomeWindow::on_toolButton_cameraChannel01_clicked()
{
    if(currentItemCameraWidget == -1){
        return;
    }

    QUrl source = QUrl("rtsp://admin:dacang80@192.168.1.38:554/Streaming/Channels/101");

    cameraWidgets.at(currentItemCameraWidget)->player.setSource(source);
    cameraWidgets.at(currentItemCameraWidget)->player.play();
}


void HomeWindow::on_toolButton_DeviceControlWidget_show_clicked()
{
    //1 表示AWidget 2 表示BWidget: 1-show
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
    //1-hide 2-hide
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
    //屏蔽圆形区域，但是有锯齿
    //ui->widget_PTZControl->setMask(QRegion(ui->widget_PTZControl->rect().adjusted(1,1,-1,-1), QRegion::Ellipse));
    //重置父窗口状态，可以修改布局位置
    ui->widget_PTZControl_PAN_AUTO->setParent(nullptr);
    ui->widget_PTZControl_PAN_AUTO->setParent(ui->widget_PTZControl);

    int width =  ui->toolButton_camera_robot_auto->width()*sqrt(2);
    int height = ui->toolButton_camera_robot_auto->height()*sqrt(2);
    qDebug()<<"修改布局位置"<<width<<height<<ui->widget_PTZControl_PAN_AUTO->width()<<ui->widget_PTZControl_PAN_AUTO->height();
    ui->widget_PTZControl_PAN_AUTO->setGeometry(
        (ui->widget_PTZControl->width() - width)/2 -1,
        (ui->widget_PTZControl->height() - height)/2 -1,
        width+4,
        height+4
        );
    qDebug()<<"修改布局位置"<<width<<height<<ui->widget_PTZControl_PAN_AUTO->width()<<ui->widget_PTZControl_PAN_AUTO->height();
    ui->widget_PTZControl_PAN_AUTO->stackUnder(ui->widget_PTZControl);
}





void HomeWindow::on_test_clicked()
{

}

void HomeWindow::on_toolButton_robot_send_cmd_clicked()
{
    qDebug()<<"HomeWindow::on_toolButton_robot_send_cmd_clicked()";
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
    deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(cmd.toUtf8()));
}

void HomeWindow::on_toolButton_robot_move_forward_pressed()
{
    qDebug()<<"HomeWindow::on_toolButton_robot_move_forward_pressed()";
    //robot.sendMessage(currentDeviceId,QByteArray::fromRawData("\x5A\x02\x09", 3));
    deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Move_Forward));
}

void HomeWindow::on_toolButton_robot_move_forward_released()
{
    deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Move_Stop));
}


void HomeWindow::on_toolButton_robot_move_backward_pressed()
{
    deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Move_Backward));
}


void HomeWindow::on_toolButton_robot_move_backward_released()
{
    deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Move_Stop));
}


void HomeWindow::on_toolButton_robot_speed_add_clicked()
{
    deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Speed_Add));
}


void HomeWindow::on_toolButton_robot_speed_subtract_clicked()
{
    deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Speed_Subtrat));
}


void HomeWindow::on_toolButton_robot_state_set_clicked()
{
    if(deviceMap[deviceId].robot->isCmdState()){
        //手动模式切换自动
        if(deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_State_Auto))){
        ui->toolButton_robot_state_set->setText("手动模式");
        }
    }else {
        if(deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_State_Cmd))){
        ui->toolButton_robot_state_set->setText("自动模式");
        }
    }

}


void HomeWindow::on_toolButton_robot_charge_set_clicked()
{
    if(!deviceMap[deviceId].robot->isCmdCharging()){
        //开启充电
        if(deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_State_Charge))){
        ui->toolButton_robot_state_set->setText("取消充电");
        ui->toolButton_robot_state_set->setStyleSheet("background-color: red;");
        }
    }else {
        if(deviceMap[deviceId].robot->sendMessage(QByteArray::fromHex(Robot_CMD_Charge_Off))){
        ui->toolButton_robot_state_set->setText("开启充电");
        ui->toolButton_robot_state_set->setStyleSheet("");
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
    byteArray.append(reinterpret_cast<const char*>(&pan), sizeof(pan));
    byteArray.append(reinterpret_cast<const char*>(&titl), sizeof(titl));

    deviceMap[deviceId].robot->sendMessage(byteArray);

}


void HomeWindow::on_toolButton_robot_map_set_clicked()
{
    QByteArray byteArray;
    byteArray.append(Robot_CMD_Header);
    byteArray.append(0x0B);
    int32_t origin = ui->lineEdit_robot_map_origin->text().toInt();
    int32_t  end = ui->lineEdit_robot_map_end->text().toInt();
    byteArray.append(reinterpret_cast<const char*>(&origin), sizeof(origin));
    byteArray.append(reinterpret_cast<const char*>(&end), sizeof(end));

    deviceMap[deviceId].robot->sendMessage(byteArray);
}


void HomeWindow::on_toolButton_robot_move_goto_clicked()
{
    QByteArray byteArray;
    byteArray.append(Robot_CMD_Header);
    byteArray.append(0x0A);
    byteArray.append(0x02);
    byteArray.append(0x01);

    int32_t arg1 = ui->lineEdit_robot_move_pose->text().toInt();
    byteArray.append(reinterpret_cast<const char*>(&arg1), sizeof(arg1));
//    byteArray.append(0xff & arg1);
//    byteArray.append((0xff00 & arg1) >> 8);
//    byteArray.append((0xff0000 & arg1) >> 16);
//    byteArray.append((0xff000000 & arg1) >> 24);
    deviceMap[deviceId].robot->sendMessage(byteArray);
}


void HomeWindow::on_toolButton_robot_move_goto_cancel_clicked()
{
    QByteArray byteArray;
    byteArray.append(Robot_CMD_Header);
    byteArray.append(0x0A);
    byteArray.append(0x02);
    byteArray.append(0x00 & 0xff);
    deviceMap[deviceId].robot->sendMessage(byteArray);
}


void HomeWindow::on_toolButton_robot_time_set_clicked()
{

    int32_t arg1 = ui->timeEdit_robot_time_begin->time().msecsSinceStartOfDay() / 1000;
    int32_t arg2 = QTime::currentTime().msecsSinceStartOfDay() / 1000;
    int32_t arg3 = ui->timeEdit_robot_time_end->time().msecsSinceStartOfDay() / 1000;
    qDebug()<<"时间："<<arg1<<arg2<<arg3;
    if(arg3<arg1){
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
    byteArray.append(reinterpret_cast<const char*>(&arg1), sizeof(arg1));
    byteArray.append(reinterpret_cast<const char*>(&arg2), sizeof(arg2));
    byteArray.append(reinterpret_cast<const char*>(&arg3), sizeof(arg3));
    deviceMap[deviceId].robot->sendMessage(byteArray);
}


