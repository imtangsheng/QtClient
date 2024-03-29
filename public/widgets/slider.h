#ifndef SLIDER_H
#define SLIDER_H

#include <QSlider>

class Slider : public QSlider
{
    Q_OBJECT
public:
    explicit Slider(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;

signals:

};

#endif // SLIDER_H
