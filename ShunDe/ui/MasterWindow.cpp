#include "MasterWindow.h"
#include "modules/sqlite.h"
#include <QTabWidget>
#include <QStyledItemDelegate>
#include <QToolTip>

MasterWindow *masterWindow;
//#include <QProxyStyle>
//#include <QStyleOptionTab>
//class CustomTabStyle : public QProxyStyle
//{
//public:
//    QSize sizeFromContents(ContentsType type, const QStyleOption *option,
//                           const QSize &size, const QWidget *widget) const
//    {
//        QSize s = QProxyStyle::sizeFromContents(type, option, size, widget);
//        if (type == QStyle::CT_TabBarTab) {
//            s.transpose(); // 将宽高进行转置，实现竖向排序
//            s.rwidth() = 100; // 设置每个tabBar中item的大小
//            s.rheight() = 40;
//        }
//        return s;
//    }
//    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
//    {
//        if (element == CE_TabBarTabLabel) {
//            if (const QStyleOptionTab *tab = qstyleoption_cast<const QStyleOptionTab *>(option)) {
//                QStyleOptionTab opt(*tab);
//                opt.shape = QTabBar::RoundedNorth; // 设置 tab 形状为圆角
//                QProxyStyle::drawControl(element,&opt,painter,widget);
//                return;
//            }
//        }
//        QProxyStyle::drawControl(element, option, painter, widget);
//    }
//};

MasterWindow::MasterWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MasterWindow)
{
    ui->setupUi(this);
    //使用tabWidget会样式改变选中的宽高，测试发现没有办法消除，只能设置长宽一样才没有差别，所以不使用该控件
    //ui->tabWidget->tabBar()->setStyle(new CustomTabStyle);
    // 连接 listWidget 的 currentRowChanged 信号到 stackedWidget 的 setCurrentIndex 槽
    connect(ui->listWidget_stackedWidget_Titler, &QListWidget::currentRowChanged, ui->stackedWidget_Window, &QStackedWidget::setCurrentIndex);

    SQL=new SQLite();

    start();
}

MasterWindow::~MasterWindow()
{
    delete ui;
    qDebug()<<"MasterWindow::~MasterWindow()";
}

void MasterWindow::start()
{
    SQL->initDb("test.db");

    QSqlError error = SQL->init_inspectionTasks();
    if (error.isValid()) {
        qWarning() << "Failed to initialize inspection tasks:" << error.text();
    }else{
        ui->tableView_inspection_data->setModel(SQL->inspectionTasksModel);
        ui->tableView_inspection_data->setColumnHidden(0,true);
        ui->tableView_inspection_data->setSortingEnabled(true);//排序
        //ui->tableView_inspection_data->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
        // 设置标题栏可以交互式调整宽度
        ui->tableView_inspection_data->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        ui->tableView_inspection_data->resizeColumnsToContents();
    }

    error = SQL->init_inspectionCheckpoints();
    if (error.isValid()) {// 初始化失败
        qWarning() << "Failed to initialize inspection Checkpoints:" << error.text();
    }else{
        ui->tableView_inspectionCheckpoints->setModel(SQL->inspectionCheckpoints_Model);
        ui->tableView_inspectionCheckpoints->resizeColumnsToContents();
        //隐藏"任务ID"列
        ui->tableView_inspectionCheckpoints->setColumnHidden(1,true);
        ui->tableView_inspectionCheckpoints->setSortingEnabled(true);//排序
        ui->tableView_inspectionCheckpoints->setItemDelegateForColumn(3,new ImageDelegate_List);
        ui->tableView_inspectionCheckpoints->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    }

    ui->dateTimeEdit_inspection_time_begin->setDateTime(QDateTime::currentDateTime().date().startOfDay());
    ui->dateTimeEdit_inspection_time_end->setDateTime(QDateTime::currentDateTime().date().endOfDay());
    qDebug()<<"MasterWindow::init()";
}

void MasterWindow::quit()
{
    delete SQL;
    qDebug()<<"MasterWindow::quit()";
}

