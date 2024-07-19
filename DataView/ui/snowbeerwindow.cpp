#include<QTimer>
#include <QFile>
#include <QTextStream>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringListModel>
#include <QJsonDocument>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QLegendMarker>

#include "snowbeerwindow.h"
#include "ui_snowbeerwindow.h"

//数据示例：2024-02-29 00:12:12 Thursday, time:1709136732.687300S, I:0.000000A, V:3.302259V, P:0.000000MPa, C:17.672727℃, cjkg:0, hxkg:0
// 预先计算需要 chopped 的长度 QString 不是一个字面量类型，它的 size() 方法不是 constexpr 的。编译时不能能够计算出值
//编码：sizeof 操作的是原始字节，而 QString::size() 计算的是 Unicode 字符数。
// sizeof 在编译时计算，QString::size() 在运行时计算。
#include <string_view>
constexpr int Chop_time = static_cast<int>(std::string_view("S").length());//既可以在编译时计算，又能正确处理 Unicode 字符。
constexpr int Chop_current = static_cast<int>(std::string_view("A").length());//既可以在编译时计算，又能正确处理 Unicode 字符。
constexpr int Chop_voltage = static_cast<int>(std::string_view("V").length());
constexpr int Chop_pressure = static_cast<int>(std::string_view("MPa").length());
constexpr int Chop_temperature = static_cast<int>(std::string_view("℃").length());//const int Chop_temperature = QString("℃").size();
Result ChartLine::readDataFromFile(const QString filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return Result(false, "无法打开文件: " + file.errorString());
    }

    QTextStream in(&file);
    QString line;
    qint64 lineCount = 0;
    QTimer updateTimer;
    connect(&updateTimer, &QTimer::timeout, this, [&](){
        emit progressUpdated(lineCount);
    });
    updateTimer.start(updateInterval); // 每3秒更新一次

    // 使用 lambda 函数来处理每个数据点
    auto processDataPoint = [this](qint64 xPosPoint, const QString& strValue, int chopLength, LineEnum lineType, qint64 lineCount) {
        bool ok;
        double doubleValue = strValue.split(':')[1].chopped(chopLength).trimmed().toDouble(&ok);
        if (ok) {
            //qDebug() << lineCount << strValue <<"--"<< xPosPoint<< doubleValue;
            //series[lineType].addPoint(xPosPoint, doubleValue);
            addPoint(lineType,xPosPoint, doubleValue);
        } else {
            qDebug() << QString("行 %1 数据格式错误: %2").arg(lineCount).arg(strValue);
        }
    };

    while (!in.atEnd()) {
        line = in.readLine();
        lineCount++;
        // 使用逗号分割数据
        QStringList parts = line.split(',');
        if (parts.size() >= 6) {
            // 解析时间戳
            bool ok;
            double doubleTime = parts[1].split(':')[1].chopped(Chop_time).trimmed().toDouble(&ok);
            if(!ok){
                qDebug() << QString("行 %1 数据格式错误: %2").arg(lineCount).arg(parts[0]);
                continue;
            }
            //qDebug() << lineCount << parts[1] <<"--"<< doubleTime;
            QDateTime timestamp = QDateTime::fromSecsSinceEpoch(doubleTime, Qt::UTC);
            // QDateTime timestamp = QDateTime::fromString(parts[0].trimmed(), "yyyy-MM-dd hh:mm:ss dddd");
            // if (!timestamp.isValid()) {
            //     qDebug() << QString("行 %1 数据格式错误: %2").arg(lineCount).arg(parts[0]);
            //     continue;
            // }

            qint64 xPosPoint = timestamp.toMSecsSinceEpoch();
            // 处理各种数据点
            if(series[Line_current].isShow)
            processDataPoint(xPosPoint, parts[2], Chop_current, Line_current, lineCount);
            if(series[Line_voltage].isShow)
            processDataPoint(xPosPoint, parts[3], Chop_voltage, Line_voltage, lineCount);
            if(series[Line_pressure].isShow)
            processDataPoint(xPosPoint, parts[4], Chop_pressure, Line_pressure, lineCount);
            if(series[Line_temperature].isShow)
            processDataPoint(xPosPoint, parts[5], Chop_temperature, Line_temperature, lineCount);
        } else {
            qDebug() << "解析失败，数据格式不正确。错误行号:" << lineCount << "内容:" << line;
        }
    }

    file.close();
    updateTimer.stop();
    emit readingFinished();
    return Result(true, QString("成功读取 %1 行数据").arg(lineCount));
}

void ChartLine::run()
{

    QDateTime startTime = QDateTime::currentDateTime();
    qDebug() <<startTime << " -文件：" <<filePath<< QThread::currentThread();

    for (int i = 0; i < Line_Count; ++i) {
        series[static_cast<LineEnum>(i)].clear();
    }

    Result result = readDataFromFile(filePath);
    qDebug() <<result<< result.message;
    if(result){
        for (int i = 0; i < Line_Count; ++i) {
            series[static_cast<LineEnum>(i)].update();
            qDebug() <<"解析i"<< i<< series[static_cast<LineEnum>(i)].line->count();
        }
    }

    qDebug() << "解析时间Run：" << startTime.msecsTo(QDateTime::currentDateTime()) << "毫秒"<<"线程:"<<QThread::currentThread();
}



SnowBeerWindow::SnowBeerWindow(QWidget *parent) :
    QMainWindow(parent),chart(new QChart()), axisTime(new QDateTimeAxis()),axisY(new QValueAxis()),
    ui(new Ui::SnowBeerWindow)
{
    ui->setupUi(this);
    init();
}

