#include "DataView.h"
#include "ui_DataView.h"
#include<QTimer>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QFile>
#include <QTextStream>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QStringListModel>
#include <QJsonDocument>
#include <QJsonArray>

QRegularExpression REGEX_DATA("^(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}).*time:(\\d+\\.\\d+)S.*I:([-+]?\\d+\\.\\d+)A.*V:([-+]?\\d+\\.\\d+)V.*P:([-+]?\\d+\\.\\d+)MPa.*C:([-+]?\\d+\\.\\d+)℃.*");
#include <QRandomGenerator>

//#include "../public/AppData.h"
#include "public/AppSystem.h"
#include "ui/SubMain.h"

DataView::DataView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DataView)
{
    ui->setupUi(this);

    init();
}

DataView::~DataView()
{
    qDebug()<<"DataView::~DataView()";
    delete ui;
//    delete m_filesUtil;
}


void DataView::init()
{
    qDebug()<<"DataView::init()";
    init_chartView();
    init_filesView();
    //    test();

}


QWidget *DataView::getDataView()
{
    return ui->widget_dataView;
}

void DataView::downloadFilesListFromNetworkLinks(QStringList linksFilesList)
{
    qDebug()<<"downloadFilesListFromNetworkLinks(QStringList:"<< linksFilesList;
//    QStringList linksFilesLists = QString("1.txt,2.csv,3.txt").split(',');
//    int key = 0;
    foreach (QString link,linksFilesList) {
        qDebug()<<"downloadFilesListFromNetworkLinks(QStringList:"<< link;
        // 定义一个正则表达式模式，用于匹配网络下载链接 Qt::CaseSensitive区分大小写
        FilesUtil *fileItem = new FilesUtil();
        if(fileItem->startDownloadFileFromLink(EXE_CONFIG["pathDataViewNetwork"].toString() + link,\
                                                EXE_CONFIG["pathDataView"].toString() +link)){
            SUB_MAIN->ui->verticalLayout_download->addWidget(fileItem->getLayoutDownloadFile());
        }

//        SUB_WINDOW->ui->gridLayout->addWidget(fileItem->getLayoutDownloadFile(),key,0);key++;
//        SUB_WINDOW->ui->gridLayout->addWidget(fileItem->getLayoutDownloadFile());
//        QHBoxLayout *layout = new QHBoxLayout;
//        QPushButton *test = new QPushButton;
    }
    SUB_MAIN->setWindowTitle("下载");
    SUB_MAIN->setCentralWidget(SUB_MAIN->ui->widget_download);
    SUB_MAIN->show();
}


void DataView::test()
{
//    QString line = "2024-01-15 09:09:22 Monday, time:1705280962.602193S, I:0.000000A, V:3.061000V, P:0.000000MPa, C:-14.836364℃";
//    QRegularExpression regex("^(\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2}).*time:(\\d+\\.\\d+)S.*I:([-+]?\\d+\\.\\d+)A.*V:([-+]?\\d+\\.\\d+)V.*P:([-+]?\\d+\\.\\d+)MPa.*C:([-+]?\\d+\\.\\d+)℃$");
//    QRegularExpressionMatch match = regex.match(line);
//    qDebug() << match.hasMatch();
//    if (match.hasMatch()) {
//        DataPoint dataPoint;
//        dataPoint.dataTime = QDateTime::fromString(match.captured(1), "yyyy-MM-dd hh:mm:ss");
//        dataPoint.current = match.captured(3).toDouble();
//        dataPoint.voltage = match.captured(4).toDouble();
//        dataPoint.pressure = match.captured(5).toDouble();
//        dataPoint.temperature = match.captured(6).toDouble();

//        qDebug() << "日期和时间：" << dataPoint.dataTime.toString("yyyy-MM-dd hh:mm:ss");
//        qDebug() << "电流：" << dataPoint.current << "安培";
//        qDebug() << "电压：" << dataPoint.voltage << "伏特";
//        qDebug() << "压力：" << dataPoint.pressure << "兆帕（MPa）";
//        qDebug() << "温度：" << dataPoint.temperature << "摄氏度";
//    }

    // 创建打开文件对话框
    QFileDialog fileDialog(this);
    fileDialog.setFileMode(QFileDialog::ExistingFile); // 设置对话框模式为选择已存在的文件

    QString filePath = fileDialog.getOpenFileName(this, "Open File"); // 获取选择的文件路径
    if (!filePath.isEmpty()) {
        // 在这里处理打开文件的逻辑
        qDebug() << "Selected file: " << filePath;
    }


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
    m_lineSeries_pressure->setName("气压数据曲线(兆帕MPa)");
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
    m_axisY->setRange(0.0,50.0);
    //在QChartView中显示 设置外边距为 负数，减少空白
    m_chart->setContentsMargins(-9,-9,-9,-9);
//    m_chart->setMargins(QMargins(-6,-6,-6,-6));
    m_chart->setBackgroundRoundness(0);

    m_chartView = new QChartView(m_chart);
//    m_chartView->setContentsMargins(0,0,0,0);
//    m_chartView->setFrameShape(QFrame::NoFrame);
//    m_chartView->setFrameShadow(QFrame::Plain);
    //    m_chartView->setRubberBand(QChartView::RectangleRubberBand); //水平方向缩   chart->zoomIn()
//    m_chartView->setRubberBand(QChartView::HorizontalRubberBand);
//    m_chartView->setStyleSheet("border: none;");
//    ui->horizontalLayout_dataView->(m_chartView);
    ui->widget_test->setChart(m_chart);
//    ui->horizontalLayout_dataView->addWidget(m_chartView);
//    ui->gridLayout_chartView->addWidget(m_chartView);
}

