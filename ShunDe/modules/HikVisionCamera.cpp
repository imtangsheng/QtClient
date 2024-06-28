#include "HikVisionCamera.h"
#include "ui_HikVisionCamera.h"
#include <QDateTime>
#include <QDir>
#include <QString>
#include <QAuthenticator>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include "AppUtil.h"
#include "modules/sqlite.h"

static QMap<QString,HikVisionCamera*> gCamera;
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
//*报警布防*//
//时间解析宏定义
#define GET_YEAR(_time_)      (((_time_)>>26) + 2000)
#define GET_MONTH(_time_)     (((_time_)>>22) & 15)
#define GET_DAY(_time_)       (((_time_)>>17) & 31)
#define GET_HOUR(_time_)      (((_time_)>>12) & 31)
#define GET_MINUTE(_time_)    (((_time_)>>6)  & 63)
#define GET_SECOND(_time_)    (((_time_)>>0)  & 63)

//lCommand
//    [out] 上传的消息类型，不同的报警信息对应不同的类型，通过类型区分是什么报警信息，详见“Remarks”中列表
//pAlarmer
//        [out] 报警设备信息，包括设备序列号、IP地址、登录IUserID句柄等
//pAlarmInfo
//                [out] 报警信息，通过lCommand值判断pAlarmer对应的结构体，详见“Remarks”中列表
//dwBufLen
//        [out] 报警信息缓存大小
//pUser
//                [out] 用户数据
void CALLBACK cbMessageCallback(LONG lCommand, NET_DVR_ALARMER *pAlarmer, char *pAlarmInfo, DWORD dwBufLen, void* pUser)
{
    qDebug()<<"回调函数信息信息lCommand: dwBufLen:"<<lCommand<<dwBufLen;
    qDebug()<<"回调函数信息信息*pAlarmer:"<<QString::fromUtf8(pAlarmer->sDeviceIP);
    QString sDeviceIP = QString::fromLocal8Bit(pAlarmer->sDeviceIP);
    qDebug()<<"回调函数信息信息*pAlarmInfo:"<<QString::fromUtf8(pAlarmInfo);
    qDebug()<<"回调函数信息信息pUser:"<<pUser;

    switch (lCommand)
    {
    case COMM_THERMOMETRY_ALARM: //温度预警或者温度报警
    {
        NET_DVR_THERMOMETRY_ALARM struThermometryAlarm = {0};
        memcpy(&struThermometryAlarm, pAlarmInfo, sizeof(NET_DVR_THERMOMETRY_ALARM));
        if (0 == struThermometryAlarm.byRuleCalibType)
        {
            qWarning("点测温: Channel:%d, RuleID:%d, ThermometryUnit:%d, PresetNo:%d, RuleTemperature:%.1f, CurrTemperature:%.1f, PTZ Info[Pan:%f, Tilt:%f, Zoom:%f], AlarmLevel:%d, AlarmType:%d, AlarmRule:%d, RuleCalibType:%d,  Point[x:%f, y:%f], PicLen:%d, ThermalPicLen:%d, ThermalInfoLen:%d", struThermometryAlarm.dwChannel, struThermometryAlarm.byRuleID, struThermometryAlarm.byThermometryUnit, struThermometryAlarm.wPresetNo, struThermometryAlarm.fRuleTemperature, struThermometryAlarm.fCurrTemperature, struThermometryAlarm.struPtzInfo.fPan, struThermometryAlarm.struPtzInfo.fTilt, struThermometryAlarm.struPtzInfo.fZoom, struThermometryAlarm.byAlarmLevel, struThermometryAlarm.byAlarmType, struThermometryAlarm.byAlarmRule, struThermometryAlarm.byRuleCalibType, struThermometryAlarm.struPoint.fX, struThermometryAlarm.struPoint.fY, struThermometryAlarm.dwPicLen, struThermometryAlarm.dwThermalPicLen, struThermometryAlarm.dwThermalInfoLen);
        }
        else if (1 == struThermometryAlarm.byRuleCalibType || 2 == struThermometryAlarm.byRuleCalibType)
        {
            int iPointNum = struThermometryAlarm.struRegion.dwPointNum;
            for (int i = 0; i < iPointNum; i++)
            {
                float fX = struThermometryAlarm.struRegion.struPos[i].fX;
                float fY = struThermometryAlarm.struRegion.struPos[i].fY;
                qWarning("测温区域坐标点:X%d:%f,Y%d:%f;", iPointNum + 1, fX, iPointNum + 1, fY);
            }
            qWarning("线测温或者框测温: Channel:%d, RuleID:%d,HighestPoint[x:%f, y:%f],ThermometryUnit:%d, PresetNo:%d, RuleTemperature:%.1f, CurrTemperature:%.1f, PTZ Info[Pan:%f, Tilt:%f, Zoom:%f], AlarmLevel:%d, AlarmType:%d, AlarmRule:%d, RuleCalibType:%d, PicLen:%d, ThermalPicLen:%d, ThermalInfoLen:%d", struThermometryAlarm.dwChannel, struThermometryAlarm.byRuleID,
                struThermometryAlarm.struHighestPoint.fX, struThermometryAlarm.struHighestPoint.fY,
                struThermometryAlarm.byThermometryUnit, struThermometryAlarm.wPresetNo, struThermometryAlarm.fRuleTemperature, struThermometryAlarm.fCurrTemperature,
                struThermometryAlarm.struPtzInfo.fPan, struThermometryAlarm.struPtzInfo.fTilt, struThermometryAlarm.struPtzInfo.fZoom, struThermometryAlarm.byAlarmLevel, struThermometryAlarm.byAlarmType, struThermometryAlarm.byAlarmRule, struThermometryAlarm.byRuleCalibType, struThermometryAlarm.dwPicLen, struThermometryAlarm.dwThermalPicLen, struThermometryAlarm.dwThermalInfoLen);
        }
        break;}

    case COMM_THERMOMETRY_DIFF_ALARM: //温差报警
    {
        NET_DVR_THERMOMETRY_DIFF_ALARM struThermometryDiffAlarm = {0};
        memcpy(&struThermometryDiffAlarm, pAlarmInfo, sizeof(NET_DVR_THERMOMETRY_DIFF_ALARM));
        if (0 == struThermometryDiffAlarm.byRuleCalibType)
        {
            qWarning("温差报警: Channel:%d, AlarmID1:%d, AlarmID2:%d, PresetNo:%d, RuleTemperatureDiff:%.1f, CurTemperatureDiff:%.1f, AlarmLevel:%d, AlarmType:%d, AlarmRule:%d, RuleCalibType:%d, Point1[x:%f, y:%f], point2[x:%f, y:%f], PTZ Info[Pan:%f, Tilt:%f, Zoom:%f], PicLen:%d, ThermalPicLen:%d, ThermalInfoLen:%d, ThermometryUnit:%d", struThermometryDiffAlarm.dwChannel, struThermometryDiffAlarm.byAlarmID1, struThermometryDiffAlarm.byAlarmID2, struThermometryDiffAlarm.wPresetNo, struThermometryDiffAlarm.fRuleTemperatureDiff, struThermometryDiffAlarm.fCurTemperatureDiff, struThermometryDiffAlarm.byAlarmLevel, struThermometryDiffAlarm.byAlarmType, struThermometryDiffAlarm.byAlarmRule, struThermometryDiffAlarm.byRuleCalibType, struThermometryDiffAlarm.struPoint[0].fX, struThermometryDiffAlarm.struPoint[0].fY, struThermometryDiffAlarm.struPoint[1].fX, struThermometryDiffAlarm.struPoint[1].fY, struThermometryDiffAlarm.struPtzInfo.fPan, struThermometryDiffAlarm.struPtzInfo.fTilt, struThermometryDiffAlarm.struPtzInfo.fZoom, struThermometryDiffAlarm.dwPicLen, struThermometryDiffAlarm.dwThermalPicLen, struThermometryDiffAlarm.dwThermalInfoLen, struThermometryDiffAlarm.byThermometryUnit);
        }
        else if (1 == struThermometryDiffAlarm.byRuleCalibType || 2 == struThermometryDiffAlarm.byRuleCalibType)
        {
            int i = 0;
            int iPointNum = struThermometryDiffAlarm.struRegion[0].dwPointNum;
            for (i = 0; i < iPointNum; i++)
            {
                float fX = struThermometryDiffAlarm.struRegion[0].struPos[i].fX;
                float fY = struThermometryDiffAlarm.struRegion[0].struPos[i].fY;
                qWarning("测温区域1坐标点: X%d:%f,Y%d:%f;", iPointNum + 1, fX, iPointNum + 1, fY);
            }
            iPointNum = struThermometryDiffAlarm.struRegion[1].dwPointNum;
            for (i = 0; i < iPointNum; i++)
            {
                float fX = struThermometryDiffAlarm.struRegion[1].struPos[i].fX;
                float fY = struThermometryDiffAlarm.struRegion[1].struPos[i].fY;
                qWarning("测温区域2坐标点: X%d:%f,Y%d:%f;", iPointNum + 1, fX, iPointNum + 1, fY);
            }

            qWarning("温差报警: Channel:%d, AlarmID1:%d, AlarmID2:%d, PresetNo:%d, RuleTemperatureDiff:%.1f, CurTemperatureDiff:%.1f, AlarmLevel:%d, AlarmType:%d, AlarmRule:%d, RuleCalibType:%d,  PTZ Info[Pan:%f, Tilt:%f, Zoom:%f], PicLen:%d, ThermalPicLen:%d, ThermalInfoLen:%d, ThermometryUnit:%d", struThermometryDiffAlarm.dwChannel, struThermometryDiffAlarm.byAlarmID1, struThermometryDiffAlarm.byAlarmID2, struThermometryDiffAlarm.wPresetNo, struThermometryDiffAlarm.fRuleTemperatureDiff, struThermometryDiffAlarm.fCurTemperatureDiff, struThermometryDiffAlarm.byAlarmLevel, struThermometryDiffAlarm.byAlarmType, struThermometryDiffAlarm.byAlarmRule, struThermometryDiffAlarm.byRuleCalibType, struThermometryDiffAlarm.struPtzInfo.fPan, struThermometryDiffAlarm.struPtzInfo.fTilt, struThermometryDiffAlarm.struPtzInfo.fZoom, struThermometryDiffAlarm.dwPicLen, struThermometryDiffAlarm.dwThermalPicLen, struThermometryDiffAlarm.dwThermalInfoLen, struThermometryDiffAlarm.byThermometryUnit);
        }
        break;}

    case COMM_ALARM_RULE: //异常行为识别报警信息
    {
        qDebug()<<COMM_ALARM_RULE<<"异常行为识别报警信息:"<<QString::fromUtf8(pAlarmInfo);
        NET_VCA_RULE_ALARM struVcaAlarm = { 0 };
        memcpy(&struVcaAlarm, pAlarmInfo, sizeof(NET_VCA_RULE_ALARM));

        QString alarm_ip = QString::fromLocal8Bit(struVcaAlarm.struDevInfo.struDevIP.sIpV4);
        QDateTime struAbsTime;
        struAbsTime.setDate(QDate(GET_YEAR(struVcaAlarm.dwAbsTime), GET_MONTH(struVcaAlarm.dwAbsTime), GET_DAY(struVcaAlarm.dwAbsTime)));
        struAbsTime.setTime(QTime(GET_HOUR(struVcaAlarm.dwAbsTime), GET_MINUTE(struVcaAlarm.dwAbsTime), GET_SECOND(struVcaAlarm.dwAbsTime)));

        //保存抓拍场景图片
        if (struVcaAlarm.dwPicDataLen > 0 && struVcaAlarm.pImage != NULL)
        {
            QString cFilename = QString("VcaAlarmPic[%1][%2].jpg")
                .arg(QString::fromLocal8Bit(struVcaAlarm.struDevInfo.struDevIP.sIpV4))
                .arg(struAbsTime.toString("yyyyMMddhhmmss"));

            QFile file(gPathCameraPicture+cFilename);
            if (file.open(QIODevice::WriteOnly))
            {
                file.write(reinterpret_cast<const char*>(struVcaAlarm.pImage), struVcaAlarm.dwPicDataLen);
                file.close();
            }
        }

        WORD wEventType = struVcaAlarm.struRuleInfo.wEventTypeEx;

        qDebug() << "\n\n"
                 << QString("异常行为识别报警[0x%1]: Abs[%2] Dev[ip:%3,port:%4,ivmsChan:%5] Smart[%6] EventTypeEx[%7]")
                        .arg(QString::number(lCommand, 16))
                        .arg(struAbsTime.toString("yyyy-MM-dd hh:mm:ss"))
                        .arg(QString::fromLocal8Bit(struVcaAlarm.struDevInfo.struDevIP.sIpV4))
                        .arg(struVcaAlarm.struDevInfo.wPort)
                        .arg(struVcaAlarm.struDevInfo.byIvmsChannel)
                        .arg(struVcaAlarm.bySmart)
                        .arg(wEventType);

        NET_VCA_TARGET_INFO tmpTargetInfo;
        memcpy(&tmpTargetInfo, &struVcaAlarm.struTargetInfo, sizeof(NET_VCA_TARGET_INFO));
        qDebug() << QString("目标信息:ID[%1]RECT[%2][%3][%4][%5]")
                        .arg(tmpTargetInfo.dwID)
                        .arg(tmpTargetInfo.struRect.fX)
                        .arg(tmpTargetInfo.struRect.fY)
                        .arg(tmpTargetInfo.struRect.fWidth)
                        .arg(tmpTargetInfo.struRect.fHeight);

        switch (wEventType)
        {
        case ENUM_VCA_EVENT_INTRUSION: //区域入侵报警
        {
            //1-120秒，建议5秒，判断是有效报警的时间  在ATM系统中触发文件阈值为 1-1000秒
            qDebug() << QString("区域入侵报警: 触发时间阈值：%1, 灵敏度参数：%2")
                            .arg(struVcaAlarm.struRuleInfo.uEventParam.struIntrusion.wDuration)
                            .arg(struVcaAlarm.struRuleInfo.uEventParam.struIntrusion.bySensitivity);

            qDebug() << "规则区域: ";
            // 保存规则区域坐标
            QString ruleRegion;
            NET_VCA_POLYGON tempPolygon;
            memcpy(&tempPolygon, &struVcaAlarm.struRuleInfo.uEventParam.struIntrusion.struRegion, sizeof(NET_VCA_POLYGON));
            for (int i = 0; i < (int)tempPolygon.dwPointNum; i++)
            {
                qDebug() << QString("[%1, %2]").arg(tempPolygon.struPos[i].fX).arg(tempPolygon.struPos[i].fY);
                ruleRegion += QString("[%1, %2]").arg(tempPolygon.struPos[i].fX).arg(tempPolygon.struPos[i].fY);
                if (i < (int)tempPolygon.dwPointNum - 1)
                    ruleRegion += ", ";
            }

            EventCenterData eventHik;
            eventHik.time = struAbsTime;
            eventHik.source = QString::fromLocal8Bit(struVcaAlarm.struDevInfo.struDevIP.sIpV4);
            eventHik.type = "区域入侵";
            eventHik.level = EventLevel_Error;
            eventHik.details = QString("区域入侵报警: 触发时间阈值：%1, 灵敏度参数：%2")
                                  .arg(struVcaAlarm.struRuleInfo.uEventParam.struIntrusion.wDuration)
                                  .arg(struVcaAlarm.struRuleInfo.uEventParam.struIntrusion.bySensitivity);
            eventHik.details += QString("\n规则区域: %1").arg(ruleRegion);

            eventHik.status = "";
            gSql->add_EventCenter(eventHik);
            if(gCamera.contains(alarm_ip)){
                QString filename_video = struAbsTime.toString("yyyy_MM_dd_hhmmss") + ".mp4";
                // 检查文件夹是否存在
                QDir dir(gPathCameraVideo);
                if (!dir.exists()) {
                    // 文件夹不存在,创建它
                    if (!dir.mkpath(dir.absolutePath())) {
                        qWarning() << "gPathCameraVideo:Failed to create directory:" << gPathCameraVideo;
                        return;
                    }
                }
                gCamera[alarm_ip]->eventVideoSava(dir.absoluteFilePath(filename_video));
            }

            break;
        }
        default:
        {
            qDebug() << QString("其他报警，报警信息类型: 0x%1").arg(QString::number(lCommand, 16));
            break;
        }
        }
        break;
    }
    case COMM_GISINFO_UPLOAD:  //云台水平角度
    {
        NET_DVR_GIS_UPLOADINFO struUploadinfo = {0};
        memcpy(&struUploadinfo, pAlarmInfo, sizeof(NET_DVR_GIS_UPLOADINFO));

        QString alarm_IP = QString(struUploadinfo.struDevInfo.struDevIP.sIpV4);
        qDebug()<<"机器人IP地址："<<alarm_IP<<struUploadinfo.fAzimuth;
        qDebug()<<struUploadinfo.struPtzPos.fPanPos;
        break;}
    case COMM_VCA_ALARM:    //区域入侵结束 COMM_VCA_ALARM 0x4993 智能检测报警(Json或者XML数据结构)
    {
        //0x4993 智能检测报警有效，0-表示所有事件,1-混合目标检测（mixedTargetDetection）,2-雷视目标检测（radarVideoDetection）
        QString message = QString::fromUtf8(pAlarmInfo);
        qDebug()<<COMM_VCA_ALARM<<"区域入侵结束:"<<message;
        if(gCamera.contains(sDeviceIP)){
            gCamera[sDeviceIP]->eventHeartBeat();
        }
        break;}
    case COMM_FIREDETECTION_ALARM:      //火点识别
    {
        QString message = QString::fromUtf8(pAlarmInfo);
        qDebug()<<COMM_FIREDETECTION_ALARM<<"火点识别"<<message;
        break;}
    default:
    {
        qDebug() << QString("其他报警，报警信息类型: 0x%1").arg(QString::number(lCommand, 16))<<QString::fromUtf8(pAlarmInfo);
        break;}
    }

    return;
}

