/**
 * @file AppOS.h
 * @brief 该文件是APP通用的大多数接口接口类模板，声明全局变量，方法
 *
 * @details
 * Author: Tang
 * Created: 2024-03
 * Version: 0.0.3
 */

#ifndef APPOS_H
#define APPOS_H

#ifndef _WINDOWS_
#if (defined(_WIN32) || defined(_WIN64))
#include <winsock2.h>
#include <windows.h>
#include <cstdint>
#endif
#endif

#if defined(_WIN64)
#define OS_WINDOWS64 1
#endif

#if (defined(_WIN32)) // windows
#if (defined(_USE_DLL_))
#define NET_ROBOT_API extern "C" __declspec(dllimport)
// #define NET_ROBOT_API Q_DECL_EXPORT
#else
#define IsSucce bool
#endif
typedef unsigned __int64 UINT64;
typedef signed __int64 INT64;
#elif defined(__linux__) || defined(__APPLE__) // linux
#if (defined(_USE_DLL_))
#define NET_ROBOT_API extern "C"
#else
#define NET_ROBOT_API
#define IsSucce bool
#endif
#define BOOL int
typedef unsigned int DWORD;
typedef unsigned short WORD;
typedef unsigned short USHORT;
typedef short SHORT;
typedef int LONG;
typedef unsigned char BYTE;
typedef unsigned int UINT;
typedef void *LPVOID;
typedef void *HANDLE;
typedef unsigned int *LPDWORD;
typedef unsigned long long UINT64;
typedef signed long long INT64;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef NULL
#define NULL 0
#endif

#define __stdcall
#define CALLBACK
#endif // windows

/** App的全局变量的声明 extern
 * 可以被多个头文件包含而不会链接错误
 * QString 是一个类，不是基本类型。全局或静态 QString 对象的初始化可能会在 main() 函数执行之前发生，
 * 这时 Qt 的一些基础设施可能还没有设置好。这可能导致一些奇怪的行为或崩溃。
 * 1. extern 改为使用 inline，并定义
 * 2.使用指针，在 main() 函数或 app 初始化后 赋值
 * 3.不在包含定义的 .cpp 文件中初始化，在 main.cpp 等使用的地方定义
**/

/*ini文件读取配置，Qt自带系统方法，指定存储在本地或者系统注册表等地方*/
#include <QSettings>
extern QSettings AppSettings;
/**全局变量 exe或者dll的本地配置ini文件的json格式存储的配置信息**/
#include <QJsonObject>
extern QJsonObject AppJson;

/**int的数字转字符到GUI中显示**/
#include<QString>
extern QString CurrentUser;
inline QString i2s(int num) {
    return QString::number(num);
}

/**定义一个结构体来包含更详细的结果信息**/
struct Result
{
    bool success;
    QString message;

    Result(bool s = false, const QString& msg =""):success(s),message(msg) {}
    operator bool() const {return success;}//重载了 bool 操作符，使其可以像之前的 bool 返回值一样使用例如：if (result)
};

#endif // APPOS_H
