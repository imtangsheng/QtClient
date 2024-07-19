#ifndef APPSYSTEM_H
#define APPSYSTEM_H

#include <QDebug>
#include <QMessageBox>

#include "AppOS.h"

/*供了互斥锁机制，用于保护共享资源，防止多个线程同时访问*/
#include <mutex>
template <typename T>
class MutexLockTemplate
{
private:
    /* data */
    std::mutex lock;
    T data;
    std::chrono::steady_clock::time_point lastUpdateTime;

public:
    MutexLockTemplate(/* args */) = default;
    MutexLockTemplate(T value) { data = value; }
    void set(T value)
    {
        lock.lock();
        data = value;
        lastUpdateTime = std::chrono::steady_clock::now();
        lock.unlock();
    }
    T get()
    {
        T tmp;
        lock.lock();
        tmp = data;
        lock.unlock();
        return tmp;
    }
    std::chrono::steady_clock::time_point getLastUpdateTime() { return lastUpdateTime; }
    ~MutexLockTemplate() = default;
};

/*原子操作，以确保数据的一致性和线程安全性*/
#include <atomic>
#include <map>
template <typename ID, typename E>
class MapAtomicEnumValue
{
private:
    /* data */
    std::map<ID, std::atomic<E>> valueMap;

public:
    MapAtomicEnumValue(/* args */) = default;
    MapAtomicEnumValue(ID sUserID, E value) { insert(sUserID, value); }
    BOOL insert(ID sUserID, E value)
    {
        valueMap[sUserID].store(value);
        return TRUE;
    }

    void set(ID sUserID, E value)
    {
        valueMap[sUserID].store(value);
    }
    E get(ID sUserID)
    {
        return valueMap[sUserID].load();
    }
    ~MapAtomicEnumValue() = default;
};

/*json文件读取软件相关配置*/
#include <QJsonDocument>
bool getExeConfigJson();
IsSucce ReadJsonData(QJsonObject& jsonData,const QString& filePath);
IsSucce SavaJsonData(QJsonObject& jsonData,const QString& filePath);
/*ini文件读取配置，Qt自带系统方法，指定存储在本地或者系统注册表等地方*/

/*使用qt自带的消息映射，qDebug自定义输出*/
#include <QtLogging>
#include <QFile>
#include <QDir>
#include <QTextStream>
// QT_MESSAGE_PATTERN="[%{time yyyyMMdd h:mm:ss.zzz t} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - %{message}"
void logToFile(QtMsgType type, const QMessageLogContext &context, const QString &msg);

extern QString PATH_EXE_CONFIG;
extern QString PATH_APP_SETTINGS;
extern QString PATH_LOG;
/**系统的自定义配置的提示信息json文件**/
inline const QString TipsPath = "tips.json";

inline Result ReadJsonDataByFile(QJsonObject &jsonData, const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return Result(false, "无法打开文件: " + file.errorString());
    }

    QByteArray jsonDataByteArray = file.readAll();
    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonDataByteArray, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        return Result(false, "JSON解析错误: " + parseError.errorString());
    }

    if (!jsonDoc.isObject()) {
        return Result(false, "JSON文档不是一个对象");
    }

    jsonData = jsonDoc.object();
    return Result(true, "JSON数据读取成功");
}

inline Result SavaJsonDataToFile(QJsonObject &jsonData, const QString &filePath)
{
    QJsonDocument jsonDoc(jsonData);
    QByteArray jsonDataByteArray = jsonDoc.toJson();

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        return Result(false, "无法打开文件: " + file.errorString());
    }

    qint64 bytesWritten = file.write(jsonDataByteArray);
    if (bytesWritten != jsonDataByteArray.size()) {
        file.close();
        return Result(false, "文件写入失败");
    }

    if (!file.flush() || !file.waitForBytesWritten(-1)) {
        file.close();
        return Result(false, "文件写入失败");
    }

    file.close();
    return Result(true, "JSON数据保存成功");
}
#endif // APPSYSTEM_H