void CALLBACK GetThermInfoCallback(DWORD dwType, void* lpBuffer, DWORD dwBufLen, void* pUserData)
{
    if (dwType == NET_SDK_CALLBACK_TYPE_DATA)
    {
        NET_DVR_THERMOMETRY_UPLOAD struThermometry = { 0 };
        memcpy(&struThermometry, lpBuffer, sizeof(NET_DVR_THERMOMETRY_UPLOAD));

        NET_DVR_TIME struAbsTime = { 0 };
        struAbsTime.dwYear = GET_YEAR(struThermometry.dwAbsTime);
        struAbsTime.dwMonth = GET_MONTH(struThermometry.dwAbsTime);
        struAbsTime.dwDay = GET_DAY(struThermometry.dwAbsTime);
        struAbsTime.dwHour = GET_HOUR(struThermometry.dwAbsTime);
        struAbsTime.dwMinute = GET_MINUTE(struThermometry.dwAbsTime);
        struAbsTime.dwSecond = GET_SECOND(struThermometry.dwAbsTime);
        //规则标定类型 0-点，1-框，2-线
        qDebug()<< QString("实时测温结果:规则[] 预置点号[] 类型[] 测温单位[] 数据状态类型")
                <<struThermometry.byRuleID<<struThermometry.wPresetNo<< struThermometry.byRuleCalibType<< struThermometry.byThermometryUnit
                <<struThermometry.byDataType
                <<struAbsTime.dwYear << struAbsTime.dwMonth<< struAbsTime.dwDay<<":"<<struAbsTime.dwHour<<struAbsTime.dwMinute<<struAbsTime.dwSecond;

        //点测温
        if (struThermometry.byRuleCalibType == 0)
        {
            qDebug()<<QString("点测温信息:%1").arg(struThermometry.struPointThermCfg.fTemperature);
        }

        //框/线测温
        if ((struThermometry.byRuleCalibType == 1) || (struThermometry.byRuleCalibType == 2))
        {
            qDebug()<<("框/线测温信息:最高温[]最低温[]平均温[]温差[]")
                <<struThermometry.struLinePolygonThermCfg.fMaxTemperature<< struThermometry.struLinePolygonThermCfg.fMinTemperature
                <<struThermometry.struLinePolygonThermCfg.fAverageTemperature<<struThermometry.struLinePolygonThermCfg.fTemperatureDiff;
        }
    }
    else if (dwType == NET_SDK_CALLBACK_TYPE_STATUS)
    {
        DWORD dwStatus = *(DWORD*)lpBuffer;
        if (dwStatus == NET_SDK_CALLBACK_STATUS_SUCCESS)
        {
            qDebug()<<("dwStatus:NET_SDK_CALLBACK_STATUS_SUCCESS\n");
        }
        else if (dwStatus == NET_SDK_CALLBACK_STATUS_FAILED)
        {
            DWORD dwErrCode = *(DWORD*)((char *)lpBuffer + 4);
            qDebug()<<("NET_DVR_GET_MANUALTHERM_INFO failed, Error code")<< dwErrCode;
        }
    }
}


