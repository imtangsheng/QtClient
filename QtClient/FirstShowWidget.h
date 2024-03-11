#ifndef FIRSTSHOWWIDGET_H
#define FIRSTSHOWWIDGET_H

#include <QWidget>

namespace Ui {
class FirstShowWidget;
}

class FirstShowWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FirstShowWidget(QWidget *parent = nullptr);
    ~FirstShowWidget();

    void init();

    bool startAutoLogin();
    bool start();

    QString username;
    QString password;
    bool isAutoLogin;
    bool isSavaPassword;

protected:

private slots:

    void on_pushButton_login_clicked();

private:
    Ui::FirstShowWidget *ui;

signals:
    void loginSuccess();
};

#endif // FIRSTSHOWWIDGET_H
