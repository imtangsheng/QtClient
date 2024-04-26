#ifndef SQLITE_H
#define SQLITE_H

#include <QTableView>
#include <QWidget>
#include <QtSql>

#include "ui_sqlite.h"

namespace Ui {
class SQLite;
}

class SQLite : public QWidget
{
    Q_OBJECT

public:
    explicit SQLite(QWidget *parent = nullptr);
    ~SQLite();
    //应用程序中需要启动一次，不用重复启用，默认为最后一个数据库
    QSqlError initDb(const QString& name=nullptr, const QString& connectionName="default");
    //数据库定义
    // 创建一个 SQLite 数据库连接
    QSqlDatabase db;
    QSqlQuery query;
    //事件中心表
    QSqlQueryModel model_events;
    QSortFilterProxyModel proxyModel_events;
    QSqlError init_events();
    QVariant add_event(const QString &source, const QString &type, const QString &level, const QString &details, const QString &status);
    void updata_eventsView();
    QSqlError delete_events(int id);
    void search_keyword_from_events(const QString& keyword);
    void filter_events_by_Time(const QDateTime& startTime, const QDateTime& endTime);
    QSqlError updata_keyword_from_events(const int &id,const QString& keyword); //modify
    //巡检任务表
    QSqlTableModel* EventCenter_Model;
    QSqlError init_EventCenter();
    bool add_EventCenter(const QString &source, const QString &type, const QString &level, const QString &details, const QString &status);


    //巡检任务表
    QString taskUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlTableModel* inspectionTasksModel;
    QSqlError init_inspectionTasks();
    bool add_inspectionTasks(const QString& taskName, int numOfCheckpoints, int numOfNormalPoints, int numOfErrorPoints, int numOfAlarmPoints, const QString& checkResult, const QDateTime& startTime, const QString& other);

    //巡检任务表详情点
    QSqlTableModel* inspectionCheckpoints_Model;
    QSqlError init_inspectionCheckpoints();
    bool add_InspectionCheckpoint(const QString& checkpointName, const QString& checkpointContent, const QString& checkResult, const QString& remark, const QDateTime& checkTime);
    //void updata_inspectionTasks_View();
//    QSqlError delete_inspectionTasks(int id);
//    void search_keyword_from_inspectionTasks(const QString& keyword);
//    void filter_inspectionTasks_by_Time(const QDateTime& startTime, const QDateTime& endTime);
//    QSqlError updata_keyword_from_inspectionTasks(const int &id,const QString& keyword); //modify

public:
    Ui::SQLite *ui;

private:

};

extern SQLite* SQL;

#include <QApplication>
#include <QDialog>
#include <QItemDelegate>
#include <QLabel>
#include <QPainter>
#include <QScreen>

class ImageDelegate1 : public QItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.column() == 3) { // "巡检结果"列
            QString imagePath = index.model()->data(index, Qt::DisplayRole).toString();
            if (!imagePath.isEmpty()) {
                QImage image(imagePath);
                painter->drawImage(option.rect, image);
            }
        } else {
            QItemDelegate::paint(painter, option, index);
        }
    }
};

class ImageDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.column() == 3) { // "巡检结果"列
            QString imagePath = index.model()->data(index, Qt::DisplayRole).toString();
            if (!imagePath.isEmpty()) {
                QImage image(imagePath);
                if (!image.isNull()) {
                    QRect imageRect = option.rect;
                    imageRect.setSize(image.size().scaled(imageRect.size(), Qt::KeepAspectRatio));
                    imageRect.moveCenter(option.rect.center());
                    painter->drawImage(imageRect, image);
                }
            }
        } else {
            QItemDelegate::paint(painter, option, index);
        }
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        if (event->type() == QEvent::MouseButtonDblClick && index.column() == 3) { // 双击"巡检结果"列
            QString imagePath = index.model()->data(index, Qt::DisplayRole).toString();
            if (!imagePath.isEmpty()) {
                QImage image(imagePath);
                if (!image.isNull()) {
                    // 创建一个模态的对话框
                    QDialog dialog(SQL, Qt::Window);
                    dialog.setWindowTitle(tr("图像查看器"));
                    QVBoxLayout layout(&dialog);
                    layout.setContentsMargins(0,0,0,0);
                    // 创建一个标签
                    QLabel label(&dialog);
                    // 创建独立的图像查看器窗口
                    label.setPixmap(QPixmap::fromImage(image));
                    label.setAlignment(Qt::AlignCenter);
                    label.setScaledContents(true);
                    label.setAlignment(Qt::AlignCenter);
                    // 显示对话框
                    layout.addWidget(&label);
//                    dialog
                    dialog.exec();
                }
            }
            return true; // 阻止事件继续传播
        }
        return QItemDelegate::editorEvent(event, model, option, index);
    }
};

#endif // SQLITE_H
