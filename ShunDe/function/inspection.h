#ifndef INSPECTION_H
#define INSPECTION_H

#include <QWidget>

#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>

#include "ui_inspection.h"

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

private:

};

#endif // INSPECTION_H
