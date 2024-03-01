#include "subwindow.h"
#include "ui_subwindow.h"



SubWindow::SubWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SubWindow)
{
    ui->setupUi(this);
}

SubWindow::~SubWindow()
{
    delete ui;
}

void SubWindow::showFiles()
{
    setCentralWidget(ui->widget_download);
//    show();
}
