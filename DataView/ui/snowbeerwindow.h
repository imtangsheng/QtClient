#ifndef SNOWBEERWINDOW_H
#define SNOWBEERWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QDateTime>
#include <QFile>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QFileSystemModel>
#include <QStringListModel>

#include "modules/FilesUtil.h"
namespace Ui {
class SnowBeerWindow;
}

class SnowBeerWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SnowBeerWindow(QWidget *parent = nullptr);
    ~SnowBeerWindow();

    void init();
    void quit();
    void parseData(const QString &line);
    bool parseDataFromFile(const QString filePath);

    void downloadFilesListFromNetworkLinks(QStringList linksFilesList);

public slots:
    void on_lineEdit_rootPath_editingFinished();

private slots:
    void on_pushButton_test_clicked();

    void on_checkBox_current_stateChanged(int arg1);

    void on_checkBox_voltage_stateChanged(int arg1);

    void on_checkBox_pressure_stateChanged(int arg1);

    void on_checkBox_temperature_stateChanged(int arg1);

    void on_themeComboBox_currentIndexChanged(int index);

    void on_animatedComboBox_currentIndexChanged(int index);

    void on_legendComboBox_currentIndexChanged(int index);

    void on_antialiasCheckBox_stateChanged(int arg1);

    void on_pushButton_zoomOut_clicked();

    void on_pushButton_zoomIn_clicked();

    void on_pushButton_zoomReset_clicked();

    void on_pushButton_downloadNetworkFile_clicked();

    void on_pushButton_axisYsetRange_clicked();

    void on_pushButton_updateNetworkFiles_clicked();

    void on_lineEdit_rootPath_filesNetwork_editingFinished();

    void on_pushButton_updateLocalFiles_clicked();

    void on_pushButton_parseData_clicked();

    void on_listView_filesNetwork_clicked(const QModelIndex &index);

    void on_listView_filesNetwork_doubleClicked(const QModelIndex &index);

    void on_pushButton_downloadedFiles_clicked();

    void on_pushButton_notDownloadedFiles_clicked();

    void on_pushButton_downloadFilesFronLinks_clicked();

    void on_pushButton_fileDelete_clicked();

private:
    Ui::SnowBeerWindow *ui;

    // 展示在QChartView部件上
    void init_chartView();
    QChartView *m_chartView;
    QChart *m_chart;
    QDateTimeAxis *m_axisTime; // 时间轴
    QValueAxis *m_axisY;
    QLineSeries *m_lineSeries_time;        // 时间
    QLineSeries *m_lineSeries_current;     // 电流
    QLineSeries *m_lineSeries_voltage;     // 电压
    QLineSeries *m_lineSeries_pressure;    // 压强
    QLineSeries *m_lineSeries_temperature; // 温度
    QDateTime m_dataTime_lineSeries;

    //
    FilesUtil *m_filesUtil;

    void init_filesView();
    QStringList fileExtensions = QString(".txt,.TXT,.csv,.CSV").split(',');

    QStringListModel *m_filesListModelNetwork;

    QFileSystemModel *m_fileSystemModel;
    QString m_currentFilePathDir;
    void fileModelSelection(QModelIndex index);
    void fileBrowserDoubleClicked(QModelIndex index);

    QStringList filesListLocal;
    QStringList filesListNetwork;
    QStringList filesListDownloaded;
    QStringList filesListNotDownloaded;
};

#endif // SNOWBEERWINDOW_H
