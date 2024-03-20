#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

//#include <QGenericPlugin>
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
//    void initialize() override;
public:
    void init() override;

    QWidget *getHomeTiler() override;
    MediaWidgets widgets;

signals:
    void sendSignal(int index = -1) override;

public slots:
    void jumpTabWidget();
private:

};

#endif // MEDIAPLAYER_H