HikVisionCamera::HikVisionCamera(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HikVisionCamera)
{
    ui->setupUi(this);

    init_NetworkRequest();
    init();
    //QObject::connect(this, &HikVisionCamera::init, &GetThermInfoCallback);
}

HikVisionCamera::~HikVisionCamera()
{
    qDebug()<<"HikVisionCamera::~HikVisionCamera() 0 ";
    delete ui;
    qDebug()<<"HikVisionCamera::~HikVisionCamera()";
    cleanup_HCNetSDK();
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
    QMetaObject::invokeMethod(this,"load",Qt::QueuedConnection);
    //QTimer::singleShot(10,this,SLOT(load()));
    //QTimer::singleShot(10,[&](){load();});
}

void HikVisionCamera::start()
{


}

void HikVisionCamera::quit()
{

    qDebug()<<"HikVisionCamera::quit()";
}

void HikVisionCamera::set_camera_config(QJsonObject obj)
{
    loginInfo.sDeviceAddress = obj["sDeviceAddress"].toString("192.168.1.64");
    loginInfo.wPort = obj["wPort"].toInt(8000);
    loginInfo.sUserName = obj["sUserName"].toString("admin");
    loginInfo.sPassword = obj["sPassword"].toString("dacang80");
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
    qDebug()<<"HikVisionCamera::PTZControl"<<lChannel<<dwPTZCommand<<dwStop;
    //目前只支持补光灯
    bool success = NET_DVR_PTZControl_Other(lUserID,lChannel , dwPTZCommand, dwStop);

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
    QNetworkReply *reply;
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
        // 释放资源
        reply->deleteLater();
        return false;
    }
    // 读取响应数据
    response = reply->readAll();
    qDebug() << "Response:" << response;
    // 释放资源
    reply->deleteLater();
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
        //qDebug() << "海康设备Initializing";
        // 初始化
        NET_DVR_Init();
        //设置连接时间与重连时间 登录时间 同步阻塞时间dwWaitTime
        NET_DVR_SetConnectTime(1000,3);//(DWORD dwWaitTime = 3000, DWORD dwTryTimes = 3);
        NET_DVR_SetReconnect(5000);//DWORD dwInterval = 30000, BOOL bEnableRecon = TRUE);
        //---------------------------------------
        isInitialized = true;
    }
    // 函数的其他逻辑
}
#include <QtConcurrent/QtConcurrent>

