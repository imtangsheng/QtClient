#include "slider.h"

#include <QMouseEvent>
#include <QStyle>

Slider::Slider(QWidget *parent) :QSlider(parent)
{

}

void Slider::mousePressEvent(QMouseEvent *event)
{
    qDebug()<<"Slider::mousePressEvent(QMouseEvent *"<<event->pos().x();
    // 获取鼠标点击的位置
    int position = QStyle::sliderValueFromPosition(minimum(), maximum(), event->pos().x(), width());

    // 设置Slider的值为点击位置的值
//    setValue(position);
    emit sliderMoved(position);
    // 调用基类的mousePressEvent处理其他事件
    return QSlider::mousePressEvent(event);
}

void Slider::enterEvent(QEnterEvent *event)
{
    emit mouseEnterEvent();
    return QSlider::enterEvent(event);
}

void Slider::leaveEvent(QEvent *event)
{
    emit mouseLeaveEvent();
    return QSlider::leaveEvent(event);
}
