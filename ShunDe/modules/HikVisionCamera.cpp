#include "HikVisionCamera.h"
#include "ui_HikVisionCamera.h"

#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QNetworkReply>


#include "AppUtil.h"


#include <QDateTime>
#include <QDir>
#include <QString>

/**
获取云台坐标
/ISAPI/PTZCtrl/channels/1/absoluteEx

[in] 云台控制命令，见下表： 宏定义 宏定义值 含义
LIGHT_PWRON 2 接通灯光电源
WIPER_PWRON 3 接通雨刷开关
FAN_PWRON 4 接通风扇开关
HEATER_PWRON 5 接通加热器开关
AUX_PWRON1 6 接通辅助设备开关
AUX_PWRON2 7 接通辅助设备开关
ZOOM_IN 11 焦距变大(倍率变大)
ZOOM_OUT 12 焦距变小(倍率变小)
FOCUS_NEAR 13 焦点前调
FOCUS_FAR 14 焦点后调
IRIS_OPEN 15 光圈扩大
IRIS_CLOSE 16 光圈缩小
TILT_UP 21 云台上仰
TILT_DOWN 22 云台下俯
PAN_LEFT 23 云台左转
PAN_RIGHT 24 云台右转
UP_LEFT 25 云台上仰和左转
UP_RIGHT 26 云台上仰和右转
DOWN_LEFT 27 云台下俯和左转
DOWN_RIGHT 28 云台下俯和右转
PAN_AUTO 29 云台左右自动扫描
TILT_DOWN_ZOOM_IN  58 云台下俯和焦距变大(倍率变大)
TILT_DOWN_ZOOM_OUT 59 云台下俯和焦距变小(倍率变小)
PAN_LEFT_ZOOM_IN 60 云台左转和焦距变大(倍率变大)
PAN_LEFT_ZOOM_OUT 61 云台左转和焦距变小(倍率变小)
PAN_RIGHT_ZOOM_IN 62 云台右转和焦距变大(倍率变大)
PAN_RIGHT_ZOOM_OUT 63 云台右转和焦距变小(倍率变小)
UP_LEFT_ZOOM_IN 64 云台上仰和左转和焦距变大(倍率变大)
UP_LEFT_ZOOM_OUT 65 云台上仰和左转和焦距变小(倍率变小)
UP_RIGHT_ZOOM_IN 66 云台上仰和右转和焦距变大(倍率变大)
UP_RIGHT_ZOOM_OUT 67 云台上仰和右转和焦距变小(倍率变小)
DOWN_LEFT_ZOOM_IN 68 云台下俯和左转和焦距变大(倍率变大)
DOWN_LEFT_ZOOM_OUT 69 云台下俯和左转和焦距变小(倍率变小)
DOWN_RIGHT_ZOOM_IN  70 云台下俯和右转和焦距变大(倍率变大)
DOWN_RIGHT_ZOOM_OUT 71 云台下俯和右转和焦距变小(倍率变小)
TILT_UP_ZOOM_IN 72 云台上仰和焦距变大(倍率变大)
TILT_UP_ZOOM_OUT 73 云台上仰和焦距变小(倍率变小)

**/

HikVisionCamera::HikVisionCamera(QWidget *parent) :
    QWidget(parent),reply(nullptr),
    ui(new Ui::HikVisionCamera)
{
    ui->setupUi(this);

    init_NetworkRequest();
    init_HCNetSDK();
    Login_V40();
    init();
}

HikVisionCamera::~HikVisionCamera()
{
    // 释放资源
    reply->deleteLater();

    delete ui;
    qDebug()<<"HikVisionCamera::~HikVisionCamera()";
}

