#include "inspection.h"
#include "AppUtil.h"

QString configPath("inspection.json");

Inspection::Inspection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Inspection)
{
    ui->setupUi(this);
}

Inspection::~Inspection()
{
    delete ui;
    qDebug()<<"Inspection::~Inspection()释放";
}

void Inspection::start()
{
    // 从JSON文件中读取配置数据
    // 默认读取，会从id中赋值的
    if(ReadJsonData(config,configPath)){
        qDebug()<<config;
    }

    qDebug()<<"void Inspection::start()";
}

void Inspection::quit()
{
    if(SavaJsonData(config,configPath)){
        qDebug()<<config;
    }
    qDebug()<<"Inspection::quit()退出";
}