void DataView::init_filesView()
{
    qDebug()<<"init_filesView()";
//    m_filesUtil = new FilesUtil(this->parentWidget());
    m_filesUtil = new FilesUtil();
    // 目录树模型
    m_fileModel_dataView = new QFileSystemModel;
//    QStringList filters;
//    filters << "*.txt";
//    m_fileModel_dataView->setNameFilters(filters);
//    m_fileModel_dataView->setNameFilterDisables(false);
//    m_fileModel_dataView->setRootPath(QDir::rootPath());
//    m_fileModel_dataView->setRootPath("http://192.168.1.15:8071");
    m_fileModel_dataView->setFilter(QDir::Files | QDir::AllDirs | QDir::NoDotAndDotDot);
    // 目录树视图
    ui->treeView_files->setModel(m_fileModel_dataView);
    ui->treeView_files->setWindowTitle("Directories");
    // 设置文件浏览视图的根目录
    ui->lineEdit_rootPath->setText(EXE_CONFIG["pathDataView"].toString());

//    m_fileModel_dataView->setRootPath(m_fileModel_dataView->myComputer().toString());
    m_fileModel_dataView->setRootPath(ui->lineEdit_rootPath->text());
    ui->treeView_files->setRootIndex(m_fileModel_dataView->index(ui->lineEdit_rootPath->text()));
    // 当前目录变化信号
    connect(ui->treeView_files->selectionModel(),&QItemSelectionModel::currentChanged,this,&DataView::fileModelSelection);
    connect(ui->treeView_files, &QTreeView::doubleClicked,this, &DataView::fileBrowserDoubleClicked);
//    connect(ui->treeView_files->selectionModel(), &QTreeView::doubleClicked,this, &DataView::fileBrowserDoubleClicked);

    // 设置网络浏览视图的根目录
    ui->listView_filesNetwork->setWindowTitle("Directories Network");
    ui->lineEdit_rootPath_filesNetwork->setText(EXE_CONFIG["pathDataViewNetwork"].toString());
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

bool DataView::parseDataFromFile(const QString filePath)
{
    qDebug() << "parseDataFromFile文件：" <<filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "无法打开文件", "文件:"+filePath+"读取失败，报错："+ file.errorString());
        qDebug() << "无法打开文件：" <<filePath<< file.errorString();
        return false;
    }

    QTextStream in(&file);
    //    QList<DataPoint> dataPoints;
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
            qDebug()<<"解析失败match.hasMatch() 错误行号:"<<in.pos()<<"-第"<<line<<"行;";
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

    return true;

}

void DataView::fileModelSelection(QModelIndex index)
{
    qDebug()<<"fileModelSelection(QModelIndex index)"<<index;
    qDebug()<<index.data(QFileSystemModel::FilePathRole).toString();
    m_currentFilePathDir = index.data(QFileSystemModel::FilePathRole).toString();
}

