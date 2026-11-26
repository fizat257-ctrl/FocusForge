#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>
#include <QDateTime>

#include "task.h"
#include "taskstore.h"
#include "subjectstore.h"
#include "subjectsmanagerpage.h"
#include "studysessionstore.h"

class QLabel;
class QPushButton;
class QTimer;
class QStackedWidget;
class QWidget;
class QVBoxLayout;
class QComboBox;

class StudyReportPage;

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
    QLabel *studyHoursCard;
    QList<QLabel *> recentTaskLabels;
    QWidget *taskCardsContainer;
    TaskStore taskStore;
    SubjectStore subjectStore;
    StudySessionStore studySessionStore;
    StudyReportPage *studyReportPage;
    QComboBox *timerSubjectCombo;
    QComboBox *timerTaskCombo;
    QComboBox *timerDurationCombo;

    int activeSessionDurationSeconds;
    qint64 remainingMilliseconds;
    QDateTime sessionEndTime;
    QDateTime activeSessionStartTime;

    void updateTimerDisplay();
    void startTimer();
    void pauseTimer();
    void resetTimer();
    void completeFocusSession();

    void updateDashboardStats();
    void refreshRecentTasks();
    void refreshTaskCards();
    void showTaskDialog(Task *existingTask = nullptr);
};

#endif