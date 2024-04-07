#ifndef HOMEWIDGETS_H
#define HOMEWIDGETS_H

#include <QWidget>

namespace Ui {
class HomeWidgets;
}

class HomeWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit HomeWidgets(QWidget *parent = nullptr);
    ~HomeWidgets();
    void init();
    void quit();

signals:
    void homeMune_jump_TabWidget();

private slots:
   void on_Button_homeWindow_clicked();

private:
    Ui::HomeWidgets *ui;
};

#endif // HOMEWIDGETS_H
