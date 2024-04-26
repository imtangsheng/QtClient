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
    void signals_show_widget_by_name(const QString& name);

private slots:
    void on_Button_HomeWindow_clicked();

    void on_Button_MasterWindow_clicked();

private:
    Ui::HomeWidgets *ui;
};

#endif // HOMEWIDGETS_H
