#include "stopwatchpage.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTimer>
#include <QElapsedTimer>
#include <QDateTime>

StopwatchPage::StopwatchPage(QWidget *parent)
    : QWidget(parent)
    , m_timeLabel(nullptr)
    , m_startButton(nullptr)
    , m_pauseButton(nullptr)
    , m_resumeButton(nullptr)
    , m_resetButton(nullptr)
    , m_timer(new QTimer(this))
    , m_isRunning(false)
    , m_pausedAtMSecs(0)
    , m_startTimeMSecs(0)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);

    m_timeLabel = new QLabel("00:00:00", this);
    m_timeLabel->setAlignment(Qt::AlignCenter);
    m_timeLabel->setStyleSheet(
        "font-size: 48px;"
        "font-weight: bold;"
        "color: #6C63FF;"
        "background-color: white;"
        "border: 3px solid #E5E0FF;"
        "border-radius: 25px;"
        "padding: 20px;"
        "min-height: 100px;"
    );

    mainLayout->addWidget(m_timeLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(20);
    buttonLayout->setContentsMargins(0, 10, 0, 0);

    m_startButton = new QPushButton("Start", this);
    m_startButton->setMinimumHeight(45);
    m_startButton->setMinimumWidth(120);
    connect(m_startButton, &QPushButton::clicked,
            this, &StopwatchPage::onStartClicked);
    buttonLayout->addWidget(m_startButton);

    m_pauseButton = new QPushButton("Pause", this);
    m_pauseButton->setMinimumHeight(45);
    m_pauseButton->setMinimumWidth(120);
    m_pauseButton->setVisible(false);
    connect(m_pauseButton, &QPushButton::clicked,
            this, &StopwatchPage::onPauseClicked);
    buttonLayout->addWidget(m_pauseButton);

    m_resumeButton = new QPushButton("Resume", this);
    m_resumeButton->setMinimumHeight(45);
    m_resumeButton->setMinimumWidth(120);
    m_resumeButton->setVisible(false);
    connect(m_resumeButton, &QPushButton::clicked,
            this, &StopwatchPage::onResumeClicked);
    buttonLayout->addWidget(m_resumeButton);

    m_resetButton = new QPushButton("Reset", this);
    m_resetButton->setMinimumHeight(45);
    m_resetButton->setMinimumWidth(120);
    connect(m_resetButton, &QPushButton::clicked,
            this, &StopwatchPage::onResetClicked);
    buttonLayout->addWidget(m_resetButton);

    mainLayout->addLayout(buttonLayout);

    m_timer->setInterval(100);
    connect(m_timer, &QTimer::timeout,
            this, &StopwatchPage::onTimerTimeout);

    setStyleSheet(
        "QWidget {"
        "background-color: #F4F3FF;"
        "}"
        "QLabel {"
        "color: #25243A;"
        "}"
        "QPushButton {"
        "background-color: #6C63FF;"
        "color: white;"
        "border: none;"
        "padding: 12px;"
        "border-radius: 8px;"
        "font-size: 14px;"
        "}"
        "QPushButton:hover {"
        "background-color: #5548E8;"
        "}"
    );
}

StopwatchPage::~StopwatchPage()
{
}

qint64 StopwatchPage::elapsedMSecs() const
{
    if (m_isRunning) {
        return m_pausedAtMSecs + m_elapsedTimer.elapsed();
    }

    return m_pausedAtMSecs;
}

void StopwatchPage::onStartClicked()
{
    if (m_isRunning)
        return;

    m_isRunning = true;
    m_elapsedTimer.start();

    m_startButton->setVisible(false);
    m_pauseButton->setVisible(true);
    m_resumeButton->setVisible(false);

    m_timer->start();
}

void StopwatchPage::onPauseClicked()
{
    if (!m_isRunning)
        return;

    m_pausedAtMSecs += m_elapsedTimer.elapsed();

    m_isRunning = false;
    m_timer->stop();

    updateDisplay();

    m_pauseButton->setVisible(false);
    m_resumeButton->setVisible(true);
}

void StopwatchPage::onResumeClicked()
{
    if (m_isRunning)
        return;

    m_isRunning = true;
    m_elapsedTimer.start();

    m_resumeButton->setVisible(false);
    m_pauseButton->setVisible(true);

    m_timer->start();
}

void StopwatchPage::onResetClicked()
{
    m_isRunning = false;
    m_timer->stop();

    m_pausedAtMSecs = 0;
    m_startTimeMSecs = 0;

    m_timeLabel->setText("00:00:00");

    m_startButton->setVisible(true);
    m_pauseButton->setVisible(false);
    m_resumeButton->setVisible(false);
}

void StopwatchPage::onTimerTimeout()
{
    updateDisplay();
}

void StopwatchPage::updateDisplay()
{
    qint64 elapsed = elapsedMSecs();

    int totalSeconds = static_cast<int>(elapsed / 1000);

    int hours = totalSeconds / 3600;
    int minutes = (totalSeconds % 3600) / 60;
    int seconds = totalSeconds % 60;

    QString formattedTime = QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));

    m_timeLabel->setText(formattedTime);

    if (m_isRunning) {
        m_startButton->setVisible(false);
        m_pauseButton->setVisible(true);
        m_resumeButton->setVisible(false);
    }
}