void MasterWindow::on_pushButton_test_clicked()
{
//    SQL->taskUuid =  QUuid::createUuid().toString(QUuid::WithoutBraces);
////    SQL->taskUuid = "test";
//    const QString &taskName = "TEST";
//    int numOfCheckpoints = 3,numOfNormalPoints =1,numOfErrorPoints=1, numOfAlarmPoints=1;
//    const QString &checkResult = "test";
//    const QDateTime &startTime = QDateTime::currentDateTime();
//    const QString &other = "other";
//    SQL->add_inspectionTasks(taskName,numOfCheckpoints,numOfNormalPoints, numOfErrorPoints,numOfAlarmPoints, checkResult, startTime, other);

//    const QString& checkpointName = "test1";
//    const QString& checkpointContent = "G:\\data\\image\\test.jpg";
//    const QString& checkResult_point="test2";
//    const QString& remark="";
//    const QDateTime& checkTime = QDateTime::currentDateTime();

//    SQL->add_InspectionCheckpoint(checkpointName,checkpointContent,checkResult_point,remark,checkTime);

    qDebug()<<"MasterWindow::on_pushButton_test_clicked()";
}


void MasterWindow::on_tableView_inspection_data_doubleClicked(const QModelIndex &index)
{
    // 获取选定行的ID值
    QString id = ui->tableView_inspection_data->model()->data(ui->tableView_inspection_data->model()->index(index.row(), 0)).toString();
    qDebug()<<"MasterWindow::on_tableView_inspection_data_doubleClicked(const QModelIndex &"<<index<<id;
    // 设置筛选条件
    SQL->inspectionCheckpoints_Model->setFilter(QString("taskId = '%1'").arg(id));

    // 提交筛选条件a
    if (SQL->inspectionCheckpoints_Model->select()) {
        // 筛选成功，更新视图
//        ui->tableView_inspectionCheckpoints->setModel(SQL->inspectionCheckpoints_Model);
//        ui->tableView_inspectionCheckpoints->resizeColumnsToContents();
//        ui->tableView_inspectionCheckpoints->setColumnHidden(1, true);
//        ui->tableView_inspectionCheckpoints->setSortingEnabled(true);
//        ui->tableView_inspectionCheckpoints->setItemDelegateForColumn(3, new ImageDelegate);
//        ui->tableView_inspectionCheckpoints->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    } else {
        // 筛选失败，输出错误信息
        qWarning() << "Failed to load inspection checkpoints:" << SQL->inspectionCheckpoints_Model->lastError().text();
    }
}


void MasterWindow::on_toolButton_inspection_query_time_clicked()
{
    qDebug()<<"MasterWindow::on_toolButton_inspection_query_time_clicked()";
    // 设置筛选条件
    SQL->inspectionTasksModel->setFilter(QString("startTime >= '%1' AND startTime <= '%2'")
        .arg(ui->dateTimeEdit_inspection_time_begin->dateTime().toString(Qt::ISODate))
        .arg(ui->dateTimeEdit_inspection_time_end->dateTime().toString(Qt::ISODate)));
    // 提交筛选条件
    if (SQL->inspectionTasksModel->select()) {
        // 筛选成功，更新视图
//        ui->tableView_inspection_data->setModel(SQL->inspectionTasksModel);
//        ui->tableView_inspection_data->resizeColumnsToContents();
//        ui->tableView_inspection_data->setColumnHidden(0, true);
//        ui->tableView_inspection_data->setSortingEnabled(true);
//        ui->tableView_inspection_data->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    } else {
        // 筛选失败，输出错误信息
        qWarning() << "筛选失败，错误信息Failed to initialize inspection tasks:" << SQL->inspectionTasksModel->lastError().text();
        QToolTip::showText(ui->toolButton_inspection_query_time->mapToGlobal(QPoint(0, 0)),
                           "筛选失败，错误信息:"+SQL->inspectionTasksModel->lastError().text(),
                           ui->toolButton_inspection_query_time);
    }
}


void MasterWindow::on_toolButton_inspection_query_value_clicked()
{
    // 获取当前选中单元格的索引
    QModelIndex currentIndex = ui->tableView_inspection_data->currentIndex();
    if (!currentIndex.isValid()) {
        // 没有选中单元格,返回 false
        return;
    }
    // 获取选中单元格所在的行和列
    int selectedRow = currentIndex.row();
    int selectedColumn = currentIndex.column();
    // 根据选中单元格的列序号,获取对应的数据
    QString selectedData = SQL->inspectionTasksModel->data(SQL->inspectionTasksModel->index(selectedRow, selectedColumn)).toString();
    // 提交筛选条件
    if (!SQL->filter_inspectionTasks(selectedColumn,selectedData)) {
        // 筛选失败，输出错误信息
        // 筛选失败，输出错误信息
        qWarning() << "筛选失败，错误信息:" << SQL->inspectionTasksModel->lastError().text();
        QToolTip::showText(ui->toolButton_inspection_query_value->mapToGlobal(QPoint(0, 0)),
            "筛选失败，错误信息:"+SQL->inspectionTasksModel->lastError().text(),
            ui->toolButton_inspection_query_value);
    }

}

