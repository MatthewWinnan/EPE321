#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "servergui.h"
#include "clientgui.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_clientButton_clicked();

    void on_serverButton_clicked();

private:
    Ui::MainWindow *ui;
    clientGUI *clientWindow;
    serverGUI *serverWindow;
};

#endif // MAINWINDOW_H