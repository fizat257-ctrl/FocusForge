#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "task.h"
#include "taskstore.h"
#include "subjectstore.h"
#include "subjectsmanagerpage.h"

class QLabel;
class QPushButton;
class QTimer;
class QStackedWidget;
class QWidget;
class QVBoxLayout;

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

    QStackedWidget *pages;
    QLabel *tasksCard;
    QLabel *completedCard;
    QLabel *subjectsCard;
    QWidget *taskCardsContainer;
    TaskStore taskStore;
    SubjectStore subjectStore;

    int remainingSeconds;

    void updateTimerDisplay();
    void startTimer();
    void pauseTimer();
    void resetTimer();

    void updateDashboardStats();
    void refreshTaskCards();
    void showTaskDialog(Task *existingTask = nullptr);
};

#endif