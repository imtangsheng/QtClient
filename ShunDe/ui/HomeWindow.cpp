#include "HomeWindow.h"
#include "ui_HomeWindow.h"

HomeWindow::HomeWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HomeWindow)
{
    ui->setupUi(this);
}

HomeWindow::~HomeWindow()
{
    delete ui;
    qDebug()<<"HomeWindow::~HomeWindow()";
}

void HomeWindow::init()
{

}

void HomeWindow::quit()
{

}
