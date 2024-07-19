/**
 * @details
 * Author: Tang
 * Created: 2024-03
 * Version: 0.0.3
 */

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
#include <QThread>
#include <QLegendMarker>
#include "AppOS.h"
#include "modules/FilesUtil.h"

enum LineEnum{
    Line_current=0,     // 电流
    Line_voltage=1,     // 电压
    Line_pressure=2,    // 压强
    Line_temperature=3, // 温度
    Line_Count=4        // 枚举计数，总是放在最后
};

/**定义折线图的信息
 * 使用append方法会导致chart重绘渲染，阻塞GUI线程，效率低
 * 需要一个数据缓存 QList<QPointF> 使用一次 replace(QList<QPointF>) 替换提高效率
**/
/****/
struct struSeries
{
    bool isShow = false;//用于是否显示的 Flag
    double max = 0.0;//用于显示超过阈值的点
    int color = Qt::gray;//用于保存设置线条颜色的属性

    QLineSeries *line;//绘制折线图
    QVector<QPointF> pointsLine;//QVector 在连续内存中存储元素，这对于大量数据的访问和迭代更高效。
    QScatterSeries *scatter;//绘制散点图
    QVector<QPointF> pointsScatter;

    struSeries() : line(new QLineSeries()), scatter(new QScatterSeries()) {
        pointsLine.reserve(100000);  // 预分配10万个元素的空间
        pointsScatter.reserve(1000); // 预分配1000个元素的空间（假设阈值点较少）

        line->setName("折线图");//设置线条名称（用于图例）
        //line->setColor(Qt::gray);//设置线条颜色
        //line->setPointLabelsColor(Qt::green);
        //line->setPointLabelsVisible(true);//设置点标签是否可见
        //line->setPointsVisible(false);//设置数据点是否可见,默认不可见
        //line->setPointLabelsFormat("@yPoint");//设置点标签格式
        line->setOpacity(0.8);//设置线条透明度
        scatter->setName("");//设置线条名称（用于图例）
        scatter->setVisible(true);//设置线条是否可见
        scatter->setPointLabelsFormat("@yPoint");//设置点标签格式
        scatter->setPointLabelsVisible();//设置点标签是否可见
         // 设置高亮点的样式
        // scatter->setMarkerShape(QScatterSeries::MarkerShapeCircle);
        // scatter->setMarkerSize(0);
        // scatter->setOpacity(QAbstractSeries::SeriesTypeLine);// 这会禁用图例功
        scatter->setColor(Qt::red);//设置线条颜色

    }
    // 不需要显式的析构函数,父子对象机制之间存在冲突时,可能会奔溃
    // ~struSeries() {
    //     delete line;
    //     delete point;
    // }

    void setShow(const bool &show ) {
        isShow = show;
        line->setVisible(isShow);
        scatter->setVisible(isShow);
    }

    void setColor(const int &lineColor ) {
        color = lineColor;
        line->setColor(static_cast<Qt::GlobalColor>(color));
    }

    void addPoint(const qreal &xpos,const qreal &ypos) {
        if(!isShow) return;
        //pointsLine.append(QPointF(xpos,ypos));
        pointsLine.emplace_back(xpos, ypos);
        if(max <= ypos){
            //pointsScatter.append(QPointF(xpos,ypos));
            pointsScatter.emplace_back(xpos, ypos);
        }
    }

    void clear(){
        if(!isShow) return;
        pointsLine.clear();
        pointsScatter.clear();
    }

    void update(){
        if(!isShow) return;
        line->replace(pointsLine);
        scatter->replace(pointsScatter);
    }
};
/**定义数据图的类型**/
using seriesList = QList<struSeries>;

