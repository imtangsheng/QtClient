/**
 * @file AppOS.h
 * @brief 该文件是APP通用的大多数接口接口类模板，声明全局变量，方法
 *
 * @details
 * Author: Tang
 * Created: 2024-03
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

#endif // APPOS_H
