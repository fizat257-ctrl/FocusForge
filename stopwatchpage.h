#ifndef STOPWATCHPAGE_H
#define STOPWATCHPAGE_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QElapsedTimer>
#include <QTime>

class StopwatchPage : public QWidget
{
    Q_OBJECT

public:
    explicit StopwatchPage(QWidget *parent = nullptr);
    ~StopwatchPage() override;

    // Returns the elapsed time in milliseconds (for persistence/state saving)
    qint64 elapsedMSecs() const;

signals:
    void pageClosed();

private slots:
    void onStartClicked();
    void onPauseClicked();
    void onResumeClicked();
    void onResetClicked();
    void onTimerTimeout();

    void updateDisplay();

private:
    // Display label showing HH:MM:SS
    QLabel *m_timeLabel;

    // Control buttons
    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_resumeButton;
    QPushButton *m_resetButton;

    // Timer and elapsed time tracking
    QTimer *m_timer;
    QElapsedTimer m_elapsedTimer;

    // State: true = running, false = paused/stopped
    bool m_isRunning;

    // Record time when paused (so we can resume from there)
    qint64 m_pausedAtMSecs;

    // Record time when started (for continuous counting even when navigating away)
    qint64 m_startTimeMSecs;
};

#endif // STOPWATCHPAGE_H