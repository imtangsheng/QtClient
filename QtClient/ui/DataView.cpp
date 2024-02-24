#include "DataView.h"
#include "ui_DataView.h"

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFile>
#include <QTextStream>

QRegularExpression REGEX_DATA("^(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}).*time:(\\d+\\.\\d+)S.*I:([-+]?\\d+\\.\\d+)A.*V:([-+]?\\d+\\.\\d+)V.*P:([-+]?\\d+\\.\\d+)MPa.*C:([-+]?\\d+\\.\\d+)℃$");
#include <QRandomGenerator>

DataView::DataView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataView)
{
    ui->setupUi(this);

    init();
}

DataView::~DataView()
{
    qDebug()<<"~DataView()";
    delete ui;
}


void DataView::parseDataFromFile(const QString filePath)
{
    qDebug() << "parseDataFromFile文件：" <<filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "无法打开文件：" <<filePath<< file.errorString();
        return ;
    }

    QTextStream in(&file);
//    QList<DataPoint> dataPoints;
    QVector<DataPoint> dataPoints;
    QDateTime startTime = QDateTime::currentDateTime();

    m_lineSeries_current->clear();
    m_lineSeries_voltage->clear();
    m_lineSeries_pressure->clear();
    m_lineSeries_temperature->clear();

    while (!in.atEnd()) {
        QString line = in.readLine();
        QRegularExpressionMatch match = REGEX_DATA.match(line);
        if (match.hasMatch()) {
            m_dataTime_lineSeries = QDateTime::fromString(match.captured(1), "yyyy-MM-dd hh:mm:ss");
            m_lineSeries_current->append(m_dataTime_lineSeries.toMSecsSinceEpoch(),match.captured(3).toDouble());
            m_lineSeries_voltage->append(m_dataTime_lineSeries.toMSecsSinceEpoch(),match.captured(4).toDouble());
            m_lineSeries_pressure->append(m_dataTime_lineSeries.toMSecsSinceEpoch(),match.captured(5).toDouble());
            m_lineSeries_temperature->append(m_dataTime_lineSeries.toMSecsSinceEpoch(),match.captured(6).toDouble());
//            qDebug() << "日期和时间：" << m_dataTime_lineSeries.toString("yyyy-MM-dd hh:mm:ss");
//            qDebug() << "电流：" << match.captured(3).toDouble() << "安培";
//            qDebug() << "电压：" << match.captured(4).toDouble() << "伏特";
//            qDebug() << "压力：" << match.captured(5).toDouble() << "兆帕（MPa）";
//            qDebug() << "温度：" << match.captured(6).toDouble() << "摄氏度";
//            qDebug() << "------------------------";
        }else {
            qDebug()<<"match.hasMatch() error:"<<in.pos()<<"-line:"<<line;
        }
    }
    file.close();
    QDateTime endTime = QDateTime::currentDateTime();
    qint64 elapsedTime = startTime.secsTo(endTime);
    qDebug() << "解析时间：" << elapsedTime << "秒";
    // 创建图表和曲线
    // 创建图表视图并显示

    m_axisTime->setRange(
        QDateTime::fromMSecsSinceEpoch(m_lineSeries_temperature->at(0).x()),
        QDateTime::fromMSecsSinceEpoch(m_lineSeries_temperature->at(m_lineSeries_temperature->count() -1 ).x()));
    m_axisY->setRange(-20,50);
    m_chartView->update();

}

void DataView::init()
{
    qDebug()<<"DataView::init()";
    init_chartView();
//    test();

}

void DataView::test()
{
    QString line = "2024-01-15 09:09:22 Monday, time:1705280962.602193S, I:0.000000A, V:3.061000V, P:0.000000MPa, C:-14.836364℃";
    QRegularExpression regex("^(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}).*time:(\\d+\\.\\d+)S.*I:([-+]?\\d+\\.\\d+)A.*V:([-+]?\\d+\\.\\d+)V.*P:([-+]?\\d+\\.\\d+)MPa.*C:([-+]?\\d+\\.\\d+)℃$");
    QRegularExpressionMatch match = regex.match(line);
    qDebug() << match.hasMatch();
    if (match.hasMatch()) {
        DataPoint dataPoint;
        dataPoint.dataTime = QDateTime::fromString(match.captured(1), "yyyy-MM-dd hh:mm:ss");
        dataPoint.current = match.captured(3).toDouble();
        dataPoint.voltage = match.captured(4).toDouble();
        dataPoint.pressure = match.captured(5).toDouble();
        dataPoint.temperature = match.captured(6).toDouble();

        qDebug() << "日期和时间：" << dataPoint.dataTime.toString("yyyy-MM-dd hh:mm:ss");
        qDebug() << "电流：" << dataPoint.current << "安培";
        qDebug() << "电压：" << dataPoint.voltage << "伏特";
        qDebug() << "压力：" << dataPoint.pressure << "兆帕（MPa）";
        qDebug() << "温度：" << dataPoint.temperature << "摄氏度";
    }

}

