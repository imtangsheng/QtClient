#ifndef WORKER_INSPECTION_THREAD_H
#define WORKER_INSPECTION_THREAD_H

#include <QThread>
#include "modules/robot.h"

class WorkerInspectionThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkerInspectionThread(Robot *robot,QObject *parent = nullptr);
    ~WorkerInspectionThread();

    QJsonObject current_task;
//    void start_inspection_task();
    void run() override;
signals:


    // QThread interface
protected:


private:
    Robot *m_robot;
};

#endif // WORKER_INSPECTION_THREAD_H
