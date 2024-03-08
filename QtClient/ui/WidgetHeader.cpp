#include "WidgetHeader.h"
#include "ui_WidgetHeader.h"
#include <QMouseEvent>

WidgetHeader::WidgetHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetHeader)
{
    ui->setupUi(this);
    this->installEventFilter(this);
}

WidgetHeader::~WidgetHeader()
{
    qDebug()<<"WidgetHeader::~WidgetHeader()";
    delete ui;
}

QWidget *WidgetHeader::getWidgetHeader()
{
    return this;
}

Ui::WidgetHeader *WidgetHeader::getUi()
{
    return ui;
}

void WidgetHeader::mousePressEvent(QMouseEvent *event)
{
    qDebug()<<"WidgetHeader::mousePressEvent "<<event->button();
    if(event->button() == Qt::LeftButton){
        isMousePressed = true;
        mouseStartMovePos = event->pos();
        return;
    }
    return QWidget::mousePressEvent(event);
}

void WidgetHeader::mouseMoveEvent(QMouseEvent *event)
{
    qDebug()<<"WidgetHeader::mouseMoveEvent "<<event->button();
    if(event->buttons() == Qt::LeftButton && isMousePressed){
        this->window()->move(this->window()->pos() + event->pos() - mouseStartMovePos);
        return;
    }
}

void WidgetHeader::mouseReleaseEvent(QMouseEvent *event)
{
    qDebug()<<"WidgetHeader::mouseReleaseEvent "<<event->button();
    if(event->button() == Qt::LeftButton){
        isMousePressed = false;
        return;
    }
    return QWidget::mouseReleaseEvent(event);
}

void WidgetHeader::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug()<<"WidgetHeader::mouseDoubleClickEvent "<<event->button();
    if(event->button() == Qt::LeftButton){
        if(this->window()->windowState() == Qt::WindowMaximized){
            this->window()->showNormal();
        }else{
            this->window()->showMaximized();
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
void WidgetHeader::showEvent(QShowEvent *event)
{
    qDebug()<<"WidgetHeader::showEvent(QShowEvent "<<event;
    QWidget::showEvent(event);
    isMaximizedWindowDisplay();
}

void WidgetHeader::on_toolButton_closeWindow_clicked()
{
    this->window()->close();
}

void WidgetHeader::isMaximizedWindowDisplay()
{
    if(this->window()->windowState() == Qt::WindowMaximized){
        ui->toolButton_MaximizedWindow->setVisible(false);
        ui->toolButton_normalWindow->setVisible(true);
    }else{
        ui->toolButton_MaximizedWindow->setVisible(true);
        ui->toolButton_normalWindow->setVisible(false);
    }
}


void WidgetHeader::on_toolButton_MaximizedWindow_clicked()
{
    window()->showMaximized();
    isMaximizedWindowDisplay();
}


void WidgetHeader::on_toolButton_normalWindow_clicked()
{
    window()->showNormal();
    isMaximizedWindowDisplay();
}


void WidgetHeader::on_toolButton_minimizedWindow_clicked()
{
    window()->showMinimized();
}

