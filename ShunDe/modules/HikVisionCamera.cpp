#include "HikVisionCamera.h"
#include "ui_HikVisionCamera.h"

#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "AppOS.h"


HikVisionCamera::HikVisionCamera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HikVisionCamera)
{
    ui->setupUi(this);
}

HikVisionCamera::~HikVisionCamera()
{
    delete ui;
    qDebug()<<"HikVisionCamera::~HikVisionCamera()";
}

bool HikVisionCamera::updateCameraPose_Pan_Tilt(int pan, int tilt)
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

bool HikVisionCamera::PTZControl(int dwPTZCommand, int dwStop)
{
    qDebug()<<"HikVisionCamera::PTZControl";
    return true;
}

bool HikVisionCamera::PTZPreset(int lChannel, int dwPresetIndex, int dwPTZPresetCmd)
{
    qDebug()<<"HikVisionCamera::PTZPreset";
    return true;
}

bool HikVisionCamera::PTZPOS(int wAction, int wPanPos, int wTiltPos, int wZoomPos)
{
    qDebug()<<"HikVisionCamera::PTZPOS";
    return true;
}

bool HikVisionCamera::CaptureJPEGPicture(int lChannel, int sPicFileName)
{
    qDebug()<<"HikVisionCamera::CaptureJPEGPicture";
    return true;
}

bool HikVisionCamera::Realtime_Thermometry(int sPicFileName)
{
    qDebug()<<"HikVisionCamera::Realtime_Thermometry";
    return true;
}
