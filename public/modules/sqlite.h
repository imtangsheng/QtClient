#ifndef SQLITE_H
#define SQLITE_H

#include <QTableView>
#include <QWidget>
#include <QtSql>
#include "ui_sqlite.h"

/*
 * EventCenter表:用于报警信息的相关信息。
*/
enum EventLevel{
    EventLevel_Debug,
    EventLevel_Info,
    EventLevel_Warning,
    EventLevel_Error,
};

struct EventCenterData {
    QDateTime time;
    QString source;
    QString type;
    EventLevel level;
    QString details;
    QString status;
};


/*
 * InspectionTasks 表:用于存储巡检任务的相关信息。
 * InspectionCheckpoints 表:用于存储每个巡检任务下的具体巡检点信息。
*/
struct InspectionTaskData {
    QString taskUuid;
    QString taskName;
    int numOfCheckpoints;
    int numOfNormalPoints = 0;
    int numOfErrorPoints = 0;
    int numOfAlarmPoints = 0;
    QString checkResult;
    QDateTime startTime;
    QString other;
};

struct InspectionCheckpointData {
    QString checkpointUuid;
    QString taskUuid; // 关联到 inspectionTasks 表
    QString checkpointName;
    QString checkpointContent;
    QString checkResult;
    QString remark;
    QDateTime checkTime;
};

#define ItemDelegateImageShow 3 //巡检结果列显示图片
#define ItemDelegateEventLevel 4 //异常报警事件等级显示

namespace Ui {
class SQLite;
}

class SQLite : public QWidget
{
    Q_OBJECT

public:
    explicit SQLite(QWidget *parent = nullptr);
    ~SQLite();

    static SQLite *instance(QWidget *parent = nullptr,const QString& db_filename="sql.db");
    static void shutdownHandler();
    //应用程序中需要启动一次，不用重复启用，默认为最后一个数据库
    QSqlError initDb(const QString& name="sql.db", const QString& connectionName="default");
    //数据库定义
    // 创建一个 SQLite 数据库连接
    QSqlDatabase db;
    QSqlQuery query;
    //事件中心表
    QSqlError delete_events(int id);
    QSqlError updata_keyword_from_events(const int &id,const QString& keyword); //modify
    //巡检任务表
    QSqlTableModel* EventCenter_Model;
    QSqlError init_EventCenter();
    bool add_EventCenter(const QString &source, const QString &type, const QString &level, const QString &details, const QString &status);
    bool add_EventCenter(const EventCenterData& data);
    bool filter_EventCenter(int column,const QString& value);

    //巡检任务表
    QString taskUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    QSqlTableModel* inspectionTasksModel;
    QSqlError init_inspectionTasks();
    bool add_inspectionTasks(const QString& taskName, int numOfCheckpoints, int numOfNormalPoints, int numOfErrorPoints, int numOfAlarmPoints, const QString& checkResult, const QDateTime& startTime, const QString& other);
    bool add_inspectionTasks(const InspectionTaskData& taskData);
    bool filter_inspectionTasks(int column,const QString& value);

    //巡检任务表详情点
    QSqlTableModel* inspectionCheckpoints_Model;
    QSqlError init_inspectionCheckpoints();
    bool add_InspectionCheckpoint(const QString& checkpointName, const QString& checkpointContent, const QString& checkResult, const QString& remark, const QDateTime& checkTime);
    bool add_InspectionCheckpoint(const InspectionCheckpointData& checkpointData);
    //void updata_inspectionTasks_View();
//    QSqlError delete_inspectionTasks(int id);
//    void search_keyword_from_inspectionTasks(const QString& keyword);
//    void filter_inspectionTasks_by_Time(const QDateTime& startTime, const QDateTime& endTime);
//    QSqlError updata_keyword_from_inspectionTasks(const int &id,const QString& keyword); //modify

public:
    Ui::SQLite *ui;

private:
    static SQLite* m_instance;
};

extern SQLite* gSql;

/*支持图片查看器*/
#include <QApplication>
#include <QDialog>
#include <QHelpEvent>
#include <QItemDelegate>
#include <QLabel>
#include <QPainter>
#include <QPushButton>
#include <QScreen>
#include <QToolTip>

