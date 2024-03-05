#include "SubMain.h"

SubMain::SubMain(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SubMain)
{
    ui->setupUi(this);
}

SubMain::~SubMain()
{
    delete ui;
}

void SubMain::showFiles()
{
    setCentralWidget(ui->widget_download);
//    show();
}
