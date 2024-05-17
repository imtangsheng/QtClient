#include "inspection.h"
#include "AppUtil.h"

#include <QDateTime>
#include <QFileDialog>
#include <QMessageBox>
#include <QToolTip>

const int SECS_PER_DAY = 24 * 60 * 60; // 86400 秒

QString configPath("inspection.json");

Inspection::Inspection(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Inspection)
{
    ui->setupUi(this);
    connect(ui->comboBox_point_type,&QComboBox::currentIndexChanged,ui->stackedWidget_pointType,&QStackedWidget::setCurrentIndex);
    connect(ui->comboBox_type_time,&QComboBox::currentIndexChanged,ui->stackedWidget_typeTime,&QStackedWidget::setCurrentIndex);

}

Inspection::~Inspection()
{
    delete ui;
    qDebug()<<"Inspection::~Inspection()释放";
}

void Inspection::start()
{
    // 从JSON文件中读取配置数据
    // 默认读取，会从id中赋值的
    if(ReadJsonData(config,configPath)){
        qDebug()<<"从JSON文件中读取配置数据"<<config;
        update_config_show();

        QString currentTask = config["currentTask"].toString();
        if(!currentTask.isEmpty()){
            int row_currentTask = ui->comboBox_task_name->findText(currentTask);
            if(row_currentTask >=0 ){
                ui->comboBox_task_name->setCurrentIndex(row_currentTask);
            }
        }

        foreach (const QString taskName, config["tasks"].toObject().keys()) {
            emit updata_task_run_time(taskName);
        }
    }

    qDebug()<<"void Inspection::start()";
}

void Inspection::quit()
{
    qDebug()<<"Inspection::quit()退出";
}

void Inspection::update_config_show()
{
    //清理显示
    ui->comboBox_task_name->clear();
    ui->tableWidget_tasks->clearContents();
    ui->tableWidget_tasks->setRowCount(0);
    QJsonObject tasks = config["tasks"].toObject();
    foreach (const QString key, tasks.keys()) {
        ui->comboBox_task_name->addItem(key);//任务名称
        QJsonObject task = tasks[key].toObject();
        int row = ui->tableWidget_tasks->rowCount();
        ui->tableWidget_tasks->insertRow(row);
        ui->tableWidget_tasks->setItem(row,0,new QTableWidgetItem(key));
        ui->tableWidget_tasks->setItem(row,1,new QTableWidgetItem(
            Qt::CheckState(task["isEnable"].toInt()) ? "已启用" : "未启用"
                                                                    ));
        QStringList timeList;
        foreach (const QJsonValue& value , task["time"].toArray()){
            timeList.append(value.toObject()["time"].toString());
        }
        ui->tableWidget_tasks->setItem(row,2,new QTableWidgetItem(timeList.join(",")));
    }
    // 自适应大小
    ui->tableWidget_tasks->resizeColumnsToContents();

}

void Inspection::update_pointsJsonArray_show()
{
    //清理显示
    ui->tableWidget_task_points->clearContents();
    ui->tableWidget_task_points->setRowCount(0);
    foreach (const QJsonValue& value , pointsJsonArray) {
        QJsonObject point = value.toObject();
        qDebug()<<"update_pointsJsonArray_show()"<<point;
        int row = ui->tableWidget_task_points->rowCount();
        ui->tableWidget_task_points->insertRow(row);
        ui->tableWidget_task_points->setItem(row,0,new QTableWidgetItem(point["pointName"].toString()));
        ui->tableWidget_task_points->setItem(row,1,new QTableWidgetItem(
                                                   Qt::CheckState(point["isEnable"].toInt()) ? "已启用" : "未启用"
                                                   ));

        ui->tableWidget_task_points->setItem(row,2,new QTableWidgetItem(i2s(point["position"].toDouble())));
        QStringList actionList;
        QJsonArray actionsArray = point["action"].toArray();
        foreach(const QJsonValue& valueAction , actionsArray){
            QJsonObject action = valueAction.toObject();
            qDebug()<<"Inspection::action:"<<action;
            foreach (const QString keyAction, action.keys()) {
                qDebug()<<"Inspection::action:keyAction"<<keyAction;
                actionList.append(get_action_operation_display(PointAction(keyAction.toInt()),action[keyAction].toObject()));
            }//action
            //点任务显示

        }//actionsArray
        ui->tableWidget_task_points->setItem(row,3,new QTableWidgetItem(actionList.join(", ")));
    }//pointsJsonArray
    // 自适应大小
    ui->tableWidget_task_points->resizeColumnsToContents();

}