void DataView::fileBrowserDoubleClicked(QModelIndex index)
{
    qDebug()<<"fileBrowserDoubleClicked(QModelIndex index)"<<index;
    qDebug()<<index.data(QFileSystemModel::FilePathRole).toString();
    m_currentFilePathDir = index.data(QFileSystemModel::FilePathRole).toString();
    // 提示窗口
    QMessageBox msgBox(this);
    msgBox.setText("等待数据解析完成");
    if(!parseDataFromFile(m_currentFilePathDir)){
        // 路径不存在
        QMessageBox::warning(this, "数据解析", "数据解析失败文件路径不存在!");
        msgBox.exec();
        return;
    }
    // 3秒后自动关闭
//    QTimer::singleShot(3000, &msgBox, &QMessageBox::close);
    msgBox.exec();
//    ui->tabWidget->setCurrentWidget(ui->tab_view);
//    QTabBar *tabBar = ui->tabWidget->tabBar();
//    ui->tabWidget->setMaximumWidth(200);
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


void DataView::on_themeComboBox_currentIndexChanged(int index)
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
        m_chart->setTheme(QChart::ChartThemeDark);
    } else if (theme == "浅色主题") {
        // 设置浅色主题的样式或设置
        m_chart->setTheme(QChart::ChartThemeLight);
    }
}


void DataView::on_animatedComboBox_currentIndexChanged(int index)
{
    qDebug()<<"on_animatedComboBox_currentIndexChanged int:"<< index;
    //QChart::NoAnimation	0x0	动画在图表中被禁用。这是默认值。
    //QChart::GridAxisAnimations	0x1	在图表中启用网格轴动画。
    //QChart::SeriesAnimations	0x2	在图表中启用系列动画。
    //QChart::AllAnimations	0x3	图表中启用了所有动画类型。
    m_chart->setAnimationOptions(QChart::AnimationOptions(index));
}

void DataView::on_legendComboBox_currentIndexChanged(int index)
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
        m_chart->legend()->hide();
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
    m_chart->legend()->setAlignment(alignment);
    m_chart->legend()->show();

}

void DataView::on_antialiasCheckBox_stateChanged(int arg1)
{
    qDebug()<<"on_antialiasComboBox_currentIndexChanged int:"<< arg1;
//    m_chart->setRenderHint(QPainter::Antialiasing,  arg1);
    m_chartView->setRenderHint(QPainter::Antialiasing,arg1);
}

void DataView::on_pushButton_zoomOut_clicked()
{
    m_chart->zoomOut();
}

void DataView::on_pushButton_zoomIn_clicked()
{
    m_chart->zoomIn();
}

void DataView::on_pushButton_zoomReset_clicked()
{
    m_chart->zoomReset();
}

#include <QProcess>
void DataView::on_pushButton_test_clicked()
{
    qDebug()<<"test()";
    //    parseDataFromFile("G:/data/雪花啤酒/dataView.txt");
    // 打开文件位置
    #ifdef Q_OS_WIN
        QProcess::startDetached("explorer.exe", {"/select,", QDir::toNativeSeparators("G:/data/雪花啤酒/dataView.txt")});
    #elif Q_OS_MAC
        QProcess::execute("osascript", {"-e", QString("tell application \"Finder\" to reveal POSIX file \"%1\" activating").arg(currentFilePath)});
    #elif Q_OS_LINUX
        QString cmd = "xdg-open \"" + QFileInfo(currentFilePath).path() + "\" --select " + QFileInfo(currentFilePath).fileName();
        QProcess::startDetached(cmd);
    #else
        QProcess::startDetached("explorer.exe /select," + QDir::toNativeSeparators(currentFilePath));
    #endif
//    test();
}

void DataView::on_lineEdit_rootPath_editingFinished()
{
    qDebug()<<"on_lineEdit_rootPath_editingFinished()";
    qDebug()<<ui->lineEdit_rootPath->text()<<EXE_CONFIG["pathDataView"].toString();
    if(!QDir(ui->lineEdit_rootPath->text()).exists()){
        // 路径不存在
        QMessageBox::warning(this, "路径验证", "文件路径不存在!");
    }
    EXE_CONFIG["pathDataView"] = ui->lineEdit_rootPath->text();
    ui->treeView_files->setRootIndex(m_fileModel_dataView->index(ui->lineEdit_rootPath->text()));
}

