#ifndef CAMERA_H
#define CAMERA_H

#include <QWidget>
#include "ui_camera.h"

namespace Ui {
class Camera;
}

class Camera : public QWidget
{
    Q_OBJECT

public:
    explicit Camera(QWidget *parent = nullptr);
    ~Camera();

    Ui::Camera *ui;
    enum CameraType {
        CameraType_default,
        CameraType_selfDeveloped,
        CameraType_laureii,
        CameraType_hikvison,
    };

    void init();
    void quit();
    bool test();
private:

};

#endif // CAMERA_H