void Inspection::update_actionsJsonArray_show()
{
    ui->listWidget_task_point_action->clear();
    // 使用 Qt 迭代器
    foreach (const QJsonValue& value , actionsJsonArray) {
        QJsonObject action = value.toObject();
        qDebug()<<"Inspection::action:"<<action;
        foreach (const QString keyAction, action.keys()) {
            qDebug()<<"Inspection::action:keyAction"<<keyAction;
            ui->listWidget_task_point_action->addItem(get_action_operation_display(PointAction(keyAction.toInt()),action[keyAction].toObject()));
        }//action
        //点任务显示

    }//actionsArray
}

void Inspection::update_timesJsonArray_show()
{
    ui->listWidget_task_time->clear();
    // 使用 Qt 迭代器
    foreach (const QJsonValue& value , timesJsonArray) {
        QJsonObject time = value.toObject();
        qDebug()<<"Inspection::time:"<<time;
        //任务时间显示
        ui->listWidget_task_time->addItem(get_time_operation_display(time));
    }
//    // 使用经典的 for 循环
//    for (int i = 0; i < timesJsonArray.size(); ++i)
//    {
//        QString time = timesJsonArray[i].toString();
//        // 处理时间数据
//        ui->listWidget_task_time->addItem("时间："+time);
//    }

}

void Inspection::set_task()
{
    QString currentTask = ui->comboBox_task_name->currentText();
    config["currentTask"] =currentTask;
    QJsonObject task = config["tasks"].toObject()[currentTask].toObject();

    ui->checkBox_taskName_isEnable->setCheckState(Qt::CheckState(task["isEnable"].toInt()));//启用
    ui->lineEdit_personName->setText(task["personName"].toString());//
    ui->lineEdit_remark->setText(task["remark"].toString());//
    ui->spinBox_loopTime->setValue(task["loopTime"].toInt());//

    pointsJsonArray = task["points"].toArray();
    update_pointsJsonArray_show();

    timesJsonArray = task["time"].toArray();
    update_timesJsonArray_show();

}

QJsonObject Inspection::get_task()
{
    QJsonObject task;
    task["isEnable"] = ui->checkBox_taskName_isEnable->checkState();
    task["personName"] = ui->lineEdit_personName->text();
    task["remark"] = ui->lineEdit_remark->text();
    task["loopTime"] = ui->spinBox_loopTime->value();

    task["time"] = timesJsonArray;
    task["points"] = pointsJsonArray;
    return task;
}

void Inspection::set_point(QJsonObject point)
{
    ui->checkBox_point_isEnable->setCheckState(Qt::CheckState(point["isEnable"].toInt()));//启用
    ui->lineEdit_point_name->setText(point["pointName"].toString());//
    ui->doubleSpinBox_poiont_position->setValue(point["position"].toDouble());

    if(point.contains("action")){
        if(!ui->widgetActionSetting->isVisible()){
            ui->widgetActionSetting->setVisible(true);
        }
        actionsJsonArray=point["action"].toArray();
    }else{
        if(ui->widgetActionSetting->isVisible()){
            ui->widgetActionSetting->setVisible(false);
        }
        actionsJsonArray = QJsonArray();
    }
    update_actionsJsonArray_show();

}

QJsonObject Inspection::get_point()
{
    QJsonObject point;
    point["isEnable"] = ui->checkBox_point_isEnable->checkState();
    point["pointName"] = ui->lineEdit_point_name->text();
    point["position"] = ui->doubleSpinBox_poiont_position->value();
    if(ui->widgetActionSetting->isVisible()){
        point["action"] = actionsJsonArray;
    }
    return point;
}