void DataView::on_pushButton_downloadNetworkFile_clicked()
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

void DataView::on_pushButton_fileDelete_clicked()
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

void DataView::on_pushButton_axisYsetRange_clicked()
{
    qDebug()<<"on_pushButton_axisYsetRange_clicked():";
    qDebug()<<ui->spinBox_axisYRangeMin->value();
    qDebug()<<ui->spinBox_axisYRangeMax->value();
    m_axisY->setRange(ui->spinBox_axisYRangeMin->value(),ui->spinBox_axisYRangeMax->value());
}

void DataView::on_pushButton_updateNetworkFiles_clicked()
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

void DataView::on_lineEdit_rootPath_filesNetwork_editingFinished()
{

}

void DataView::on_pushButton_updateLocalFiles_clicked()
{
    qDebug()<<"on_pushButton_updateNetworkFiles_clicked():"<<EXE_CONFIG["pathDataView"].toString();

//    qDebug()<<QRegularExpression(".*(" + fileExtensions.join("|") + ")");
    filesListLocal = m_filesUtil->getFilesListLocalPath(EXE_CONFIG["pathDataView"].toString());
    qDebug()<<"filesListLocal:"<<filesListLocal;
    //使用规则表达式，区分大小，字符匹配
    filesListLocal = filesListLocal.filter(QRegularExpression(".*(" + fileExtensions.join("|") + ")"));
    qDebug()<<"filesListLocal:"<<filesListLocal;
    m_filesListModelNetwork->setStringList(filesListLocal.replaceInStrings(EXE_CONFIG["pathDataView"].toString(),""));
}


void DataView::on_pushButton_parseData_clicked()
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
    }
    QDateTime startTime = QDateTime::currentDateTime();

    if(!parseDataFromFile(m_currentFilePathDir)){
        // 路径不存在
        return;
    }

    QDateTime endTime = QDateTime::currentDateTime();
    qint64 elapsedTime = startTime.secsTo(endTime);
    qDebug() << "解析时间：" << elapsedTime << "秒";
    // 提示窗口
    QMessageBox msgBox(this);
    msgBox.setText("数据解析完成花费时间：" + QString::number(elapsedTime) + "秒");
//     3秒后自动关闭
    QTimer::singleShot(3000, &msgBox, &QMessageBox::close);
    msgBox.exec();
}


void DataView::on_listView_filesNetwork_clicked(const QModelIndex &index)
{
    qDebug()<<"on_listView_filesNetwork_clicked(QModelIndex index)"<<index<<index.row();
    qDebug()<<index.data(Qt::DisplayRole).toString();
    m_currentFilePathDir = EXE_CONFIG["pathDataView"].toString() + index.data(Qt::DisplayRole).toString();
}


void DataView::on_listView_filesNetwork_doubleClicked(const QModelIndex &index)
{
    qDebug()<<"on_listView_filesNetwork_doubleClicked(QModelIndex index)"<<index<<index.row();
    qDebug()<<index.data(Qt::DisplayRole).toString();
    m_currentFilePathDir = EXE_CONFIG["pathDataView"].toString() + index.data(Qt::DisplayRole).toString();
}


void DataView::on_pushButton_downloadedFiles_clicked()
{
    filesListDownloaded.clear();
    // 获取a中存在但b中不存在的元素（不区分大小写）
    for (const QString& element : filesListLocal) {
        if(filesListNetwork.contains(element, Qt::CaseInsensitive)){
            filesListDownloaded.append(element);
        }
    }
    m_filesListModelNetwork->setStringList(filesListDownloaded.replaceInStrings(EXE_CONFIG["pathDataView"].toString(),""));

}


void DataView::on_pushButton_notDownloadedFiles_clicked()
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


void DataView::on_pushButton_downloadFilesFronLinks_clicked()
{
    qDebug()<<"on_pushButton_downloadFilesFronLinks_clicked()"<<filesListNotDownloaded;
    if(filesListNotDownloaded.isEmpty()){
        QMessageBox::warning(this, "文件下载", "不存在未下载的文件");
        return;
    }
    downloadFilesListFromNetworkLinks(filesListNotDownloaded);
}
