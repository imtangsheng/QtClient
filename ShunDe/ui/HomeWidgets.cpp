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

void HomeWidgets::on_Button_HomeWindow_clicked()
{
    emit signals_show_widget_by_name("HomeWindow");
}


void HomeWidgets::on_Button_MasterWindow_clicked()
{
    emit signals_show_widget_by_name("MasterWindow");
}
