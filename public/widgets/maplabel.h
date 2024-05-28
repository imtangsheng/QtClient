#ifndef MAPLABEL_H
#define MAPLABEL_H

#include<QLabel>
#include <QMenu>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QPushButton>
#include <QWheelEvent>

class MapLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MapLabel(QWidget *parent = nullptr);
    void init();
    QMenu menu;

    QMap<int,QWidget*> robotIcons;
    QMap<int,QString> mapRobotName;
    bool add_robot_icon(int id,QString name,QWidget* robot_icon);


protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event ) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override; //鼠标滚轮事件
    void mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
    void move_robot_icon(const int& id,const QPoint& pos);
private slots:
    void isShowFullScreen();
    void showContextMenu(const QPoint& pos);

private:
    QPoint mousePressPos;
    QPoint mouseNowPos;
    //bool isMousePress = false;
    bool isMouseMove = false;

};

#endif // MAPLABEL_H
