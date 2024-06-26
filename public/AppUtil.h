#ifndef APPUTIL_H
#define APPUTIL_H

#include "AppOS.h"

#include <QFile>

/*摄像头文件相关配置*/
extern QString gPathCameraPicture;
extern QString gPathCameraVideo;

/*json文件读取软件相关配置*/
#include <QJsonDocument>
IsSucce ReadJsonData(QJsonObject& jsonData,const QString& filePath);
IsSucce SavaJsonData(QJsonObject& jsonData,const QString& filePath);

#endif // APPUTIL_H
