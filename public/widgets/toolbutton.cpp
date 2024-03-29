#include "toolbutton.h"

ToolButton::ToolButton(QWidget *parent) : QToolButton(parent)
{

}

void ToolButton::enterEvent(QEnterEvent *event)
{
    qDebug()<<"void ToolButton::enterEvent(QEnterEvent *";
    emit hovered();
    return QToolButton::enterEvent(event);
}