QJsonObject Inspection::get_action_operation(PointAction operation)
{
    QJsonObject action;
    switch (operation) {
    case PointAction::PointAction_Time:
        action["sleep"] = ui->spinBox_time_second->value();
        break;
    case PointAction::PointAction_Vision_PTZControl:
        action["dwStop"] = ui->checkBox_vision_dwStop->checkState();
        action["dwPTZCommand"] = ui->comboBox_vision_dwPTZCommand->currentIndex();
        break;
    case PointAction::PointAction_Vision_PTZPreset:
        action["lChannel"] = ui->comboBox_vision_preset_lChannel->currentIndex() + 1;
        action["dwPresetIndex"] = ui->comboBox_vision_preset_dwPresetIndex->currentIndex() +1 ;
        action["dwPTZPresetCmd"] = ui->comboBox_vision_preset_dwPresetCmd->currentIndex();
        break;
    case PointAction::PointAction_Vision_PTZPOS:
        action["wAction"] = ui->comboBox_vision_PTZPOS_wAction->currentIndex() +1;
        action["wPanPos"] = ui->doubleSpinBox_vision_PTZPOS_wPanPos->value() * 10;
        action["wTiltPos"] = ui->doubleSpinBox_vision_PTZPOS_wTiltPos->value() * 10;
        action["wZoomPos"] = ui->doubleSpinBox_vision_PTZPOS_wZoomPos->value() * 10;
        break;
    case PointAction::PointAction_Vision_CaptureJPEGPicture:
        action["lChannel"] = ui->comboBox_vision_capture_lChannel->currentIndex() + 1;
        action["sPicFileName"] = ui->comboBox_vision_capture_sPicFileName->currentIndex();
        break;
    case PointAction::PointAction_Vision_Realtime_Thermometry:
        action["sPicFileName"] = ui->comboBox_vision_realtime_thermometry_dwCommand->currentIndex();
        break;
    case PointAction::PointAction_Vision_Other:
        action["other"] = ui->textEdit_other->toPlainText();
        break;
    default:
        break;
    }
    return action;
}

void Inspection::set_action_operation(PointAction operation, QJsonObject action)
{
    ui->comboBox_point_type->setCurrentIndex(operation);
    switch (operation) {
    case PointAction::PointAction_Time:
        ui->spinBox_time_second->setValue(action["sleep"].toInt());
        break;
    case PointAction::PointAction_Vision_PTZControl:
        ui->checkBox_vision_dwStop->setCheckState(Qt::CheckState(action["dwStop"].toInt()));
        ui->comboBox_vision_dwPTZCommand->setCurrentIndex(action["dwPTZCommand"].toInt());
        break;
    case PointAction::PointAction_Vision_PTZPreset:
        ui->comboBox_vision_preset_lChannel->setCurrentIndex(action["lChannel"].toInt() - 1);
        ui->comboBox_vision_preset_dwPresetIndex->setCurrentIndex(action["dwPresetIndex"].toInt() - 1);
        ui->comboBox_vision_preset_dwPresetCmd->setCurrentIndex(action["dwPTZPresetCmd"].toInt());
        break;
    case PointAction::PointAction_Vision_PTZPOS:
        ui->comboBox_vision_PTZPOS_wAction->setCurrentIndex(action["wAction"].toInt() - 1);
        ui->doubleSpinBox_vision_PTZPOS_wPanPos->setValue(action["wPanPos"].toDouble() / 10.0);
        ui->doubleSpinBox_vision_PTZPOS_wTiltPos->setValue(action["wTiltPos"].toDouble() / 10.0);
        ui->doubleSpinBox_vision_PTZPOS_wZoomPos->setValue(action["wZoomPos"].toDouble() / 10.0);
        break;
    case PointAction::PointAction_Vision_CaptureJPEGPicture:
        ui->comboBox_vision_capture_lChannel->setCurrentIndex(action["lChannel"].toInt() - 1);
        ui->comboBox_vision_capture_sPicFileName->setCurrentIndex(action["sPicFileName"].toInt());
        break;
    case PointAction::PointAction_Vision_Realtime_Thermometry:
        ui->comboBox_vision_realtime_thermometry_dwCommand->setCurrentIndex(action["sPicFileName"].toInt());
        break;
    case PointAction::PointAction_Vision_Other:
        ui->textEdit_other->setPlainText(action["other"].toString());
        break;
    default:
        break;
    }
}

QString Inspection::get_action_operation_display(PointAction operation, QJsonObject action)
{
    QString display;
    switch (operation) {
    case PointAction::PointAction_Time:
        display = "时间:"+i2s(action["sleep"].toInt());
        break;
    case PointAction::PointAction_Vision_PTZControl:
        display = "云台基本控制";
        break;
    case PointAction::PointAction_Vision_PTZPreset:
        display = "云台预置点功能:"+i2s(action["dwPresetIndex"].toInt());
        break;
    case PointAction::PointAction_Vision_PTZPOS:
        display = "云台设置PTZ参数";
        break;
    case PointAction::PointAction_Vision_CaptureJPEGPicture:
        display = "云台抓图:"+i2s(action["lChannel"].toInt());
        break;
    case PointAction::PointAction_Vision_Realtime_Thermometry:
        display = "热成像测温";
        break;
    case PointAction::PointAction_Vision_Other:
        display = "其他";
        break;
    default:
        break;
    }

    return display;
}

