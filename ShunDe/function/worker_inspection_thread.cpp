#include "worker_inspection_thread.h"
#include "modules/sqlite.h"

WorkerInspectionThread::WorkerInspectionThread(Robot *robot, QObject *parent)
    : QThread{parent}, m_robot(robot)
{
}

WorkerInspectionThread::~WorkerInspectionThread()
{
    qDebug() << "线程释放 WorkerInspectionThread::~WorkerInspectionThread()";
}

void WorkerInspectionThread::task_time_array_insert_data(QString task_name, QString task_time, QDateTime time)
{
    QJsonObject timeObj;
    timeObj["task_name"] = task_name;
    timeObj["task_time"] = task_time;
    timeObj["QDateTime"] = time.toString("yyyy-MM-dd hh:mm:ss");

    // 找到合适的插入位置
    int index = 0;
    for (int i = 0; i < task_time_array.size(); ++i)
    {
        QJsonObject varObj = task_time_array[i].toObject();
        if (QDateTime::fromString(varObj["QDateTime"].toString(), "yyyy-MM-dd hh:mm:ss") > time)
        {
            break;
        }
        index = i+1;
    }
    // 插入新的任务信息
    task_time_array.insert(index, timeObj);

    //qDebug() << "task_time_array_insert_data" <<task_time_array;
}

void WorkerInspectionThread::task_time_array_delete_data_by_task_name(QString task_name)
{
    // 遍历 task_time_array,找到所有匹配的任务名称
    QList<int> indexesToRemove;
    for (int i = 0; i < task_time_array.size(); ++i)
    {
        QJsonObject obj = task_time_array[i].toObject();
        if (obj["task_name"].toString() == task_name)
        {
            indexesToRemove.append(i);
        }
    }

    // 从后向前删除元素,避免索引变动
    for (int i = indexesToRemove.size() - 1; i >= 0; --i)
    {
        task_time_array.removeAt(indexesToRemove[i]);
    }
}

QJsonObject WorkerInspectionThread::task_time_array_get_current_enable_task()
{
    // 从第一个任务开始遍历
    for (const auto &taskTimeValue : task_time_array)
    {
        QJsonObject taskTimeObject = taskTimeValue.toObject();
        QString taskName = taskTimeObject["task_name"].toString();
        if (Qt::CheckState(m_robot->inspection.config["tasks"].toObject()[taskName].toObject()["isEnable"].toInt(0)) == Qt::Checked)
        {
            return taskTimeObject;
        }
    }
    // 如果没有找到被启用的任务,返回一个空的 QJsonObject
    return QJsonObject();
}

void WorkerInspectionThread::updata_task_run_time(const QString &taskName)
{
    //qDebug() << "更新任务时间WorkerInspectionThread::updata_task_run_time(const QString &" << taskName;
    QJsonObject task = m_robot->inspection.config["tasks"].toObject()[taskName].toObject();
    //qDebug() << "任务时间:" << task["time"].toArray();
    if (tasksTimers.contains(taskName))
    {
        for (QTimer *timer : tasksTimers[taskName])
        {
            timer->stop();
            timer->deleteLater();
        }
        tasksTimers.remove(taskName);
        // delete tasksTimers[taskName].detach();
        task_time_array_delete_data_by_task_name(taskName);
    }

    QJsonArray taskTimeArray = task["time"].toArray();
    // 创建新的定时器
    QList<QTimer *> timers;
    for (const auto &timeValue : taskTimeArray)
    {
        QJsonObject time = timeValue.toObject();

        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, this, [=]()
                {
            qDebug() << "任务:"<<taskName;
            qint64 value = m_robot->inspection.getNextTimeInterval(time);
            //qDebug() << "任务名称:"<<taskName<<"time:"<<time<<"msec"<<value;
            if(value==-1){
                qWarning() << "任务时间错误，任务名称:"<<taskName<<"time:"<<time;
                return;
            }
            timer->setInterval(value);
            current_task = task;
            current_task_name = taskName;
            //QJsonObject task_time = task_time_array.first().toObject();
            task_time_array.removeFirst();
            task_time_array_insert_data(taskName, time["time"].toString(), QDateTime::currentDateTime().addMSecs(value));
            start(); });
        int msec = m_robot->inspection.getFirstTime(time);
        if (msec == -1)
        {
            qWarning() << "任务时间错误，任务名称:" << taskName << "time:" << time;
        }
        //qDebug() << "任务名称:" << taskName << "time:" << time << "msec" << msec;
        task_time_array_insert_data(taskName, time["time"].toString(), QDateTime::currentDateTime().addMSecs(msec));
        timer->start(msec);
        timers.append(timer);
    }

    tasksTimers[taskName] = timers;

    QJsonObject task_time_next = task_time_array_get_current_enable_task();
    m_robot->inspection_data.task_next_name = task_time_next["task_name"].toString();
    m_robot->inspection_data.task_next_time = task_time_next["task_time"].toString();
    m_robot->update_inspection_data_show(InspectionUpdata_task_next);
}

