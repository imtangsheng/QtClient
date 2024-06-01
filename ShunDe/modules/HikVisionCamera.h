#ifndef HIKVISIONCAMERA_H
#define HIKVISIONCAMERA_H

#include <QEventLoop>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QWidget>

#include "HCNetSDK.h"

enum HttpMethod{
    Http_GET,
    Http_PUT,
    Http_POST,
    Http_DELETE
};

class NetworkRequest : public QObject {
    Q_OBJECT

public:
    NetworkRequest(QObject* parent = nullptr) : QObject(parent) {}

    QNetworkAccessManager manager;
    QByteArray response;

    bool sendRequest(const QUrl& url, const QByteArray& data = QByteArray(),HttpMethod httpMethod = HttpMethod::Http_GET) {
        const QNetworkRequest request(url);
        QNetworkReply* reply = nullptr;
        switch (httpMethod) {
        case HttpMethod::Http_GET:
            reply = manager.get(request);
            break;
        case HttpMethod::Http_PUT:
            reply = manager.put(request, data);
            break;
        case HttpMethod::Http_POST:
            reply = manager.post(request, data);
            break;
        case HttpMethod::Http_DELETE:
            reply = manager.deleteResource(request);
            break;
        }

        QEventLoop loop;
        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        loop.exec();

        // 检查请求是否成功
        if (reply->error() != QNetworkReply::NoError) {
            qDebug() << "Error:" << reply->errorString();
            reply->deleteLater();
            return false;
        }
        response = reply->readAll();
        emit requestFinished(response);
        reply->deleteLater();
        return true;
    }

signals:
    void requestFinished(const QByteArray& response);


};


namespace Ui {
class HikVisionCamera;
}

class HikVisionCamera : public QWidget
{
    Q_OBJECT

public:
    explicit HikVisionCamera(QWidget *parent = nullptr);
    ~HikVisionCamera();
    void init();
    //摄像头操作方法
    QJsonObject camera;

    bool updateCameraPose_Pan_Tilt(int pan, int tilt);// ISAPI 协议

    bool PTZControl(int lChannel,int dwPTZCommand,int dwStop);
    bool PTZPreset(int lChannel,int dwPresetIndex,int dwPTZPresetCmd);
    QJsonObject PTZPOS_get(int lChannel);
    bool PTZPOS_set(int lChannel,int wAction,int wPanPos,int wTiltPos,int wZoomPos);
    bool CaptureJPEGPicture(int lChannel,QString fileName);
    bool Realtime_Thermometry(QString fileName);
    // ISAPI 协议

    QNetworkReply *reply;
    QNetworkAccessManager manager;
    QString request_URL;
    QByteArray request_xmlData;
    QEventLoop loop_NetworkReply;
    QByteArray response;
    bool api_request_ISAPI(const QString& url,QByteArray requestData=NULL,HttpMethod methods=Http_GET);

    NetworkRequest request;
    void init_NetworkRequest();
    // SDK 协议
    // 注册设备
    LONG lUserID;
    void init_HCNetSDK();
    void Login_V40();
    void start();



private:
    Ui::HikVisionCamera *ui;
};

#endif // HIKVISIONCAMERA_H
