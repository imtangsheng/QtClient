#ifndef HIKVISIONCAMERA_H
#define HIKVISIONCAMERA_H

#include <QJsonObject>
#include <QWidget>

namespace Ui {
class HikVisionCamera;
}

class HikVisionCamera : public QWidget
{
    Q_OBJECT

public:
    explicit HikVisionCamera(QWidget *parent = nullptr);
    ~HikVisionCamera();

    //摄像头操作方法
    QJsonObject camera;
    bool updateCameraPose_Pan_Tilt(int pan, int tilt);
    bool PTZControl(int dwPTZCommand,int dwStop);
    bool PTZPreset(int lChannel,int dwPresetIndex,int dwPTZPresetCmd);
    bool PTZPOS(int wAction,int wPanPos,int wTiltPos,int wZoomPos);
    bool CaptureJPEGPicture(int lChannel,int sPicFileName);
    bool Realtime_Thermometry(int sPicFileName);


private:
    Ui::HikVisionCamera *ui;
};

#endif // HIKVISIONCAMERA_H
