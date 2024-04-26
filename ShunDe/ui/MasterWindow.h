#ifndef MASTERWINDOW_H
#define MASTERWINDOW_H

#include <QMainWindow>

namespace Ui {
class MasterWindow;
}

class MasterWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MasterWindow(QWidget *parent = nullptr);
    ~MasterWindow();

    void init();
    void quit();

private slots:
    void on_pushButton_test_clicked();

    void on_tableView_inspection_data_doubleClicked(const QModelIndex &index);

    void on_toolButton_inspection_query_time_clicked();

private:
    Ui::MasterWindow *ui;
};

#endif // MASTERWINDOW_H