/**定义折线图表处理线程，解析数据不阻塞GUI线程**/
class ChartLine : public QThread
{
    Q_OBJECT //使用信号和槽机制的类所必需的
public:
    explicit ChartLine(QObject *parent = nullptr):QThread{nullptr}
    {
        series.resize(Line_Count);
    };
    // 不需要显式的析构函数,父子对象机制之间存在冲突时,可能会奔溃
    // ~ChartLine() {
        // delete chart;
        // delete axisTime;
        // delete axisY;
    // }
    seriesList series;
    QString filePath;
    int updateInterval = 3000; // 更新进度频率 毫秒
    Result readDataFromFile(const QString filePath);

    virtual void addPoint(const LineEnum &lineType,const qreal &xpos,const qreal &ypos){
        series[lineType].addPoint(xpos,ypos);
    };
signals:
    void progressUpdated(qint64 progress);
    void readingFinished();

protected:
    void run() override;
private:

};

class ComparionChartLine:public ChartLine {
public:
    int TypeComparion = 0;
    bool isTypeComparion_1_first = true;
    qreal offsetMs_xpos = 0;
    void addPoint(const LineEnum &lineType,const qreal &xpos,const qreal &ypos) override {
        switch (TypeComparion) {
        case 0:
            //按照设定偏移时间计算x轴的时间
            series[lineType].addPoint(xpos+offsetMs_xpos,ypos);
            break;
        case 1:{
            if(isTypeComparion_1_first){
                offsetMs_xpos = offsetMs_xpos - xpos;
                isTypeComparion_1_first = false;
            }
            //按类型自动比对，从第一个时间开始自动计算偏移量
            series[lineType].addPoint(xpos+offsetMs_xpos,ypos);
            break;}
        default:
            break;
        }
    };

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
    void downloadFilesListFromNetworkLinks(QStringList linksFilesList);

    QChart *chart;
    QDateTimeAxis *axisTime; // 时间轴
    QValueAxis *axisY;
    void autoRangesAxisTime(const ChartLine &lineChart);
    void autoRangesAxisY(const ChartLine &lineChart);
    void initChart();

    ChartLine lines;//折线图
    // 展示在QChartView部件上
    void init_lines_chartView(ChartLine &lineChart);

    Result comparsion_lines_by_file(const QString &filePath,const int &type = 0);

    //隐藏所有名字为空的图例项 用于在设置数据线条可见时图例又会恢复可见属性
    void chartHideEmptyLegendItems(QChart* chartHide=nullptr) {
        if(chartHide == nullptr) chartHide = chart;
        const auto allMarkers = chartHide->legend()->markers();
        for (QLegendMarker* marker : allMarkers) {
            if (marker->series() && marker->series()->name().isEmpty()) {
                marker->setVisible(false);
            }
        }
    };
public slots:
    void parse_file_progress(qint64 progress);

    void on_lineEdit_localFiles_dir_editingFinished();

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

    void on_pushButton_axisX_autoRange_reset_clicked();

    void on_pushButton_axisYsetRange_clicked();

    void on_pushButton_setMax_current_clicked();

    void on_pushButton_setMax_voltage_clicked();

    void on_pushButton_setMax_pressure_clicked();

    void on_pushButton_setMax_temperature_clicked();

    void on_pushButton_comparison_getFilePath_clicked();

    void on_pushButton_comparison_axisTimeOffset_ok_clicked();

    void on_pushButton_comparison_start_clicked();

    void on_pushButton_network_url_save_clicked();

    void on_pushButton_localFiles_dir_save_clicked();

    void on_pushButton_network_updateFilesList_clicked();

    void on_pushButton_localFiles_updateFilesList_clicked();

    void on_pushButton_network_downloadedFiles_clicked();

    void on_pushButton_network_notDownloadedFiles_clicked();

    void on_pushButton_network_download_allFiles_FromLinks_clicked();

    void on_pushButton_network_download_networkFile_clicked();

    void on_pushButton_localFiles_parseData_byFile_clicked();

    void on_pushButton_localFiles_fileDelete_clicked();

    void on_listView_network_filesList_clicked(const QModelIndex &index);

    void on_listView_network_filesList_doubleClicked(const QModelIndex &index);

private:
    Ui::SnowBeerWindow *ui;

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
