#ifndef WIDGETHEADER_H
#define WIDGETHEADER_H

#include <QWidget>
#include "ui_WidgetHeader.h"

namespace Ui {
class WidgetHeader;
}

class WidgetHeader : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetHeader(QWidget *parent = nullptr);
    ~WidgetHeader();

    QWidget *getWidgetHeader();
    Ui::WidgetHeader *getUi();

private:
    bool isMousePressed;
    QPoint mouseStartMovePos;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void showEvent(QShowEvent *event) override;

private slots:
    void on_toolButton_closeWindow_clicked();

    void on_toolButton_MaximizedWindow_clicked();

    void on_toolButton_normalWindow_clicked();

    void on_toolButton_minimizedWindow_clicked();

private:
    Ui::WidgetHeader *ui;
    void isMaximizedWindowDisplay();
};

#endif // WIDGETHEADER_H
