#ifndef MEDIAWIDGETS_H
#define MEDIAWIDGETS_H

#include <QWidget>
#include "ui_MediaWidgets.h"

namespace Ui {
class MediaWidgets;
}

class MediaWidgets : public QWidget
{
    Q_OBJECT

public:
    explicit MediaWidgets(QWidget *parent = nullptr);
    ~MediaWidgets();

    Ui::MediaWidgets *ui;

    void init();

private slots:
    void on_Button_videoPlayback_clicked();

private:

};

#endif // MEDIAWIDGETS_H