QJsonObject Inspection::get_time_operation()
{
    QJsonObject time;
    TypeTime type = TypeTime(ui->comboBox_type_time->currentIndex());
    time["type"] = type;
    time["time"] = ui->timeEdit_taks_time->time().toString("hh:mm:ss");
    switch (type) {
    case TypeTime_Day:
        time["date"] = ui->dateEdit_days_interval_begin->date().toString("yyyy-MM-dd");
        time["days_interval"] = ui->spinBox_days_interval->value();
        break;
    case TypeTime_Week:{
        QStringList week;
        week.append(i2s(ui->checkBox_time_week_1->checkState()));
        week.append(i2s(ui->checkBox_time_week_2->checkState()));
        week.append(i2s(ui->checkBox_time_week_3->checkState()));
        week.append(i2s(ui->checkBox_time_week_4->checkState()));
        week.append(i2s(ui->checkBox_time_week_5->checkState()));
        week.append(i2s(ui->checkBox_time_week_6->checkState()));
        week.append(i2s(ui->checkBox_time_week_7->checkState()));
        time["week"] = week.join(",");
        break;}
    default:
        break;
    }
    return time;
}


void Inspection::set_time_operation(QJsonObject time)
{
    qDebug()<<time;
    TypeTime type = TypeTime(time["type"].toInt());
    ui->comboBox_type_time->setCurrentIndex(type);
    ui->timeEdit_taks_time->setTime(QTime::fromString(time["time"].toString(), "hh:mm:ss"));
    switch (type) {
    case TypeTime_Day:
        ui->dateEdit_days_interval_begin->setDate(QDate::fromString(time["date"].toString(),"yyyy-MM-dd"));
        ui->spinBox_days_interval->setValue(time["days_interval"].toInt());
        break;
    case TypeTime_Week:{
        QStringList week = time["week"].toString().split(",");
        ui->checkBox_time_week_1->setCheckState(Qt::CheckState(week.at(0).toInt()));
        ui->checkBox_time_week_2->setCheckState(Qt::CheckState(week.at(1).toInt()));
        ui->checkBox_time_week_3->setCheckState(Qt::CheckState(week.at(2).toInt()));
        ui->checkBox_time_week_4->setCheckState(Qt::CheckState(week.at(3).toInt()));
        ui->checkBox_time_week_5->setCheckState(Qt::CheckState(week.at(4).toInt()));
        ui->checkBox_time_week_6->setCheckState(Qt::CheckState(week.at(5).toInt()));
        ui->checkBox_time_week_7->setCheckState(Qt::CheckState(week.at(6).toInt()));
        break;}
    default:
        break;
    }
    return;
}

QString Inspection::get_time_operation_display(QJsonObject time)
{
    QString display = time["time"].toString();
    TypeTime type = TypeTime(time["type"].toInt());
    switch (type) {
    case TypeTime_Day:
        display =display + QString(" 每隔 %1 天").arg(time["days_interval"].toInt());
        break;
    case TypeTime_Week:
        display = display + " 每星期";
        break;
    default:
        break;
    }
    return display;
}

int Inspection::getFirstTime(QJsonObject time)
{
    qDebug()<<"getFirstTime(QJsonObject time)"<<time;
    int msec = -1;
    QDateTime now = QDateTime::currentDateTime();
    TypeTime type = TypeTime(time["type"].toInt());
    switch (type) {
    case TypeTime_Day:{
        QDateTime taskDateTime = QDateTime(QDate::fromString(time["date"].toString(),"yyyy-MM-dd"), QTime::fromString(time["time"].toString(), "hh:mm:ss"));
        while (taskDateTime < now) {
            taskDateTime = taskDateTime.addDays(time["days_interval"].toInt());
        }
        msec = taskDateTime.toMSecsSinceEpoch() - now.toMSecsSinceEpoch();
        break;}
    case TypeTime_Week:{
        QStringList weekDays = time["week"].toString().split(",");
        int currentDay = now.date().dayOfWeek();
        int targetDay = 0;
        QDateTime taskDateTime;
        do{
            for (;;) { //一周最大 加7次
                if(targetDay >= 8){return -1;}

                int dayOfWeek = (currentDay+targetDay -1 )%7 +1;
                if(weekDays.at(dayOfWeek-1).toInt() == Qt::Checked){
                    taskDateTime = QDateTime(now.date().addDays(targetDay), QTime::fromString(time["time"].toString(), "hh:mm:ss"));
                    qDebug()<<"currentDay"<<currentDay<<"targetDay"<<targetDay;
                    qDebug()<<"taskDateTime"<<taskDateTime;
                    targetDay ++;//for跳出后不会执行++
                    break;
                }
                targetDay ++;

            }

        }while (taskDateTime < now);
        msec = taskDateTime.toMSecsSinceEpoch() - now.toMSecsSinceEpoch();
        break;}
    default:
        break;
    }

    return msec;
}
#include <QDateTime>
qint64 Inspection::getNextTimeInterval(QJsonObject time)
{
    qint64 value = -1;//msec
    TypeTime type = TypeTime(time["type"].toInt());
    switch (type) {
    case TypeTime_Day: {
        int daysInterval = time["days_interval"].toInt();
        value = daysInterval*SECS_PER_DAY*1000;
        break;
    }
    case TypeTime_Week: {
        QStringList weekDays = time["week"].toString().split(",");
        int currentDay = QDateTime::currentDateTime().date().dayOfWeek();
        int targetDay = 1;
        for(;targetDay<=7;targetDay++){
            int dayOfWeek = (currentDay+targetDay -1 )%7 +1;
            if(weekDays.at(dayOfWeek-1).toInt() == Qt::Checked){
                value = targetDay*SECS_PER_DAY*1000;
                break;
            }
        }
        break;
    }
    default:
        break;
    }
    return value;
}

