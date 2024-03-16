#include "dataview.h"

DataView::DataView(QObject *parent)
    : QGenericPlugin(parent)
{
}

DataView::~DataView()
{
    qDebug()<<"创建的插件DataView::~DataView()释放";
}

QObject *DataView::create(const QString &name, const QString &spec)
{
//    static_assert(false, "You need to implement this function");
    return this;
}

void DataView::init()
{
    qDebug()<<"创建的插件DataView::init()";
}
