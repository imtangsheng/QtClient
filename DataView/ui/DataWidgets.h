#ifndef DATAWIDGETS_H
#define DATAWIDGETS_H

#include <QWidget>

namespace Ui {
class DataWidgets;
}

class DataWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit DataWidgets(QWidget *parent = nullptr);
    ~DataWidgets();

    Ui::DataWidgets *ui;
    void init();
    void quit();

signals:
    void homeMune_jump_TabWidget();

private slots:
    void on_Button_dataView_clicked();

private:

};

#endif // DATAWIDGETS_H