void Inspection::on_toolButton_task_update_clicked()
{
    QJsonObject tasks = config["tasks"].toObject();
    QString taskName = ui->comboBox_task_name->currentText();
    qDebug()<<"tasks"<<tasks;
    if(tasks.contains(taskName)){
        qDebug()<<"从JSON文件中保存数据22"<<tasks[taskName].toObject();
    }

}


void Inspection::on_toolButton_task_delete_clicked()
{
    QJsonObject tasks = config["tasks"].toObject();
    QString taskName = ui->comboBox_task_name->currentText();
    if(tasks.contains(taskName)){
        qDebug()<<"从JSON文件中保存数据11"<<tasks[taskName].toObject();
        tasks.remove(taskName);
        config["tasks"] = tasks;
    }
}

//保存文件
void Inspection::on_toolButton_save_config_clicked()
{
    QJsonObject tasks = config["tasks"].toObject();
    QString taskName = ui->comboBox_task_name->currentText();
    QJsonObject old_task = tasks[taskName].toObject();
    QJsonObject task = get_task();
    tasks[taskName] = task;
    config["tasks"] = tasks;
    if(tasks.contains(taskName)){
        qDebug()<<"保存修改配置:"<<config;
        qDebug()<<"保存修改配置，之前配置为："<<old_task;
        qDebug()<<"保存修改配置，配置为："<<task;
    }

    if(SavaJsonData(config,configPath)){
        qDebug()<<"配置:"<<old_task["isEnable"].toInt()<<task["isEnable"].toInt()<<old_task["time"].toArray()<<task["time"].toArray();
        if(old_task["time"].toArray() != task["time"].toArray()){
            emit updata_task_run_time(taskName);
            qDebug()<<"配置已经改变，需要改变显示，重启"<<config;
            update_config_show();
        }
    }
}

void Inspection::on_pushButton_read_config_from_file_clicked()
{
    // 打开文件选择对话框，选择文件
    QString filePath = QFileDialog::getOpenFileName(this, "选择文件", "", "JSON文件 (*.json)");
    if (filePath.isEmpty()) {
        qDebug() << "未选择文件";
        return ;
    }
    qDebug() << "选择文件"<<filePath;

    if(SavaJsonData(config,filePath)){
        QMessageBox::information(this,"配置","读取成功");
        qDebug()<<"从JSON文件中保存配置数据"<<config;
    }

}

void Inspection::on_pushButton_save_config_to_file_clicked()
{
    // 选择保存的文件名和路径
    QString saveFilePath = QFileDialog::getSaveFileName(this, "保存文件", "", "JSON文件 (*.json)");
    if (saveFilePath.isEmpty()) {
        qDebug() << "未选择保存文件";
        return;
    }
    qDebug() << "文件"<<saveFilePath;
    if(SavaJsonData(config,saveFilePath)){
        qDebug()<<"从JSON文件中保存配置数据"<<config;
        QMessageBox::information(this,"配置","保存成功："+saveFilePath);
    }

}

void Inspection::on_pushButton_widgetSetting_isShow_clicked()
{
    ui->widgetInspectionSetting->setVisible(!ui->widgetInspectionSetting->isVisible());
}


