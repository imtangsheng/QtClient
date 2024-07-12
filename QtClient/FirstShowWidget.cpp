#include "FirstShowWidget.h"
#include "ui_FirstShowWidget.h"
#include "public/AppSystem.h"

FirstShowWidget::FirstShowWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::FirstShowWidget)
{
    ui->setupUi(this);
    init();
}

FirstShowWidget::~FirstShowWidget()
{
    qDebug()<<"FirstShowWidget::~FirstShowWidget()";
    delete ui;
    qDebug()<<"FirstShowWidget::~FirstShowWidget() end";
}

void FirstShowWidget::init()
{
    setWindowFlags( Qt::FramelessWindowHint);
    AppSettings.beginGroup("FirstShowWidget");
    username = AppSettings.value("username", "admin").toString();
    password =  AppSettings.value("password", "123456").toString();
    isAutoLogin = AppSettings.value("isAutoLogin", false).toBool();
    isSavaPassword = AppSettings.value("isSavaPassword", false).toBool();
    AppSettings.endGroup();

    ui->lineEdit_username->setText(username);
    if(isSavaPassword){
        ui->lineEdit_password->setText(password);
    }
    ui->checkBox_savaPassword->setCheckState(isSavaPassword ? Qt::Checked : Qt::Unchecked);
    ui->checkBox_autoLogin->setCheckState(isAutoLogin ? Qt::Checked : Qt::Unchecked);
}

bool FirstShowWidget::startAutoLogin()
{
    //qDebug()<<"FirstShowWidget::startAutoLogin()"<<isAutoLogin<<isAutoLogin;
    if(isAutoLogin){
        if(start()){return true;}
    }
    return false;
}

bool FirstShowWidget::start()
{
    //qDebug()<<"FirstShowWidget::start()";
    if("admin" != ui->lineEdit_username->text()){
        qDebug()<<"username error"<<username<<ui->lineEdit_username->text();
        QMessageBox::warning(this, "警告", "username error");
        return false;
    }else if("123456" != ui->lineEdit_password->text()){
        qDebug()<<"password error"<<password << ui->lineEdit_password->text();
        QMessageBox::warning(this, "警告", "password error");
        return false;
    }

    AppSettings.beginGroup("FirstShowWidget");
    if(ui->checkBox_savaPassword->isChecked()){
        AppSettings.setValue("username", ui->lineEdit_username->text());
        AppSettings.setValue("password", ui->lineEdit_password->text());
        //        APP_SETTINGS.setValue("isAutoLogin", true);
        AppSettings.setValue("isSavaPassword", true);
    }
    else {
//        APP_SETTINGS.setValue("username", "");
        AppSettings.setValue("password", "");
        //        APP_SETTINGS.setValue("isAutoLogin", false);
        AppSettings.setValue("isSavaPassword", false);
        }
    if(ui->checkBox_autoLogin->isChecked()){
//        APP_SETTINGS.setValue("username", ui->lineEdit_username->text());
//        APP_SETTINGS.setValue("password", ui->lineEdit_password->text());
//        APP_SETTINGS.setValue("isSavaPassword", true);
        AppSettings.setValue("isAutoLogin", true);
    }
    else {
    //        APP_SETTINGS.setValue("username", "");
    //        APP_SETTINGS.setValue("password", "");
    //        APP_SETTINGS.setValue("isAutoLogin", false);
    //        APP_SETTINGS.setValue("isSavaPassword", false);
    AppSettings.setValue("isAutoLogin", false);
    }
    AppSettings.endGroup();
    CurrentUser =  ui->lineEdit_username->text();
    return true;

}


void FirstShowWidget::on_pushButton_login_clicked()
{
    if(start()){
        emit loginSuccess(); //加载需要时间
        //setAttribute(Qt::WA_DeleteOnClose);//close(); //先关闭再发信号，不然点击后会等待发送信号的时间才会触发
        close(); //先关闭再发信号，不然点击后会等待发送信号的时间才会触发
//        deleteLater(); //会主动释放，但是不能直接调用，需要先show，其他退出导致程序奔溃，。只能用于小部件
//        destroy(); //不会主动释放
    }
}
