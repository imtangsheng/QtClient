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

protected:
    int test1 = 0;
    int m_nBorderWidth = 10;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result);
private slots:
    void on_pushButton_isShowFootMain_clicked();

    void on_pushButton_test_clicked();

private:
    Ui::FirstShowWidget *ui;
};

#endif // FIRSTSHOWWIDGET_H
