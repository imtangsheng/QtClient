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
    qDebug()<<"m_robot"<<m_robot->id<<m_robot->pose<<QThread::currentThreadId();
    qDebug()<<"m_robot current_task"<<current_task;
    int i = 0;
    while (1) {
        qDebug() <<QTime::currentTime()<<"timer;"<<i++<<m_robot->pose<<QThread::currentThreadId();
        QThread::sleep(1);
        if(i>2000000) break;
    }
}
