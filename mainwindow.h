#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDir>
#include <QTextStream>
#include <QFile>
#include <QDataStream>
#include <QThread>
#include <QTimer>
#include <wiringPi.h>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:


private slots:
    void on_SetPoint_valueChanged(double arg1);
    void timer_TimeOut_event_slot();
    void realTimePlot();

  // void on_currentTp_overflow();

    void on_kd_value_valueChanged(double arg1);

    void on_kp_value_valueChanged(double arg1);

    void on_ki_value_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;

    QTimer *Timer;
    QTimer timer_plot;
};

#endif // MAINWINDOW_H
