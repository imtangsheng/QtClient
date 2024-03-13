#include "SubMain.h"

SubMain::SubMain(QWidget *parent) : QMainWindow(parent),
                                    ui(new Ui::SubMain)
{
    ui->setupUi(this);
    init();
}

SubMain::~SubMain()
{
    qDebug()<<"SubMain::~SubMain()";
    delete ui;
}

void SubMain::init()
{
    qDebug()<<"SubMain::init()";
}

void SubMain::showFiles()
{
    setCentralWidget(ui->widget_download);
    //    show();
}
