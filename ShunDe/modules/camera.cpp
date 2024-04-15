#include "camera.h"

#include <QNetworkAccessManager>
#include <QtNetwork>

Camera::Camera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Camera)
{
    ui->setupUi(this);
    init();
}

Camera::~Camera()
{
    delete ui;
    qDebug()<<"Camera::~Camera()";
}

//#include <QHttpClient>
//#include <QHttpHeaders>
//#include <QHttpParams>
//#include <QHttpResult>

void Camera::init()
{

qDebug()<<"void Camera::init()";
}

void Camera::quit()
{
qDebug()<<"void Camera::quit()";
}

bool Camera::test()
{
    qDebug() << " Camera::test()";
    // 创建一个QNetworkAccessManager对象
    QNetworkAccessManager manager;
    // 创建一个QNetworkRequest对象，并设置请求的URL
    QNetworkRequest request(QUrl("http://192.168.1.38/ISAPI/PTZCtrl/channels/1/continuous"));

    // 设置Digest Authorization认证参数
    QString username = "admin";
    QString password = "dacang80";
//    QString credentials = QString("%1:%2").arg(username).arg(password);
//    QByteArray encodedCredentials = credentials.toUtf8().toBase64();
//    QString authorizationHeader = "Digest " + encodedCredentials;
//    request.setRawHeader("Authorization", authorizationHeader.toUtf8());
    // 设置Digest认证信息
//    QAuthenticator authenticator;
//    authenticator.setUser("admin");
//    authenticator.setPassword("dacang80");

//    authenticator.realm();
//    authenticator.detach();



    // 设置请求头部为XML数据
//    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/xml");

    // 创建XML数据
    QByteArray xmlData = "<?xml version=\"1.0\" encoding=\"UTF-8\"?><PTZData><pan>60</pan><tilt>0</tilt></PTZData>";

    // 发送POST请求，并将XML数据作为请求的主体数据
    QNetworkReply* reply = manager.put(request, xmlData);

//    manager.authenticationRequired(reply,&authenticator);

    // 等待请求完成
    QEventLoop loop;
    // 处理认证
    QObject::connect(&manager, &QNetworkAccessManager::authenticationRequired, [&](QNetworkReply *reply, QAuthenticator *authenticator) {
        authenticator->setUser("admin");
        authenticator->setPassword("dacang80");
        qDebug() << "Response:处理认证";
    });

    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    // 检查请求是否成功
    if (reply->error() == QNetworkReply::NoError) {
        // 读取响应数据
        QByteArray response = reply->readAll();
        qDebug() << "Response:" << response;
    } else {
        // 处理请求错误
        qDebug() << "Error:" << reply->errorString();
    }

    // 释放资源
    reply->deleteLater();

    return true;
}
