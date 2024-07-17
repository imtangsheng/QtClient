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

#include "snowbeerwindow.h"
#include "ui_snowbeerwindow.h"

void ChartLine::autoRangesAxisTime()
{
    QDateTime min = QDateTime::currentDateTime(); // 初始化为当前时间
    QDateTime max = QDateTime::fromMSecsSinceEpoch(0); // 初始化为Unix纪元开始时间
    for (int i = 0; i < Line_Count; ++i) {
        LineEnum lineType = static_cast<LineEnum>(i);
        if(series[lineType].isShow && series[lineType].line->count() > 0){
            // 检查每条线的第一个点
            QDateTime firstPoint = QDateTime::fromMSecsSinceEpoch(series[lineType].line->at(0).x());
            if (firstPoint < min) {
                min = firstPoint;
            }
            // 检查每条线的最后一个点
            QDateTime lastPoint = QDateTime::fromMSecsSinceEpoch(series[lineType].line->at(series[lineType].line->count() - 1).x());
            if (lastPoint > max) {
                max = lastPoint;
            }

        }
    }
    // 添加一些边距（例如，在两端各添加5%的时间范围）
    qint64 timeRange = max.toMSecsSinceEpoch() - min.toMSecsSinceEpoch();
    qint64 margin = timeRange * 0.05;
    min = min.addMSecs(-margin);
    max = max.addMSecs(margin);

    // 设置轴的范围
    axisTime->setRange(min, max);
}

