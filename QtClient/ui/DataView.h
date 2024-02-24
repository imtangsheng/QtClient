#ifndef DATAVIEW_H
#define DATAVIEW_H

#include <QWidget>
#include<QDateTime>
#include <QFile>
#include <QChartView>
#include <QLineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>

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
    void parseDataFromFile(const QString filePath);


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

private:
    Ui::DataView *ui;

    // 展示在QChartView部件上
    void init_chartView();
    QChartView *m_chartView;
    QChart *m_chart;
    QChart *createLineChart() const;
    QDateTimeAxis *m_axisTime; //时间轴
    QValueAxis *m_axisY;
    QLineSeries *m_lineSeries_time; //时间
    QLineSeries *m_lineSeries_current; //电流
    QLineSeries *m_lineSeries_voltage; //电压
    QLineSeries *m_lineSeries_pressure; //压强
    QLineSeries *m_lineSeries_temperature; //温度
    QDateTime m_dataTime_lineSeries;

};

#endif // DATAVIEW_H