bool HikVisionCamera::Login_V40()
{
    qDebug()<<QDateTime::currentDateTime()<<QString("海康设备正常尝试登录");
    static bool isTryLogin = false;
    static int msec = 5000;
    if(isTryLogin){
        msec = 5000;
        qDebug()<<QString("海康设备正常尝试登录，请勿重复登录，参考错误码: %1\n").arg(NET_DVR_GetLastError());
        return false;
    }

    static QTimer loginTimer;

    connect(&loginTimer,&QTimer::timeout,this,[=](){
        //QFuture<void> future = QtConcurrent::run([this]() {
        //登录参数，包括设备地址、登录用户、密码等
        NET_DVR_USER_LOGIN_INFO struLoginInfo = {0};
        //对于同步登录，接口返回-1表示登录失败，其他值表示返回的用户ID值
        struLoginInfo.bUseAsynLogin = 0; //是否异步登录：0- 否，1- 是  //此处会阻塞主线程，ui界面卡顿NET_DVR_Login_V40
        strcpy(struLoginInfo.sDeviceAddress, loginInfo.sDeviceAddress.toStdString().c_str()); //设备IP地址
        struLoginInfo.wPort = loginInfo.wPort; //设备服务端口
        strcpy(struLoginInfo.sUserName, loginInfo.sUserName.toStdString().c_str()); //设备登录用户名
        strcpy(struLoginInfo.sPassword, loginInfo.sPassword.toStdString().c_str()); //设备登录密码

        //设备信息, 输出参数
        NET_DVR_DEVICEINFO_V40 struDeviceInfoV40 = {0};

        lUserID = NET_DVR_Login_V40(&struLoginInfo, &struDeviceInfoV40);
        //});
        //future.waitForFinished();
        if (lUserID == -1)
        {
            qWarning()<<QDateTime::currentDateTime()<<QString("海康设备Login failed, error code: %1\n").arg(NET_DVR_GetLastError())<<"等待下次登录（秒）"<<msec/1000;
            msec = msec + 5000;
            loginTimer.setInterval(msec);
            loginTimer.start(msec);
        }else{
            qDebug()<<QString("海康设备Login  code: %1\n").arg(lUserID);
            SetupAlarmChan_V41();
            gCamera[loginInfo.sDeviceAddress] = this;
            loginTimer.stop();
            msec = 0;
            isTryLogin = false;
        }
    });

    loginTimer.setSingleShot(true);
    loginTimer.start(msec);

    isTryLogin = true;
    return true;
}

