#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QObject>
#include <qwidget.h>

class PluginInterface
{
public:
    virtual ~PluginInterface() = default;
    virtual void init() = 0; //添加=0显式声明为纯虚函数,必须实现
    virtual QWidget* getHomeTiler() = 0;

};

QT_BEGIN_NAMESPACE

#define PluginInterface_iid "org.qt-project.Qt.Examples.PluginInterface"
//通过使用Q_DECLARE_INTERFACE宏，我们告诉Qt框架这个类是一个接口
Q_DECLARE_INTERFACE(PluginInterface, PluginInterface_iid)
QT_END_NAMESPACE

#endif // PLUGININTERFACE_H
