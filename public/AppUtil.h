#ifndef APPUTIL_H
#define APPUTIL_H

#include "AppOS.h"

#include <QFile>

/*json文件读取软件相关配置*/
#include <QJsonDocument>
IsSucce ReadJsonData(QJsonObject& jsonData,const QString& filePath);
IsSucce SavaJsonData(QJsonObject& jsonData,const QString& filePath);

/*json文件读取软件相关配置*/


#endif // APPUTIL_H
