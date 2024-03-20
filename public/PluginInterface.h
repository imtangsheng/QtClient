/**
 * @file PluginInterface.h
 * @brief 该文件是插件调用接口类模板
 *
 * @details
 * Author: Tang
 * Created: 2024-03
 */

#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QObject>
#include <qwidget.h>
#include <QtPlugin>
#include <QIcon>

//struct PluginMetaDate
//{

//};

//Q_DECLARE_METATYPE(PluginMetaDate);//确保类型可以通过信号槽传递
#define HomeMenu_WidgetName "HomeMenu"
#define HomeMain_TabWidgetName "HomeMainWidget"

class PluginInterface// : public QObject
{
public:
/**声明方法函数**/
    virtual ~PluginInterface() = default;
    virtual void init() = 0; //添加=0显式声明为纯虚函数,必须实现
    virtual QString getObjectNane() = 0;
    virtual QWidget* getWidgetByName(QString name = "") = 0;
    virtual QWidget* getHomeTiler() = 0;
/**声明信号函数的声明，在信号连接器中使用字符串的信号连接模式**/
    virtual void signalShowMainWidget(int index = -1,QString name = HomeMain_TabWidgetName) = 0;

/**声明定义变量**/
    QString ObjectName;
    int id = -1; //顺序list 的下标id
//    在TabWidget中的顺序index
    int indexTabBar = -1;
    QIcon WindowIcon;
    QString WindowTitle;
};

QT_BEGIN_NAMESPACE

#define PluginInterface_iid "org.qt-project.Qt.Examples.PluginInterface"
//通过使用Q_DECLARE_INTERFACE宏，我们告诉Qt框架这个类是一个接口
Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)
QT_END_NAMESPACE

#endif // PLUGININTERFACE_H
