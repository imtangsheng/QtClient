#ifndef DATAVIEW_H
#define DATAVIEW_H

#include <QWidget>
#include<QDateTime>
#include <QFile>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>
#include <QFileSystemModel>

namespace Ui {
class DataView;
}

class DataView : public QWidget
{
    Q_OBJECT

public:
    explicit DataView(QWidget *parent = nullptr);
    ~DataView();

    struct DataPoint
    {
        //    DataPoint() {}
        QDateTime dataTime;
        double current, voltage,pressure,temperature;
    };

    QVector<DataPoint> dataPoints;

    void parseData(const QString &line);
    bool parseDataFromFile(const QString filePath);


    void init();
    void test();


    QWidget *getDataView();

private slots:
    void on_pushButton_test_clicked();

    void on_pushButton_view_clicked();

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

    void on_lineEdit_rootPath_editingFinished();

private:
    Ui::DataView *ui;

    // 展示在QChartView部件上
    void init_chartView();
    QChartView *m_chartView;
    QChart *m_chart;
    QDateTimeAxis *m_axisTime; //时间轴
    QValueAxis *m_axisY;
    QLineSeries *m_lineSeries_time; //时间
    QLineSeries *m_lineSeries_current; //电流
    QLineSeries *m_lineSeries_voltage; //电压
    QLineSeries *m_lineSeries_pressure; //压强
    QLineSeries *m_lineSeries_temperature; //温度
    QDateTime m_dataTime_lineSeries;

    //
    void init_filesView();
    QFileSystemModel *m_fileModel_dataView;
    QString m_currentFilePathDir;
    void fileModelSelection(QModelIndex index);
    void fileBrowserDoubleClicked(QModelIndex index);

};

#endif // DATAVIEW_H