QWidget *DataView::getDataView()
{
    return ui->widget_dataView;
}

void DataView::on_pushButton_test_clicked()
{
    qDebug()<<"test()";
    parseDataFromFile("dataView.txt");

}

void DataView::init_chartView()
{
    m_chart = new QChart();
    m_chart->setTitle("数据曲线图示");
    // 创建X轴和Y轴
//    QDateTimeAxis *m_axisTime = new QDateTimeAxis();
    m_axisTime = new QDateTimeAxis();
    m_axisTime->setTitleText("X轴时间");
//    m_axisTime->setFormat("hh:mm:ss"); // 设置时间格式
    m_axisTime->setFormat("yyyy-MM-dd hh:mm:ss");
    //    axisX->setTitleVisible(true);
    //    axisX->setLineVisible(true);
    //    axisX->setLinePen(pen);

//    QValueAxis *m_axisY = new QValueAxis();
    m_axisY = new QValueAxis();
    m_axisY->setTitleText("Y轴数值");

    m_lineSeries_current = new QLineSeries();
    m_lineSeries_current->setName("电流数据曲线(安培)");
    m_lineSeries_current->setColor(Qt::green);
    m_lineSeries_current->setVisible(ui->checkBox_current->checkState()== Qt::Checked);
    qDebug()<<"checkState"<<ui->checkBox_current->checkState()<<(ui->checkBox_current->checkState()== Qt::Checked);

    m_lineSeries_voltage = new QLineSeries();
    m_lineSeries_voltage->setName("电压数据曲线(伏特)");
    m_lineSeries_voltage->setColor("#ffaa00");
    m_lineSeries_voltage->setVisible(ui->checkBox_voltage->checkState()== Qt::Checked);

    m_lineSeries_pressure = new QLineSeries();
    m_lineSeries_pressure->setName("压强数据曲线(兆帕MPa)");
    m_lineSeries_pressure->setColor(Qt::blue);
    m_lineSeries_pressure->setVisible(ui->checkBox_pressure->checkState()== Qt::Checked);

    m_lineSeries_temperature = new QLineSeries();
    m_lineSeries_temperature->setName("温度数据曲线(摄氏度)");
    m_lineSeries_temperature->setColor(Qt::red);
    m_lineSeries_temperature->setVisible(ui->checkBox_temperature->checkState()== Qt::Checked);

    /**添加测试数据**/
    QDateTime currentTime = QDateTime::currentDateTime();
    for(int i=0;i<100;i++){
        //        series->append(i,20);
        currentTime = currentTime.addSecs(1);
        m_lineSeries_current->append(currentTime.toMSecsSinceEpoch(), 0.0);
        m_lineSeries_voltage->append(currentTime.toMSecsSinceEpoch(),5.0);
        m_lineSeries_pressure->append(currentTime.toMSecsSinceEpoch(),QRandomGenerator::global()->bounded(10, 20));
        m_lineSeries_temperature->append(currentTime.toMSecsSinceEpoch(),QRandomGenerator::global()->bounded(0, 40));
//        qDebug()<<i<<currentTime;
    }
    /**添加测试数据end**/

    m_chart->addSeries(m_lineSeries_current);
    m_chart->addSeries(m_lineSeries_voltage);
    m_chart->addSeries(m_lineSeries_pressure);
    m_chart->addSeries(m_lineSeries_temperature);
    m_chart->addAxis(m_axisTime, Qt::AlignBottom);
    m_chart->addAxis(m_axisY, Qt::AlignLeft);

    m_lineSeries_current->attachAxis(m_axisTime);
    m_lineSeries_current->attachAxis(m_axisY);
    m_lineSeries_voltage->attachAxis(m_axisTime);
    m_lineSeries_voltage->attachAxis(m_axisY);
    m_lineSeries_pressure->attachAxis(m_axisTime);
    m_lineSeries_pressure->attachAxis(m_axisY);
    m_lineSeries_temperature->attachAxis(m_axisTime);
    m_lineSeries_temperature->attachAxis(m_axisY);


    m_axisTime->setRange(
        QDateTime::fromMSecsSinceEpoch(m_lineSeries_temperature->at(0).x()),
        QDateTime::fromMSecsSinceEpoch(m_lineSeries_temperature->at(m_lineSeries_temperature->count() -1 ).x()));
    m_axisY->setRange(0,50);
//    m_chart->createDefaultAxes();

    //在QChartView中显示
    m_chartView = new QChartView(m_chart);
    m_chartView->setFrameShape(QFrame::NoFrame);
    m_chartView->setFrameShadow(QFrame::Plain);
    //    m_chartView->setRubberBand(QChartView::RectangleRubberBand); //水平方向缩   chart->zoomIn()
    m_chartView->setRubberBand(QChartView::HorizontalRubberBand);
    ui->gridLayout_chartView->addWidget(m_chartView,0,0);
//    m_chart->setContentsMargins(0,0,0,0);
//    m_chartView->setContentsMargins(0,0,0,0);
//    ui->gridLayout_chartView->setContentsMargins(0, 0, 0, 0); // 设置布局的边距为0

}