bool HikVisionCamera::SetupAlarmChan_V41()
{
    //设置报警回调函数
    /*注：多台设备对接时也只需要调用一次设置一个回调函数，不支持不同设备的事件在不同的回调函数里面返回*/
    NET_DVR_SetDVRMessageCallBack_V50(0, cbMessageCallback, NULL);

    //启用布防
    NET_DVR_SETUPALARM_PARAM struAlarmParam = { 0 };
    struAlarmParam.dwSize = sizeof(struAlarmParam);
    //其他报警布防参数不需要设置，不支持

    lHandle = NET_DVR_SetupAlarmChan_V41(lUserID, &struAlarmParam);
    if (lHandle < 0)
    {
        qDebug()<<("NET_DVR_SetupAlarmChan_V41 error, %d")<<NET_DVR_GetLastError();
        NET_DVR_Logout(lUserID);
        NET_DVR_Cleanup();
        return false;
    }
    //事件信息在回调函数里面获取
    return true;
}

bool HikVisionCamera::StartRemoteConfig()
{
    DWORD dwChannel = 2;  //热成像通道

    //启动实时温度检测
    NET_DVR_REALTIME_THERMOMETRY_COND struThermCond = { 0 };
    struThermCond.dwSize = sizeof(struThermCond);
    struThermCond.byRuleID = 1;       //规则ID，0代表获取全部规则，具体规则ID从1开始
    struThermCond.dwChan = dwChannel; //从1开始，0xffffffff代表获取全部通道

    LONG lHandle = NET_DVR_StartRemoteConfig(lUserID, NET_DVR_GET_REALTIME_THERMOMETRY, &struThermCond, sizeof(struThermCond), GetThermInfoCallback, NULL);
    if (lHandle < 0)
    {
        qDebug()<<"NET_DVR_GET_REALTIME_THERMOMETRY failed, error code: %d\n"<<NET_DVR_GetLastError();
    }
    else
    {
        qDebug()<<("NET_DVR_GET_REALTIME_THERMOMETRY is successful!");
    }

    //关闭长连接配置接口所创建的句柄，释放资源
    if (!NET_DVR_StopRemoteConfig(lHandle))
    {
        qDebug()<<("NET_DVR_StopRemoteConfig failed, error code: %d\n")<<NET_DVR_GetLastError();
    }

    return true;
}

