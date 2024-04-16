#include "robot.h"
#include <QScreen>

Robot::Robot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Robot)
{
    ui->setupUi(this);
}

Robot::~Robot()
{
    delete ui;
    qDebug()<<"Robot::~Robot()";
}

void Robot::init()
{
    qDebug()<<"void Robot::init()"<<config;
    if(config.contains("camera")){
        QJsonObject camera = config["camera"].toObject();
        ui->lineEdit_robot_name->setText(camera["name"].toString());

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

    }
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

    QJsonObject camera = config["camera"].toObject();
    QUrl source = QUrl("rtsp://"+
                       camera["video_username"].toString()+":"+
                       camera["video_password"].toString()+"@"+
                       camera["video_ip"].toString()+":554/Streaming/Channels/"+
                       camera["video_stream"].toString());
    emit setCameraWidgetPlay(id,source);
}


void Robot::on_toolButton_channel02_thermal_play_clicked()
{
    QJsonObject camera = config["camera"].toObject();
    QUrl source = QUrl("rtsp://"+
            camera["thermal_username"].toString()+":"+
            camera["thermal_password"].toString()+"@"+
            camera["thermal_ip"].toString()+"/"+
            camera["thermal_stream"].toString()+"/main");
    emit setCameraWidgetPlay(id,source);
}

void Robot::on_toolButton_robto_config_save_clicked()
{

    QJsonObject camera = config["camera"].toObject();
    camera["name"] = ui->lineEdit_robot_name->text();

    camera["video_name"] =    ui->lineEdit_channel01_video_name->text();
    camera["video_username"] =   ui->lineEdit_channel01_video_username->text();
    camera["video_password"] =   ui->lineEdit_channel01_video_password->text();
    camera["video_ip"]  =  ui->lineEdit_channel01_video_ip->text();
    camera["video_stream"]   = ui->comboBox_channel01_video_stream->currentText();

    camera["thermal_name"]  =  ui->lineEdit_channel02_thermal_name->text();
    camera["thermal_username"] =   ui->lineEdit_channel02_thermal_username->text();
    camera["thermal_password"] =   ui->lineEdit_channel02_thermal_password->text();
    camera["thermal_ip"]   = ui->lineEdit_channel02_thermal_ip->text();
    camera["thermal_stream"] =   ui->comboBox_channel02_thermal_stream->currentText();

    config["camera"] = camera;
    ui->widgetSetting->close();

    ui->pushButton_robot->setText(camera["name"].toString("机器人"));
}

void Robot::on_pushButton_robot_clicked()
{
    ui->widgetSetting->setWindowFlags(Qt::Dialog);
    ui->widgetSetting->setWindowTitle(this->objectName());
    ui->widgetSetting->setWindowIcon(QIcon(":/asset/Robot/Robot.svg"));
    ui->widgetSetting->setAttribute(Qt::WA_ShowModal,true);
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
    QScreen* targetScreen = nullptr;
    for (QScreen* screen : QGuiApplication::screens()) {
        if (screen->geometry().contains(cursorPos)) {
            targetScreen = screen;
            break;
        }
    }
    if (targetScreen) {
        QRect targetGeometry = targetScreen->geometry();
        int x = targetGeometry.center().x() - ui->widgetSetting->width() / 2;
        int y = targetGeometry.center().y() - ui->widgetSetting->height() / 2;
        ui->widgetSetting->move(x, y);
    }

    ui->widgetSetting->show();
}

