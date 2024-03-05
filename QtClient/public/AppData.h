#ifndef APPDATA_H
#define APPDATA_H

#include <atomic>
#include <mutex>
#include <map>
#include <QDebug>
#include "AppOS.h"

template <typename T> class MutexLockTemplate
{
private:
    /* data */
    std::mutex lock;
    T data;
    std::chrono::steady_clock::time_point lastUpdateTime;
public:
    MutexLockTemplate(/* args */) = default;
    MutexLockTemplate(T value) { data = value; }
    void set(T value) {
        lock.lock();data = value;lastUpdateTime = std::chrono::steady_clock::now();lock.unlock();
    }
    T get() {
        T tmp;lock.lock();tmp = data;lock.unlock();
        return tmp;
    }
    std::chrono::steady_clock::time_point getLastUpdateTime() {return lastUpdateTime;}
    ~MutexLockTemplate() = default;
};

template <typename ID, typename E> class MapAtomicEnumValue
{
private:
    /* data */
    std::map<ID, std::atomic<E> > valueMap;
public:
    MapAtomicEnumValue(/* args */) = default;
    MapAtomicEnumValue(ID sUserID, E value) { insert(sUserID,value); }
    BOOL insert(ID sUserID, E value) {
        valueMap[sUserID].store(value);
        return TRUE;
    }

    void set(ID sUserID, E value) {
        valueMap[sUserID].store(value);
    }
    E get(ID sUserID) {
        return valueMap[sUserID].load();
    }
    ~MapAtomicEnumValue() = default;
};

#include <QJsonObject>

extern QJsonObject EXE_CONFIG;
#include <QSettings>

extern QSettings APP_SETTINGS;

#include <QtLogging>
//QT_MESSAGE_PATTERN="[%{time yyyyMMdd h:mm:ss.zzz t} %{if-debug}D%{endif}%{if-info}I%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{file}:%{line} - %{message}"



#endif // APPDATA_H
