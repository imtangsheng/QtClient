#include "videowindow.h"

//#include <QSettings>


//QSettings *config; //("config/config.ini",QSettings::IniFormat); //无编码配置，已经移除，使用UTF-8

VideoWindow::VideoWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VideoWindow)
{
    ui->setupUi(this);
    init();
}

VideoWindow::~VideoWindow()
{

    delete ui;
    qDebug() << "VideoWindow::~VideoWindow()释放";
}

void VideoWindow::init()
{
    qDebug() << "VideoWindow::init()";

    connect(ui->video,&VideoWidget::mouseEnterEvent,this,&VideoWindow::mouseEnterVideo);
    connect(ui->video,&VideoWidget::mouseLeaveEvent,this,&VideoWindow::mouseLeaveVideo);

//    config = new QSettings("config/" + objectName() + ".ini", QSettings::IniFormat); // 无编码配置，已经移除，使用UTF-8


    ui->video->init();
    ui->video->player.setSource(QUrl::fromLocalFile("G:/data/雪花啤酒/test.mp4"));
    ui->video->player.play();

//    ui->PlayerControls->setTitleBarWidget(ui->WidgetControlsTitleBar);
//    ui->PlayerControls->setAttribute(Qt::WA_TranslucentBackground);
//    ui->PlayerControls->setWindowOpacity(0.6);

    ui->WidgetMore->setTitleBarWidget(ui->WidgetMoreTitleBar);


}

void VideoWindow::quit()
{
//    config->beginGroup(objectName());
//    config->setValue("geometry", saveGeometry());
//    config->endGroup();
//    config->sync();
    ui->video->quit();
    qDebug() << "VideoWindow::closeEvent";
}

void VideoWindow::mouseEnterVideo()
{
qDebug() << "VideoWindow::mouseEnterVideo()";
}

void VideoWindow::mouseLeaveVideo()
{
qDebug() << "VideoWindow::mouseLeaveVideo()";
}

void VideoWindow::showEvent(QShowEvent *event)
{
    qDebug() <<"VideoWindow::showEvent(QShowEvent *"<<event;
//    const auto geometry = config->value("geometry", QByteArray()).toByteArray(); // QByteArray 类型
//    if(!geometry.isEmpty()){
//        restoreGeometry(geometry);
//    }
}

void VideoWindow::on_Button_moreFloatable_clicked()
{
    if(ui->WidgetMore->isFloating()){
        ui->WidgetMore->setFloating(false);
    }else{
        ui->WidgetMore->setFloating(true);
    }
}


