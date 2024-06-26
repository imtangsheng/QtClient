#include "MediaWidgets.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

MediaWidgets::MediaWidgets(QWidget *parent) : QWidget(parent),
                                              ui(new Ui::HomeMainWidget)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    //    this->installEventFilter(this);
    init();
}

MediaWidgets::~MediaWidgets()
{
    delete ui;
    qDebug() << "MediaWidgets::~MediaWidgets() 释放 成功" << objectName();
}

void MediaWidgets::init()
{
}

void MediaWidgets::quit()
{
    // 1.~MediaWidgets()不可以在这里设置改变,会直接跳过不执行，怀疑是先一步与析构函数释放全局变量
    // 2.使用变量不能声明，使用改使用指针。同上，会触发三个无窗口句柄，无效的窗口句柄 但是会执行。
    // 3.解决方案：在父析构中调用保存配置的操作，且不使用全局变量

    close();
    qDebug() << "MediaWidgets::quit() 成功" << objectName();
}

void MediaWidgets::on_Button_videoPlayback_clicked()
{
    qDebug() << "MediaWidgets::on_Button_videoPlayback_clicked()";
    emit homeMune_jump_TabWidget();
}
