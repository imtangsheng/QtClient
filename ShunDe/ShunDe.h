#ifndef SHUNDE_H
#define SHUNDE_H

#include "PluginInterface.h"
#include "ui/HomeWidgets.h"
#include "ui/HomeWindow.h"
#include "ui/MasterWindow.h"

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
    void jumpTabWidget(const QString& name);

public:
    HomeWidgets *widgets;
    HomeWindow *homeWindow;
    MasterWindow *masterWindow;
    void init();

signals:
    void signalShowMainWidget(int index, QString name) override;
    void signalShowMessage(const int& level,const QString& message) override;
};

#endif // SHUNDE_H