QChart *DataView::createLineChart() const
{
    QChart *chart = new QChart();
    chart->setTitle("Line chart数据曲线");
    QString name("Series ");

    // 设置线条颜色
    QPen pen(Qt::red); // 设置为红色

    // 创建X轴和Y轴
    QDateTimeAxis *axisX = new QDateTimeAxis();
    axisX->setTitleText("X轴时间");
    axisX->setFormat("hh:mm:ss"); // 设置时间格式
//    axisX->setTitleVisible(true);
//    axisX->setLineVisible(true);
//    axisX->setLinePen(pen);



    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("Y轴数值");


    QLineSeries *series = new QLineSeries();
    series->setName("随机温度数据曲线");
    QLineSeries *series2 = new QLineSeries();
    series2->setName("数据曲线2");


    QDateTime currentTime = QDateTime::currentDateTime();
    for(int i=0;i<100;i++){

//        series->append(i,20);
        currentTime = currentTime.addSecs(1);

        series->append(currentTime.toMSecsSinceEpoch(), 30);
        series2->append(currentTime.toMSecsSinceEpoch(),QRandomGenerator::global()->bounded(10, 50));

        qDebug()<<i<<currentTime;
    }

    chart->addSeries(series);
    chart->addSeries(series2);
    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->setPen(QPen(Qt::yellow));
    series->setPointLabelsColor(Qt::black);
    // 将数据曲线与轴关联
    series->attachAxis(axisX);
    series->attachAxis(axisY);
    series->setName("数据曲线");
    series->setPointLabelsVisible(true);

//    series->setPointLabelsFormat("@yName");

    series2->setName("test2");
    series2->setColor(Qt::green);

//    series2->setPointLabelsColor(Qt::red);
    series2->attachAxis(axisX);
    series2->attachAxis(axisY);


    qDebug()<<"chart:"<<chart;
    axisX->setRange(
        QDateTime::fromMSecsSinceEpoch(series->at(0).x()),
        QDateTime::fromMSecsSinceEpoch(series->at(series->count() -1 ).x()));
    axisY->setRange(0,50);

//    chart->createDefaultAxes();
//    chart->legend()->setVisible(true);


    return chart;
}


void DataView::on_pushButton_view_clicked()
{
    qDebug()<<"on_pushButton_view_clicked:";
    m_lineSeries_current->clear();
    m_lineSeries_voltage->clear();
    m_lineSeries_pressure->clear();
    m_lineSeries_temperature->clear();
    QDateTime currentTime = QDateTime::currentDateTime();
    currentTime = currentTime.addSecs(1000000);
    for(int i=0;i<100;i++){
        //        series->append(i,20);
        currentTime = currentTime.addSecs(1);
        m_lineSeries_current->append(currentTime.toMSecsSinceEpoch(), 0.0);
        m_lineSeries_voltage->append(currentTime.toMSecsSinceEpoch(),15.0);
        m_lineSeries_pressure->append(currentTime.toMSecsSinceEpoch(),QRandomGenerator::global()->bounded(10, 20));
        m_lineSeries_temperature->append(currentTime.toMSecsSinceEpoch(),QRandomGenerator::global()->bounded(0, 40));
        qDebug()<<i<<currentTime;
    }

    m_axisTime->setRange(
        QDateTime::fromMSecsSinceEpoch(m_lineSeries_temperature->at(0).x()),
        QDateTime::fromMSecsSinceEpoch(m_lineSeries_temperature->at(m_lineSeries_temperature->count() -1 ).x()));
//    m_chart->update();
    m_chartView->update();
}


void DataView::on_checkBox_current_stateChanged(int arg1)
{
    qDebug()<<"on_checkBox_current_stateChanged int:"<< arg1;
    m_lineSeries_current->setVisible(arg1);

}


void DataView::on_checkBox_voltage_stateChanged(int arg1)
{
    switch (arg1) {
    case Qt::Checked:
        m_lineSeries_voltage->setVisible(true);
        break;
    case Qt::PartiallyChecked:

        break;
    case Qt::Unchecked:
        m_lineSeries_voltage->setVisible(false);
        break;
    default:
        qDebug()<<"error: on_checkBox_*_stateChanged int:"<< arg1;
        break;
    }
}


void DataView::on_checkBox_pressure_stateChanged(int arg1)
{
    m_lineSeries_pressure->setVisible(arg1);
}


void DataView::on_checkBox_temperature_stateChanged(int arg1)
{
    m_lineSeries_temperature->setVisible(arg1);
}

