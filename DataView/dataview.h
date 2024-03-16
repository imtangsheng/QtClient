#ifndef DATAVIEW_H
#define DATAVIEW_H
#include <QGenericPlugin>

#include "PluginInterface.h"

#define DataView_iid "org.qt-project.Qt.Examples.DataView"

class DataView : public QGenericPlugin, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid FILE "DataView.json")
    //接口是一种纯虚函数的集合，用于定义一组共享的行为或功能,需要在类的声明中的public部分使用它
    Q_INTERFACES(PluginInterface)
public:
    explicit DataView(QObject *parent = nullptr);
    ~DataView();

public:
    void init() override;

private:
    QObject *create(const QString &name, const QString &spec) override;
};

#endif // DATAVIEW_H