void WorkerInspectionThread::run()
{
    qDebug() << "线程WorkerInspectionThread m_robot" << m_robot->id << m_robot->pose << QThread::currentThreadId();
    qDebug() << "m_robot current_task" << current_task;
    // #1任务启用
    if (Qt::CheckState(current_task["isEnable"].toInt(0)) != Qt::Checked)
    {
        qDebug() << QTime::currentTime() << "任务未启用";
        return;
    }
    // get next task
    QJsonObject task_time_next = task_time_array_get_current_enable_task();

    m_robot->inspection_data.task_next_name = task_time_next["task_name"].toString();
    m_robot->inspection_data.task_next_time = task_time_next["task_time"].toString();
    m_robot->inspection_data.current_task_name = current_task_name;
    m_robot->start_inspection_task_and_data_show();

    // #2任务点
    int32_t point_timeout = current_task["point_timeout"].toInt(30 * 3600);   // 默认30分钟
    int32_t point_deviation = current_task["point_deviation"].toInteger(500); // 默认 0.05 米

    QJsonArray pointsArray = current_task["points"].toArray();
    int totalPoints = pointsArray.size();
    m_robot->inspection_data.totalPoints = totalPoints;
    m_robot->inspection_data.not_completed = totalPoints;
    m_robot->update_inspection_data_show(InspectionUpdata_task_current_completion_progress);

    /*数据记录*/
    InspectionTaskData taskData;
    taskData.taskUuid = QUuid::createUuid().toString(QUuid::WithoutBraces);
    taskData.taskName = current_task_name;
    taskData.numOfCheckpoints = totalPoints;
    taskData.startTime = QDateTime::currentDateTime();


    for (int i = 0; i < totalPoints; ++i)
    {
        QJsonObject point = pointsArray.at(i).toObject();
        // #2->0 任务点启用
        if (Qt::CheckState(point["isEnable"].toInt(0)) != Qt::Checked)
        {
            qDebug() << QTime::currentTime() <<"点："<<i<< point["pointName"].toString()<<"任务未启用";
            ++m_robot->inspection_data.completed;
            continue;
        }
        bool the_warnings = false;
        //    foreach (const QJsonValue& value , current_task["points"].toArray()) {
        //        QJsonObject point = value.toObject();
        m_robot->inspection_data.current_task_point_name = point["pointName"].toString();
        m_robot->inspection_data.current_task_point_next_name = pointsArray.at(i + 1).toObject()["pointName"].toString();
        m_robot->start_inspection_task_point_and_data_show();
        m_robot->update_inspection_data_show(InspectionUpdata_task_current_point);

         /*数据记录 保存巡检点数据*/
        InspectionCheckpointData checkpointData;
        checkpointData.taskUuid = taskData.taskUuid;
        checkpointData.checkpointName = m_robot->inspection_data.current_task_point_name;
        checkpointData.checkTime = QDateTime::currentDateTime();
        // #2->1 移动到任务点
        int32_t gotoPose = point["position"].toDouble() * 1000; //(单位m)
                                                                //        QEventLoop movePlace_Loop;
                                                                //        QTimer movePlaceTimer;
                                                                //        connect(&movePlaceTimer, &QTimer::timeout, &movePlace_Loop, &QEventLoop::quit);
                                                                //        int outer = 0;
                                                                //        int outerExec = 10000;
                                                                //        while (std::abs(gotoPose - m_robot->pose) < 500) {
                                                                //            QThread::sleep(1);
                                                                //            outer++;
                                                                //            if(outer > outerExec) break;
                                                                //        }

        m_robot->inspection_data.current_task_point_current_action = tr("导航中");
        m_robot->update_inspection_data_show(InspectionUpdata_task_current_point_action);

        if (m_robot->moveTo(gotoPose))
        {
            int32_t prevDistance = -1.0; // 上一次的距离
            for (int i = 0; i < point_timeout; i++)
            {
                int32_t distance = std::abs(gotoPose - m_robot->pose);
                // 只有距离变化时才更新显示
                if (distance != prevDistance)
                {
                    prevDistance = distance;
                    m_robot->inspection_data.current_task_point_current_progress = tr("距离: %1 m").arg(prevDistance / 1000.0,0,'f',3);
                    m_robot->update_inspection_data_show(InspectionUpdata_task_current_poiont_progress);
                }
                // 距离小于误差表示完成
                if (distance < point_deviation)
                {
                    m_robot->inspection_data.current_task_point_current_progress = tr("导航完成");
                    m_robot->update_inspection_data_show(InspectionUpdata_task_current_poiont_progress);
                    break;
                }
                QThread::sleep(1);
            }
        }
        else
        {
            m_robot->inspection_data.current_task_point_current_progress = tr("导航未完成");
            m_robot->update_inspection_data_show(InspectionUpdata_task_current_poiont_progress);
            qDebug() << QTime::currentTime() << "导航失败，未完成";
            EventCenterData eventPoint;
            eventPoint.time = QDateTime::currentDateTime();
            eventPoint.source = point["pointName"].toString();
            eventPoint.type = "导航";
            eventPoint.level = EventLevel_Warning;
            QString position = QString("位置: %1 m").arg(point["position"].toDouble(),0,'f',3);;
            eventPoint.details = QString("%1,%2,%3,%4").arg(m_robot->name,taskData.taskName,point["pointName"].toString(),position);
            eventPoint.status = "";
            SQL->add_EventCenter(eventPoint);
            the_warnings = true;
            // QThread::sleep(5);
        }

        // #2->2 任务点操作
        m_robot->inspection_data.current_task_point_current_progress = tr("巡检中");
        m_robot->update_inspection_data_show(InspectionUpdata_task_current_poiont_progress);
        QJsonArray actionsArray = point["action"].toArray();
        foreach (const QJsonValue &valueAction, actionsArray)
        {
            QJsonObject action = valueAction.toObject();
            // #2->2->1 任务点动作操作，json格式
            qDebug() << "WorkerInspectionThread::run() Inspection::action:" << action;
            foreach (const QString keyAction, action.keys())
            {
                qDebug() << "WorkerInspectionThread::run() Inspection::action:keyAction" << keyAction;
                m_robot->inspection_data.current_task_point_current_action = m_robot->inspection.get_action_operation_display(PointAction(keyAction.toInt()), action[keyAction].toObject());
                m_robot->update_inspection_data_show(InspectionUpdata_task_current_point_action);

                if (!m_robot->run_action_operation(PointAction(keyAction.toInt()), action[keyAction].toObject()))
                {
                    the_warnings = true;
                    EventCenterData eventPoint;
                    eventPoint.time = QDateTime::currentDateTime();
                    eventPoint.source = point["pointName"].toString();
                    eventPoint.type = "任务点操作";
                    eventPoint.level = EventLevel_Warning;
                    QString eventPoint_position = QString("位置: %1 m").arg(point["position"].toDouble(),0,'f',3);
                    QString eventPoint_action = m_robot->inspection.get_action_operation_display(PointAction(keyAction.toInt()),action[keyAction].toObject());
                    eventPoint.details = QString("%1,%2,%3,%4,%5").arg(m_robot->name,taskData.taskName,point["pointName"].toString(),eventPoint_position,eventPoint_action);
                    eventPoint.status = "";
                    SQL->add_EventCenter(eventPoint);
                    qDebug() << QTime::currentTime() << "任务点操作失败，未完成";
                };
            } // action
            // 点任务显示
        } // actionsArray

        // #2->3 任务点操作完成，结算
        m_robot->inspection_data.current_task_point_current_progress = tr("完成");
        m_robot->update_inspection_data_show(InspectionUpdata_task_current_poiont_progress);

        if (the_warnings)
        {
            m_robot->inspection_data.warnings++; // 异常点加1
            m_robot->inspection_data.task_current_state = tr("异常%1个").arg(m_robot->inspection_data.warnings);
            m_robot->update_inspection_data_show(InspectionUpdata_task_current_state);
            checkpointData.checkResult = "异常";
        }
        else
        {
            m_robot->inspection_data.completed++;
            checkpointData.checkResult = "正常";
        }
        m_robot->inspection_data.not_completed = totalPoints - i - 1;
        m_robot->update_inspection_data_show(InspectionUpdata_task_current_completion_progress);

        checkpointData.checkpointContent = m_robot->inspection_data.current_task_point_content_strList.join(",");

        //checkpointData.remark = "";
        SQL->add_InspectionCheckpoint(checkpointData);

    } // points
    // #3任务完成操作
    m_robot->end_inspection_task_and_data_show();

    taskData.numOfErrorPoints = m_robot->inspection_data.warnings;
    taskData.numOfNormalPoints = m_robot->inspection_data.completed;
    taskData.checkResult = m_robot->inspection_data.task_current_state;

    SQL->add_inspectionTasks(taskData);

}
