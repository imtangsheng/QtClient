#ifndef SUBWINDOW_H
#define SUBWINDOW_H

#include <QMainWindow>
#include "ui_SubWindow.h"

namespace Ui {
class SubWindow;
}

class SubWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SubWindow(QWidget *parent = nullptr);
    ~SubWindow();
    void showFiles();

public:
    Ui::SubWindow *ui;

private:

};


extern SubWindow *SUB_WINDOW;

#endif // SUBWINDOW_H
