#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include "ui_TitleBar.h"

namespace Ui {
class TitleBar;
}

class TitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit TitleBar(QWidget *parent = nullptr);
    ~TitleBar();

private:
    bool isMousePressed;
    QPoint mouseStartMovePos;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

    void showEvent(QShowEvent *event) override;

signals:
    void posChange(const QPoint& pos);

private slots:
    void on_toolButton_closeWindow_clicked();

    void on_toolButton_MaximizedWindow_clicked();

    void on_toolButton_normalWindow_clicked();

    void on_toolButton_minimizedWindow_clicked();

private:
    Ui::TitleBar *ui;
    void isMaximizedWindowDisplay();
};

#endif // TITLEBAR_H
