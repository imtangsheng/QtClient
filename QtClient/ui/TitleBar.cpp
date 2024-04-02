#include "TitleBar.h"
#include "ui_TitleBar.h"
#include <QMouseEvent>

#include <QFile>

TitleBar::TitleBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TitleBar)
{
    ui->setupUi(this);
    // 定义文件路径
    QString filePath = "logo.png";
    // 判断文件是否存在
    QFile file(filePath);
    if (file.exists()) {
        // 文件存在，加载并显示图像
        QPixmap pixmap(filePath);
        ui->label_logo->setPixmap(pixmap);
    }
}

TitleBar::~TitleBar()
{
    delete ui;
    qDebug()<<"TitleBar::~TitleBar()";
}

void TitleBar::mousePressEvent(QMouseEvent *event)
{
    qDebug()<<"WidgetHeader::mousePressEvent "<<event->button();
    if(event->button() == Qt::LeftButton){
        isMousePressed = true;
        mouseStartMovePos = event->pos();
        return;
    }
    return QWidget::mousePressEvent(event);
}

void TitleBar::mouseMoveEvent(QMouseEvent *event)
{
//    qDebug()<<"WidgetHeader::mouseMoveEvent "<<event->button();
    if(event->buttons() == Qt::LeftButton && isMousePressed){
        this->window()->move(this->window()->pos() + event->pos() - mouseStartMovePos);
        emit posChange(event->pos());
        return;
    }
}

void TitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug()<<"WidgetHeader::mouseReleaseEvent "<<event->button();
    if(event->button() == Qt::LeftButton){
        isMousePressed = false;
        return;
    }
    return QWidget::mouseReleaseEvent(event);
}

void TitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug()<<"WidgetHeader::mouseDoubleClickEvent "<<event->button();
    if(event->button() == Qt::LeftButton){
        if(this->window()->windowState() == Qt::WindowFullScreen){
            this->window()->showNormal();
        }else if(this->window()->windowState() == Qt::WindowMaximized){
            this->window()->showNormal();
        }else{
//            this->window()->showMaximized();
            this->window()->showFullScreen();
        }
        isMaximizedWindowDisplay();
        return;
    }
    return QWidget::mouseDoubleClickEvent(event);
}

/*
当窗口第一次显示时，即调用QWidget的show()或QMainWindow的show()函数时。
当窗口从最小化状态恢复到正常显示状态时。
当窗口从隐藏状态恢复到正常显示状态时。
*/
void TitleBar::showEvent(QShowEvent *event)
{
    qDebug()<<"TitleBar::showEvent(QShowEvent "<<event;
    QWidget::showEvent(event);
    isMaximizedWindowDisplay();
}

void TitleBar::on_toolButton_closeWindow_clicked()
{
    this->window()->close();
}

void TitleBar::isMaximizedWindowDisplay()
{
    if(this->window()->windowState() == Qt::WindowMaximized){
        ui->toolButton_MaximizedWindow->setVisible(false);
        ui->toolButton_normalWindow->setVisible(true);
    }else{
        ui->toolButton_MaximizedWindow->setVisible(true);
        ui->toolButton_normalWindow->setVisible(false);
    }
}


void TitleBar::on_toolButton_MaximizedWindow_clicked()
{
    window()->showMaximized();
    isMaximizedWindowDisplay();
}


void TitleBar::on_toolButton_normalWindow_clicked()
{
    window()->showNormal();
    isMaximizedWindowDisplay();
}


void TitleBar::on_toolButton_minimizedWindow_clicked()
{
    window()->showMinimized();
}