class ImageDelegate_Single : public QItemDelegate
{
public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.column() == ItemDelegateImageShow) { // "巡检结果"列
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
        if (index.column() == ItemDelegateImageShow) { // "巡检结果"列
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
                    QDialog dialog(gSql, Qt::Window);
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

class ImageDelegate_List : public QItemDelegate
{
    Q_OBJECT

public:
    int spacing = 0;
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.column() == ItemDelegateImageShow) { // "巡检结果"列 使用，号隔开
            QStringList imagePaths = index.model()->data(index, Qt::DisplayRole).toString().split(",", Qt::SkipEmptyParts);
            if (!imagePaths.isEmpty()) {
                int imageCount = imagePaths.size();
                int thumbnailSize = qMin(option.rect.width() / imageCount, option.rect.height());


                for (int i = 0; i < imageCount; ++i) {
                    QRect imageRect(option.rect.left() + i * (thumbnailSize + spacing),
                                   option.rect.top(),
                                   thumbnailSize, thumbnailSize);
                    paintImage(painter, imagePaths[i], imageRect, option.rect);
                }
            }
        } else {
            QItemDelegate::paint(painter, option, index);
        }
    }

    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        if (event->type() == QEvent::MouseButtonDblClick && index.column() == 3) { // 双击"巡检结果"列
            QStringList imagePaths = index.model()->data(index, Qt::DisplayRole).toString().split(",", Qt::SkipEmptyParts);
            if (!imagePaths.isEmpty()) {
                showImageViewer(imagePaths);
            }
            return true; // 阻止事件继续传播
        }
        return QItemDelegate::editorEvent(event, model, option, index);
    }

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override
    {
        if (event->type() == QEvent::ToolTip && index.column() == ItemDelegateImageShow) {
            QStringList imagePaths = index.model()->data(index, Qt::DisplayRole).toString().split(",", Qt::SkipEmptyParts);
            if (!imagePaths.isEmpty()) {
                QToolTip::showText(event->globalPos(), imagePaths.join("\n"));
            }
            return true;
        }
        return QItemDelegate::helpEvent(event, view, option, index);
    }

private:
    void paintImage(QPainter* painter, const QString& imagePath, const QRect& imageRect, const QRect& cellRect) const {
        QImage image(imagePath);
        if (!image.isNull()) {
            QRect scaledRect = imageRect.adjusted(spacing, spacing, -spacing, -spacing);
            scaledRect.setSize(image.size().scaled(scaledRect.size(), Qt::KeepAspectRatio));
            scaledRect.moveCenter(imageRect.center());
            painter->drawImage(scaledRect, image);
        } else {
            painter->fillRect(imageRect, Qt::lightGray);
            painter->drawText(cellRect, Qt::AlignCenter, imagePath);
        }
    }

    void showImageViewer(const QStringList& imagePaths) {
        // 创建一个模态的对话框
        QDialog dialog(gSql, Qt::Window);
        dialog.setWindowTitle(tr("图像查看器"));
        QVBoxLayout layout(&dialog);
        layout.setContentsMargins(0, 0, 0, 0);

        // 创建一个标签
        QLabel label(&dialog);
        label.setAlignment(Qt::AlignCenter);
        label.setScaledContents(true);

        // 设置当前显示的图像索引
        int currentIndex = 0;

        // 显示第一张图像
        if (!imagePaths.isEmpty()) {
            updateLabelImage(label, imagePaths[currentIndex]);
        }

        // 如果只有一张图片,不显示导航按钮
        if (imagePaths.size() == 1) {
            layout.addWidget(&label);
            dialog.exec();
            return;
        }

        // 添加导航按钮
        QHBoxLayout buttonLayout;
        QPushButton* prevButton = new QPushButton(tr("上一张"));
        QPushButton* nextButton = new QPushButton(tr("下一张"));
        buttonLayout.addWidget(prevButton);
        buttonLayout.addWidget(nextButton);
        layout.addLayout(&buttonLayout);

        // 连接按钮的点击信号
        connect(prevButton, &QPushButton::clicked, [&]() {
            currentIndex = (currentIndex - 1 + imagePaths.size()) % imagePaths.size();
            updateLabelImage(label, imagePaths[currentIndex]);
        });
        connect(nextButton, &QPushButton::clicked, [&]() {
            currentIndex = (currentIndex + 1) % imagePaths.size();
            updateLabelImage(label, imagePaths[currentIndex]);
        });

        // 显示对话框
        layout.addWidget(&label);
        dialog.exec();
    }

    void updateLabelImage(QLabel& label, const QString& imagePath) {
        QImage image(imagePath);
        if (!image.isNull()) {
            label.setPixmap(QPixmap::fromImage(image));
        } else {
            label.setText(tr("图片不存在: %1").arg(imagePath));
        }
    }
};


class EventLevelDelegate : public QItemDelegate {
    Q_OBJECT

public:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.column() == ItemDelegateEventLevel) {
            const EventLevel eventLevel = static_cast<EventLevel>(index.data(Qt::DisplayRole).toInt());
            QString text;
            switch (eventLevel) {
            case EventLevel::EventLevel_Debug:
                text = tr("提示");
                break;
            case EventLevel::EventLevel_Info:
                text = tr("显示");
                break;
            case EventLevel::EventLevel_Warning:
                text = tr("警告");
                painter->save();
                painter->setPen(Qt::yellow);
                break;
            case EventLevel::EventLevel_Error:
                text = tr("错误");
                painter->save();
                painter->setPen(Qt::red);
                break;
            default:
                text = tr("未知等级:%1").arg(eventLevel);
            }

            painter->drawText(option.rect, Qt::AlignCenter, text);
            if (eventLevel == EventLevel::EventLevel_Error) {
                painter->restore();
            }
        } else {
            QItemDelegate::paint(painter, option, index);
        }
    }
};

#endif // SQLITE_H
