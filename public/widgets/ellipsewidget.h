#ifndef ELLIPSEWIDGET_H
#define ELLIPSEWIDGET_H

#include <QPainter>
#include <QPainterPath>
#include <QWidget>

class EllipseWidget : public QWidget
{
    Q_OBJECT
public:
    explicit EllipseWidget(QWidget *parent = nullptr) : QWidget{parent} {

    }
    QString colorBrushHtml = "#37474f";

signals:

protected:
    void paintEvent(QPaintEvent *event) override {
        Q_UNUSED(event);
        // 创建椭圆形路径
        QPainterPath path;
        //path.addEllipse(rect());
        path.addEllipse(rect().adjusted(1,1,-2,-2));
        // 设置遮罩
        setMask(path.toFillPolygon().toPolygon());
        // 重写窗口的绘图事件
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true); // 设置渲染提示为抗锯齿
        // 绘制椭圆形区域
        QColor color(colorBrushHtml);
        painter.setBrush(color);
        painter.drawEllipse(rect());
        //painter.setBrush(Qt::darkGray);
        painter.drawEllipse(rect().adjusted(-1,-1,0,0));

        return QWidget::paintEvent(event);
    }

};

#endif // ELLIPSEWIDGET_H
