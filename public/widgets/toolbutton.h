#ifndef TOOLBUTTON_H
#define TOOLBUTTON_H

#include <QToolButton>

class ToolButton : public QToolButton
{
    Q_OBJECT
public:
    ToolButton(QWidget *parent = nullptr);

signals:
    void hovered();

protected:
    void enterEvent(QEnterEvent *event) override;
//    void leaveEvent(QEvent *event) override;

};

#endif // TOOLBUTTON_H
