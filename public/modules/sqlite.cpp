#include "sqlite.h"

SQLite* SQL;

#include <QTimeEdit>
#define EventCenter "EventCenter"
const auto events_SQL = QLatin1String(R"(
    CREATE TABLE IF NOT EXISTS EventCenter (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        time TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
        source TEXT,
        type TEXT,
        level TEXT,
        details TEXT,
        status TEXT)
)");

#define InspectionTasks "InspectionTasks"
const auto InspectionTasks_SQL = QLatin1String(R"(
    CREATE TABLE IF NOT EXISTS InspectionTasks (
        id TEXT PRIMARY KEY,
        taskName TEXT,
        numOfCheckpoints INTEGER,
        numOfNormalPoints INTEGER,
        numOfErrorPoints INTEGER,
        numOfAlarmPoints INTEGER,
        checkResult TEXT,
        startTime DATETIME,
        other TEXT
    )
)");

#define InspectionCheckpoints "InspectionCheckpoints"
const auto InspectionCheckpoints_SQL = QLatin1String(R"(
    CREATE TABLE IF NOT EXISTS InspectionCheckpoints (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        taskId TEXT,
        checkpointName TEXT,
        checkpointContent TEXT,
        checkResult TEXT,
        remark TEXT,
        checkTime DATETIME,
        FOREIGN KEY (taskId) REFERENCES inspectionTasks(id))
)");

class EventTimeDelegate : public QStyledItemDelegate {
public:
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        if (index.column() == 1) { // 事件时间列
            QDateTimeEdit* editor = new QDateTimeEdit(parent);
            editor->setDisplayFormat("yyyy-MM-dd HH:mm:ss");
            return editor;
        }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        if (index.column() == 1) {
            QDateTimeEdit* dateTimeEdit = qobject_cast<QDateTimeEdit*>(editor);
            if (dateTimeEdit) {
                dateTimeEdit->setDateTime(index.data().toDateTime());
            }
        } else {
            QStyledItemDelegate::setEditorData(editor, index);
        }
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        if (index.column() == 1) {
            QDateTimeEdit* dateTimeEdit = qobject_cast<QDateTimeEdit*>(editor);
            if (dateTimeEdit) {
                model->setData(index, dateTimeEdit->dateTime());
            }
        } else {
            QStyledItemDelegate::setModelData(editor, model, index);
        }
    }
};

SQLite::SQLite(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SQLite)
{
    ui->setupUi(this);
}

SQLite::~SQLite()
{
    // 关闭数据库连接
    db.close();
    delete ui;
    qDebug() << "SQLite::~SQLite()";
}

QSqlError SQLite::initDb(const QString& name, const QString& connectionName) {
    if (!QSqlDatabase::contains(connectionName)) {
        db = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        if (name.isEmpty()) {db.setDatabaseName(":memory:");}
        else {db.setDatabaseName(name);}
        qDebug() << "Database connection created, name:" << connectionName;
    } else {
        db = QSqlDatabase::database(connectionName);
        qDebug() << "Database connection already exists, name:" << connectionName;
    }
    // 打开数据库
    if (!db.open()) {
        qWarning() << "Failed to open database:" << db.lastError().text();
        return db.lastError();
    }
    return QSqlError();
}

QSqlError SQLite::init_events()
{
    if(!db.tables().contains(EventCenter, Qt::CaseInsensitive)){
        if(!query.exec(events_SQL))
            return query.lastError();
    }
    model_events.setQuery("SELECT * FROM EventCenter",db);
    if(model_events.lastError().isValid()){
        qDebug() << "SQLite::init_events() Failed to load data:" << model_events.lastError().text();
        return model_events.lastError();
    }

    proxyModel_events.setSourceModel(&model_events);
    proxyModel_events.setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel_events.setFilterKeyColumn(-1); // 对所有列过滤
    // 设置表格的标题
    proxyModel_events.setHeaderData(0,Qt::Horizontal,"序号");
    proxyModel_events.setHeaderData(1,Qt::Horizontal,"事件时间");
    proxyModel_events.setHeaderData(2,Qt::Horizontal,"事件源");
    proxyModel_events.setHeaderData(3,Qt::Horizontal,"事件类型");
    proxyModel_events.setHeaderData(4,Qt::Horizontal,"事件级别");
    proxyModel_events.setHeaderData(5,Qt::Horizontal,"事件细节");
    proxyModel_events.setHeaderData(6,Qt::Horizontal,"状态");

    ui->tableView_events->setModel(&proxyModel_events);
    ui->tableView_events->setSortingEnabled(true);//排序
    ui->tableView_events->setItemDelegateForColumn(1, new EventTimeDelegate);
    ui->tableView_events->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    return QSqlError();
}

