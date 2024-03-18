#include "MediaWidgets.h"


MediaWidgets::MediaWidgets(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MediaWidgets)
{
    ui->setupUi(this);
    init();
}

MediaWidgets::~MediaWidgets()
{
    qDebug()<<"MediaWidgets::~MediaWidgets()";
    delete ui;
}

void MediaWidgets::init()
{
    qDebug()<<"MediaWidgets::init()";
}

void MediaWidgets::on_Button_videoPlayback_clicked()
{
    qDebug()<<"MediaWidgets::on_Button_videoPlayback_clicked()";
}