void HikVisionCamera::SaveRealData()
{
    //---------------------------------------
    //启动预览并设置回调数据流
    LONG lRealPlayHandle;
    //HWND hWnd = GetConsoleWindowAPI();     //获取窗口句柄
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.hPlayWnd = NULL;//hWnd;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo.lChannel     = 1;       //预览通道号
    struPlayInfo.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    struPlayInfo.dwLinkMode   = 0;       //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo.bBlocked     = 1;       //0- 非阻塞取流，1- 阻塞取流

    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, NULL, NULL);
    qDebug()<<"NET_DVR_RealPlay_V40"<<lUserID<<lRealPlayHandle;
    if (lRealPlayHandle < 0)
    {
        qWarning()<<QString("预览失败，错误码, error code: %1\n").arg(NET_DVR_GetLastError());
        return;
    }
    QString sFileName= "test.mp4";

    bool bValues = NET_DVR_SaveRealData(lRealPlayHandle, sFileName.toLocal8Bit().data());
    if(bValues){
        Sleep(10000);
        if(!NET_DVR_StopSaveRealData(lRealPlayHandle)){
            qWarning()<<QString("关闭存盘失败，错误码, error code: %1\n").arg(NET_DVR_GetLastError());
        }
    }else{
        qWarning()<<QString("开启存盘失败，错误码, error code: %1\n").arg(NET_DVR_GetLastError());
    }

    //---------------------------------------
    //关闭预览
    NET_DVR_StopRealPlay(lRealPlayHandle);

}

