#include "worker_inspection_thread.h"

WorkerInspectionThread::WorkerInspectionThread(Robot *robot,QObject *parent)
    : QThread{parent},m_robot(robot)
{

}

WorkerInspectionThread::~WorkerInspectionThread()
{
    qDebug() << "线程释放 WorkerInspectionThread::~WorkerInspectionThread()";
}

void WorkerInspectionThread::run()
{
    qDebug()<<"线程WorkerInspectionThread m_robot"<<m_robot->id<<m_robot->pose<<QThread::currentThreadId();
    qDebug()<<"m_robot current_task"<<current_task;
    int i = 0;
    while (1) {
        qDebug() <<"线程执行任务:"<<QTime::currentTime()<<"timer;"<<i++<<m_robot->pose<<QThread::currentThreadId();
        QThread::sleep(1);
        if(i>5) break;
    }
}

void WorkerInspectionThread::updata_task_run_time(const QString &taskName)
{
    qDebug() << "更新任务时间WorkerInspectionThread::updata_task_run_time(const QString &"<<taskName;
    QJsonObject task = m_robot->inspection.config["tasks"].toObject()[taskName].toObject();
    qDebug() << "任务时间:"<<task["time"].toArray();
    if(tasksTimers.contains(taskName)){
        for (QTimer* timer : tasksTimers[taskName]) {
            timer->stop();
            timer->deleteLater();
        }
        tasksTimers.remove(taskName);
        //delete tasksTimers[taskName].detach();
    }

    QJsonArray taskTimeArray = task["time"].toArray();
    // 创建新的定时器
    QList<QTimer*> timers;
    for (const auto& timeValue : taskTimeArray) {
        QJsonObject time = timeValue.toObject();

        QTimer* timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [=]() {
            qDebug() << "任务:"<<taskName;
            qint64 value = m_robot->inspection.getNextTimeInterval(time);
            qDebug() << "任务名称:"<<taskName<<"time:"<<time<<"msec"<<value;
            if(value==-1){
                qWarning() << "任务时间错误，任务名称:"<<taskName<<"time:"<<time;
                return;
            }
            timer->setInterval(value);
            current_task = task;
            start();
        });
        int msec = m_robot->inspection.getFirstTime(time);
        if(msec == -1){
            qWarning() << "任务时间错误，任务名称:"<<taskName<<"time:"<<time;
        }
        qDebug() << "任务名称:"<<taskName<<"time:"<<time<<"msec"<<msec;
        timer->start(msec);
        timers.append(timer);
    }

    tasksTimers[taskName]  = timers;
}