void ChartLine::autoRangesAxisY()
{
    qreal minY = 0;
    qreal maxY = 50;
    // 遍历所有线条
    for (int i = 0; i < Line_Count; ++i) {
        LineEnum lineType = static_cast<LineEnum>(i);
        if (series[lineType].isShow && series[lineType].line->count() > 0) {
            // 遍历该线条的所有点
            for (const QPointF& point : series[lineType].line->points()) {
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

void ChartLine::initChart()
{
    chart->setTitle("数据曲线图示");
    //创建X和Y轴
    axisTime->setTitleText("X轴时间");
    axisTime->setFormat("hh:mm:ss");
    axisY->setTitleText("Y轴数值");

    chart->addAxis(axisTime, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);

    for (int i = 0; i < Line_Count; ++i) {
        LineEnum lineType = static_cast<LineEnum>(i);
        chart->addSeries(series[lineType].line);
        chart->addSeries(series[lineType].scatter);
        series[lineType].line->attachAxis(axisTime);
        series[lineType].line->attachAxis(axisY);
        series[lineType].scatter->attachAxis(axisTime);
        series[lineType].scatter->attachAxis(axisY);
        // 图例不显示
        // for(QLegendMarker *marker:chart->legend()->markers(series[lineType].point)){
        //     marker->setVisible(false);
        // }
    }

    //在QChartView中显示 设置外边距为 负数，减少空白
    chart->setContentsMargins(-9,-9,-9,-9);
    chart->setBackgroundRoundness(0);
}

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

    // 使用 lambda 函数来处理每个数据点
    auto processDataPoint = [this](qint64 xPosPoint, const QString& strValue, int chopLength, int lineType, qint64 lineCount) {
        bool ok;
        double doubleValue = strValue.split(':')[1].chopped(chopLength).trimmed().toDouble(&ok);
        if (ok) {
            //qDebug() << lineCount << strValue <<"--"<< xPosPoint<< doubleValue;
            series[lineType].addPoint(xPosPoint, doubleValue);
        } else {
            qDebug() << QString("行 %1 数据格式错误: %2").arg(lineCount).arg(strValue);
        }
    };

    while (!in.atEnd()) {
        line = in.readLine();
        lineCount++;
        // 每读取1000行更新一次进度
        if (lineCount % 1000 == 0) {
            emit progressUpdated(lineCount);
        }
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
        autoRangesAxisTime();//自动调整时间轴
    }
    qDebug() << "解析时间Run：" << startTime.msecsTo(QDateTime::currentDateTime()) << "毫秒"<<"线程:"<<QThread::currentThread();
}



SnowBeerWindow::SnowBeerWindow(QWidget *parent) :
    QMainWindow(parent),
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

    ui->doubleSpinBox_Max_current->setValue( AppJson["current_max"].toDouble(2.0));
    ui->doubleSpinBox_Max_voltage->setValue(AppJson["voltage_max"].toDouble(5.0));
    ui->doubleSpinBox_Max_pressure->setValue( AppJson["pressure_max"].toDouble(1.5));
    ui->doubleSpinBox_Max_temperature->setValue(AppJson["temperature_max"].toDouble(50.0));

    init_lines_chartView(lines);

    ui->checkBox_current->setCheckState(lines.series[Line_current].isShow ? Qt::Checked : Qt::Unchecked);
    ui->checkBox_voltage->setCheckState(lines.series[Line_voltage].isShow ? Qt::Checked : Qt::Unchecked);
    ui->checkBox_pressure->setCheckState(lines.series[Line_pressure].isShow ? Qt::Checked : Qt::Unchecked);
    ui->checkBox_temperature->setCheckState(lines.series[Line_temperature].isShow ? Qt::Checked : Qt::Unchecked);

    ui->widget->setChart(lines.chart);
    // 在QChartView中显示 设置可以鼠标操作
    // QChartView::NoRubberBand	0x0	未指定缩放区域，因此未启用缩放。
    // QChartView::VerticalRubberBand	0x1	橡皮筋水平锁定到图表的大小，可以垂直拉动以指定缩放区域。
    // QChartView::HorizontalRubberBand	0x2	橡皮筋垂直锁定为图表大小，可以水平拉动以指定缩放区域。
    // QChartView::RectangleRubberBand	0x3	橡皮筋固定在点击的点上，可以垂直和水平拉动。
    ui->widget->setRubberBand(QChartView::RectangleRubberBand);
    init_filesView();

    test();
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
        currentTime = currentTime.addSecs(10);
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

    lines.autoRangesAxisTime();
}

void SnowBeerWindow::downloadFilesListFromNetworkLinks(QStringList linksFilesList)
{
    qDebug()<<"downloadFilesListFromNetworkLinks(QStringList:"<< linksFilesList;
//    QStringList linksFilesLists = QString("1.txt,2.csv,3.txt").split(',');
//    int key = 0;
    foreach (QString link,linksFilesList) {
        qDebug()<<"downloadFilesListFromNetworkLinks(QStringList:"<< link;
        // 定义一个正则表达式模式，用于匹配网络下载链接 Qt::CaseSensitive区分大小写
        FilesUtil *fileItem = new FilesUtil();
        if(fileItem->startDownloadFileFromLink(AppJson["pathSnowBeerWindowNetwork"].toString() + link,\
                                                                                                     AppJson["pathSnowBeerWindow"].toString() +link)){
            ui->verticalLayout_download->addWidget(fileItem->getLayoutDownloadFile());
        }

//        SUB_WINDOW->ui->gridLayout->addWidget(fileItem->getLayoutDownloadFile(),key,0);key++;
//        SUB_WINDOW->ui->gridLayout->addWidget(fileItem->getLayoutDownloadFile());
//        QHBoxLayout *layout = new QHBoxLayout;
//        QPushButton *test = new QPushButton;
    }
//    SUB_MAIN->setWindowTitle("下载");
//    SUB_MAIN->setCentralWidget(SUB_MAIN->ui->widget_download);
//    SUB_MAIN->show();
}

void SnowBeerWindow::init_lines_chartView(ChartLine &lineChart)
{
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
    ui->treeView_files->setModel(m_fileSystemModel);
    ui->treeView_files->setWindowTitle("Directories");
    // 设置文件浏览视图的根目录
    ui->lineEdit_rootPath->setText(AppJson["pathSnowBeerWindow"].toString());

//    m_fileModel_SnowBeerWindow->setRootPath(m_fileModel_SnowBeerWindow->myComputer().toString());
    m_fileSystemModel->setRootPath(ui->lineEdit_rootPath->text());
    ui->treeView_files->setRootIndex(m_fileSystemModel->index(ui->lineEdit_rootPath->text()));
    // 当前目录变化信号
    connect(ui->treeView_files->selectionModel(),&QItemSelectionModel::currentChanged,this,&SnowBeerWindow::fileModelSelection);
    connect(ui->treeView_files, &QTreeView::doubleClicked,this, &SnowBeerWindow::fileBrowserDoubleClicked);
//    connect(ui->treeView_files->selectionModel(), &QTreeView::doubleClicked,this, &SnowBeerWindow::fileBrowserDoubleClicked);

    // 设置网络浏览视图的根目录
    ui->listView_filesNetwork->setWindowTitle("Directories Network");
    ui->lineEdit_rootPath_filesNetwork->setText(AppJson["pathSnowBeerWindowNetwork"].toString());
//    m_networkFileModel->setRootPath(ui->lineEdit_rootPath_filesNetwork->text());
//    m_networkFileModel->setRootUrl(ui->lineEdit_rootPath_filesNetwork->text());

    // 假设你有一个名为fileList的QStringList，其中包含了要显示的文件列表数据
    QString json = "{\"filesList\":[\"1.txt\", \"2/2.txt\", \"3/3/3.txt\"]}";
//    QString json = "";
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
    ui->listView_filesNetwork->setModel(m_filesListModelNetwork);
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

void SnowBeerWindow::on_checkBox_current_stateChanged(int arg1)
{
    qDebug()<<"on_checkBox_current_stateChanged int:"<< arg1;
    lines.series[Line_current].setShow(arg1);
    AppJson["current_isShow"] = arg1 == Qt::Checked ? true: false ;
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
}


void SnowBeerWindow::on_checkBox_pressure_stateChanged(int arg1)
{
    lines.series[Line_pressure].setShow(arg1);
    AppJson["pressure_isShow"] = arg1 == Qt::Checked ? true: false ;
}


void SnowBeerWindow::on_checkBox_temperature_stateChanged(int arg1)
{
    lines.series[Line_temperature].setShow(arg1);
    AppJson["temperature_isShow"] = arg1 == Qt::Checked ? true: false ;
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
        lines.chart->setTheme(QChart::ChartThemeDark);
    } else if (theme == "浅色主题") {
        // 设置浅色主题的样式或设置
        lines.chart->setTheme(QChart::ChartThemeLight);
    }
}


void SnowBeerWindow::on_animatedComboBox_currentIndexChanged(int index)
{
    qDebug()<<"on_animatedComboBox_currentIndexChanged int:"<< index;
    //QChart::NoAnimation	0x0	动画在图表中被禁用。这是默认值。
    //QChart::GridAxisAnimations	0x1	在图表中启用网格轴动画。
    //QChart::SeriesAnimations	0x2	在图表中启用系列动画。
    //QChart::AllAnimations	0x3	图表中启用了所有动画类型。
    lines.chart->setAnimationOptions(QChart::AnimationOptions(index));
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
        lines.chart->legend()->hide();
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
    lines.chart->legend()->setAlignment(alignment);
    lines.chart->legend()->show();

}

void SnowBeerWindow::on_antialiasCheckBox_stateChanged(int arg1)
{
    qDebug()<<"on_antialiasComboBox_currentIndexChanged int:"<< arg1;
//    lines.chart->setRenderHint(QPainter::Antialiasing,  arg1);
    ui->widget->setRenderHint(QPainter::Antialiasing,arg1);
}

void SnowBeerWindow::on_pushButton_zoomOut_clicked()
{
    lines.chart->zoomOut();
}

void SnowBeerWindow::on_pushButton_zoomIn_clicked()
{
    lines.chart->zoomIn();
}

void SnowBeerWindow::on_pushButton_zoomReset_clicked()
{
    lines.chart->zoomReset();
}


void SnowBeerWindow::on_pushButton_test_clicked()
{
    qDebug()<<"test()"<<QThread::currentThread();
    QDateTime startTime = QDateTime::currentDateTime();
    //const QString filePath = "E:/works/debug/QtClient/CJCS100.txt";
    lines.filePath = "E:/works/debug/QtClient/CJCS100.txt";
    // lines.parseDataFromFile(lines.filePath);
    // 设置线程池的最大线程数
    lines.start();
    // lines.chart->update();
    // ui->widget->update();
    qDebug() << "解析时间Test：" << startTime.msecsTo(QDateTime::currentDateTime()) << "毫秒"<<"线程:"<<QThread::currentThread();
}

void SnowBeerWindow::on_lineEdit_rootPath_editingFinished()
{
    qDebug()<<"on_lineEdit_rootPath_editingFinished()";
        qDebug()<<ui->lineEdit_rootPath->text()<<AppJson["pathSnowBeerWindow"].toString();
    if(!QDir(ui->lineEdit_rootPath->text()).exists()){
        // 路径不存在
        QMessageBox::warning(this, "路径验证", "文件路径不存在!");
    }
    AppJson["pathSnowBeerWindow"] = ui->lineEdit_rootPath->text();
    ui->treeView_files->setRootIndex(m_fileSystemModel->index(ui->lineEdit_rootPath->text()));
}

void SnowBeerWindow::on_pushButton_downloadNetworkFile_clicked()
{
    qDebug()<<"on_pushButton_downloadNetworkFile_clicked():"<<ui->listView_filesNetwork->currentIndex().data(Qt::DisplayRole).toString();
    // 获取选中的项
    //方法1
//    QItemSelectionModel *selectionModel = ui->listView_filesNetwork->selectionModel();
//    QModelIndexList selectedIndexes = selectionModel->selectedIndexes();
//    for (const QModelIndex &index : selectedIndexes) {
//        qDebug() << "Selected item:" << index.data(Qt::DisplayRole).toString();
//    }
    QString selectedText = ui->listView_filesNetwork->currentIndex().data(Qt::DisplayRole).toString();
    if(selectedText.isEmpty()){
        QMessageBox::warning(this, "未选中文件", "文件为空!请先选中一个文件");
        return;
    }

    if(!m_filesUtil->getLayoutDownloadFile()->isVisible()){
        m_filesUtil->getLayoutDownloadFile()->setVisible(true);
    }
    m_filesUtil->startDownloadFileFromLink(ui->lineEdit_rootPath_filesNetwork->text()+selectedText,ui->lineEdit_rootPath->text()+selectedText);
}

void SnowBeerWindow::on_pushButton_fileDelete_clicked()
{
    QString selectedText = ui->listView_filesNetwork->currentIndex().data(Qt::DisplayRole).toString();
    if(selectedText.isEmpty()){
        QMessageBox::warning(this, "未选中文件", "文件为空!请先选中一个文件");
        return;
    }
    QString filename = ui->lineEdit_rootPath->text()+selectedText;
    if(QFile::exists(filename)){
        QString alreadyExists =  ui->lineEdit_rootPath->text().isEmpty()
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

void SnowBeerWindow::on_pushButton_axisYsetRange_clicked()
{
    qDebug()<<"on_pushButton_axisYsetRange_clicked():";
    qDebug()<<ui->doubleSpinBox_axisYRangeMin->value();
    qDebug()<<ui->doubleSpinBox_axisYRangeMax->value();
    lines.axisY->setRange(ui->doubleSpinBox_axisYRangeMin->value(),ui->doubleSpinBox_axisYRangeMax->value());
}

void SnowBeerWindow::on_pushButton_updateNetworkFiles_clicked()
{
    qDebug()<<"on_pushButton_updateNetworkFiles_clicked():"<<ui->lineEdit_rootPath_filesNetwork->text();

    filesListNetwork = m_filesUtil->getFilesListNetworkPath(ui->lineEdit_rootPath_filesNetwork->text());
    qDebug()<<"filesListNetwork:"<<filesListNetwork<<filesListNetwork.filter(ui->lineEdit_rootPath_filesNetwork->text());
    // 创建QStringListModel对象，并设置字符串列表作为数据
    m_filesListModelNetwork->setStringList(filesListNetwork.replaceInStrings(ui->lineEdit_rootPath_filesNetwork->text(),""));
//    QStringListModel model(filesListNetwork);
//    ui->listView_filesNetwork->setModel(&model);
//    QStringListModel *fileModel = new QStringListModel(filesListNetwork, this);
//    ui->listView_filesNetwork->setModel(fileModel);


}

void SnowBeerWindow::on_lineEdit_rootPath_filesNetwork_editingFinished()
{
    qDebug()<<"SnowBeerWindow::on_lineEdit_rootPath_filesNetwork_editingFinished()";
}

void SnowBeerWindow::on_pushButton_updateLocalFiles_clicked()
{
    qDebug()<<"on_pushButton_updateNetworkFiles_clicked():"<<AppJson["pathSnowBeerWindow"].toString();

//    qDebug()<<QRegularExpression(".*(" + fileExtensions.join("|") + ")");
    filesListLocal = m_filesUtil->getFilesListLocalPath(AppJson["pathSnowBeerWindow"].toString());
    qDebug()<<"filesListLocal:"<<filesListLocal;
    //使用规则表达式，区分大小，字符匹配
    filesListLocal = filesListLocal.filter(QRegularExpression(".*(" + fileExtensions.join("|") + ")"));
    qDebug()<<"filesListLocal:"<<filesListLocal;
    m_filesListModelNetwork->setStringList(filesListLocal.replaceInStrings(AppJson["pathSnowBeerWindow"].toString(),""));
}


void SnowBeerWindow::on_pushButton_parseData_clicked()
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


void SnowBeerWindow::on_listView_filesNetwork_clicked(const QModelIndex &index)
{
    qDebug()<<"on_listView_filesNetwork_clicked(QModelIndex index)"<<index<<index.row();
    qDebug()<<index.data(Qt::DisplayRole).toString();
    m_currentFilePathDir = AppJson["pathSnowBeerWindow"].toString() + index.data(Qt::DisplayRole).toString();
}


void SnowBeerWindow::on_listView_filesNetwork_doubleClicked(const QModelIndex &index)
{
    qDebug()<<"on_listView_filesNetwork_doubleClicked(QModelIndex index)"<<index<<index.row();
    qDebug()<<index.data(Qt::DisplayRole).toString();
    m_currentFilePathDir = AppJson["pathSnowBeerWindow"].toString() + index.data(Qt::DisplayRole).toString();
}


void SnowBeerWindow::on_pushButton_downloadedFiles_clicked()
{
    filesListDownloaded.clear();
    // 获取a中存在但b中不存在的元素（不区分大小写）
    for (const QString& element : filesListLocal) {
        if(filesListNetwork.contains(element, Qt::CaseInsensitive)){
            filesListDownloaded.append(element);
        }
    }
    m_filesListModelNetwork->setStringList(filesListDownloaded.replaceInStrings(AppJson["pathSnowBeerWindow"].toString(),""));

}


void SnowBeerWindow::on_pushButton_notDownloadedFiles_clicked()
{
    filesListNotDownloaded.clear();
    // 获取a中存在但b中不存在的元素（不区分大小写）
    for (const QString& element : filesListNetwork) {
        if(!filesListLocal.contains(element, Qt::CaseInsensitive)){
            filesListNotDownloaded.append(element);
        }
    }
    // 创建QStringListModel对象，并设置字符串列表作为数据
    m_filesListModelNetwork->setStringList(filesListNotDownloaded.replaceInStrings(ui->lineEdit_rootPath_filesNetwork->text(),""));
}


void SnowBeerWindow::on_pushButton_downloadFilesFronLinks_clicked()
{
    qDebug()<<"on_pushButton_downloadFilesFronLinks_clicked()"<<filesListNotDownloaded;
    if(filesListNotDownloaded.isEmpty()){
        QMessageBox::warning(this, "文件下载", "不存在未下载的文件");
        return;
    }
    downloadFilesListFromNetworkLinks(filesListNotDownloaded);
}


void SnowBeerWindow::on_pushButton_getFilePath_comparison_clicked()
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
        ui->lineEdit_filePath_comparison->setText(filePath);
    }

}


void SnowBeerWindow::on_pushButton_setMax_current_clicked()
{
    AppJson["current_max"] = ui->doubleSpinBox_Max_current->value();
}


void SnowBeerWindow::on_pushButton_setMax_voltage_clicked()
{
    AppJson["voltage_max"] = ui->doubleSpinBox_Max_voltage->value();
}


void SnowBeerWindow::on_pushButton_setMax_pressure_clicked()
{
    AppJson["pressure_max"] = ui->doubleSpinBox_Max_pressure->value();
}


void SnowBeerWindow::on_pushButton_setMax_temperature_clicked()
{
    AppJson["temperature_max"] = ui->doubleSpinBox_Max_temperature->value();
}

