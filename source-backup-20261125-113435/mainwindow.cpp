#include "mainwindow.h"

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QStackedWidget>
#include <QFrame>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      timer(new QTimer(this)),
      timerLabel(nullptr),
      startButton(nullptr),
      pauseButton(nullptr),
      resetButton(nullptr),
      remainingSeconds(25 * 60)
{
    setWindowTitle("FocusForge - Smart Study Manager");
    resize(1000, 650);

    QStackedWidget *pages = new QStackedWidget(this);
    setCentralWidget(pages);

    // ================= DASHBOARD PAGE =================

    QWidget *dashboardPage = new QWidget;
    QVBoxLayout *mainLayout = new QVBoxLayout(dashboardPage);

    QLabel *title = new QLabel("FocusForge");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet(
        "font-size: 36px;"
        "font-weight: bold;"
        "color: #6C63FF;"
        "padding: 10px;"
    );

    QLabel *subtitle = new QLabel("Your Smart Study Manager");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet(
        "font-size: 16px;"
        "color: gray;"
    );

    QPushButton *dashboardBtn = new QPushButton("Dashboard");
    QPushButton *timerBtn = new QPushButton("Focus Timer");
    QPushButton *tasksBtn = new QPushButton("Tasks Manager");
    QPushButton *subjectsBtn = new QPushButton("Subjects");
    QPushButton *reportBtn = new QPushButton("Study Report");

    QPushButton *buttons[] = {
        dashboardBtn, timerBtn, tasksBtn, subjectsBtn, reportBtn
    };

    for (QPushButton *btn : buttons) {
        btn->setCursor(Qt::PointingHandCursor);
        btn->setMinimumHeight(45);
        btn->setMinimumWidth(140);
    }

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(12);
    buttonLayout->setContentsMargins(10, 20, 10, 20);

    for (QPushButton *btn : buttons) {
        buttonLayout->addWidget(btn);
    }

    buttonLayout->setAlignment(Qt::AlignCenter);

    QLabel *welcome = new QLabel("Welcome to FocusForge!");
    welcome->setStyleSheet(
        "font-size: 28px;"
        "font-weight: bold;"
        "color: #25243A;"
        "padding-top: 20px;"
    );

    QLabel *description = new QLabel(
        "Organize your studies, manage tasks and stay focused."
    );

    description->setStyleSheet(
        "font-size: 16px;"
        "color: #777777;"
        "padding-bottom: 10px;"
    );

    mainLayout->addWidget(title);
    mainLayout->addWidget(subtitle);
    mainLayout->addLayout(buttonLayout);
    mainLayout->addSpacing(30);
    mainLayout->addWidget(welcome);
    mainLayout->addWidget(description);

    // Summary Cards

    QHBoxLayout *cardsLayout = new QHBoxLayout;

    QLabel *tasksCard = new QLabel("Total Tasks\n12");
    QLabel *completedCard = new QLabel("Completed\n5");
    QLabel *hoursCard = new QLabel("Study Hours\n3.5");
    QLabel *subjectsCard = new QLabel("Subjects\n4");

    QLabel *cards[] = {
        tasksCard, completedCard, hoursCard, subjectsCard
    };

    for (QLabel *card : cards) {
        card->setAlignment(Qt::AlignCenter);
        card->setMinimumHeight(120);

        card->setStyleSheet(
            "background-color: white;"
            "border: 1px solid #E5E0FF;"
            "border-radius: 15px;"
            "padding: 15px;"
            "font-size: 18px;"
            "font-weight: bold;"
            "color: #6C63FF;"
        );

        cardsLayout->addWidget(card);
    }

    mainLayout->addSpacing(20);
    mainLayout->addLayout(cardsLayout);

    // Recent Tasks

    QLabel *recentTasksTitle = new QLabel("Recent Tasks");

    recentTasksTitle->setStyleSheet(
        "font-size: 24px;"
        "font-weight: bold;"
        "color: #25243A;"
        "padding: 12px 0px;"
    );

    QLabel *task1 = new QLabel("Complete OOP Assignment");
    QLabel *task2 = new QLabel("Study Computer Networks");
    QLabel *task3 = new QLabel("Practice Calculus");

    QLabel *recentTasks[] = {task1, task2, task3};

    for (QLabel *task : recentTasks) {
        task->setStyleSheet(
            "background-color: white;"
            "border: 1px solid #E5E0FF;"
            "border-radius: 10px;"
            "padding: 15px;"
            "font-size: 15px;"
            "color: #333333;"
        );
    }

    mainLayout->addSpacing(25);
    mainLayout->addWidget(recentTasksTitle);
    mainLayout->addWidget(task1);
    mainLayout->addWidget(task2);
    mainLayout->addWidget(task3);
    mainLayout->addStretch();

    // ================= TIMER PAGE =================

    QWidget *timerPage = new QWidget;
    QVBoxLayout *timerLayout = new QVBoxLayout(timerPage);

    QPushButton *backButton = new QPushButton("← Back to Dashboard");

    QLabel *timerTitle = new QLabel("Focus Timer");
    timerTitle->setAlignment(Qt::AlignCenter);
    timerTitle->setStyleSheet(
        "font-size: 32px;"
        "font-weight: bold;"
        "color: #6C63FF;"
    );

    QLabel *timerDescription = new QLabel("Stay focused. One task at a time.");
    timerDescription->setAlignment(Qt::AlignCenter);
    timerDescription->setStyleSheet(
        "font-size: 16px;"
        "color: gray;"
    );

    timerLabel = new QLabel;
    timerLabel->setAlignment(Qt::AlignCenter);
    timerLabel->setStyleSheet(
        "font-size: 72px;"
        "font-weight: bold;"
        "color: #6C63FF;"
        "background-color: white;"
        "border: 3px solid #E5E0FF;"
        "border-radius: 25px;"
        "padding: 30px;"
    );

    startButton = new QPushButton("Start");
    pauseButton = new QPushButton("Pause");
    resetButton = new QPushButton("Reset");

    startButton->setMinimumSize(130, 50);
    pauseButton->setMinimumSize(130, 50);
    resetButton->setMinimumSize(130, 50);

    QHBoxLayout *timerButtonsLayout = new QHBoxLayout;
    timerButtonsLayout->setSpacing(15);

    timerButtonsLayout->addWidget(startButton);
    timerButtonsLayout->addWidget(pauseButton);
    timerButtonsLayout->addWidget(resetButton);

    timerLayout->addWidget(backButton);
    timerLayout->addSpacing(30);
    timerLayout->addWidget(timerTitle);
    timerLayout->addWidget(timerDescription);
    timerLayout->addSpacing(30);
    timerLayout->addWidget(timerLabel);
    timerLayout->addSpacing(20);
    timerLayout->addLayout(timerButtonsLayout);
    timerLayout->addStretch();

    updateTimerDisplay();

    // Add pages

    pages->addWidget(dashboardPage);
    pages->addWidget(timerPage);

    pages->setCurrentWidget(dashboardPage);

    // ================= BUTTON ACTIONS =================

    connect(timerBtn, &QPushButton::clicked, this, [=]() {
        pages->setCurrentWidget(timerPage);
    });

    connect(dashboardBtn, &QPushButton::clicked, this, [=]() {
        pages->setCurrentWidget(dashboardPage);
    });

    connect(backButton, &QPushButton::clicked, this, [=]() {
        pages->setCurrentWidget(dashboardPage);
    });

    connect(startButton, &QPushButton::clicked, this, [=]() {
        startTimer();
    });

    connect(pauseButton, &QPushButton::clicked, this, [=]() {
        pauseTimer();
    });

    connect(resetButton, &QPushButton::clicked, this, [=]() {
        resetTimer();
    });

    // Timer Countdown

    timer->setInterval(1000);

    connect(timer, &QTimer::timeout, this, [=]() {
        if (remainingSeconds > 0) {
            remainingSeconds--;
            updateTimerDisplay();
        }

        if (remainingSeconds <= 0) {
            timer->stop();
            timerLabel->setText("Time's Up!");
        }
    });

    // ================= APP STYLE =================

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

// ================= TIMER FUNCTIONS =================

void MainWindow::updateTimerDisplay()
{
    int minutes = remainingSeconds / 60;
    int seconds = remainingSeconds % 60;

    timerLabel->setText(
        QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
    );
}

void MainWindow::startTimer()
{
    if (remainingSeconds > 0) {
        timer->start();
    }
}

void MainWindow::pauseTimer()
{
    timer->stop();
}

void MainWindow::resetTimer()
{
    timer->stop();
    remainingSeconds = 25 * 60;
    updateTimerDisplay();
}