SnowBeerWindow::~SnowBeerWindow()
{
    delete ui;
//    delete m_filesUtil;
    qDebug()<<"SnowBeerWindow::~SnowBeerWindow()";
}


void SnowBeerWindow::init()
{
    qDebug()<<"SnowBeerWindow::init()";
    AppSettings.beginGroup(objectName());
    AppJson = AppSettings.value("AppJson",QJsonObject()).toJsonObject();
    AppSettings.endGroup();

    ui->lineEdit_localFiles_dir->setText(AppJson["localFiles_dir"].toString());
    ui->lineEdit_network_url->setText(AppJson["network_url"].toString());

    ui->doubleSpinBox_Max_current->setValue(AppJson["current_max"].toDouble(2.0));
    ui->doubleSpinBox_Max_voltage->setValue(AppJson["voltage_max"].toDouble(5.0));
    ui->doubleSpinBox_Max_pressure->setValue( AppJson["pressure_max"].toDouble(1.5));
    ui->doubleSpinBox_Max_temperature->setValue(AppJson["temperature_max"].toDouble(50.0));

    initChart();
    init_lines_chartView(lines);

    ui->checkBox_current->setCheckState(lines.series[Line_current].isShow ? Qt::Checked : Qt::Unchecked);
    ui->checkBox_voltage->setCheckState(lines.series[Line_voltage].isShow ? Qt::Checked : Qt::Unchecked);
    ui->checkBox_pressure->setCheckState(lines.series[Line_pressure].isShow ? Qt::Checked : Qt::Unchecked);
    ui->checkBox_temperature->setCheckState(lines.series[Line_temperature].isShow ? Qt::Checked : Qt::Unchecked);

    ui->widget->setChart(chart);
    // 在QChartView中显示 设置可以鼠标操作
    // QChartView::NoRubberBand	0x0	未指定缩放区域，因此未启用缩放。
    // QChartView::VerticalRubberBand	0x1	橡皮筋水平锁定到图表的大小，可以垂直拉动以指定缩放区域。
    // QChartView::HorizontalRubberBand	0x2	橡皮筋垂直锁定为图表大小，可以水平拉动以指定缩放区域。
    // QChartView::RectangleRubberBand	0x3	橡皮筋固定在点击的点上，可以垂直和水平拉动。
    ui->widget->setRubberBand(QChartView::RectangleRubberBand);
    init_filesView();

    //test();//测试
}

void SnowBeerWindow::quit()
{
    qDebug()<<"SnowBeerWindow::quit()";
    AppSettings.beginGroup(objectName());
    AppSettings.setValue("AppJson",AppJson);
    AppSettings.endGroup();
}

void SnowBeerWindow::test()
{
    /**添加测试数据**/
    QDateTime currentTime(QDate(2024, 2, 29), QTime(0, 0, 0));
    for (int i = 0; i < Line_Count; ++i) {
        lines.series[static_cast<LineEnum>(i)].clear();
    }
    QRandomGenerator gen;  // 创建随机数生成器
    for(int i=0;i<1000;++i){
        //        series->append(i,20);
        currentTime = currentTime.addSecs(1);
        lines.series[Line_current].addPoint(currentTime.toMSecsSinceEpoch(),1 + gen.bounded(2.0));
        lines.series[Line_voltage].addPoint(currentTime.toMSecsSinceEpoch(),3+ gen.bounded(2.0));
        lines.series[Line_pressure].addPoint(currentTime.toMSecsSinceEpoch(),1+gen.bounded(2.0));
        lines.series[Line_temperature].addPoint(currentTime.toMSecsSinceEpoch(),45+gen.bounded(10.0));
        // for (int i = 0; i < Line_Count; ++i) {
        //     lines.addData(static_cast<LineEnum>(i),currentTime,gen.bounded(50.0));
        // }
    }
    for (int i = 0; i < Line_Count; ++i) {
        lines.series[static_cast<LineEnum>(i)].update();
    }
    autoRangesAxisTime(lines);
}

void SnowBeerWindow::downloadFilesListFromNetworkLinks(QStringList linksFilesList)
{
    qDebug()<<"downloadFilesListFromNetworkLinks(QStringList:"<< linksFilesList;
    foreach (QString link,linksFilesList) {
        qDebug()<<"downloadFilesListFromNetworkLinks(QStringList:"<< link;
        // 定义一个正则表达式模式，用于匹配网络下载链接 Qt::CaseSensitive区分大小写
        FilesUtil *fileItem = new FilesUtil();
        if(fileItem->startDownloadFileFromLink(AppJson["network_url"].toString() + link,\
                                                                                                     AppJson["localFiles_dir"].toString() +link)){
            ui->verticalLayout_download->addWidget(fileItem->getLayoutDownloadFile());
        }
    }
}

void SnowBeerWindow::autoRangesAxisTime(const ChartLine &lineChart)
{

    qDebug()<<"自动调整时间轴 autoRangesAxisTime()";
    QDateTime min; // 初始化为当前时间
    QDateTime max; //
    for (int i = 0; i < Line_Count; ++i) {
        LineEnum lineType = static_cast<LineEnum>(i);
        qDebug()<<lineChart.series[lineType].isShow<<lineChart.series[lineType].line->count();

        if(lineChart.series[lineType].isShow && lineChart.series[lineType].line->count() > 2){
            // 检查其中的一条线即可
            min = QDateTime::fromMSecsSinceEpoch(lineChart.series[lineType].line->at(0).x());
            max = QDateTime::fromMSecsSinceEpoch(lineChart.series[lineType].line->at(lineChart.series[lineType].line->count() - 1).x());
            // 添加一些边距（例如，在两端各添加5%的时间范围）
            qint64 timeRange = max.toMSecsSinceEpoch() - min.toMSecsSinceEpoch();
            qint64 margin = timeRange * 0.05;
            min = min.addMSecs(-margin);
            max = max.addMSecs(margin);
            qDebug()<<"自动调整时间轴 autoRangesAxisTime()"<<min<<max;
            // 设置轴的范围
            axisTime->setRange(min, max);
            break;
        }
    }

}

