#ifndef SUBMAIN_H
#define SUBMAIN_H

#include <QMainWindow>
#include "ui_SubMain.h"

namespace Ui
{
    class SubMain;
}

class SubMain : public QMainWindow
{
    Q_OBJECT

public:
    explicit SubMain(QWidget *parent = nullptr);
    ~SubMain();

    void init();

    void showFiles();

public:
    Ui::SubMain *ui;

private:
};

extern SubMain *SUB_MAIN;

#endif // SUBMAIN_H