QVariant SQLite::add_event(const QString &source, const QString &type, const QString &level, const QString &details, const QString &status)
{
    // 插入数据
    QSqlQuery q(db);
    q.prepare("INSERT INTO EventCenter (time, source, type, level, details, status) "
              "VALUES (:time, :source, :type, :level, :details, :status)");
    //q.bindValue(":time", QDateTime::currentDateTime().toString(Qt::ISODate));
    q.bindValue(":time", QDateTime::currentDateTime());
    q.bindValue(":source", source);
    q.bindValue(":type", type);
    q.bindValue(":level", level);
    q.bindValue(":details", details);
    q.bindValue(":status", status);

    if (!q.exec()) {
        qWarning() << "Failed to add event:" << q.lastError().text();
        return QVariant();
    }
    return q.lastInsertId();
}

void SQLite::updata_eventsView()
{
    // 定义每页显示的行数和当前页码
    int rowsPerPage = 10;
    int currentPage = 1;
    // 构建带有限制和偏移量的查询语句
    QString queryStr = QString("SELECT * FROM EventCenter LIMIT %1 OFFSET %2")
                           .arg(rowsPerPage).arg((currentPage - 1) * rowsPerPage);

    // 执行查询语句
    model_events.setQuery(queryStr,db);

    if(model_events.lastError().isValid()){
        qWarning() << "Failed to load data:" << model_events.lastError().text();
        //return model_events.lastError();
    }
    // 调整列宽和行高以适应内容
    ui->tableView_events->resizeColumnsToContents();
    ui->tableView_events->resizeRowsToContents();
    ui->tableView_events->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
}

QSqlError SQLite::delete_events(int id)
{
    //QSqlQuery query(db);
    query.prepare("DELETE FROM EventCenter WHERE id = :id");
    query.bindValue(":id", id);
    if (!query.exec()) {
        qWarning() << "Failed to delete data:" << query.lastError().text();
        return query.lastError();
    }
    return QSqlError();
}

void SQLite::search_keyword_from_events(const QString &keyword)
{
    //QSqlQuery query(db);
    query.prepare("SELECT * FROM EventCenter WHERE source LIKE :keyword OR type LIKE :keyword OR level LIKE :keyword OR details LIKE :keyword OR status LIKE :keyword");
    query.bindValue(":keyword", "%" + keyword + "%");
    if (!query.exec()) {
        qWarning() << "Failed to search data:" << query.lastError().text();

    }
    model_events.setQuery(std::move(query));
}

void SQLite::filter_events_by_Time(const QDateTime& startTime, const QDateTime& endTime)
{
    QSqlQuery q(db);
    q.prepare("SELECT * FROM EventCenter WHERE timestamp BETWEEN :startTime AND :endTime");
    q.bindValue(":startTime", startTime);
    q.bindValue(":endTime", endTime);
    if (!q.exec()) {
        qWarning() << "Failed to filter data:" << q.lastError().text();
    }

    model_events.setQuery(std::move(q));
}

QSqlError SQLite::updata_keyword_from_events(const int &id, const QString &keyword)
{
    QSqlQuery q(db);
    q.prepare("UPDATE EventCenter SET keyword = :keyword WHERE id = :id");
    q.bindValue(":keyword", keyword);
    q.bindValue(":id", id);
    if (!q.exec()) {
        qWarning() << "Failed to UPDATE data:" <<id<<keyword<< q.lastError().text();
        return q.lastError();
    }
    return QSqlError();
}

