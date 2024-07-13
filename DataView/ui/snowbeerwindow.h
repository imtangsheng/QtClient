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
#include <QScatterSeries>

#include "modules/FilesUtil.h"

struct struLineSeries
{
    QDateTime time;        // 时间
    QLineSeries *current;     // 电流
    QLineSeries *voltage;     // 电压
    QLineSeries *pressure;    // 压强
    QScatterSeries *scatter;//绘制散点图，用于显示超过阈值的点
    QLineSeries *temperature; // 温度
    struLineSeries() {
        time = QDateTime::currentDateTime();

        current = new QLineSeries();
        current->setName("电流数据曲线(安培)");
        current->setColor(Qt::green);

        voltage = new QLineSeries();
        voltage->setName("电压数据曲线(伏特)");
        voltage->setColor(Qt::cyan);

        pressure = new QLineSeries();
        pressure->setName("气压数据曲线(兆帕MPa)");
        pressure->setColor(Qt::blue);

        temperature = new QLineSeries();
        temperature->setName("温度数据曲线(摄氏度)");
        temperature->setColor(Qt::yellow);

        scatter = new QScatterSeries();
        scatter->setName("阈值");
        scatter->setVisible(true);
        scatter->setPointLabelsFormat("@yPoint");
        scatter->setPointLabelsVisible();
        scatter->setMarkerSize(5);
        scatter->setColor(Qt::red);
    }
};
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
    void test();
    void parseData(const QString &line);
    bool parseDataFromFile(const QString filePath);

    void downloadFilesListFromNetworkLinks(QStringList linksFilesList);

    double max_current;     // 电流
    double max_voltage;     // 电压
    double max_pressure;    // 压强
    double max_temperature; // 温度
    struLineSeries line;
    bool setLineSeries(QDateTime time,double current,double voltage,double pressure,double temperature);

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
