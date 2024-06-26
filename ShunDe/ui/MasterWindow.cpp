#include "MasterWindow.h"
#include "modules/sqlite.h"
#include <QTabWidget>
#include <QStyledItemDelegate>
#include <QToolTip>
#include <QFileDialog>
#include <QMessageBox>

MasterWindow* MasterWindow::instance = nullptr;
MasterWindow *masterWindow;

#include <QCoreApplication>
#include <QDir>

QString getCurrentDirectory() {
    return QCoreApplication::applicationDirPath();
}
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

    gSql = SQLite::instance();
}

MasterWindow::~MasterWindow()
{
    delete ui;
    qDebug()<<"MasterWindow::~MasterWindow()";
}

MasterWindow *MasterWindow::getInstance(QWidget *parent)
{
    if(instance == nullptr){
        instance = new MasterWindow(parent);
        //std::atexit(shutdownHandler);
    }
    return instance;
}
/**
QHeaderView::Stretch 是 QHeaderView 中的一种列宽度调整模式,它的作用是根据表格内容自动调整列宽,使所有列宽度相等。
QHeaderView::Interactive: 允许用户通过拖动列分隔线来手动调整列宽。这是默认的调整模式。
QHeaderView::Fixed: 列宽度固定,不会自动调整。
QHeaderView::ResizeToContents: 根据列中内容的大小自动调整列宽。
QHeaderView::Custom: 使用自定义的列宽调整逻辑。
*/
void MasterWindow::start()
{
    QSqlError error;
    error = gSql->init_inspectionTasks();
    if (error.isValid()) {
        qWarning() << "Failed to initialize inspection tasks:" << error.text();
    }else{
        ui->tableView_inspection_data->setModel(gSql->inspectionTasksModel);
        ui->tableView_inspection_data->setColumnHidden(0,true);
        ui->tableView_inspection_data->setSortingEnabled(true);//排序
        // 设置标题栏可以交互式调整宽度
        ui->tableView_inspection_data->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        ui->tableView_inspection_data->horizontalHeader()->setSectionResizeMode(6,QHeaderView::Stretch);//checkResult 6
        ui->tableView_inspection_data->resizeColumnsToContents();
    }

    error = gSql->init_inspectionCheckpoints();
    if (error.isValid()) {// 初始化失败
        qWarning() << "Failed to initialize inspection Checkpoints:" << error.text();
    }else{
        ui->tableView_inspectionCheckpoints->setModel(gSql->inspectionCheckpoints_Model);
        //隐藏"任务ID"列
        ui->tableView_inspectionCheckpoints->setColumnHidden(1,true);
        ui->tableView_inspectionCheckpoints->setSortingEnabled(true);//排序
        //ui->tableView_inspectionCheckpoints->setStyleSheet("QTableView {qproperty-textElideMode: ElideMiddle;}");//让表格自动为过长的文本添加省略号,以限制单元格的宽度
        ui->tableView_inspectionCheckpoints->setItemDelegateForColumn(ItemDelegateImageShow,new ImageListDelegate);//第3列显示
        ui->tableView_inspectionCheckpoints->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);//让用户通过拖动表格的列分隔线来手动调整列宽。
        // 设置第 ItemDelegateImageShow 列宽度调整模式,它的作用是根据表格内容自动调整列宽,使所有列宽度相等。
        ui->tableView_inspectionCheckpoints->horizontalHeader()->setSectionResizeMode(ItemDelegateImageShow, QHeaderView::Stretch);
        //ui->tableView_inspectionCheckpoints->setColumnWidth(ItemDelegateImageShow, 50);
        //ui->tableView_inspectionCheckpoints->horizontalHeader()->resizeSection(1, 10);
        ui->tableView_inspectionCheckpoints->resizeColumnsToContents();// 调整列宽以适应内容，使用会导致显示该行列宽的显示为文字长度，非常宽
    }

    ui->dateTimeEdit_inspection_time_begin->setDateTime(QDateTime::currentDateTime().date().startOfDay());
    ui->dateTimeEdit_inspection_time_end->setDateTime(QDateTime::currentDateTime().date().endOfDay());
    qDebug()<<"MasterWindow::init()";
}

void MasterWindow::quit()
{
    qDebug()<<"MasterWindow::quit()";
}

