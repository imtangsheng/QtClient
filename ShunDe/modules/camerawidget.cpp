#include "camerawidget.h"
#include <QMouseEvent>


CameraWidget::CameraWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::CameraWidget),
    parent1(parent)
{
    ui->setupUi(this);

    init();
}

CameraWidget::~CameraWidget()
{
    delete ui;
}

void CameraWidget::init()
{
//    player.setAudioOutput(&audioOutput);
    ui->VideoWidget->setAspectRatioMode(Qt::IgnoreAspectRatio);//设置图像自动拉伸铺满全屏
    player.setVideoOutput(ui->VideoWidget);

}

void CameraWidget::quit()
{

}

void CameraWidget::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Escape || event->key() == Qt::Key_Back) && isFullScreen()) {
        this->showFullScreen();
        event->accept();
    } else if (event->key() == Qt::Key_Enter && event->modifiers() & Qt::Key_Alt) {
//        ui->VideoWidget->setFullScreen(!isFullScreen());
        this->showFullScreen();
        event->accept();
    }

    return QWidget::keyPressEvent(event);
}

void CameraWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug()<<"CameraWidget::mouseDoubleClickEvent 全屏："<<this->isFullScreen();
//    ui->VideoWidget->setFullScreen(!ui->VideoWidget->isFullScreen());
    if(this->isFullScreen()){
//        this->setParent(parent1);
        this->showNormal();
    }else{
//        parent1 = this->parentWidget();
//        this->setParent(nullptr);
        this->showFullScreen();
    }
    return QWidget::mouseDoubleClickEvent(event);
}

void CameraWidget::mousePressEvent(QMouseEvent *event)
{
    emit mousePress();
//    setStyleSheet("border: 2px solid red;");
    return QWidget::mousePressEvent(event);
}

void CameraWidget::enterEvent(QEnterEvent *event)
{
    emit mouseEnterEvent();
   return QWidget::enterEvent(event);
}

void CameraWidget::leaveEvent(QEvent *event)
{
   emit mouseLeaveEvent();
   return QWidget::leaveEvent(event);
}

void CameraWidget::paintEvent(QPaintEvent *event)
{
   Q_UNUSED(event)
//   qDebug()<<"void CameraWidget::paintEvent(QPaintEvent *"<<event;

   // 在paintEvent函数中使用绑定的QPainter对象进行绘图操作
   //该QPainter只能在paintEvent函数中使用，使用后会显示后会析构防止修改
   QPainter painter(this);
   //painter.begin(this); // 绑定到当前的QWidget设备
   painter.setPen(QPen(Qt::red, 1));
   //   p.drawRect(0,0,width()-1,height() -1 );
   painter.drawRect(rect().adjusted(0, 0, -1, -1));

}
