#include "FirstShowWidget.h"
#include "ui_FirstShowWidget.h"

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

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <windowsx.h>
#endif


bool FirstShowWidget::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
//    qDebug() << "MainWidget::nativeEvent:"<<test1++<<eventType<<message<<result;
#ifdef Q_OS_WIN
    //Workaround for known bug -> check Qt forum : https://forum.qt.io/topic/93141/qtablewidget-itemselectionchanged/13
    Q_UNUSED(eventType)
    MSG *param = static_cast<MSG*> (message);
    switch (param->message) {
    case WM_NCHITTEST:
    {
        int nX = GET_X_LPARAM(param->lParam) - this->geometry().x();
        int nY = GET_Y_LPARAM(param->lParam) - this->geometry().y();

        //       位于子控件上，不处理
        if(childAt(nX,nY) != nullptr){
            qDebug() << "位于子控件上，不处理";
            return QWidget::nativeEvent(eventType,message,result);
        }
        *result = HTCAPTION;
        //        鼠标位于窗体边框，进行缩放
        if((nX>0)&&(nX<m_nBorderWidth)){
            *result = HTLEFT;
        }
        if((nX > this->width() - m_nBorderWidth) && (nX<this->width())){
            *result = HTRIGHT;
        }

        if((nY>0)&&(nY<m_nBorderWidth)){
            *result = HTTOP;
        }
        if((nY > this->height() - m_nBorderWidth) && (nX<this->height())){
            *result = HTBOTTOM;
        }

        if((nX>0)&& (nX <m_nBorderWidth) && (nY>0) && (nY<m_nBorderWidth)){
            *result = HTTOPLEFT;
        }

        if((nX>this->width()-m_nBorderWidth)&& (nX <this->width()) && (nY>0) && (nY<m_nBorderWidth)){
            *result = HTTOPRIGHT;
        }

        if((nX>0)&& (nX <m_nBorderWidth) && (nY>this->height() - m_nBorderWidth) && (nY<this->height())){
            *result = HTBOTTOMLEFT;
        }

        if((nX>this->width()-m_nBorderWidth)&& (nX <this->width()) && (nY>this->height() - m_nBorderWidth) && (nY<this->height())){
            *result = HTBOTTOMRIGHT;
        }
        qDebug() << "位于控件上，处理11111111111111111111111111";
        return true;
    }
    }
#endif
//    qDebug() << "非处理事件";
    return QWidget::nativeEvent(eventType, message, result);
}

void FirstShowWidget::on_pushButton_isShowFootMain_clicked()
{

}

void FirstShowWidget::init()
{
    this->setWindowFlags(Qt::Widget | Qt::CustomizeWindowHint | Qt::FramelessWindowHint);//Qt::FramelessWindowHint
    //    this->setContentsMargins(2,2,2,2); //设置QWidget内部内容的边距
//    this->setAttribute(Qt::WA_TranslucentBackground);窗体透明
//    ui->tabWidget_main->setStyleSheet(" background-color:transparent;background-color: rgb(0, 0, 255);QTabWidget::pane{border:none}QTabWidget::pane { border: 0; }");
//    ui->tabWidget_main->setF
//    ui->tabWidget_main->setStyleSheet("QTabWidget::pane { border: 0; }");
//    this->setStyleSheet("QMainWindow::separator {width: 1px; border: none;} ;\n background-color: rgb(170, 0, 0);");
}

void FirstShowWidget::on_pushButton_test_clicked()
{
//    ui->dockWidget->setWindowFlags(Qt::FramelessWindowHint);

//    ui->dockWidget->layout()->setOccupiedSpace(ui->dockWidget->widget());
//    if(ui->dockWidget->isFloating()){
//        ui->dockWidget->setFloating(false);
//    }else{
//        ui->dockWidget->setFloating(true);
//    }

}