bool MasterWindow::saveFileToCSV(const QString &filePath)
{
//    QString fileExtension = ".csv";
//    QString csvfile = filePath
//    // 检查文件扩展名是否正确
//    if (!filePath.endsWith(fileExtension, Qt::CaseInsensitive)) {
//        filePath += fileExtension;
//    }
    // 打开文件
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "导出失败", "无法写入文件: " + file.errorString());
        return false;
    }

    // 创建文本流
    QTextStream out(&file);
    // 获取数据模型
    QSqlTableModel* model = qobject_cast<QSqlTableModel*>(ui->tableView_inspectionCheckpoints->model());
    if (!model) {
        QMessageBox::warning(this, "导出失败", "无法获取数据模型");
        file.close();
        return false;
    }

    // 获取保存文件的路径
    QModelIndex index = ui->tableView_inspection_data->currentIndex();
    if(index.isValid()){
        QStringList headers_title;
        QStringList headers_data;
        QList<int> headers_title_nonExportColumns = {0}; // 这里列举了不导出的列号
        for (int title_i = 0; title_i < ui->tableView_inspection_data->model()->columnCount(); ++title_i) {
            if (!headers_title_nonExportColumns.contains(title_i)){
                headers_title << ui->tableView_inspection_data->model()->headerData(title_i, Qt::Horizontal).toString();
                headers_data << ui->tableView_inspection_data->model()->data(ui->tableView_inspection_data->model()->index(index.row(), title_i)).toString(); // 从数据中获取任务名称
            }
        }
        out << headers_title.join("|") << "\n";
        out << headers_data.join("|") << "\n";
    }

//    QString taskName = ui->tableView_inspection_data->model()->data(ui->tableView_inspection_data->model()->index(index.row(), 1)).toString(); // 从数据中获取任务名称
//    QDateTime startDateTime = ui->tableView_inspection_data->model()->data(ui->tableView_inspection_data->model()->index(index.row(), 7)).toDateTime(); // 从数据中获取开始日期"yyyy-MM-dd HH:mm:ss"

    // 写入表头
    QList<int> nonExportColumns = {0, 1}; // 这里列举了不导出的列号
    QStringList headers;
    for (int i = 0; i < model->columnCount(); i++) {
        if (!nonExportColumns.contains(i))
            headers << model->headerData(i, Qt::Horizontal).toString();
    }
    out << headers.join("|") << "\n";

    // 写入数据
    for (int row = 0; row < model->rowCount(); ++row) {
        QStringList rowData;
        for (int col = 0; col < model->columnCount(); col++) {
            if (!nonExportColumns.contains(col))
                rowData << model->data(model->index(row, col)).toString();
        }
        out << rowData.join("|") << "\n";
    }

    file.close();
    return true;
}

bool MasterWindow::saveFileToExcel(const QString &filePath)
{
    QString csvFilePath = filePath.left(filePath.lastIndexOf('.')) + ".csv";
    if(saveFileToCSV(csvFilePath)){
        QProcess process;
        process.start("python", QStringList() << getCurrentDirectory()+"/scripts/csvFileToExcel.py" << csvFilePath);
        if (!process.waitForFinished()) {
            return false;
        }
        qDebug()<<"process.exitCode"<< (process.exitCode() == 0);

    }else {
        return false;
    }
    return true;
}

void MasterWindow::showEvent(QShowEvent *event)
{
    qDebug() << "MasterWindow::showEvent(QShowEvent *event)" ;
    static bool is_first_show = true;
    if(is_first_show){
        start();
        is_first_show = false;
    }

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
    //ui->tableView_inspectionCheckpoints->resizeColumnsToContents();
}


void MasterWindow::on_tableView_inspection_data_doubleClicked(const QModelIndex &index)
{
    // 获取选定行的ID值
    QString id = ui->tableView_inspection_data->model()->data(ui->tableView_inspection_data->model()->index(index.row(), 0)).toString();
    qDebug()<<"MasterWindow::on_tableView_inspection_data_doubleClicked(const QModelIndex &"<<index<<id;
    // 设置筛选条件
    gSql->inspectionCheckpoints_Model->setFilter(QString("taskId = '%1'").arg(id));

    // 提交筛选条件a
    if (gSql->inspectionCheckpoints_Model->select()) {
        // 筛选成功，更新视图
//        ui->tableView_inspectionCheckpoints->setModel(SQL->inspectionCheckpoints_Model);
//        ui->tableView_inspectionCheckpoints->resizeColumnsToContents();
//        ui->tableView_inspectionCheckpoints->setColumnHidden(1, true);
//        ui->tableView_inspectionCheckpoints->setSortingEnabled(true);
//        ui->tableView_inspectionCheckpoints->setItemDelegateForColumn(3, new ImageDelegate);
//        ui->tableView_inspectionCheckpoints->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    } else {
        // 筛选失败，输出错误信息
        qWarning() << "Failed to load inspection checkpoints:" << gSql->inspectionCheckpoints_Model->lastError().text();
    }
}