void HikVisionCamera::init()
{
    // 处理认证
    QObject::connect(&manager, &QNetworkAccessManager::authenticationRequired, [&](QNetworkReply *reply, QAuthenticator *authenticator){
         // 设置Digest Authorization认证参数
         authenticator->setUser(camera["video_username"].toString());
         authenticator->setPassword(camera["video_password"].toString());
         qDebug() << "Response:处理认证"<<reply; });

    //start();
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

bool HikVisionCamera::PTZControl(int lChannel,int dwPTZCommand, int dwStop)
{
    qDebug()<<"HikVisionCamera::PTZControl";
    //目前只支持补光灯
    bool success = NET_DVR_PTZControl_Other(lUserID,lChannel , LIGHT_PWRON, dwStop);

    if(!success) qDebug()<<"云台角度失败!错误码:"+i2s(NET_DVR_GetLastError());
    return true;
}

bool HikVisionCamera::PTZPreset(int lChannel, int dwPresetIndex, int dwPTZPresetCmd)
{
    qDebug()<<"HikVisionCamera::PTZPreset";
    bool success;

//    dwPTZPresetCmd
//        [in] 操作云台预置点命令，见下表： 宏定义 宏定义值 含义
//        SET_PRESET 8 设置预置点
//        CLE_PRESET 9 清除预置点
//        GOTO_PRESET 39 转到预置点


    switch (dwPTZPresetCmd) {
    case SET_PRESET:
//        request_URL = QString("http://%1/ISAPI/PTZCtrl/channels/%2/presets/%3").arg(camera["video_ip"].toString()).arg(lChannel).arg(dwPresetIndex);
//        request_xmlData = ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
//                            "<PTZPreset >"
//                                "<id>"+i2s(lChannel)+"</id>"
//                            "</PTZPreset>").toUtf8();
//        success = api_request_ISAPI(request_URL,request_xmlData,Http_PUT);
        break;
    case CLE_PRESET:
//        request_URL = QString("http://%1/ISAPI/PTZCtrl/channels/%2/presets/%3").arg(camera["video_ip"].toString()).arg(lChannel).arg(dwPresetIndex);
//        success = api_request_ISAPI(request_URL,request_xmlData,Http_DELETE);
        break;
    default:
        dwPTZPresetCmd = GOTO_PRESET;
//        request_URL = QString("http://%1/ISAPI/PTZCtrl/channels/%2/presets/%3/goto").arg(camera["video_ip"].toString()).arg(lChannel).arg(dwPresetIndex);
//        success = api_request_ISAPI(request_URL,request_xmlData,Http_GET);
        break;
    }

    success = NET_DVR_PTZPreset_Other(lUserID,lChannel , dwPTZPresetCmd, dwPresetIndex);

    if(!success) qDebug()<<"云台角度失败!错误码:"+i2s(NET_DVR_GetLastError());
    return success;

}

QJsonObject HikVisionCamera::PTZPOS_get(int lChannel)
{
    //实际显示的PTZ值是获取到的十六进制值的十分之一，如获取的水平参数P的值是0x1750，实际显示的P值为175度；获取到的垂直参数T的值是0x0789，实际显示的T值为78.9度；获取到的变倍参数Z的值是0x1100，实际显示的Z值为110倍。
    QJsonObject pose;
    DWORD BytesReturn;
    NET_DVR_PTZPOS Hik_PTZPos;// = {0};     //海康云台PTZ参数
    bool success = NET_DVR_GetDVRConfig(lUserID, NET_DVR_GET_PTZPOS, lChannel, &Hik_PTZPos, sizeof(NET_DVR_PTZPOS), &BytesReturn);
    if(success){
        bool ok = true;
        int iHor_Angle = QString::number(Hik_PTZPos.wPanPos,16).toInt(&ok,10);//为实际的*10倍数
        int iVer_Angle = QString::number(Hik_PTZPos.wTiltPos,16).toInt(&ok,10);
        int iZoom_Pos = Hik_PTZPos.wZoomPos;
        if(iVer_Angle > 900) (iVer_Angle = iVer_Angle - 3600);//垂直方向如果是负数则需要减去为360度
        pose["wPanPos"] = iHor_Angle;
        pose["wTiltPos"] = iVer_Angle;
        pose["wZoomPos"] = iZoom_Pos;
    }
    pose["success"] = success;
    return pose;
}

bool HikVisionCamera::PTZPOS_set(int lChannel,int wAction, int wPanPos, int wTiltPos, int wZoomPos)
{
    qDebug()<<"HikVisionCamera::PTZPOS";

//    request_URL = QString("http://%1/ISAPI/PTZCtrl/channels/%2/absoluteEx").arg(camera["video_ip"].toString()).arg(lChannel);
//    request_xmlData = ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
//                       "<PTZAbsoluteEx >"
//                       "<elevation>"+i2s(wTiltPos)+"</elevation>" //垂直
//                       "azimuth>"+i2s(wPanPos)+"</azimuth>" //水平方向的角度
//                       "<absoluteZoom>"+i2s(wZoomPos)+"</absoluteZoom>" //变倍的倍率，值是真实倍率乘以10倍。范围0,1000-->0
//                        "</PTZAbsoluteEx>").toUtf8();

//    return api_request_ISAPI(request_URL,request_xmlData,Http_PUT);
//    lPTZ_Command = -1;

    QString panPosHexStr = QString("0x%1").arg(QString::number(wPanPos*10));
//    DC_ASSERT(DC_FAILURE,MPP_ASSERT,"云台角度wPanPos:"+panPosHexStr);
    wPanPos = panPosHexStr.toInt(0,16);
    if(wTiltPos < 0)
    {
        wTiltPos = wTiltPos + 360;
    }
    QString tiltPosHexStr = QString("0x%1").arg(QString::number(wTiltPos*10));
//    DC_ASSERT(DC_FAILURE,MPP_ASSERT,"云台角度tiltPosHexStr:"+tiltPosHexStr);
    wTiltPos = tiltPosHexStr.toInt(0,16);
    //海康云台PTZ参数
    NET_DVR_PTZPOS Hik_PTZPos = {0};
    Hik_PTZPos.wAction = wAction;
    Hik_PTZPos.wPanPos = wPanPos;
    Hik_PTZPos.wTiltPos = wTiltPos;
    Hik_PTZPos.wZoomPos = wZoomPos;
    bool bret = NET_DVR_SetDVRConfig(lUserID, NET_DVR_SET_PTZPOS, lChannel, &Hik_PTZPos, sizeof(NET_DVR_PTZPOS));
    if(!bret)
    {
        qDebug()<<"云台角度失败!错误码:"+QString::number(NET_DVR_GetLastError());
        return false;
    }
    return true;


}

bool HikVisionCamera::CaptureJPEGPicture(int lChannel, QString fileName)
{
    qDebug()<<"HikVisionCamera::CaptureJPEGPicture";
    //QString fileName = getAbsoluteFilePath(sPicFileName);
    NET_DVR_JPEGPARA jpeginfo;
    jpeginfo.wPicQuality = 0;   /* 图片质量系数 0-最好 1-较好 2-一般 */
    jpeginfo.wPicSize = 9;      /* 9-HD1080 */

    bool bret = NET_DVR_CaptureJPEGPicture(lUserID, lChannel, &jpeginfo, fileName.toLocal8Bit().data());
    if (!bret)
    {
        //弹窗提示抓拍失败
        qDebug()<<("海康设备可见光抓拍失败，错误码:" + QString::number(NET_DVR_GetLastError()));
        return false;
    }
    qDebug()<<"可见光抓拍成功!"<<fileName;

    return true;
}

bool HikVisionCamera::Realtime_Thermometry(QString fileName)
{
    qDebug()<<"HikVisionCamera::Realtime_Thermometry";
    CaptureJPEGPicture(2,fileName);
    return true;
}

bool HikVisionCamera::api_request_ISAPI(const QString& url,QByteArray requestData,HttpMethod methods)
{
    QUrl api_path = QUrl(url);
    QNetworkRequest request(api_path);
    switch (methods) {
    case Http_GET:
        reply = manager.get(request);
        break;
    case Http_PUT:
        reply = manager.put(request, requestData);
        break;
    case Http_POST:
        reply = manager.post(request, requestData);
        break;
    case Http_DELETE:
        reply = manager.deleteResource(request);
        break;
    default:
        break;
    }
    QObject::connect(reply, &QNetworkReply::finished, &loop_NetworkReply, &QEventLoop::quit);
    loop_NetworkReply.exec();
    // 检查请求是否成功
    if (reply->error() != QNetworkReply::NoError){
        // 处理请求错误
        qDebug() << "Error:" << reply->errorString();
        return false;
    }
    // 读取响应数据
    response = reply->readAll();
    qDebug() << "Response:" << response;
    return true;
}

void HikVisionCamera::init_NetworkRequest()
{
    // 处理认证
    connect(&request.manager,&QNetworkAccessManager::authenticationRequired, [&](QNetworkReply *reply, QAuthenticator *authenticator){
         // 设置Digest Authorization认证参数
         authenticator->setUser(camera["video_username"].toString());
         authenticator->setPassword(camera["video_password"].toString());
         qDebug() << "Response:处理认证"<<reply; });
}

void HikVisionCamera::init_HCNetSDK()
{
    static bool isInitialized = false;

    if (!isInitialized) {
        // 执行初始化操作
        qDebug() << "海康设备Initializing";
        // 初始化
        NET_DVR_Init();
        //设置连接时间与重连时间
        NET_DVR_SetConnectTime();//(DWORD dwWaitTime = 3000, DWORD dwTryTimes = 3);
        NET_DVR_SetReconnect();//DWORD dwInterval = 30000, BOOL bEnableRecon = TRUE);
        //---------------------------------------
        isInitialized = true;
    }
    // 函数的其他逻辑
}

void HikVisionCamera::Login_V40()
{
    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "192.168.1.38"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "dacang80"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0)
    {
        qWarning()<<QString("海康设备Login failed, error code: %1\n").arg(NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return;
    }
    qDebug()<<QString("海康设备Login  code: %1\n").arg(lUserID);
}

void HikVisionCamera::start()
{
    init_HCNetSDK();
    //---------------------------------------
    //登录参数，包括设备地址、登录用户、密码等
    NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
    struLoginInfo.bUseAsynLogin = 0; //同步登录方式
    strcpy(struLoginInfo.sDeviceAddress, "192.168.1.38"); //设备IP地址
    struLoginInfo.wPort = 8000; //设备服务端口
    strcpy(struLoginInfo.sUserName, "admin"); //设备登录用户名
    strcpy(struLoginInfo.sPassword, "dacang80"); //设备登录密码

    //设备信息, 输出参数
    NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

    lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
    if (lUserID < 0)
    {
        qWarning()<<QString("海康设备Login failed, error code: %1\n").arg(NET_DVR_GetLastError());
        NET_DVR_Cleanup();
        return;
    }
    qDebug()<<QString("海康设备Login  code: %1\n").arg(lUserID);

}