QSqlError SQLite::init_EventCenter()
{
    if(!db.tables().contains(EventCenter, Qt::CaseInsensitive)){
        if(!query.exec(events_SQL))
            return query.lastError();
    }
    //QSqlTableModel是一个不可复制的类，只能通过构造函数进行初始化
    EventCenter_Model = new QSqlTableModel(nullptr, db);
    EventCenter_Model->setTable(EventCenter);
    EventCenter_Model->setEditStrategy(QSqlTableModel::OnFieldChange);
    // 设置表格的标题
    EventCenter_Model->setHeaderData(0,Qt::Horizontal,"序号");//id
    EventCenter_Model->setHeaderData(1,Qt::Horizontal,"事件时间");//time
    EventCenter_Model->setHeaderData(2,Qt::Horizontal,"事件源");//source
    EventCenter_Model->setHeaderData(3,Qt::Horizontal,"事件类型");//type
    EventCenter_Model->setHeaderData(4,Qt::Horizontal,"事件级别");//level
    EventCenter_Model->setHeaderData(5,Qt::Horizontal,"事件细节");//details
    EventCenter_Model->setHeaderData(6,Qt::Horizontal,"状态");//status
    EventCenter_Model->select();

    //    tableView_inspectionTasks->setModel(inspectionTasksModel);
    //    tableView_inspectionTasks->resizeColumnsToContents();
    return QSqlError();
}

bool SQLite::add_EventCenter(const QString &source, const QString &type, const QString &level, const QString &details, const QString &status)
{
    int row = 0;//第一行插入
    EventCenter_Model->insertRow(row);
    //EventCenter_Model->setData(EventCenter_Model->index(row, 0), id);
    EventCenter_Model->setData(EventCenter_Model->index(row, 1), QDateTime::currentDateTime());
    EventCenter_Model->setData(EventCenter_Model->index(row, 2), source);
    EventCenter_Model->setData(EventCenter_Model->index(row, 3), type);
    EventCenter_Model->setData(EventCenter_Model->index(row, 4), level);
    EventCenter_Model->setData(EventCenter_Model->index(row, 5), details);
    EventCenter_Model->setData(EventCenter_Model->index(row, 6), status);
    if (EventCenter_Model->submitAll()) {
        EventCenter_Model->select();
        return true;
    } else {
        qWarning() << "Failed to insert Event Center:" << EventCenter_Model->lastError().text();
        return false;
    }
}

QSqlError SQLite::init_inspectionTasks()
{
    if(!db.tables().contains(InspectionTasks, Qt::CaseInsensitive)){
        QSqlQuery q(db);
        if(!q.exec(InspectionTasks_SQL))
            return q.lastError();
    }

    inspectionTasksModel = new QSqlTableModel(nullptr, db);
    inspectionTasksModel->setTable(InspectionTasks);
    inspectionTasksModel->setEditStrategy(QSqlTableModel::OnFieldChange);
    inspectionTasksModel->setHeaderData(0, Qt::Horizontal, tr("ID"));//id
    inspectionTasksModel->setHeaderData(1, Qt::Horizontal, tr("任务名称"));//taskName
    inspectionTasksModel->setHeaderData(2, Qt::Horizontal, tr("任务点数"));//numOfCheckpoints
    inspectionTasksModel->setHeaderData(3, Qt::Horizontal, tr("正常点数"));//numOfNormalPoints
    inspectionTasksModel->setHeaderData(4, Qt::Horizontal, tr("错误点数"));//numOfErrorPoints
    inspectionTasksModel->setHeaderData(5, Qt::Horizontal, tr("报警点数"));//numOfAlarmPoints
    inspectionTasksModel->setHeaderData(6, Qt::Horizontal, tr("巡检结果"));//checkResult
    inspectionTasksModel->setHeaderData(7, Qt::Horizontal, tr("巡检时间"));//startTime
    inspectionTasksModel->setHeaderData(8, Qt::Horizontal, tr("其他"));//other
    inspectionTasksModel->select();

//    tableView_inspectionTasks->setModel(inspectionTasksModel);
//    tableView_inspectionTasks->resizeColumnsToContents();
    return QSqlError();
}

