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

        scatter->setName("阈值");//设置线条名称（用于图例）
        scatter->setVisible(true);//设置线条是否可见
        scatter->setPointLabelsFormat("@yPoint");//设置点标签格式
        scatter->setPointLabelsVisible();//设置点标签是否可见
         // 设置高亮点的样式
        scatter->setMarkerSize(6);
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
    explicit ChartLine(QObject *parent = nullptr):QThread{nullptr},
        chart(new QChart()), axisTime(new QDateTimeAxis()),axisY(new QValueAxis())
    {
        series.resize(Line_Count);
        initChart();

        // connect(axisTime,  &QDateTimeAxis::minChanged, this, [=](qreal min) {
        //     qDebug()<<"QDateTimeAxis::minChanged"<<min;
        // });
        // connect(axisTime,  &QDateTimeAxis::maxChanged, this, [=](qreal max) {
        //     qDebug()<<"QDateTimeAxis::maxChanged"<<max;
        // });

        connect(axisY,  &QValueAxis::minChanged, this, [=](qreal min) {
            qDebug()<<"axisY::minChanged"<<min;
        });
        // connect(axisY,  &QValueAxis::maxChanged, this, [=](qreal max) {
        //     qDebug()<<"axisY::maxChanged"<<max;
        // });

    };
    // 不需要显式的析构函数,父子对象机制之间存在冲突时,可能会奔溃
    // ~ChartLine() {
        // delete chart;
        // delete axisTime;
        // delete axisY;
    // }
    seriesList series;
    QChart *chart;
    QDateTimeAxis *axisTime; // 时间轴
    QValueAxis *axisY;
    void autoRangesAxisTime();
    void autoRangesAxisY();

    void initChart();
    QString filePath;
    Result readDataFromFile(const QString filePath);


signals:
    void progressUpdated(int progress);
    void readingFinished();

protected:
    void run() override;
private:

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

    ChartLine lines;//折线图
    // 展示在QChartView部件上
    void init_lines_chartView(ChartLine &lineChart);

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

    void on_pushButton_getFilePath_comparison_clicked();

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

    double max_current;     // 电流
    double max_voltage;     // 电压
    double max_pressure;    // 压强
    double max_temperature; // 温度
};

#endif // SNOWBEERWINDOW_H
