#include "DataWidgets.h"
#include "ui_DataWidgets.h"

DataWidgets::DataWidgets(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataWidgets)
{
    ui->setupUi(this);
}

DataWidgets::~DataWidgets()
{
    delete ui;
}

void DataWidgets::init()
{

}

void DataWidgets::quit()
{

}

void DataWidgets::on_Button_dataView_clicked()
{
    emit homeMune_jump_TabWidget();
}

