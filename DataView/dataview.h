#ifndef DATAVIEW_H
#define DATAVIEW_H
#include "PluginInterface.h"
#include "ui/DataWidgets.h"
#include "ui/snowbeerwindow.h"

#define DataView_iid "org.qt-project.Qt.Examples.DataView"

class DataView : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DataView_iid FILE "DataView.json")
    //接口是一种纯虚函数的集合，用于定义一组共享的行为或功能,需要在类的声明中的public部分使用它
    Q_INTERFACES(PluginInterface)
public:
    explicit DataView(QObject *parent = nullptr);
    ~DataView();
    QObject *instance() override;
    bool Start() override;
    QString getObjectNane() const override;
    QWidget *getWidgetByName(QString name,int *type = nullptr) override;
    bool widgetReturnAfterRemoved(QWidget *widget) override;

public slots:
    void quit() override;
    void jumpTabWidget();

public:
    DataWidgets *widgets;
    SnowBeerWindow *snowBeerWindow;
    void init();

signals:
    void signalShowMainWidget(int index, QString name) override;
    void signalShowMessage(const int& level,const QString& message) override;


private:

};

#endif // DATAVIEW_H
