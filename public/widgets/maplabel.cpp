#include "maplabel.h"
#include <QMenu>
#include <QPushButton>


MapLabel::MapLabel(QWidget *parent)
    : QLabel{parent}
{

    connect(this,&MapLabel::customContextMenuRequested,this,&MapLabel::showContextMenu);
    init();
}

void MapLabel::init()
{
    //QMenu menu(this);
//    menu.addAction("Full Screen",this,[this](){
//        qDebug()<<"showContextMenu(const QPoint &pos)"<<this->isFullScreen();
//    });

//    menu.addAction(tr("导航"),this,[this](){
//        qDebug()<<"导航menu:"<<menu.pos()<<"mousePressPos"<<mousePressPos;

//        emit move_robot_icon(mousePressPos);
//    });
}


bool MapLabel::add_robot_icon(int id,QString name, QWidget *robot_icon)
{
    robot_icon->setParent(this);
    //robot_icon->move(0,0);
    robotIcons[id] = robot_icon;
    mapRobotName[id]= name;
    return true;
}

void MapLabel::paintEvent(QPaintEvent *event)
{
    //先调用父类的paintEvent为了显示'背景'!!!
    QLabel::paintEvent(event);
}

void MapLabel::mousePressEvent(QMouseEvent *event)
{
    //qDebug()<<"mousePressEvent mousePressPos:"<<mousePressPos<<event->pos();
    mousePressPos = event->pos();
    QLabel::mousePressEvent(event);
}

void MapLabel::mouseMoveEvent(QMouseEvent *event)
{
    //如果鼠标跟踪处于关闭状态，则仅当在移动鼠标时按下鼠标按钮时，才会发生鼠标移动事件。如果鼠标跟踪已打开，即使未按下鼠标按钮，也会发生鼠标移动事件。
    if(!isMouseMove) {isMouseMove=true;}
    mouseNowPos = event->pos() - mousePressPos;
    QLabel::mouseMoveEvent(event);
}

void MapLabel::mouseReleaseEvent(QMouseEvent *event)
{
    if(isMouseMove){
        isMouseMove = false;
        mousePressPos = event->pos();
        //        move(x()+mouseNowPos.x(),y()+mouseNowPos.y());
        move(mouseNowPos);

    }
    //qDebug()<<"mouseReleaseEvent mousePressPos2:"<<mousePressPos<<event->pos();
}

void MapLabel::wheelEvent(QWheelEvent *event)
{
    qDebug()<<"MapLabel::wheelEvent(QWheelEvent *event)"<<event->pixelDelta()<<event->angleDelta() ;
    if (event->angleDelta().y() > 0) {
        //向上滚动
        qDebug()<<"向上滚动";
    }else{
        qDebug()<<"向下滚动";
    }
    event->accept();
}

void MapLabel::mouseDoubleClickEvent(QMouseEvent *event)
{
    qDebug()<<"mouseDoubleClickEvent(QMouseEvent *event)";
    isShowFullScreen();
}

void MapLabel::isShowFullScreen()
{
    if (isFullScreen()) {
        // 如果当前是全屏状态,则恢复正常大小
        setWindowFlags(Qt::Widget);
        showNormal();
    } else {
        // 否则进入全屏显示
        setWindowFlags(Qt::Window | Qt::WindowFullscreenButtonHint | Qt::WindowMinMaxButtonsHint);
        showFullScreen();
    }
}

void MapLabel::showContextMenu(const QPoint &pos)
{
    //menu.setParent(this);
    qDebug()<<"showContextMenu(const QPoint &pos:"<<pos;

    QMenu map_menu(this);
    map_menu.addAction(this->isFullScreen() ? tr("退出全屏") : tr("全屏"),this,[this](){
        qDebug()<<"this->isFullScreen()"<<this->isFullScreen();
        isShowFullScreen();
    });

    for(auto it = mapRobotName.begin();it != mapRobotName.end();it++){
        map_menu.addAction(it.value()+ tr("-导航至该处"),this,[=](){
            qDebug()<<"导航menu:"<<"mousePressPos"<<mousePressPos;
            emit move_robot_icon(it.key(),mousePressPos);
        });
    }
    map_menu.exec(mapToGlobal(pos));
}
