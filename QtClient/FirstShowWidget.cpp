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
    delete ui;
}

void FirstShowWidget::init()
{
    setWindowFlags( Qt::FramelessWindowHint);

    APP_SETTINGS.beginGroup("FirstShowWidget");
    username = APP_SETTINGS.value("username", "").toString();
    password =  APP_SETTINGS.value("password", "").toString();
    isAutoLogin = APP_SETTINGS.value("isAutoLogin", false).toBool();
    isSavaPassword = APP_SETTINGS.value("isSavaPassword", false).toBool();
    APP_SETTINGS.endGroup();

    ui->lineEdit_username->setText(username);
    ui->checkBox_savaPassword->setCheckState(isSavaPassword ? Qt::Checked : Qt::Unchecked);
    ui->checkBox_autoLogin->setCheckState(isAutoLogin ? Qt::Checked : Qt::Unchecked);
}

bool FirstShowWidget::startAutoLogin()
{
    qDebug()<<"FirstShowWidget::startAutoLogin()"<<isAutoLogin<<isAutoLogin;
    if(isSavaPassword){
        ui->lineEdit_password->setText(password);
        if(isAutoLogin){
            if(start()){return true;}
        }
    }
    return false;
}

bool FirstShowWidget::start()
{
    qDebug()<<"FirstShowWidget::start()";
    if("admin" != ui->lineEdit_username->text()){
        qDebug()<<"username error"<<username<<ui->lineEdit_username->text();
        QMessageBox::warning(this, "警告", "username error");
        return false;
    }else if("12345" != ui->lineEdit_password->text()){
        qDebug()<<"password error"<<password << ui->lineEdit_password->text();
        QMessageBox::warning(this, "警告", "password error");
        return false;
    }
    qDebug()<<"FirstShowWidget::start() loginSuccess";

    APP_SETTINGS.beginGroup("FirstShowWidget");
    if(ui->checkBox_savaPassword->isChecked()){
        APP_SETTINGS.setValue("username", ui->lineEdit_username->text());
        APP_SETTINGS.setValue("password", ui->lineEdit_password->text());
        //        APP_SETTINGS.setValue("isAutoLogin", true);
        APP_SETTINGS.setValue("isSavaPassword", true);
    }
    else {
//        APP_SETTINGS.setValue("username", "");
        APP_SETTINGS.setValue("password", "");
        //        APP_SETTINGS.setValue("isAutoLogin", false);
        APP_SETTINGS.setValue("isSavaPassword", false);
        }
    if(ui->checkBox_autoLogin->isChecked()){
//        APP_SETTINGS.setValue("username", ui->lineEdit_username->text());
//        APP_SETTINGS.setValue("password", ui->lineEdit_password->text());
//        APP_SETTINGS.setValue("isSavaPassword", true);
        APP_SETTINGS.setValue("isAutoLogin", true);
    }
    else {
    //        APP_SETTINGS.setValue("username", "");
    //        APP_SETTINGS.setValue("password", "");
    //        APP_SETTINGS.setValue("isAutoLogin", false);
    //        APP_SETTINGS.setValue("isSavaPassword", false);
    APP_SETTINGS.setValue("isAutoLogin", false);
    }
    APP_SETTINGS.endGroup();
    return true;

}


void FirstShowWidget::on_pushButton_login_clicked()
{
    if(start()){
        emit loginSuccess();
        close();
    }
}
