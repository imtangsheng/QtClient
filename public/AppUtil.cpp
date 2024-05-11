#include "Apputil.h"

IsSucce ReadJsonData(QJsonObject &jsonObj, const QString &filePath)
{
    QFile file(filePath);
    if(file.exists() == false){
        qDebug() << "file not exists";
        return false;
    }
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray jsonDataByteArray = file.readAll();
        QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonDataByteArray);
        jsonObj = jsonDoc.object();
        return true;
    }
    qDebug() << "file open failed";
    return false;
}

IsSucce SavaJsonData(QJsonObject &jsonObj, const QString &filePath)
{
    QJsonDocument jsonDoc(jsonObj);
    QByteArray jsonDataByteArray = jsonDoc.toJson();

    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.write(jsonDataByteArray);
        file.close();
        return true;
    }
    qDebug() << "file open failed";
    return false;
}