void HikVisionCamera::cleanup_HCNetSDK()
{
    //撤销布防上传通道
    qDebug()<<"HikVisionCamera::cleanup_HCNetSDK()";
    if (lHandle != -1){
        if(!NET_DVR_CloseAlarmChan_V30(lHandle))
        qDebug()<<QString("NET_DVR_CloseAlarmChan_V30 error, %1").arg(NET_DVR_GetLastError());
    }
    if (lUserID != -1){
        //注销用户
        NET_DVR_Logout(lUserID);
        //释放SDK资源
        NET_DVR_Cleanup();
    }

}

void HikVisionCamera::updataCameraConfig()
{
    bool isChangeLogin = false;
    if(loginInfo.sDeviceAddress != camera["video_ip"].toString()){
        loginInfo.sDeviceAddress = camera["video_ip"].toString();
        isChangeLogin = true;
    }
    if(loginInfo.sUserName != camera["video_username"].toString()){
        loginInfo.sUserName = camera["video_username"].toString();
        isChangeLogin = true;
    }
    if(loginInfo.sPassword != camera["video_password"].toString()){
        loginInfo.sPassword = camera["video_password"].toString();
        isChangeLogin = true;
    }
    if(isChangeLogin){
        Login_V40();
    }

}

void HikVisionCamera::load()
{
    //qDebug() << "HikVisionCamera::load() 延迟加载";
    init_HCNetSDK();
    Login_V40();
}

