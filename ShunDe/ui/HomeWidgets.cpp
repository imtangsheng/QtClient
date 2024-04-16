#include "HomeWidgets.h"
#include "ui_HomeWidgets.h"

HomeWidgets::HomeWidgets(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::HomeWidgets)
{
    ui->setupUi(this);
}

HomeWidgets::~HomeWidgets()
{
    delete ui;
    qDebug()<<"创建的插件ShunDe HomeWidgets::~HomeWidgets()";
}

void HomeWidgets::init()
{

}

void HomeWidgets::quit()
{

}

void HomeWidgets::on_Button_homeWindow_clicked()
{
     emit homeMune_jump_TabWidget();
}