void MasterWindow::on_toolButton_inspection_query_time_clicked()
{
    qDebug()<<"MasterWindow::on_toolButton_inspection_query_time_clicked()";
    // 设置筛选条件
    gSql->inspectionTasksModel->setFilter(QString("startTime >= '%1' AND startTime <= '%2'")
        .arg(ui->dateTimeEdit_inspection_time_begin->dateTime().toString(Qt::ISODate))
        .arg(ui->dateTimeEdit_inspection_time_end->dateTime().toString(Qt::ISODate)));
    // 提交筛选条件
    if (gSql->inspectionTasksModel->select()) {
        // 筛选成功，更新视图
//        ui->tableView_inspection_data->setModel(SQL->inspectionTasksModel);
//        ui->tableView_inspection_data->resizeColumnsToContents();
//        ui->tableView_inspection_data->setColumnHidden(0, true);
//        ui->tableView_inspection_data->setSortingEnabled(true);
//        ui->tableView_inspection_data->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    } else {
        // 筛选失败，输出错误信息
        qWarning() << "筛选失败，错误信息Failed to initialize inspection tasks:" << gSql->inspectionTasksModel->lastError().text();
        QToolTip::showText(ui->toolButton_inspection_query_time->mapToGlobal(QPoint(0, 0)),
                           "筛选失败，错误信息:"+gSql->inspectionTasksModel->lastError().text(),
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
    QString selectedData = gSql->inspectionTasksModel->data(gSql->inspectionTasksModel->index(selectedRow, selectedColumn)).toString();
    // 提交筛选条件
    if (!gSql->filter_inspectionTasks(selectedColumn,selectedData)) {
        // 筛选失败，输出错误信息
        // 筛选失败，输出错误信息
        qWarning() << "筛选失败，错误信息:" << gSql->inspectionTasksModel->lastError().text();
        QToolTip::showText(ui->toolButton_inspection_query_value->mapToGlobal(QPoint(0, 0)),
                           "筛选失败，错误信息:"+gSql->inspectionTasksModel->lastError().text(),
            ui->toolButton_inspection_query_value);
    }

}

void MasterWindow::on_toolButton__inspectionPoints_export_file_download_clicked()
{
    // 从 comboBox 获取下载类型
    FileDownloadType downloadType = static_cast<FileDownloadType>(ui->comboBox_export_file_style->currentIndex());

    // 获取保存文件的路径
    QModelIndex index = ui->tableView_inspection_data->currentIndex();

    QString taskName = ui->tableView_inspection_data->model()->data(ui->tableView_inspection_data->model()->index(index.row(), 1)).toString(); // 从数据中获取任务名称
    QDateTime startDateTime = ui->tableView_inspection_data->model()->data(ui->tableView_inspection_data->model()->index(index.row(), 7)).toDateTime(); // 从数据中获取开始日期"yyyy-MM-dd HH:mm:ss"
    QString fileName = QString("%1_%2").arg(taskName, startDateTime.toString("yyyy-MM-dd_HH-mm-ss"));
    QString filePath;
    switch (downloadType) {
    case FileDownloadType::CSV:
        filePath = QFileDialog::getSaveFileName(this, "导出到 CSV", fileName + ".csv", "CSV 文件 (*.csv)");
        if (filePath.isEmpty())
            return;
        if(saveFileToCSV(filePath))
            QMessageBox::information(this, "导出成功", "数据已成功导出到 CSV 文件。");
        break;
    case FileDownloadType::XLSX:
        filePath = QFileDialog::getSaveFileName(this, "导出到 XLSX", fileName + ".xlsx", "XLSX 文件 (*.xlsx)");
        if (filePath.isEmpty())
            return;
        // 将 XLSX 后缀修改为 CSV 后缀
        filePath = filePath.left(filePath.lastIndexOf('.')) + ".csv";
        if(saveFileToExcel(filePath))
        QMessageBox::information(this, "导出成功", "数据已成功导出到 Excel 文件。");
        break;
    default:
        //return;
        break;

    }

}

