#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include "PluginInterface.h"
#include "ui/MediaWidgets.h"

//! [0] 定义插件接入
class MediaPlayer : public QObject,PluginInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID PluginInterface_iid FILE "MediaPlayer.json")
    //接口是一种纯虚函数的集合，用于定义一组共享的行为或功能,需要在类的声明中的public部分使用它
    Q_INTERFACES(PluginInterface)

public:
    explicit MediaPlayer(QObject *parent = nullptr);
    ~MediaPlayer();
     void init();
//    void initialize() override;
public:
    QString getObjectNane() override;
    QWidget *getWidgetByName(QString name) override;
    bool widgetReturnAfterRemoved(QWidget* widget) override;
    QWidget *getHomeTiler() override;


    MediaWidgets *widgets;

signals:
    void signalShowMainWidget(int index,QString name) override;

public slots:
    void quit() override;
    void jumpTabWidget();
private:

};

#endif // MEDIAPLAYER_H





