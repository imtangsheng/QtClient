#ifndef MASTERWINDOW_H
#define MASTERWINDOW_H

#include <QMainWindow>
#include "ui_MasterWindow.h"

enum FileDownloadType {
    XLSX,
    CSV,
};

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
    static MasterWindow* getInstance(QWidget *parent = nullptr);

    void start();
    void quit();

    void init_page_robot_system();

    bool saveFileToCSV(const QString &filePath);
    bool saveFileToExcel(const QString &filePath);

private:
    void showEvent(QShowEvent *event);

private slots:
    void on_pushButton_test_clicked();

    void on_tableView_inspection_data_doubleClicked(const QModelIndex &index);

    void on_toolButton_inspection_query_time_clicked();

    void on_toolButton_inspection_query_value_clicked();

    void on_toolButton__inspectionPoints_export_file_download_clicked();

private:
    static MasterWindow* instance;
};

extern MasterWindow *masterWindow;
#endif // MASTERWINDOW_H