void Inspection::on_toolButton_task_time_add_clicked()
{
    //每个任务不可重复
    QJsonObject newTime = get_time_operation();
    // 检查时间是否重复
    if (timesJsonArray.contains(newTime)){
        QToolTip::showText(ui->toolButton_task_time_add->mapToGlobal(QPoint(0, 0)),
                           "不能重复添加!",
                           ui->toolButton_task_time_add);
        return;
    }

    // 将新时间添加到 timesJsonArray 并更新 listWidget
    timesJsonArray.append(newTime);
    ui->listWidget_task_time->addItem(get_time_operation_display(newTime));

}


void Inspection::on_toolButton_task_time_update_clicked()
{
    qDebug() << "Inspection::on_toolButton_task_time_update_clicked()"<<ui->listWidget_task_time->currentRow();
    int currentRow = ui->listWidget_task_time->currentRow();
    if (currentRow == -1){
        QToolTip::showText(ui->toolButton_task_time_update->mapToGlobal(QPoint(0, 0)),
                           "未选择要修改的项!",
                           ui->toolButton_task_time_update);
        return;
    }
    QJsonObject newTime = get_time_operation();

    // 检查时间是否重复
    if (timesJsonArray.contains(newTime)){
        QToolTip::showText(ui->toolButton_task_time_update->mapToGlobal(QPoint(0, 0)),
                           "要修改的时间已经存在!",
                           ui->toolButton_task_time_update);
        return;
    }

    // 更新 timesJsonArray 和 listWidget
    timesJsonArray[currentRow] = newTime;
    ui->listWidget_task_time->item(currentRow)->setText(get_time_operation_display(newTime));
}


void Inspection::on_toolButton_task_time_delete_clicked()
{
    qDebug() << "Inspection::on_toolButton_task_time_delete_clicked()"<<ui->listWidget_task_time->currentRow();
    int currentRow = ui->listWidget_task_time->currentRow();
    if (currentRow == -1){
        QToolTip::showText(ui->toolButton_task_time_delete->mapToGlobal(QPoint(0, 0)),
                           "未选择要删除的项!",
                           ui->toolButton_task_time_delete);
        return;
    }
    // 从 timesJsonArray 和 listWidget 中删除
    timesJsonArray.removeAt(currentRow);
    ui->listWidget_task_time->takeItem(currentRow);
}


void Inspection::on_toolButton_task_time_sort_clicked()
{
    qDebug()<<timesJsonArray.toVariantList().toList();
//    QStringList stringList;
//    QStringList stringTimeList;
//    QMap<QString,int> mapTimeJson;
//    for(int i = 0;i < timesJsonArray.size();i++)
//    {
//        QJsonObject time = timesJsonArray.at(i).toObject();
//        stringTimeList.append(time["time"].toString());
//        mapTimeJson[time["time"].toString()] = i;
//    }
//    std::sort(stringList.begin(), stringList.end());
//    timesJsonArray = QJsonArray::fromStringList(stringList);
//    ui->listWidget_task_time->clear();
//    ui->listWidget_task_time->addItems(stringList);


    // 获取 timesJsonArray 中的所有 QJsonObject
    QList<QJsonObject> timeObjs;
    for (int i = 0; i < timesJsonArray.size(); ++i) {
        timeObjs.append(timesJsonArray.at(i).toObject());
    }

    // 对 timeObjs 进行排序
    std::sort(timeObjs.begin(), timeObjs.end(), [](const QJsonObject& a, const QJsonObject& b) {
        QTime timeA = QTime::fromString(a["time"].toString(), "hh:mm:ss");
        QTime timeB = QTime::fromString(b["time"].toString(), "hh:mm:ss");
        return timeA < timeB;
    });

    // 更新 timesJsonArray 和 listWidget_task_time
    timesJsonArray = QJsonArray();
    for (const auto& timeObj : timeObjs) {
        timesJsonArray.append(timeObj);
    }
    update_timesJsonArray_show();
}


void Inspection::on_toolButton_task_point_add_clicked()
{
    pointsJsonArray.append(get_point());
    update_pointsJsonArray_show();
}


void Inspection::on_toolButton_task_point_update_clicked()
{

    int currentRow = ui->tableWidget_task_points->currentRow();
    if (currentRow >= 0 && currentRow < pointsJsonArray.size()) {
        pointsJsonArray[currentRow] = get_point();
        update_pointsJsonArray_show();
    }else{
        QToolTip::showText(ui->toolButton_task_point_update->mapToGlobal(QPoint(0, 0)),
                           "未选择要修改的项!",ui->toolButton_task_point_update);
    }
}


void Inspection::on_toolButton_task_point_insert_clicked()
{
    int currentRow = ui->tableWidget_task_points->currentRow();
    if (currentRow == -1){
        QToolTip::showText(ui->toolButton_task_point_insert->mapToGlobal(QPoint(0, 0)),
                           "未选择要插入的项!",ui->toolButton_task_point_insert);
        return;
    }

    pointsJsonArray.insert(currentRow, get_point());
    update_pointsJsonArray_show();
}