bool SQLite::add_inspectionTasks(const QString &taskName, int numOfCheckpoints, int numOfNormalPoints, int numOfErrorPoints, int numOfAlarmPoints, const QString &checkResult, const QDateTime &startTime, const QString &other)
{
//    int row = inspectionTasksModel->rowCount();
    int row = 0;//第一行插入
    inspectionTasksModel->insertRow(row);
    inspectionTasksModel->setData(inspectionTasksModel->index(row, 0), taskUuid);
    inspectionTasksModel->setData(inspectionTasksModel->index(row, 1), taskName);
    inspectionTasksModel->setData(inspectionTasksModel->index(row, 2), numOfCheckpoints);
    inspectionTasksModel->setData(inspectionTasksModel->index(row, 3), numOfNormalPoints);
    inspectionTasksModel->setData(inspectionTasksModel->index(row, 4), numOfErrorPoints);
    inspectionTasksModel->setData(inspectionTasksModel->index(row, 5), numOfAlarmPoints);
    inspectionTasksModel->setData(inspectionTasksModel->index(row, 6), checkResult);
    inspectionTasksModel->setData(inspectionTasksModel->index(row, 7), startTime);
    inspectionTasksModel->setData(inspectionTasksModel->index(row, 8), other);
    if (inspectionTasksModel->submitAll()) {
        inspectionTasksModel->select();
        return true;
    } else {
        qWarning() << "Failed to insert inspection task:" << inspectionTasksModel->lastError().text();
        return false;
    }
}

QSqlError SQLite::init_inspectionCheckpoints()
{
    if(!db.tables().contains(InspectionCheckpoints, Qt::CaseInsensitive)){
        QSqlQuery q(db);
        if(!q.exec(InspectionCheckpoints_SQL))
            return q.lastError();
    }

    inspectionCheckpoints_Model = new QSqlTableModel(nullptr, db);
    inspectionCheckpoints_Model->setTable(InspectionCheckpoints);
    inspectionCheckpoints_Model->setEditStrategy(QSqlTableModel::OnManualSubmit);

    inspectionCheckpoints_Model->setHeaderData(0, Qt::Horizontal, tr("ID"));//id
    inspectionCheckpoints_Model->setHeaderData(1, Qt::Horizontal, tr("任务ID"));//taskId
    inspectionCheckpoints_Model->setHeaderData(2, Qt::Horizontal, tr("巡检名称"));//checkpointName
    inspectionCheckpoints_Model->setHeaderData(3, Qt::Horizontal, tr("巡检内容"));//checkpointContent
    inspectionCheckpoints_Model->setHeaderData(4, Qt::Horizontal, tr("巡检结果"));//checkResult
    inspectionCheckpoints_Model->setHeaderData(5, Qt::Horizontal, tr("备注"));//remark
    inspectionCheckpoints_Model->setHeaderData(6, Qt::Horizontal, tr("巡检时间"));//
    inspectionCheckpoints_Model->select();

    //    tableView_inspectionTasks->setModel(inspectionTasksModel);
    //    tableView_inspectionTasks->resizeColumnsToContents();
    return QSqlError();
}

bool SQLite::add_InspectionCheckpoint(const QString &checkpointName, const QString &checkpointContent, const QString &checkResult, const QString &remark, const QDateTime &checkTime)
{
//    int row = inspectionCheckpoints_Model->rowCount();
    int row = 0;//第一行插入
    inspectionCheckpoints_Model->insertRow(row);
    inspectionCheckpoints_Model->setData(inspectionCheckpoints_Model->index(row, 1), taskUuid);
    inspectionCheckpoints_Model->setData(inspectionCheckpoints_Model->index(row, 2), checkpointName);
    inspectionCheckpoints_Model->setData(inspectionCheckpoints_Model->index(row, 3), checkpointContent);
    inspectionCheckpoints_Model->setData(inspectionCheckpoints_Model->index(row, 4), checkResult);
    inspectionCheckpoints_Model->setData(inspectionCheckpoints_Model->index(row, 5), remark);
    inspectionCheckpoints_Model->setData(inspectionCheckpoints_Model->index(row, 6), checkTime);

    if (inspectionCheckpoints_Model->submitAll()) {
        inspectionCheckpoints_Model->select();
        return true;
    } else {
        qWarning() << "Failed to insert inspection checkpoint:" << inspectionCheckpoints_Model->lastError().text();
        return false;
    }
}
