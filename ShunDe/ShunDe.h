#ifndef SHUNDE_H
#define SHUNDE_H

#include "PluginInterface.h"
#include "ui/HomeWidgets.h"
#include "ui/HomeWindow.h"

#define ShunDe_iid "org.qt-project.Qt.Examples.ShunDe"
class ShunDe : public QObject, PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID ShunDe_iid FILE "ShunDe.json")
    Q_INTERFACES(PluginInterface)

public:
    explicit ShunDe(QObject *parent = nullptr);
    ~ShunDe();
    QObject *instance() override;
    bool Start() override;
    QString getObjectNane() const override;
    QWidget *getWidgetByName(QString name,int *type = nullptr) override;
    bool widgetReturnAfterRemoved(QWidget *widget) override;

public slots:
    void quit() override;
    void jumpTabWidget();

public:
    HomeWidgets *widgets;
    HomeWindow *window;
    void init();

signals:
    void signalShowMainWidget(int index, QString name) override;
};

#endif // SHUNDE_H