void Inspection::on_toolButton_task_poiont_down_clicked()
{

    int currentRow = ui->tableWidget_task_points->currentRow();
    if (currentRow >= 0 && currentRow < pointsJsonArray.size() - 1) {
        QJsonObject temp = pointsJsonArray.takeAt(currentRow).toObject();
        pointsJsonArray.insert(currentRow + 1, temp);
        update_pointsJsonArray_show();
    }else{
        QToolTip::showText(ui->toolButton_task_poiont_down->mapToGlobal(QPoint(0, 0)),
                           "未选择要下移的项!",ui->toolButton_task_poiont_down);
    }

}


void Inspection::on_toolButton_task_point_up_clicked()
{
    int currentRow = ui->tableWidget_task_points->currentRow();
    if (currentRow >= 1 && currentRow < pointsJsonArray.size()) {
        QJsonObject temp = pointsJsonArray.takeAt(currentRow).toObject();
        pointsJsonArray.insert(currentRow - 1, temp);
        update_pointsJsonArray_show();
    }else{
        QToolTip::showText(ui->toolButton_task_point_up->mapToGlobal(QPoint(0, 0)),
                           "未选择要上移的项!",ui->toolButton_task_point_up);
        return;
    }

}


void Inspection::on_toolButton_task_point_delete_clicked()
{
    int currentRow = ui->tableWidget_task_points->currentRow();
    if (currentRow >= 0 && currentRow < pointsJsonArray.size()) {
        pointsJsonArray.removeAt(currentRow);
        update_pointsJsonArray_show();
    }else{
        QToolTip::showText(ui->toolButton_task_point_delete->mapToGlobal(QPoint(0, 0)),
                           "未选择要删除的项!",ui->toolButton_task_point_delete);
        return;
    }

}


void Inspection::on_listWidget_task_time_currentRowChanged(int currentRow)
{
    qDebug() << "Inspection::on_listWidget_task_time_currentRowChanged(int "<<currentRow;
    set_time_operation(timesJsonArray.at(currentRow).toObject());
}


void Inspection::on_tableWidget_task_points_itemClicked(QTableWidgetItem *item)
{
    qDebug() << "Inspection::on_tableWidget_task_points_itemClicked(QTableWidgetItem *item)"<<item->row()<<item->text();
    QJsonObject point = pointsJsonArray.at(item->row()).toObject();
    set_point(point);
}


void Inspection::on_tableWidget_tasks_itemClicked(QTableWidgetItem *item)
{
    qDebug() << "Inspection::on_tableWidget_tasks_itemClicked"<<item->row()<<item->text();
    ui->comboBox_task_name->setCurrentIndex(item->row());
    set_task();
}

void Inspection::on_comboBox_task_name_activated(int index)
{
    qDebug() << "void Inspection::on_comboBox_task_name_activated(int "<<index<<ui->comboBox_task_name->currentText();
    set_task();
}

void Inspection::on_comboBox_task_points_currentIndexChanged(int index)
{
    qDebug() << "Inspection::on_comboBox_task_points_currentIndexChanged(int index)"<<index;
}


void Inspection::on_toolButton_task_point_action_add_clicked()
{
    PointAction operation = static_cast<PointAction>(ui->comboBox_point_type->currentIndex());
    QJsonObject newAction;
    newAction[i2s(operation)] = get_action_operation(operation);
    actionsJsonArray.append(newAction);

    update_actionsJsonArray_show();
}


void Inspection::on_toolButton_task_point_action_update_clicked()
{
    int currentRow = ui->listWidget_task_point_action->currentRow();
    if (currentRow >= 0 && currentRow < actionsJsonArray.size()) {
        PointAction operation = static_cast<PointAction>(ui->comboBox_point_type->currentIndex());
        QJsonObject newAction;
        newAction[i2s(operation)] = get_action_operation(operation);
        actionsJsonArray[currentRow] = newAction;
        update_actionsJsonArray_show();
    }else{
        QToolTip::showText(ui->toolButton_task_point_action_update->mapToGlobal(QPoint(0, 0)),
                           "未选择要修改的项!",ui->toolButton_task_point_action_update);
    }
}


