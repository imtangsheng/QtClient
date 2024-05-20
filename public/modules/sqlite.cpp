#include "sqlite.h"
SQLite* SQL;
#include <QTimeEdit>

QMap<int, QString> EventCenter_column_name;
#define EventCenter "EventCenter"

QMap<int, QString> InspectionTasks_column_name;
#define InspectionTasks "InspectionTasks"


#define InspectionCheckpoints "InspectionCheckpoints"


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
        if(!query.exec(events_SQL))
            return query.lastError();
    }
    EventCenter_column_name.clear();
    EventCenter_column_name[0] = "id";
    EventCenter_column_name[1] = "time";
    EventCenter_column_name[2] = "source";
    EventCenter_column_name[3] = "type";
    EventCenter_column_name[4] = "level";
    EventCenter_column_name[5] = "details";
    EventCenter_column_name[6] = "status";

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

bool SQLite::filter_EventCenter(int column, const QString &value)
{
    // 设置筛选条件
    QString filter = QString("%1 = '%2'").arg(EventCenter_column_name[column]).arg(value);
    EventCenter_Model->setFilter(filter);
    if (EventCenter_Model->select()) {
        return true;
    } else {
        qWarning() << "Failed to insert inspection task:" << EventCenter_Model->lastError().text();
        return false;
    }
}

QSqlError SQLite::init_inspectionTasks()
{
    if(!db.tables().contains(InspectionTasks, Qt::CaseInsensitive)){
        QSqlQuery q(db);
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
        if(!q.exec(InspectionTasks_SQL))
            return q.lastError();
    }

    InspectionTasks_column_name.clear();
    InspectionTasks_column_name[0] = "id";
    InspectionTasks_column_name[1] = "taskName";
    InspectionTasks_column_name[2] = "numOfCheckpoints";
    InspectionTasks_column_name[3] = "numOfNormalPoints";
    InspectionTasks_column_name[4] = "numOfErrorPoints";
    InspectionTasks_column_name[5] = "numOfAlarmPoints";
    InspectionTasks_column_name[6] = "checkResult";
    InspectionTasks_column_name[7] = "startTime";
    InspectionTasks_column_name[8] = "other";

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

bool SQLite::filter_inspectionTasks(int column, const QString &value)
{
    // 设置筛选条件
    QString filter = QString("%1 = '%2'").arg(InspectionTasks_column_name[column]).arg(value);
    inspectionTasksModel->setFilter(filter);
    if (inspectionTasksModel->select()) {
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
