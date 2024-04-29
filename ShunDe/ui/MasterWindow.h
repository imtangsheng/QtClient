#ifndef MASTERWINDOW_H
#define MASTERWINDOW_H

#include <QMainWindow>
#include "ui_MasterWindow.h"

namespace Ui {
class MasterWindow;
}

class MasterWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MasterWindow(QWidget *parent = nullptr);
    ~MasterWindow();
    Ui::MasterWindow *ui;

    void start();
    void quit();

    void init_page_robot_system();

private slots:
    void on_pushButton_test_clicked();

    void on_tableView_inspection_data_doubleClicked(const QModelIndex &index);

    void on_toolButton_inspection_query_time_clicked();

private:

};

extern MasterWindow *masterWindow;
#endif // MASTERWINDOW_H
