#ifndef WORKER_INSPECTION_THREAD_H
#define WORKER_INSPECTION_THREAD_H

#include <QThread>
#include "modules/robot.h"

class WorkerInspectionThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkerInspectionThread(Robot *robot, QObject *parent = nullptr);
    ~WorkerInspectionThread();

    QMap<QString, QList<QTimer *>> tasksTimers;

    QJsonObject current_task;
    QString current_task_name;
    //    void start_inspection_task();
    QJsonArray task_time_array;
    void task_time_array_insert_data(QString task_name, QString task_time, QDateTime time);
    void task_time_array_delete_data_by_task_name(QString task_name);
    QJsonObject task_time_array_get_current_enable_task();

signals:

public slots:
    void updata_task_run_time(const QString &taskName);
    // QThread interface
protected:
    void run() override;

private:
    Robot *m_robot;
};

#endif // WORKER_INSPECTION_THREAD_H
