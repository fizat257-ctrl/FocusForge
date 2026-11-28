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
#include "quizstore.h"
#include "quizattemptstore.h"
#include "quizgenerator.h"
#include "quiztextractor.h"
#include "quizgeneratorpage.h"

class QLabel;
class QPushButton;
class QTimer;
class QStackedWidget;
class QWidget;
class QVBoxLayout;
class QComboBox;

class StudyReportPage;
class QuizGeneratorPage;
class StopwatchPage;

class MainWindow : public QMainWindow
{
public:
    explicit MainWindow(QWidget *parent = nullptr);

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
    QuizStore quizStore;
    QuizAttemptStore quizAttemptStore;
    QuizGenerator quizGenerator;
QuizTextExtractor quizTextExtractor;

    StudyReportPage *studyReportPage;
    QuizGeneratorPage *quizGeneratorPage;
    StopwatchPage *stopwatchPage;

    QComboBox *timerSubjectCombo;
    QComboBox *timerTaskCombo;
    QComboBox *timerDurationCombo;

    int activeSessionDurationSeconds;
    qint64 remainingMilliseconds;

    QDateTime sessionEndTime;
    QDateTime activeSessionStartTime;

    QString activeTimerTaskId;
    QString activeTimerTaskTitle;
    QString activeTimerSubjectId;
    QString activeTimerSubjectName;

    int previouslySelectedGlobalTimerDurationSeconds;

    StudySession unsavedCompletedSession;
    bool unsavedCompletedSessionPending;

    void updateTimerDisplay();
    void startTimer();
    void pauseTimer();
    void resetTimer();
    void completeFocusSession();

    void updateDashboardStats();
    void refreshRecentTasks();
    void refreshTaskCards();

    void showTaskDialog(Task *existingTask = nullptr);

    void refreshTimerTaskChoices();
    void loadSelectedTaskDuration();

    bool retryUnsavedCompletedSession();
    void showUnsavedSessionRecovery(const QString &requestedAction);
    void clearUnsavedSessionAfterSuccess();

    void onApiKeyManagement();
};

#endif // MAINWINDOW_H