void SnowBeerWindow::autoRangesAxisY(const ChartLine &lineChart)
{
    qDebug() << "自动调整Y轴 autoRangesAxisY()";
    qreal minY = 0;
    qreal maxY = 1;
    // 遍历所有线条
    for (int i = 0; i < Line_Count; ++i) {
        LineEnum lineType = static_cast<LineEnum>(i);
        if (lineChart.series[lineType].isShow && lineChart.series[lineType].line->count() > 0) {
            // 检查每条可见的线
            for (const QPointF& point : lineChart.series[lineType].line->points()) {
                qreal y = point.y();
                if (y < minY) minY = y;
                if (y > maxY) maxY = y;
            }
        }
    }

    // 添加一些边距（例如，在上下各添加10%的数值范围）
    qreal range = maxY - minY;
    qreal margin = range * 0.1;
    minY -= margin;
    maxY += margin;
    // 设置Y轴的范围
    axisY->setRange(minY, maxY);
    // 应用 nice numbers
    axisY->applyNiceNumbers();
}

void SnowBeerWindow::initChart()
{
    chart->setTitle("数据曲线图示");
    //创建X和Y轴
    axisTime->setTitleText("X轴时间");
    axisTime->setFormat("hh:mm:ss");
    axisY->setTitleText("Y轴数值");

    chart->addAxis(axisTime, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    //在QChartView中显示 设置外边距为 负数，减少空白
    chart->setContentsMargins(-9,-9,-9,-9);
    chart->setBackgroundRoundness(0);
}

void SnowBeerWindow::init_lines_chartView(ChartLine &lineChart)
{
    for (int i = 0; i < Line_Count; ++i) {
        LineEnum lineType = static_cast<LineEnum>(i);
        chart->addSeries(lineChart.series[lineType].line);
        chart->addSeries(lineChart.series[lineType].scatter);
        lineChart.series[lineType].line->attachAxis(axisTime);
        lineChart.series[lineType].line->attachAxis(axisY);
        lineChart.series[lineType].scatter->attachAxis(axisTime);
        lineChart.series[lineType].scatter->attachAxis(axisY);
        // 图例不显示
        // for(QLegendMarker *marker:chart->legend()->markers(lineChart.series[lineType].scatter)){
        //     marker->setVisible(false);
        // }
    }

    chartHideEmptyLegendItems();

    // connect(&lineChart,&ChartLine::progressUpdated,this,[&](int progress){
    //     qDebug()<<"数据解析进度至"<<progress;
    //     ui->label_tips->setText(QString("数据解析进度至1% 行！").arg(progress));
    //     ui->label_tips->show();
    // }, Qt::QueuedConnection);
    connect(&lineChart,&ChartLine::progressUpdated,this,&SnowBeerWindow::parse_file_progress, Qt::QueuedConnection);

    connect(&lineChart,&ChartLine::finished,this,[&](){
        qDebug()<<"数据解析完毕！";
        autoRangesAxisTime(lineChart);
        //autoRangesAxisY(lineChart);//暂不开启Y轴的自动功能
        ui->label_tips->setText("数据解析完毕！");
        QTimer::singleShot(10000, this, [=]() {
            ui->label_tips->hide();
        });
    }, Qt::QueuedConnection);

    // 电流
    lineChart.series[Line_current].line->setName(AppJson["current_name"].toString("电流数据曲线(安培)"));
    lineChart.series[Line_current].setShow(AppJson["current_isShow"].toBool(true));
    lineChart.series[Line_current].max = AppJson["current_max"].toDouble(2.0);
    lineChart.series[Line_current].setColor(AppJson["current_color"].toInt(Qt::green));

    // 电压
    lineChart.series[Line_voltage].line->setName(AppJson["voltage_name"].toString("电压数据曲线(伏特)"));
    lineChart.series[Line_voltage].setShow(AppJson["voltage_isShow"].toBool(true));
    lineChart.series[Line_voltage].max = AppJson["voltage_max"].toDouble(5.0);
    lineChart.series[Line_voltage].setColor(AppJson["voltage_color"].toInt(Qt::cyan));

    // 压强
    lineChart.series[Line_pressure].line->setName(AppJson["pressure_name"].toString("气压数据曲线(兆帕MPa)"));
    lineChart.series[Line_pressure].setShow(AppJson["pressure_isShow"].toBool(true));
    lineChart.series[Line_pressure].max = AppJson["pressure_max"].toDouble(1.5);
    lineChart.series[Line_pressure].setColor(AppJson["pressure_color"].toInt(Qt::blue));

    // 温度
    lineChart.series[Line_temperature].line->setName(AppJson["temperature_name"].toString("温度数据曲线(摄氏度)"));
    lineChart.series[Line_temperature].setShow(AppJson["temperature_isShow"].toBool(true));
    lineChart.series[Line_temperature].max = AppJson["temperature_max"].toDouble(50.0);
    lineChart.series[Line_temperature].setColor(AppJson["temperature_color"].toInt(Qt::yellow));

    // 使用 QComboBox 进行颜色选择
    //     你可以创建一个带有预定义颜色的下拉框。

    //             cpp
    //                 复制
    //                     QComboBox* createColorComboBox()
    // {
    //     QComboBox* comboBox = new QComboBox();
    //     QStringList colorNames = QColor::colorNames();

    //     for (const QString& colorName : colorNames) {
    //         QPixmap pixmap(20, 20);
    //         pixmap.fill(QColor(colorName));
    //         comboBox->addItem(QIcon(pixmap), colorName, QColor(colorName));
    //     }

    //     return comboBox;
    // }

    // // 使用
    // QComboBox* colorCombo = createColorComboBox();
    // connect(colorCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
    //         [this, lineType](int index) {
    //             QColor color = colorCombo->itemData(index).value<QColor>();
    //             series[lineType].line->setColor(color);
    //         });
}

Result SnowBeerWindow::comparsion_lines_by_file(const QString &filePath, const int &type)
{
    qDebug()<<filePath<<type;
    static ComparionChartLine lines_comparison;//折线图
    static bool is_comparison = true;
    if(is_comparison){
    init_lines_chartView(lines_comparison);
    for (int i = 0; i < Line_Count; ++i) {
        LineEnum lineType = static_cast<LineEnum>(i);
        lines_comparison.series[lineType].line->setName("对比-"+lines.series[lineType].line->name()) ;
        lines_comparison.series[lineType].setColor(lines.series[lineType].color + 6);
    }
    is_comparison = false;
    }

    lines_comparison.filePath = filePath;
    lines_comparison.TypeComparion = type;
    switch (type) {
    case 0:{
        //按时间
        lines_comparison.offsetMs_xpos = ui->dateTimeEdit_comparison_axisTime_origin2->dateTime().msecsTo(ui->dateTimeEdit_comparison_axisTime_origin->dateTime());
        qDebug()<<lines_comparison.TypeComparion<<"-时间-"<<lines_comparison.offsetMs_xpos;
        break;}
    case 1:{
        //按类型自动比对，从第一个时间开始自动计算偏移量
        std::optional<qreal> originTime;
        for (int i = 0; i < Line_Count; ++i) {
            const QLineSeries* line = lines.series[static_cast<LineEnum>(i)].line;
            if (line && line->count() > 2) {
                originTime =  line->at(0).x();
                break;
            }
        }
        if (!originTime.has_value()) {
            return Result(false, QString("没有需要的对比的数据"));
        }
        // 在这里继续处理数据
        lines_comparison.isTypeComparion_1_first = true;
        lines_comparison.offsetMs_xpos = originTime.value();
        break;}
    default:
        return Result(false, QString("暂时没有支持的类型可选"));
        break;
    }
    lines_comparison.start();
    lines_comparison.wait();
    return Result(true, QString("成功读取"));
}

void SnowBeerWindow::parse_file_progress(qint64 progress)
{
    //错误 "ASSERT failure in QList::at: "index out of range"
    qDebug()<<"parse_file_progress(qint64 "<<progress;
    //每读取3000行更新一次进度 原来的1000行会导致奔溃，报 QList 超过范围，该值尽量设置大 145毫秒，可能信号发送过快
    if(ui->label_tips->isHidden())  ui->label_tips->show();
    QString strProgress = i2s(progress);
    ui->label_tips->setText("数据解析进度至:" + strProgress);
}


void SnowBeerWindow::init_filesView()
{
    qDebug()<<"init_filesView()";
//    m_filesUtil = new FilesUtil(this->parentWidget());
    m_filesUtil = new FilesUtil();
    // 目录树模型
    m_fileSystemModel = new QFileSystemModel;
//    QStringList filters;
//    filters << "*.txt";
//    m_fileModel_SnowBeerWindow->setNameFilters(filters);
//    m_fileModel_SnowBeerWindow->setNameFilterDisables(false);
//    m_fileModel_SnowBeerWindow->setRootPath(QDir::rootPath());
//    m_fileModel_SnowBeerWindow->setRootPath("http://192.168.1.15:8071");
    m_fileSystemModel->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    // 目录树视图
    ui->treeView_localFiles_files->setModel(m_fileSystemModel);
    ui->treeView_localFiles_files->setWindowTitle("Directories");
    // 设置文件浏览视图的根目录
    ui->lineEdit_localFiles_dir->setText(AppJson["localFiles_dir"].toString());

//    m_fileModel_SnowBeerWindow->setRootPath(m_fileModel_SnowBeerWindow->myComputer().toString());
    m_fileSystemModel->setRootPath(ui->lineEdit_localFiles_dir->text());
    ui->treeView_localFiles_files->setRootIndex(m_fileSystemModel->index(ui->lineEdit_localFiles_dir->text()));
    // 当前目录变化信号
    connect(ui->treeView_localFiles_files->selectionModel(),&QItemSelectionModel::currentChanged,this,&SnowBeerWindow::fileModelSelection);
    connect(ui->treeView_localFiles_files, &QTreeView::doubleClicked,this, &SnowBeerWindow::fileBrowserDoubleClicked);
//    connect(ui->treeView_files->selectionModel(), &QTreeView::doubleClicked,this, &SnowBeerWindow::fileBrowserDoubleClicked);

    // 设置网络浏览视图的根目录
    ui->listView_network_filesList->setWindowTitle("网络浏览视图目录");
    ui->lineEdit_network_url->setText(AppJson["network_url"].toString());
//    m_networkFileModel->setRootPath(ui->lineEdit_rootPath_filesNetwork->text());
//    m_networkFileModel->setRootUrl(ui->lineEdit_rootPath_filesNetwork->text());

    // 假设你有一个名为fileList的QStringList，其中包含了要显示的文件列表数据
    //QString json = "{\"filesList\":[\"测试.txt\", \"2/2.txt\", \"3/3/3.txt\"]}";
    QString json = "";
    QJsonDocument jsonDoc = QJsonDocument::fromJson(json.toUtf8());
    QJsonValue filesListValue = jsonDoc.object().value("filesList");
    QJsonArray filesArray = filesListValue.toArray();
    QStringList fileList;
    for (const QJsonValue& value : filesArray) {
        fileList.append(value.toString());
    }

//    QStringList fileList = {"1.txt", "2/2.txt", "3/3/3.txt"};
    // 创建一个QStringListModel，并将fileList作为构造函数的参数
    m_filesListModelNetwork = new QStringListModel(fileList, this);
    ui->listView_network_filesList->setModel(m_filesListModelNetwork);
//    ui->listView_filesNetwork->setModel(m_networkFileModel);
//    ui->listView_filesNetwork->setRootIndex(m_networkFileModel->index(ui->lineEdit_rootPath_filesNetwork->text()));

    // 显示下载进度
    ui->verticalLayout_download->addWidget(m_filesUtil->getLayoutDownloadFile());
    m_filesUtil->getLayoutDownloadFile()->setVisible(false);
}

void SnowBeerWindow::fileModelSelection(QModelIndex index)
{
    qDebug()<<"fileModelSelection(QModelIndex index)"<<index;
    qDebug()<<index.data(QFileSystemModel::FilePathRole).toString();
    m_currentFilePathDir = index.data(QFileSystemModel::FilePathRole).toString();
}

void SnowBeerWindow::fileBrowserDoubleClicked(QModelIndex index)
{
    qDebug()<<"fileBrowserDoubleClicked(QModelIndex index)"<<index;
    qDebug()<<index.data(QFileSystemModel::FilePathRole).toString();
    m_currentFilePathDir = index.data(QFileSystemModel::FilePathRole).toString();
    //开启线程的数据解析
    lines.filePath = m_currentFilePathDir;
    lines.start();
}

void SnowBeerWindow::on_pushButton_test_clicked()
{
    qDebug()<<"test()"<<QThread::currentThread();
    QDateTime startTime = QDateTime::currentDateTime();
    lines.filePath = "test.txt";
    // lines.parseDataFromFile(lines.filePath);
    lines.start();
    // chart->update();
    // ui->widget->update();
    qDebug() << "解析时间Test：" << startTime.msecsTo(QDateTime::currentDateTime()) << "毫秒"<<"线程:"<<QThread::currentThread();
}

void SnowBeerWindow::on_checkBox_current_stateChanged(int arg1)
{
    qDebug()<<"on_checkBox_current_stateChanged int:"<< arg1;
    lines.series[Line_current].setShow(arg1);
    AppJson["current_isShow"] = arg1 == Qt::Checked ? true: false ;
    chartHideEmptyLegendItems();
}

void SnowBeerWindow::on_checkBox_voltage_stateChanged(int arg1)
{
    AppJson["voltage_isShow"] = arg1 == Qt::Checked ? true: false ;
    switch (arg1) {
    case Qt::Checked:
        lines.series[Line_voltage].setShow(true);
        break;
    case Qt::PartiallyChecked:

        break;
    case Qt::Unchecked:
        lines.series[Line_voltage].setShow(false);
        break;
    default:
        qDebug()<<"error: on_checkBox_*_stateChanged int:"<< arg1;
        break;
    }
    chartHideEmptyLegendItems();
}


void SnowBeerWindow::on_checkBox_pressure_stateChanged(int arg1)
{
    lines.series[Line_pressure].setShow(arg1);
    AppJson["pressure_isShow"] = arg1 == Qt::Checked ? true: false ;
    chartHideEmptyLegendItems();
}


void SnowBeerWindow::on_checkBox_temperature_stateChanged(int arg1)
{
    lines.series[Line_temperature].setShow(arg1);
    AppJson["temperature_isShow"] = arg1 == Qt::Checked ? true: false ;
    chartHideEmptyLegendItems();
}


void SnowBeerWindow::on_themeComboBox_currentIndexChanged(int index)
{
//    Constant	 值	描述
//    QChart::ChartThemeLight	0	浅色主题，这是默认主题。
//    QChart::ChartThemeBlueCerulean	1	蔚蓝主题。
//    QChart::ChartThemeDark	2	黑暗主题。
//    QChart::ChartThemeBrownSand	3	沙褐色主题。
//    QChart::ChartThemeBlueNcs	4	自然色系 （NCS） 蓝色主题。
//    QChart::ChartThemeHighContrast	5	高对比度主题。
//    QChart::ChartThemeBlueIcy	6	冰蓝色的主题。
//    QChart::ChartThemeQt	7	Qt 主题。

    qDebug()<<"on_themeComboBox_currentIndexChanged int:"<< index;
    QString theme = ui->themeComboBox->itemText(index);
    if (theme == "默认主题") {
        // 设置默认主题的样式或设置
        // ...
    } else if (theme == "黑色主题") {
        // 设置黑色主题的样式或设置
        chart->setTheme(QChart::ChartThemeDark);
    } else if (theme == "浅色主题") {
        // 设置浅色主题的样式或设置
        chart->setTheme(QChart::ChartThemeLight);
    }
}


void SnowBeerWindow::on_animatedComboBox_currentIndexChanged(int index)
{
    qDebug()<<"on_animatedComboBox_currentIndexChanged int:"<< index;
    //QChart::NoAnimation	0x0	动画在图表中被禁用。这是默认值。
    //QChart::GridAxisAnimations	0x1	在图表中启用网格轴动画。
    //QChart::SeriesAnimations	0x2	在图表中启用系列动画。
    //QChart::AllAnimations	0x3	图表中启用了所有动画类型。
    chart->setAnimationOptions(QChart::AnimationOptions(index));
}

void SnowBeerWindow::on_legendComboBox_currentIndexChanged(int index)
{
    Qt::Alignment alignment;
    qDebug()<<"on_legendComboBox_currentIndexChanged int:"<< index;
//    qDebug()<<int(Qt::AlignTop) <<int(Qt::AlignBottom)<<int(Qt::AlignLeft)<<int(Qt::AlignRight);
//    Qt::Alignment alignment(ui->legendComboBox->itemData(index).toInt());
//    qDebug()<<alignment<<int(alignment);
//    m_chart->legend()->setAlignment(Qt::Alignment(index));
    switch (index) {
    //
    case 0:
        chart->legend()->hide();
        return;
        break;
    case 1:
        alignment = Qt::AlignTop;
        break;
    case 2:
        alignment = Qt::AlignBottom;
        break;
    case 3:
        alignment = Qt::AlignLeft;
        break;
    case 4:
        alignment = Qt::AlignRight;
        break;
    default:
        return;
        break;
    }
    chart->legend()->setAlignment(alignment);
    chart->legend()->show();

}

void SnowBeerWindow::on_antialiasCheckBox_stateChanged(int arg1)
{
    qDebug()<<"on_antialiasComboBox_currentIndexChanged int:"<< arg1;
//    chart->setRenderHint(QPainter::Antialiasing,  arg1);
    ui->widget->setRenderHint(QPainter::Antialiasing,arg1);
}

void SnowBeerWindow::on_pushButton_zoomOut_clicked()
{
    chart->zoomOut();
}

void SnowBeerWindow::on_pushButton_zoomIn_clicked()
{
    chart->zoomIn();
}

void SnowBeerWindow::on_pushButton_zoomReset_clicked()
{
    chart->zoomReset();
}

void SnowBeerWindow::on_pushButton_axisX_autoRange_reset_clicked()
{
    autoRangesAxisTime(lines);
}

void SnowBeerWindow::on_lineEdit_localFiles_dir_editingFinished()
{
    qDebug()<<ui->lineEdit_localFiles_dir->text()<<AppJson["localFiles_dir"].toString();
    if(!QDir(ui->lineEdit_localFiles_dir->text()).exists()){
        // 路径不存在
        QMessageBox::warning(this, "路径验证", "文件路径不存在!");
    }
    AppJson["localFiles_dir"] = ui->lineEdit_localFiles_dir->text();
    ui->treeView_localFiles_files->setRootIndex(m_fileSystemModel->index(ui->lineEdit_localFiles_dir->text()));
}

void SnowBeerWindow::on_pushButton_axisYsetRange_clicked()
{
    qDebug()<<"on_pushButton_axisYsetRange_clicked():";
    qDebug()<<ui->doubleSpinBox_axisYRangeMin->value();
    qDebug()<<ui->doubleSpinBox_axisYRangeMax->value();
    axisY->setRange(ui->doubleSpinBox_axisYRangeMin->value(),ui->doubleSpinBox_axisYRangeMax->value());
}

void SnowBeerWindow::on_pushButton_setMax_current_clicked()
{
    AppJson["current_max"] = ui->doubleSpinBox_Max_current->value();
    lines.series[Line_current].max = AppJson["current_max"].toDouble(2.0);
}


void SnowBeerWindow::on_pushButton_setMax_voltage_clicked()
{
    AppJson["voltage_max"] = ui->doubleSpinBox_Max_voltage->value();
    lines.series[Line_voltage].max = AppJson["voltage_max"].toDouble(5.0);
}


void SnowBeerWindow::on_pushButton_setMax_pressure_clicked()
{
    AppJson["pressure_max"] = ui->doubleSpinBox_Max_pressure->value();
    lines.series[Line_pressure].max = AppJson["pressure_max"].toDouble(1.5);
}


void SnowBeerWindow::on_pushButton_setMax_temperature_clicked()
{
    AppJson["temperature_max"] = ui->doubleSpinBox_Max_temperature->value();
    lines.series[Line_temperature].max = AppJson["temperature_max"].toDouble(50.0);
}


void SnowBeerWindow::on_pushButton_comparison_getFilePath_clicked()
{
    //QStringList fileExtensions
    QString title= "选择对比的文件";
    QString directory = QDir::homePath();
    // 构建文件过滤器字符串
    // 正确构建文件过滤器字符串
    QString filter = "支持的文件类型 (";
    for (const QString& ext : fileExtensions) {
        filter += "*" + ext + " ";
    }
    filter = filter.trimmed() + ")";

    // 添加所有文件选项
    filter += ";;All Files (*)";
    // QFileDialog dialog(this, title, directory, filter);
    // dialog.setAcceptMode(QFileDialog::AcceptOpen);
    // dialog.setFileMode(QFileDialog::ExistingFile);

    QString filePath = QFileDialog::getOpenFileName(this, title, directory, filter);
    if (!filePath.isEmpty()) {
        ui->lineEdit_comparison_filePath->setText(filePath);
    }
}


void SnowBeerWindow::on_pushButton_comparison_axisTimeOffset_ok_clicked()
{
    // 获取两个 QDateTimeEdit 小部件的时间
    QDateTime originTime = ui->dateTimeEdit_comparison_axisTime_origin->dateTime();
    QDateTime origin2Time = ui->dateTimeEdit_comparison_axisTime_origin2->dateTime();

    // 计算时间差(以毫秒为单位)
    qint64 offsetMs = origin2Time.msecsTo(originTime);
    // 打印时间差信息
    QString timeOffsetStr = "时间偏移量:";

    // 根据时间差计算天、小时、分钟、秒
    int days = 0;
    int hours = 0;
    int minutes = 0;
    int seconds = 0;

    if (offsetMs >= 0) {
        timeOffsetStr += "差";
        days = offsetMs / (1000 * 60 * 60 * 24);
        hours = (offsetMs / (1000 * 60 * 60)) % 24;
        minutes = (offsetMs / (1000 * 60)) % 60;
        seconds = (offsetMs / 1000) % 60;
    } else {
        timeOffsetStr += "多";
        days = (-offsetMs / (1000 * 60 * 60 * 24));
        hours = ((-offsetMs / (1000 * 60 * 60)) % 24);
        minutes = ((-offsetMs / (1000 * 60)) % 60);
        seconds = ((-offsetMs / 1000) % 60);
    }


    if (days != 0) {
        timeOffsetStr += QString("%1天%2时%3分%4秒").arg(days).arg(hours).arg(minutes).arg(seconds);
    } else if (hours != 0) {
        timeOffsetStr += QString("%1时%2分%3秒").arg(hours).arg(minutes).arg(seconds);
    } else if (minutes != 0) {
        timeOffsetStr += QString("%1分%2秒").arg(minutes).arg(seconds);
    } else {
        timeOffsetStr += QString("%1秒").arg(seconds);
    }

    qDebug() <<"计算时间差(以毫秒为单位):"<< offsetMs <<""<< timeOffsetStr;
    ui->label_offsetTime_dateTimeEdit_comparison_axisTime->setText(timeOffsetStr);
}


void SnowBeerWindow::on_pushButton_comparison_start_clicked()
{
    QString filePath = ui->lineEdit_comparison_filePath->text();
    // 检查文件是否存在
    QFile file(filePath);
    if (!file.exists()) {
        QMessageBox::warning(this, "错误", "文件不存在，请检查路径");
        return;
    }
    int type = ui->comboBox_comparison_type->currentIndex();
    Result result = comparsion_lines_by_file(filePath,type);
    //检查结果
    if (!result.success) {
        QMessageBox::warning(this, "错误", "比较失败: " + result.message);
        return;
    }
}

void SnowBeerWindow::on_pushButton_network_url_save_clicked()
{
    QString input = ui->lineEdit_network_url->text();
    if(input.isEmpty()){
        QMessageBox::warning(this,
                             tr("无效的网络路径"),
                             tr("输入的路径为空，请输入网络文件路径"));
        return;
    }
    QUrl url(input);
    if (url.isValid() && !url.isLocalFile() &&
        (url.scheme() == "http" || url.scheme() == "https" || url.scheme() == "ftp")){
        AppJson["network_url"] = input;
    }else
    {
        QMessageBox::warning(this,
                             tr("无效的URL"),
                             tr("请输入有效的网络文件URL (http://, https://, 或 ftp://)"));
        return;
    }
}

void SnowBeerWindow::on_pushButton_localFiles_dir_save_clicked()
{
    QString rootPath = ui->lineEdit_localFiles_dir->text();
    if(rootPath.isEmpty()){
        rootPath = QFileDialog::getExistingDirectory(this,
                                                     tr("选择根目录"),
                                                     QDir::homePath(),
                                                     QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
        if (!rootPath.isEmpty()) {
            ui->lineEdit_localFiles_dir->setText(rootPath);
        } else {
            // 用户取消了选择，你可以在这里处理这种情况
            qDebug() << "用户取消了目录选择";
            return;
        }
    }
    AppJson["localFiles_dir"] = rootPath;
}

void SnowBeerWindow::on_pushButton_network_updateFilesList_clicked()
{
    qDebug()<<"on_pushButton_network_updateFilesList_clicked()"<<ui->lineEdit_network_url->text();

    filesListNetwork = m_filesUtil->getFilesListNetworkPath(ui->lineEdit_network_url->text());
    qDebug()<<"filesListNetwork:"<<filesListNetwork<<filesListNetwork.filter(ui->lineEdit_network_url->text());
    // 创建QStringListModel对象，并设置字符串列表作为数据
    m_filesListModelNetwork->setStringList(filesListNetwork.replaceInStrings(ui->lineEdit_network_url->text(),""));

}

void SnowBeerWindow::on_pushButton_localFiles_updateFilesList_clicked()
{
    qDebug()<<"on_pushButton_localFiles_updateFilesList_clicked:"<<AppJson["localFiles_dir"].toString();
    //    qDebug()<<QRegularExpression(".*(" + fileExtensions.join("|") + ")");
    filesListLocal = m_filesUtil->getFilesListLocalPath(AppJson["localFiles_dir"].toString());
    qDebug()<<"filesListLocal:"<<filesListLocal;
    //使用规则表达式，区分大小，字符匹配
    filesListLocal = filesListLocal.filter(QRegularExpression(".*(" + fileExtensions.join("|") + ")"));
    qDebug()<<"filesListLocal:"<<filesListLocal;
    m_filesListModelNetwork->setStringList(filesListLocal.replaceInStrings(AppJson["localFiles_dir"].toString(),""));
}

void SnowBeerWindow::on_pushButton_network_downloadedFiles_clicked()
{
    filesListDownloaded.clear();
    // 获取a中存在但b中不存在的元素（不区分大小写）
    for (const QString& element : filesListLocal) {
        if(filesListNetwork.contains(element, Qt::CaseInsensitive)){
            filesListDownloaded.append(element);
        }
    }
    m_filesListModelNetwork->setStringList(filesListDownloaded.replaceInStrings(AppJson["localFiles_dir"].toString(),""));

}

void SnowBeerWindow::on_pushButton_network_notDownloadedFiles_clicked()
{
    filesListNotDownloaded.clear();
    // 获取a中存在但b中不存在的元素（不区分大小写）
    for (const QString& element : filesListNetwork) {
        if(!filesListLocal.contains(element, Qt::CaseInsensitive)){
            filesListNotDownloaded.append(element);
        }
    }
    // 创建QStringListModel对象，并设置字符串列表作为数据
    m_filesListModelNetwork->setStringList(filesListNotDownloaded.replaceInStrings(AppJson["network_url"].toString(),""));
}

void SnowBeerWindow::on_pushButton_network_download_allFiles_FromLinks_clicked()
{
    qDebug()<<"on_pushButton_network_download_allFiles_FromLinks_clicked"<<filesListNotDownloaded;
    if(filesListNotDownloaded.isEmpty()){
        QMessageBox::warning(this, "文件下载", "不存在未下载的文件");
        return;
    }
    downloadFilesListFromNetworkLinks(filesListNotDownloaded);
}

void SnowBeerWindow::on_pushButton_network_download_networkFile_clicked()
{
    qDebug()<<"on_pushButton_network_download_networkFile_clicked:"<<ui->listView_network_filesList->currentIndex().data(Qt::DisplayRole).toString();
    // 获取选中的项
    //方法1
    //    QItemSelectionModel *selectionModel = ui->listView_networkFiles_list->selectionModel();
    //    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
    //    for (const QModelIndex &index : selectedIndexes) {
    //        qDebug() << "Selected item:" << index.data(Qt::DisplayRole).toString();
    //    }
    QString selectedText = ui->listView_network_filesList->currentIndex().data(Qt::DisplayRole).toString();
    if(selectedText.isEmpty()){
        QMessageBox::warning(this, "未选中文件", "文件为空!请先选中一个文件");
        return;
    }

    if(!m_filesUtil->getLayoutDownloadFile()->isVisible()){
        m_filesUtil->getLayoutDownloadFile()->setVisible(true);
    }
    m_filesUtil->startDownloadFileFromLink(ui->lineEdit_network_url->text()+selectedText,ui->lineEdit_localFiles_dir->text()+selectedText);

}

void SnowBeerWindow::on_pushButton_localFiles_parseData_byFile_clicked()
{
    qDebug()<<"on_pushButton_parseData_clicked()";
    //    m_currentFilePathDir = index.data(QFileSystemModel::FilePathRole).toString();
    if(m_currentFilePathDir.isEmpty()){
        // 提示窗口
        QMessageBox msgBox(this);
        msgBox.setText("当前路径为空，请在本地文件中选中一个文件");
        // 3秒后自动关闭
        QTimer::singleShot(3000, &msgBox, &QMessageBox::close);
        //        msgBox.exec();
    }else {
        //开启线程的数据解析
        lines.filePath = m_currentFilePathDir;
        lines.start();
    }
}

void SnowBeerWindow::on_pushButton_localFiles_fileDelete_clicked()
{
    QString selectedText = ui->listView_network_filesList->currentIndex().data(Qt::DisplayRole).toString();
    if(selectedText.isEmpty()){
        QMessageBox::warning(this, "未选中文件", "文件为空!请先选中一个文件");
        return;
    }
    QString filename = ui->lineEdit_localFiles_dir->text()+selectedText;
    if(QFile::exists(filename)){
        QString alreadyExists =  ui->lineEdit_localFiles_dir->text().isEmpty()
                                    ? tr("There already exists a file called %1. 确定删除?")
                                    : tr("There already exists a file called %1 in the current directory. "
                                         "确定删除?");
        QMessageBox::StandardButton response = QMessageBox::question(nullptr,
                                                                     tr("确定删除 Existing File"),
                                                                     alreadyExists.arg(QDir::toNativeSeparators(filename)),
                                                                     QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if (response == QMessageBox::No)
            return ;
        QFile::remove(filename);
    }else {
        QMessageBox::warning(this, "文件不存在", "文件不存在!请选中一个存在的文件！");
    }
}

void SnowBeerWindow::on_listView_network_filesList_clicked(const QModelIndex &index)
{
    qDebug()<<"on_listView_filesNetwork_clicked(QModelIndex index)"<<index<<index.row();
    qDebug()<<index.data(Qt::DisplayRole).toString();
    m_currentFilePathDir = AppJson["localFiles_dir"].toString() + index.data(Qt::DisplayRole).toString();

}

void SnowBeerWindow::on_listView_network_filesList_doubleClicked(const QModelIndex &index)
{
    qDebug()<<"on_listView_filesNetwork_doubleClicked(QModelIndex index)"<<index<<index.row();
    qDebug()<<index.data(Qt::DisplayRole).toString();
    m_currentFilePathDir = AppJson["localFiles_dir"].toString() + index.data(Qt::DisplayRole).toString();
}
