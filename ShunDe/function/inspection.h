#ifndef INSPECTION_H
#define INSPECTION_H

#include <QWidget>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

#include "ui_inspection.h"

enum PointAction{
    PointAction_Time=0,
    PointAction_Vision_PTZControl,
    PointAction_Vision_PTZPreset,
    PointAction_Vision_PTZPOS,
    PointAction_Vision_CaptureJPEGPicture,
    PointAction_Vision_Realtime_Thermometry,
    PointAction_Vision_Other,
};

namespace Ui {
class Inspection;
}

class Inspection : public QWidget
{
    Q_OBJECT

public:
    explicit Inspection(QWidget *parent = nullptr);
    ~Inspection();
    Ui::Inspection *ui;
    void start();
    void quit();

    QJsonObject config;
    QJsonArray pointsJsonArray;
    QJsonArray actionsJsonArray;
    QJsonArray timesJsonArray;

    void update_config_show();
    void update_pointsJsonArray_show();
    void update_actionsJsonArray_show();
    void update_timesJsonArray_show();

    void set_task();//根据选中的编号读取
    QJsonObject get_task();

    void set_point(QJsonObject point);
    QJsonObject get_point();


    QJsonObject get_action_operation(PointAction operation);
    void set_action_operation(PointAction operation,QJsonObject action);
    QString get_action_operation_display(PointAction operation,QJsonObject action);

    enum TypeTime{
        TypeTime_Day = 0,
        TypeTime_Week,
    };

    QJsonObject get_time_operation();
    void set_time_operation(QJsonObject time);
    QString get_time_operation_display(QJsonObject time);
    int getFirstTime(QJsonObject time);
    qint64 getNextTimeInterval(QJsonObject time);

signals:
    void updata_task_run_time(const QString &taskName);

private slots:
    void on_toolButton_task_update_clicked();

    void on_toolButton_task_delete_clicked();

    void on_toolButton_save_config_clicked();

    void on_pushButton_read_config_from_file_clicked();

    void on_pushButton_save_config_to_file_clicked();

    void on_pushButton_widgetSetting_isShow_clicked();

    void on_toolButton_task_time_add_clicked();

    void on_toolButton_task_time_update_clicked();

    void on_toolButton_task_time_delete_clicked();

    void on_toolButton_task_time_sort_clicked();

    void on_toolButton_task_point_add_clicked();

    void on_toolButton_task_point_update_clicked();

    void on_toolButton_task_point_insert_clicked();

    void on_toolButton_task_poiont_down_clicked();

    void on_toolButton_task_point_up_clicked();

    void on_toolButton_task_point_delete_clicked();

    void on_listWidget_task_time_currentRowChanged(int currentRow);

    void on_tableWidget_task_points_itemClicked(QTableWidgetItem *item);

    void on_tableWidget_tasks_itemClicked(QTableWidgetItem *item);

    void on_comboBox_task_name_activated(int index);

    void on_comboBox_task_points_currentIndexChanged(int index);

    void on_toolButton_task_point_action_add_clicked();

    void on_toolButton_task_point_action_update_clicked();

    void on_toolButton_task_point_action_insert_clicked();

    void on_toolButton_task_poiont_action_down_clicked();

    void on_toolButton_task_point_action_up_clicked();

    void on_toolButton_task_point_action_delete_clicked();

    void on_toolButton_task_point_action_save_clicked();

    void on_listWidget_task_point_action_itemClicked(QListWidgetItem *item);

    void on_pushButton_widgetActionSetting_isShow_clicked();

    void on_listWidget_task_point_action_currentRowChanged(int currentRow);

    void on_pushButton_robot_name_clicked();

    void on_horizontalSlider_time_valueChanged(int value);

    void on_timeEdit_taks_time_timeChanged(const QTime &time);

private:

};

#endif // INSPECTION_H
