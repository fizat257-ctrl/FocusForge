#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QLabel;
class QPushButton;
class QTimer;

class MainWindow : public QMainWindow
{
public:
    MainWindow(QWidget *parent = nullptr);

private:
    QTimer *timer;
    QLabel *timerLabel;
    QPushButton *startButton;
    QPushButton *pauseButton;
    QPushButton *resetButton;

    int remainingSeconds;

    void updateTimerDisplay();
    void startTimer();
    void pauseTimer();
    void resetTimer();
};

#endif