void HikVisionCamera::eventVideoSava(const QString &filepath)
{
    qDebug() << "HikVisionCamera::eventVideoSava(const QString &)"<<filepath;
    QString sFileName= filepath;
    //---------------------------------------
    //启动预览并设置回调数据流
    LONG lRealPlayHandle;
    //HWND hWnd = GetConsoleWindowAPI();     //获取窗口句柄
    NET_DVR_PREVIEWINFO struPlayInfo = {0};
    struPlayInfo.hPlayWnd = NULL;//hWnd;         //需要SDK解码时句柄设为有效值，仅取流不解码时可设为空
    struPlayInfo.lChannel     = 1;       //预览通道号
    struPlayInfo.dwStreamType = 0;       //0-主码流，1-子码流，2-码流3，3-码流4，以此类推
    struPlayInfo.dwLinkMode   = 0;       //0- TCP方式，1- UDP方式，2- 多播方式，3- RTP方式，4-RTP/RTSP，5-RSTP/HTTP
    struPlayInfo.bBlocked     = 1;       //0- 非阻塞取流，1- 阻塞取流

    lRealPlayHandle = NET_DVR_RealPlay_V40(lUserID, &struPlayInfo, NULL, NULL);
    qDebug()<<"NET_DVR_RealPlay_V40"<<lUserID<<lRealPlayHandle;
    if (lRealPlayHandle < 0)
    {
        qWarning()<<QString("预览失败，错误码, error code: %1\n").arg(NET_DVR_GetLastError());
        return;
    }
    bool bValues = NET_DVR_SaveRealData(lRealPlayHandle, sFileName.toLocal8Bit().data());
    if(bValues){
        Sleep(1000);
        m_loopVideoSava.exec();//等待关闭预览
        if(!NET_DVR_StopSaveRealData(lRealPlayHandle)){
            qWarning()<<QString("关闭预览存盘失败，错误码, error code: %1\n").arg(NET_DVR_GetLastError());
        }
    }else{
        qWarning()<<QString("开启预览存盘失败，错误码, error code: %1\n").arg(NET_DVR_GetLastError());
    }
    //---------------------------------------
    //关闭预览
    NET_DVR_StopRealPlay(lRealPlayHandle);

    qDebug() << "HikVisionCamera::eventVideoSava end";
}

void HikVisionCamera::eventHeartBeat()
{
    qDebug() << "HikVisionCamera::eventHeartBeat()";
    if(m_loopVideoSava.isRunning()) m_loopVideoSava.quit();

}


