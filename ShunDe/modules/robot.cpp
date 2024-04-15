#include "robot.h"


Robot::Robot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Robot)
{
    ui->setupUi(this);
    init();
}

Robot::~Robot()
{
    delete ui;
    qDebug()<<"Robot::~Robot()";
}

void Robot::init()
{
    qDebug()<<"void Robot::init()";
}

void Robot::quit()
{
    qDebug()<<"void Robot::quit()";
}


bool Robot::sendMessage(const QString &message)
{
    if(!client){
        qWarning() << "客户端sendMessage deviceId 不存在"<<client;
        return false;
    }
    qDebug() << "客户端sendMessage"<<client->isValid()<<client->isOpen();
    if(!client->isValid()){
        qWarning() << "无法启动服务器：" <<client->isOpen()<< client->error();
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
    if(!client){
        qWarning() << "客户端sendMessage deviceId 不存在";
        return false;
    }
    qDebug() << "客户端sendMessage"<<client->isValid()<<client->isOpen();
    if(!client->isValid()){
        qWarning() << "无法启动服务器：" <<client->isOpen()<< client->error();
        return false;
    }

    client->write(message);
    return true;
}

bool Robot::isCmdState()
{
    if(!client){
        return false;
    }
    return true;
}

bool Robot::isCmdCharging()
{
    if(!client){
        return false;
    }
    return true;
}

void Robot::updateDataShow()
{
    qDebug()<<"HomeWindow::updateRobotDataShow(int )";
//    ui->label_robot_temperature->setText(i2s(data->temperature));
//    ui->label_robot_humidity->setText(i2s(data->humidity));
//    ui->label_robot_CO->setText(i2s(data->CO));
//    ui->label_robot_H2S->setText(i2s(data->H2S));
//    ui->label_robot_O2->setText(i2s(data->O2));
//    ui->label_robot_CH4->setText(i2s(data->CH4));
//    ui->label_robot_smoke->setText(i2s(data->smoke));

//    ui->label_robot_pose->setText(i2s(data->pose));
//    ui->label_robot_state->setText(i2s(data->state));
//    ui->label_robot_batteryLevel->setText(i2s(data->batteryLevel));
}

void Robot::on_toolButton_widget_cameraChannel_isShow_clicked()
{
    if(ui->widget_cameraChannel->isVisible()){
        ui->widget_cameraChannel->setVisible(false);
        ui->toolButton_widget_cameraChannel_isShow->setIcon(QIcon(":/asset/Robot/Robot_menu_show.svg"));
    }else{
         ui->widget_cameraChannel->setVisible(true);
        ui->toolButton_widget_cameraChannel_isShow->setIcon(QIcon(":/asset/Robot/Robot_menu_hide.svg"));
    }

}

void Robot::on_toolButton_channel01_video_play_clicked()
{

    QUrl source = QUrl("rtsp://admin:dacang80@192.168.1.66:554/Streaming/Channels/101");
    emit setCameraWidgetPlay(source);
}


void Robot::on_toolButton_channel02_thermal_play_clicked()
{
    QUrl source = QUrl("rtsp://admin:123456@192.168.1.19/0/main");
    emit setCameraWidgetPlay(source);
}