void Inspection::on_toolButton_task_point_action_insert_clicked()
{
    int currentRow = ui->listWidget_task_point_action->currentRow();
    qDebug()<<"on_toolButton_task_point_action_insert_clicked"<<currentRow;
    if (currentRow == -1){
        QToolTip::showText(ui->toolButton_task_point_action_insert->mapToGlobal(QPoint(0, 0)),
                           "未选择要插入的项!",ui->toolButton_task_point_action_insert);
        return;
    }

    PointAction operation = static_cast<PointAction>(ui->comboBox_point_type->currentIndex());
    QJsonObject newAction;
    newAction[i2s(operation)] = get_action_operation(operation);
    actionsJsonArray.insert(currentRow, newAction);

    update_actionsJsonArray_show();
}


void Inspection::on_toolButton_task_poiont_action_down_clicked()
{
    int currentRow = ui->listWidget_task_point_action->currentRow();
    qDebug()<<"on_toolButton_task_poiont_action_down_clicked()"<<currentRow<<actionsJsonArray.size()<<actionsJsonArray;

    if (currentRow >= 0 && currentRow < actionsJsonArray.size() - 1) {
        QJsonValue tempValue = actionsJsonArray.takeAt(currentRow);
        actionsJsonArray.insert(currentRow + 1, tempValue);
        update_actionsJsonArray_show();
    }else{
        QToolTip::showText(ui->toolButton_task_poiont_action_down->mapToGlobal(QPoint(0, 0)),
                           "未选择要下移的项!",ui->toolButton_task_poiont_action_down);
    }
}


void Inspection::on_toolButton_task_point_action_up_clicked()
{
    int currentRow = ui->listWidget_task_point_action->currentRow();
    if (currentRow >= 1 && currentRow < actionsJsonArray.size()) {
        actionsJsonArray.insert(currentRow - 1, actionsJsonArray.takeAt(currentRow));
        update_actionsJsonArray_show();
    }else{
        QToolTip::showText(ui->toolButton_task_point_action_up->mapToGlobal(QPoint(0, 0)),
                           "未选择要上移的项!",ui->toolButton_task_point_action_up);
        return;
    }
}


void Inspection::on_toolButton_task_point_action_delete_clicked()
{
    int currentRow = ui->listWidget_task_point_action->currentRow();
    if (currentRow == -1){
        QToolTip::showText(ui->toolButton_task_point_action_delete->mapToGlobal(QPoint(0, 0)),
                           "未选择要删除的项!",ui->toolButton_task_point_action_delete);
        return;
    }
    actionsJsonArray.removeAt(currentRow);
    update_actionsJsonArray_show();
}

void Inspection::on_toolButton_task_point_action_save_clicked()
{
    int currentRow = ui->tableWidget_task_points->currentRow();
    if(currentRow >= 0 && currentRow < pointsJsonArray.size()){
        QJsonObject point = pointsJsonArray.at(currentRow).toObject();
        point["action"] = actionsJsonArray;
        pointsJsonArray.replace(currentRow,point);
    }

}

void Inspection::on_listWidget_task_point_action_itemClicked(QListWidgetItem *item)
{
    qDebug() << "Inspection::on_listWidget_task_point_action_itemClicked"<<item->text()<<ui->listWidget_task_point_action->currentRow();

}


void Inspection::on_pushButton_widgetActionSetting_isShow_clicked()
{
    ui->widgetActionSetting->setVisible(!ui->widgetActionSetting->isVisible());
}


void Inspection::on_listWidget_task_point_action_currentRowChanged(int currentRow)
{
    qDebug() << "on_listWidget_task_point_action_currentRowChanged "<<currentRow;
    if(currentRow >= 0 && !actionsJsonArray.isEmpty()){
        QJsonObject action = actionsJsonArray.at(currentRow).toObject();
        foreach (const QString operation, action.keys()) {
            set_action_operation(PointAction(operation.toInt()),action[operation].toObject());
        }

    }
}


void Inspection::on_pushButton_robot_name_clicked()
{
    ui->tableWidget_tasks->setVisible(!ui->tableWidget_tasks->isVisible());
}


void Inspection::on_horizontalSlider_time_valueChanged(int value)
{
    qDebug() <<"Inspection::on_horizontalSlider_time_valueChanged(int "<<value<<ui->horizontalSlider_time->isSliderDown();

    QTime time(0, 0, 0);
    time = time.addSecs(value);
    ui->timeEdit_taks_time->setTime(time);
}


void Inspection::on_timeEdit_taks_time_timeChanged(const QTime &time)
{
    qDebug() <<"Inspection::on_timeEdit_taks_time_timeChanged(const QTime &"<<time<<time.msec();
    int seconds = time.hour() * 3600 + time.minute() * 60 + time.second();
    ui->horizontalSlider_time->setValue(seconds);
}
