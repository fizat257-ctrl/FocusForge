
#include "mainwindow.h"
#include "stopwatchpage.h"
#include "studyreportpage.h"
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QStandardPaths>
#include <QUrl>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTimer>
#include <QStackedWidget>
#include <QFrame>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QMessageBox>
#include "apikeyeditordialog.h"
#include <QScrollArea>
#include <QTextEdit>
#include <QUuid>
#include <QTimeEdit>
#include <QSpinBox>
#include <functional>

namespace {

QString attachmentFolderName(const QString &taskId)
{
    QString folderName;
    for (const QChar character : taskId) {
        if (character.isLetterOrNumber() || character == QLatin1Char('-') || character == QLatin1Char('_'))
            folderName.append(character);
        else
            folderName.append(QLatin1Char('_'));
    }
    return folderName.isEmpty() ? QStringLiteral("unknown-task") : folderName;
}

QString attachmentRootPath()
{
    const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return appDataPath.isEmpty()
        ? QString()
        : QDir::cleanPath(appDataPath + QLatin1String("/attachments"));
}

QString attachmentAbsolutePath(const QString &relativePath)
{
    const QString root = attachmentRootPath();
    if (root.isEmpty() || relativePath.isEmpty())
        return QString();

    const QString absolute = QDir::cleanPath(QDir(root).filePath(relativePath));
    const QString rootPrefix = QDir::cleanPath(root) + QLatin1Char('/');
    if (absolute != QDir::cleanPath(root) && !absolute.startsWith(rootPrefix, Qt::CaseInsensitive))
        return QString();

    return absolute;
}

bool isSupportedAttachmentType(const QString &filePath)
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    if (suffix != QLatin1String("pdf") && suffix != QLatin1String("png"))
        return false;

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (suffix == QLatin1String("pdf")) {
        const QByteArray header = file.read(5);
        return header.startsWith("%PDF-");
    }

    const QByteArray header = file.read(8);
    const QByteArray pngSignature("\x89PNG\r\n\x1A\n", 8);
    return header == pngSignature;
}

bool copyAttachmentToStorage(const QString &sourcePath,
                             const QString &taskId,
                             TaskAttachment &attachment)
{
    const QFileInfo source(sourcePath);
    if (!source.isFile() || !isSupportedAttachmentType(sourcePath))
        return false;

    const QString root = attachmentRootPath();
    if (root.isEmpty())
        return false;

    const QString taskFolder = QDir::cleanPath(root + QLatin1Char('/') + attachmentFolderName(taskId));
    QDir directory;
    if (!directory.mkpath(taskFolder))
        return false;

    const QString extension = source.suffix().toLower();
    const QString fileName = QUuid::createUuid().toString(QUuid::WithoutBraces) + QLatin1Char('.') + extension;
    const QString destination = QDir::cleanPath(taskFolder + QLatin1Char('/') + fileName);

    if (!QFile::copy(sourcePath, destination))
        return false;

    attachment.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    attachment.fileName = source.fileName();
    attachment.relativePath = QDir::cleanPath(
        attachmentFolderName(taskId) + QLatin1Char('/') + fileName);
    return true;
}

void removeAttachmentFile(const TaskAttachment &attachment)
{
    const QString absolutePath = attachmentAbsolutePath(attachment.relativePath);
    if (!absolutePath.isEmpty())
        QFile::remove(absolutePath);
}

void removeTaskAttachmentFolder(const QString &taskId)
{
    const QString root = attachmentRootPath();
    if (root.isEmpty())
        return;

    const QString taskFolder = QDir::cleanPath(root + QLatin1Char('/') + attachmentFolderName(taskId));
    QDir taskDirectory(taskFolder);
    if (taskDirectory.exists())
        taskDirectory.removeRecursively();
}

} // namespace

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      timer(new QTimer(this)),
      timerLabel(nullptr),
      startButton(nullptr),
      pauseButton(nullptr),
      resetButton(nullptr),
      tasksCard(nullptr),
      completedCard(nullptr),
      subjectsCard(nullptr),
    studyHoursCard(nullptr),
      taskCardsContainer(nullptr),
      studyReportPage(nullptr),
      quizGeneratorPage(nullptr),
      timerSubjectCombo(nullptr),
      timerTaskCombo(nullptr),
        timerDurationCombo(nullptr),
        activeSessionDurationSeconds(25 * 60),
    remainingMilliseconds(25 * 60 * 1000),
    activeTimerTaskId(),
    activeTimerTaskTitle(),
    activeTimerSubjectId(),
    activeTimerSubjectName(),
    previouslySelectedGlobalTimerDurationSeconds(25 * 60),
    unsavedCompletedSession(),
    unsavedCompletedSessionPending(false)
{
    setWindowTitle("FocusForge - Smart Study Manager");
    resize(1000, 650);

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
    QPushButton *quizBtn = new QPushButton("Quiz Generator");
    QPushButton *stopwatchBtn = new QPushButton("Stopwatch");

    QPushButton *buttons[] = {
        dashboardBtn, timerBtn, tasksBtn, subjectsBtn, reportBtn, quizBtn, stopwatchBtn
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

    tasksCard = new QLabel(QString("Total Tasks\n%1").arg(taskStore.tasks().size()));
    completedCard = new QLabel("Completed\n0");
    studyHoursCard = new QLabel("Study Hours\n0m");
    subjectsCard = new QLabel(QString("Subjects\n%1").arg(subjectStore.subjects().size()));

    QLabel *cards[] = {
        tasksCard, completedCard, studyHoursCard, subjectsCard
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

    recentTaskLabels = {new QLabel, new QLabel, new QLabel};

    for (QLabel *task : recentTaskLabels) {
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
    for (QLabel *task : recentTaskLabels)
        mainLayout->addWidget(task);
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

    timerDurationCombo = new QComboBox;
    const int durationOptionsMinutes[] = {1, 2, 3, 5, 10, 15, 20, 25, 30, 45, 60};
    for (int minutes : durationOptionsMinutes) {
        timerDurationCombo->addItem(
            QString("%1 minute%2").arg(minutes).arg(minutes == 1 ? "" : "s"),
            minutes * 60
        );
    }
    timerDurationCombo->setCurrentIndex(timerDurationCombo->findData(25 * 60));
    previouslySelectedGlobalTimerDurationSeconds = 25 * 60;

    QHBoxLayout *timerButtonsLayout = new QHBoxLayout;
    timerButtonsLayout->setSpacing(15);

    timerButtonsLayout->addWidget(startButton);
    timerButtonsLayout->addWidget(pauseButton);
    timerButtonsLayout->addWidget(resetButton);

    timerSubjectCombo = new QComboBox;
    timerSubjectCombo->addItem("Uncategorized", QString());
    for (const Subject &subject : subjectStore.subjects())
        timerSubjectCombo->addItem(subject.name, subject.id);

    timerTaskCombo = new QComboBox;
    timerTaskCombo->addItem("No task", QString());
    for (const Task &task : taskStore.tasks()) {
        if (!task.completed)
            timerTaskCombo->addItem(task.title, task.id);
    }

    QLabel *subjectSelectionLabel = new QLabel("Subject (optional)");
    QLabel *taskSelectionLabel = new QLabel("Task (optional)");
    QLabel *durationSelectionLabel = new QLabel("Duration");

    QHBoxLayout *selectionLayout = new QHBoxLayout;
    QVBoxLayout *subjectSelectionLayout = new QVBoxLayout;
    QVBoxLayout *taskSelectionLayout = new QVBoxLayout;
    QVBoxLayout *durationSelectionLayout = new QVBoxLayout;
    subjectSelectionLayout->addWidget(subjectSelectionLabel);
    subjectSelectionLayout->addWidget(timerSubjectCombo);
    taskSelectionLayout->addWidget(taskSelectionLabel);
    taskSelectionLayout->addWidget(timerTaskCombo);
    durationSelectionLayout->addWidget(durationSelectionLabel);
    durationSelectionLayout->addWidget(timerDurationCombo);
    selectionLayout->addLayout(subjectSelectionLayout);
    selectionLayout->addLayout(taskSelectionLayout);
    selectionLayout->addLayout(durationSelectionLayout);

    timerLayout->addWidget(backButton);
    timerLayout->addSpacing(30);
    timerLayout->addWidget(timerTitle);
    timerLayout->addWidget(timerDescription);
    timerLayout->addSpacing(30);
    timerLayout->addWidget(timerLabel);
    timerLayout->addSpacing(20);
    timerLayout->addLayout(selectionLayout);
    timerLayout->addLayout(timerButtonsLayout);
    timerLayout->addStretch();

    updateTimerDisplay();

    // ================= TASKS PAGE =================

    QWidget *tasksPage = new QWidget;
    QVBoxLayout *tasksLayout = new QVBoxLayout(tasksPage);

    QPushButton *tasksBackButton = new QPushButton("← Back to Dashboard");

    QLabel *tasksTitle = new QLabel("Tasks Manager");
    tasksTitle->setAlignment(Qt::AlignCenter);
    tasksTitle->setStyleSheet("font-size: 32px; font-weight: bold; color: #6C63FF;");

    QLabel *tasksDescription = new QLabel("Plan, organize and complete your study tasks.");
    tasksDescription->setAlignment(Qt::AlignCenter);
    tasksDescription->setStyleSheet("font-size: 16px; color: gray;");

    QPushButton *addTaskButton = new QPushButton("+ Add New Task");
    addTaskButton->setMinimumHeight(45);
    addTaskButton->setMinimumWidth(180);

    QHBoxLayout *tasksHeaderLayout = new QHBoxLayout;
    tasksHeaderLayout->addWidget(tasksBackButton);
    tasksHeaderLayout->addStretch();

    QScrollArea *taskScroll = new QScrollArea;
    taskScroll->setWidgetResizable(true);
    taskScroll->setFrameShape(QFrame::NoFrame);

    taskCardsContainer = new QWidget;
    QVBoxLayout *taskCardsLayout = new QVBoxLayout(taskCardsContainer);
    taskCardsLayout->setContentsMargins(10, 10, 10, 10);
    taskCardsLayout->setSpacing(12);
    taskCardsLayout->addStretch();
    taskScroll->setWidget(taskCardsContainer);

    tasksLayout->addLayout(tasksHeaderLayout);
    tasksLayout->addWidget(tasksTitle);
    tasksLayout->addWidget(tasksDescription);
    tasksLayout->addSpacing(10);

    QHBoxLayout *addTaskLayout = new QHBoxLayout;
    addTaskLayout->addStretch();
    addTaskLayout->addWidget(addTaskButton);
    addTaskLayout->addStretch();
    tasksLayout->addLayout(addTaskLayout);
    tasksLayout->addSpacing(10);
    tasksLayout->addWidget(taskScroll);

    // ================= SUBJECTS PAGE =================

    SubjectsManagerPage *subjectsPage = new SubjectsManagerPage(&subjectStore);

    // ================= REPORT PAGE =================

    studyReportPage = new StudyReportPage(&studySessionStore,
                                          &taskStore,
                                          &subjectStore);

    // ================= BUTTON ACTIONS =================

    pages = new QStackedWidget(this);
    setCentralWidget(pages);
    pages->addWidget(dashboardPage);
    pages->addWidget(timerPage);
    pages->addWidget(tasksPage);
    pages->addWidget(subjectsPage);
    pages->addWidget(studyReportPage);
    quizGeneratorPage = new QuizGeneratorPage(
    &taskStore,
    &quizGenerator,
    &quizTextExtractor,
    &quizStore,
    &quizAttemptStore,
    this
);

pages->addWidget(quizGeneratorPage);
stopwatchPage = new StopwatchPage(this);
    pages->addWidget(stopwatchPage);
    pages->setCurrentWidget(dashboardPage);

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

    connect(tasksBtn, &QPushButton::clicked, this, [=]() {
        pages->setCurrentWidget(tasksPage);
    });

    connect(tasksBackButton, &QPushButton::clicked, this, [=]() {
        pages->setCurrentWidget(dashboardPage);
    });

    connect(addTaskButton, &QPushButton::clicked, this, [=]() {
        showTaskDialog();
    });

    connect(subjectsBtn, &QPushButton::clicked, this, [=]() {
        pages->setCurrentWidget(subjectsPage);
    });

    

    connect(reportBtn, &QPushButton::clicked, this, [=]() {
    studyReportPage->refresh();
    pages->setCurrentWidget(studyReportPage);
});

connect(quizBtn, &QPushButton::clicked, this, [this]() {
    quizGeneratorPage->refresh();
    pages->setCurrentWidget(quizGeneratorPage);
});
connect(stopwatchBtn, &QPushButton::clicked, this, [=]() {
    pages->setCurrentWidget(stopwatchPage);
});
connect(quizGeneratorPage, &QuizGeneratorPage::pageClosed,
        this, [this, dashboardPage]() {
    pages->setCurrentWidget(dashboardPage);
});


    connect(subjectsPage, &SubjectsManagerPage::backRequested, this, [=]() {
        pages->setCurrentWidget(dashboardPage);
    });

    connect(studyReportPage, &StudyReportPage::backRequested, this, [=]() {
        pages->setCurrentWidget(dashboardPage);
    });

    connect(subjectsPage, &SubjectsManagerPage::subjectsChanged, this, [=]() {
        updateDashboardStats();
    });

    // Timer Countdown

    timer->setInterval(1000);

    connect(timerDurationCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int index) {
        if (timer->isActive() || !activeSessionStartTime.isNull())
            return;

        const int selectedSeconds = timerDurationCombo->itemData(index).toInt();
        if (selectedSeconds > 0)
            remainingMilliseconds = static_cast<qint64>(selectedSeconds) * 1000;

        if (timerTaskCombo->currentIndex() == 0 && selectedSeconds > 0)
            previouslySelectedGlobalTimerDurationSeconds = selectedSeconds;

        updateTimerDisplay();
    });

    connect(timerTaskCombo, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int) {
        if (timer->isActive() || !activeSessionStartTime.isNull())
            return;
        loadSelectedTaskDuration();
    });

    connect(timer, &QTimer::timeout, this, [=]() {
        remainingMilliseconds = qMax<qint64>(0,
            QDateTime::currentDateTime().msecsTo(sessionEndTime));
        updateTimerDisplay();

        if (remainingMilliseconds <= 0) {
            timer->stop();
            timerLabel->setText("Time's Up!");
            completeFocusSession();
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

    // ================= MENU BAR =================
    // Create the menubar with a Settings menu for API Key Management
    QMenu *settingsMenu = menuBar()->addMenu("Settings");
    QAction *apiKeyAction = settingsMenu->addAction("API Key...");
    connect(apiKeyAction, &QAction::triggered, this, &MainWindow::onApiKeyManagement);

    refreshTaskCards();
    updateDashboardStats();
}

// ================= TIMER FUNCTIONS =================
void MainWindow::onApiKeyManagement()
{
    ApiKeyEditorDialog dialog(this);
    dialog.exec();
}
void MainWindow::updateTimerDisplay()
{
    const qint64 displayedSeconds = (remainingMilliseconds + 999) / 1000;
    const int minutes = static_cast<int>(displayedSeconds / 60);
    const int seconds = static_cast<int>(displayedSeconds % 60);

    timerLabel->setText(
        QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'))
    );
}

void MainWindow::startTimer()
{
    if (unsavedCompletedSessionPending) {
        showUnsavedSessionRecovery("start a new session");
        return;
    }

    if (timer->isActive() || remainingMilliseconds <= 0)
        return;

    if (activeSessionStartTime.isNull()) {
        activeSessionDurationSeconds = timerDurationCombo->currentData().toInt();
        remainingMilliseconds = static_cast<qint64>(activeSessionDurationSeconds) * 1000;
        activeSessionStartTime = QDateTime::currentDateTime();

        activeTimerTaskId = timerTaskCombo->currentData().toString();
        activeTimerTaskTitle.clear();
        for (const Task &task : taskStore.tasks()) {
            if (task.id == activeTimerTaskId) {
                activeTimerTaskTitle = task.title;
                break;
            }
        }

        const int subjectIndex = timerSubjectCombo->currentIndex();
        if (subjectIndex > 0) {
            activeTimerSubjectId = timerSubjectCombo->currentData().toString();
            activeTimerSubjectName = timerSubjectCombo->currentText();
        } else {
            activeTimerSubjectId.clear();
            activeTimerSubjectName = "Uncategorized";
        }

        timerDurationCombo->setEnabled(false);
        timerTaskCombo->setEnabled(false);
        timerSubjectCombo->setEnabled(false);
    }

    sessionEndTime = QDateTime::currentDateTime().addMSecs(remainingMilliseconds);
    timer->start();
}

void MainWindow::pauseTimer()
{
    bool deadlineReached = false;
    if (timer->isActive()) {
        const QDateTime now = QDateTime::currentDateTime();
        deadlineReached = now >= sessionEndTime;
        remainingMilliseconds = qMax<qint64>(0,
            now.msecsTo(sessionEndTime));
        sessionEndTime = QDateTime();
    }
    timer->stop();

    remainingMilliseconds = qMax<qint64>(0, remainingMilliseconds);
    if (deadlineReached && !activeSessionStartTime.isNull()) {
        timerLabel->setText("Time's Up!");
        completeFocusSession();
        return;
    }

    updateTimerDisplay();
}

void MainWindow::resetTimer()
{
    if (unsavedCompletedSessionPending) {
        showUnsavedSessionRecovery("reset the timer");
        return;
    }

    timer->stop();
    sessionEndTime = QDateTime();
    activeSessionStartTime = QDateTime();
    activeTimerTaskId.clear();
    activeTimerTaskTitle.clear();
    activeTimerSubjectId.clear();
    activeTimerSubjectName.clear();

    activeSessionDurationSeconds = timerDurationCombo->currentData().toInt();
    if (activeSessionDurationSeconds <= 0)
        activeSessionDurationSeconds = previouslySelectedGlobalTimerDurationSeconds;
    remainingMilliseconds = static_cast<qint64>(activeSessionDurationSeconds) * 1000;

    timerDurationCombo->setEnabled(true);
    timerTaskCombo->setEnabled(true);
    timerSubjectCombo->setEnabled(true);
    updateTimerDisplay();
}

void MainWindow::completeFocusSession()
{
    if (unsavedCompletedSessionPending) {
        showUnsavedSessionRecovery("complete the session save");
        return;
    }

    if (activeSessionStartTime.isNull() || remainingMilliseconds > 0)
        return;

    StudySession session;
    session.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.startTime = activeSessionStartTime;
    session.endTime = QDateTime::currentDateTime();
    session.plannedSeconds = activeSessionDurationSeconds;
    session.actualSeconds = activeSessionDurationSeconds;
    session.subjectId = activeTimerSubjectId;
    session.subjectName = activeTimerSubjectName.isEmpty()
        ? "Uncategorized"
        : activeTimerSubjectName;
    session.taskId = activeTimerTaskId;
    session.taskTitle = activeTimerTaskTitle;

    unsavedCompletedSession = session;

    if (studySessionStore.addSession(session)) {
        clearUnsavedSessionAfterSuccess();
        return;
    }

    unsavedCompletedSessionPending = true;
    activeSessionStartTime = QDateTime();
    remainingMilliseconds = 0;
    timer->stop();
    sessionEndTime = QDateTime();

    timerDurationCombo->setEnabled(false);
    timerTaskCombo->setEnabled(false);
    timerSubjectCombo->setEnabled(false);
    timerLabel->setText("Time's Up!");

    showUnsavedSessionRecovery("save the completed session");
}

// ================= TASKS FUNCTIONS =================

void MainWindow::updateDashboardStats()
{
    const QVector<Task> tasks = taskStore.tasks();
    int completed = 0;
    int totalStudySeconds = 0;

    for (const Task &task : tasks) {
        if (task.completed)
            ++completed;
    }

    for (const StudySession &session : studySessionStore.sessions())
        totalStudySeconds += session.actualSeconds;

    const int studyHours = totalStudySeconds / 3600;
    const int studyMinutes = (totalStudySeconds % 3600) / 60;
    const QString studyDuration = studyHours > 0
        ? QString("%1h %2m").arg(studyHours).arg(studyMinutes)
        : QString("%1m").arg(studyMinutes);

    tasksCard->setText(QString("Total Tasks\n%1").arg(tasks.size()));
    completedCard->setText(QString("Completed\n%1").arg(completed));
    subjectsCard->setText(QString("Subjects\n%1").arg(subjectStore.subjects().size()));
    studyHoursCard->setText("Study Hours\n" + studyDuration);
    refreshRecentTasks();
}

void MainWindow::refreshRecentTasks()
{
    const QVector<Task> tasks = taskStore.tasks();

    if (tasks.isEmpty()) {
        recentTaskLabels[0]->setText("No tasks yet. Add a task to get started.");
        recentTaskLabels[0]->show();
        for (int i = 1; i < recentTaskLabels.size(); ++i)
            recentTaskLabels[i]->hide();
        return;
    }

    for (int i = 0; i < recentTaskLabels.size(); ++i) {
        const int taskIndex = tasks.size() - 1 - i;
        if (taskIndex >= 0) {
            recentTaskLabels[i]->setText(tasks[taskIndex].title);
            recentTaskLabels[i]->show();
        } else {
            recentTaskLabels[i]->hide();
        }
    }
}

void MainWindow::refreshTaskCards()
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(taskCardsContainer->layout());

    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    const QVector<Task> tasks = taskStore.tasks();

    if (tasks.isEmpty()) {
        QLabel *emptyLabel = new QLabel("No tasks yet. Click \"Add New Task\" to get started.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 16px; color: #777777; padding: 35px;");
        layout->addWidget(emptyLabel);
    }

    for (const Task &task : tasks) {
        QFrame *card = new QFrame;
        card->setStyleSheet("QFrame { background-color: white; border: 1px solid #E5E0FF; border-radius: 15px; }");

        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(18, 15, 18, 15);

        QCheckBox *completeBox = new QCheckBox;
        completeBox->setChecked(task.completed);

        QVBoxLayout *textLayout = new QVBoxLayout;
        QLabel *titleLabel = new QLabel(task.title);
        titleLabel->setStyleSheet(task.completed
            ? "font-size: 17px; font-weight: bold; color: #777777; text-decoration: line-through;"
            : "font-size: 17px; font-weight: bold; color: #25243A;");

        QString description = task.description.isEmpty() ? "No description" : task.description;
        QLabel *descriptionLabel = new QLabel(description);
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setStyleSheet("font-size: 14px; color: #777777;");

        QString dueText = task.dueDate.isValid() ? task.dueDate.toString("dd MMM yyyy") : "No due date";
        if (task.dueTime.isValid())
            dueText += " • " + task.dueTime.toString("h:mm AP");
        const int studyMinutes = qBound(1, task.studyDurationSeconds / 60, 180);
        QLabel *detailsLabel = new QLabel("Due: " + dueText + "  •  Study: " +
                                         QString::number(studyMinutes) + " min  •  Priority: " + task.priority);
        detailsLabel->setStyleSheet("font-size: 13px; color: #6C63FF;");

        textLayout->addWidget(titleLabel);
        textLayout->addWidget(descriptionLabel);
        textLayout->addWidget(detailsLabel);

        if (!task.attachments.isEmpty()) {
            QStringList attachmentNames;
            for (const TaskAttachment &attachment : task.attachments) {
                const QString absolutePath = attachmentAbsolutePath(attachment.relativePath);
                const bool missing = absolutePath.isEmpty() || !QFileInfo::exists(absolutePath);
                attachmentNames.append(missing
                    ? QStringLiteral("Missing: ") + attachment.fileName
                    : attachment.fileName);
            }

            QLabel *attachmentsLabel = new QLabel(
                QStringLiteral("Attachments (%1): ").arg(task.attachments.size()) +
                attachmentNames.join(QStringLiteral(" • ")));
            attachmentsLabel->setWordWrap(true);
            attachmentsLabel->setStyleSheet("font-size: 13px; color: #777777;");
            textLayout->addWidget(attachmentsLabel);
        }

        QPushButton *editButton = new QPushButton("Edit");
        QPushButton *deleteButton = new QPushButton("Delete");
        editButton->setMinimumWidth(90);
        deleteButton->setMinimumWidth(90);

        QVBoxLayout *actionsLayout = new QVBoxLayout;
        actionsLayout->addWidget(editButton);
        actionsLayout->addWidget(deleteButton);

        cardLayout->addWidget(completeBox);
        cardLayout->addLayout(textLayout, 1);
        cardLayout->addLayout(actionsLayout);
        layout->addWidget(card);

        connect(completeBox, &QCheckBox::toggled, this, [=](bool checked) {
            Task updated = task;
            updated.completed = checked;
            if (!taskStore.updateTask(updated)) {
                QMessageBox::warning(this, "Task Not Saved",
                                     "The task completion state could not be saved. Please try again.");
            }
            refreshTaskCards();
            updateDashboardStats();
        });

        connect(editButton, &QPushButton::clicked, this, [=]() {
            Task editable = task;
            showTaskDialog(&editable);
        });

        connect(deleteButton, &QPushButton::clicked, this, [=]() {
            QMessageBox confirmation;
            confirmation.setWindowTitle("Delete Task");
            confirmation.setText("Delete \"" + task.title + "\"?");
            confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            confirmation.setDefaultButton(QMessageBox::No);

            if (confirmation.exec() == QMessageBox::Yes) {
                if (!taskStore.removeTask(task.id)) {
                    QMessageBox::warning(this, "Task Not Deleted",
                                         "The task could not be deleted. Please try again.");
                } else {
                    removeTaskAttachmentFolder(task.id);
                    // Cascade: safely remove this task's quizzes and attempts.
                    // Attempts are removed first so none reference a deleted quiz.
                    quizAttemptStore.removeAttemptsForTask(task.id);
                    quizStore.removeQuizzesForTask(task.id);
                }
                refreshTaskCards();
                refreshTimerTaskChoices();
                updateDashboardStats();
            }
        });
    }

    layout->addStretch();
    updateDashboardStats();
}

void MainWindow::showTaskDialog(Task *existingTask)
{
    QDialog dialog(this);
    dialog.setWindowTitle(existingTask ? "Edit Task" : "Add New Task");
    dialog.resize(520, 560);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLineEdit *titleEdit = new QLineEdit;
    QTextEdit *descriptionEdit = new QTextEdit;
    QDateEdit *dueDateEdit = new QDateEdit;
    QCheckBox *dueTimeCheck = new QCheckBox("Set Due Time");
    QTimeEdit *dueTimeEdit = new QTimeEdit;
    QComboBox *studyDurationPresetBox = new QComboBox;
    QSpinBox *customDurationSpin = new QSpinBox;
    QComboBox *priorityBox = new QComboBox;

    titleEdit->setPlaceholderText("Task title");
    descriptionEdit->setPlaceholderText("Task description");
    descriptionEdit->setMinimumHeight(90);

    dueDateEdit->setCalendarPopup(true);
    dueDateEdit->setMinimumDate(QDate::currentDate().addYears(-1));
    dueDateEdit->setMaximumDate(QDate::currentDate().addYears(10));
    dueDateEdit->setSpecialValueText("No due date");

    dueTimeEdit->setDisplayFormat("h:mm AP");
    dueTimeEdit->setTime(QTime(9, 0));
    dueTimeEdit->setEnabled(false);

    for (int minutes : {2, 3, 5, 10, 20, 25})
        studyDurationPresetBox->addItem(QString::number(minutes) + " minutes", minutes);
    studyDurationPresetBox->addItem("Custom", 0);

    customDurationSpin->setRange(1, 180);
    customDurationSpin->setValue(25);
    customDurationSpin->setSuffix(" minutes");
    customDurationSpin->setEnabled(false);

    priorityBox->addItems({"Low", "Medium", "High"});

    if (existingTask) {
        titleEdit->setText(existingTask->title);
        descriptionEdit->setPlainText(existingTask->description);
        if (existingTask->dueDate.isValid())
            dueDateEdit->setDate(existingTask->dueDate);
        else
            dueDateEdit->setDate(dueDateEdit->minimumDate());

        if (existingTask->dueTime.isValid()) {
            dueTimeCheck->setChecked(true);
            dueTimeEdit->setTime(existingTask->dueTime);
            dueTimeEdit->setEnabled(true);
        }

        if (existingTask->studyDurationPreset == "custom") {
            studyDurationPresetBox->setCurrentIndex(studyDurationPresetBox->findData(0));
            customDurationSpin->setValue(qBound(1, existingTask->studyDurationSeconds / 60, 180));
            customDurationSpin->setEnabled(true);
        } else {
            const int presetMinutes = existingTask->studyDurationSeconds / 60;
            int presetIndex = studyDurationPresetBox->findData(presetMinutes);
            if (presetIndex < 0)
                presetIndex = studyDurationPresetBox->findData(25);
            studyDurationPresetBox->setCurrentIndex(presetIndex);
        }

        priorityBox->setCurrentText(existingTask->priority);
    } else {
        dueDateEdit->setDate(QDate::currentDate());
        studyDurationPresetBox->setCurrentIndex(studyDurationPresetBox->findData(25));
        priorityBox->setCurrentText("Medium");
    }

    connect(dueTimeCheck, &QCheckBox::toggled, dueTimeEdit, &QWidget::setEnabled);
    connect(studyDurationPresetBox, qOverload<int>(&QComboBox::currentIndexChanged), this,
            [=](int index) {
                customDurationSpin->setEnabled(studyDurationPresetBox->itemData(index).toInt() == 0);
            });

    layout->addWidget(new QLabel("Title *"));
    layout->addWidget(titleEdit);
    layout->addWidget(new QLabel("Description"));
    layout->addWidget(descriptionEdit);

    QHBoxLayout *dateTimePriorityLayout = new QHBoxLayout;
    QVBoxLayout *dateLayout = new QVBoxLayout;
    QVBoxLayout *timeLayout = new QVBoxLayout;
    QVBoxLayout *priorityLayout = new QVBoxLayout;
    dateLayout->addWidget(new QLabel("Due Date"));
    dateLayout->addWidget(dueDateEdit);
    timeLayout->addWidget(dueTimeCheck);
    timeLayout->addWidget(dueTimeEdit);
    priorityLayout->addWidget(new QLabel("Priority"));
    priorityLayout->addWidget(priorityBox);
    dateTimePriorityLayout->addLayout(dateLayout, 1);
    dateTimePriorityLayout->addLayout(timeLayout, 1);
    dateTimePriorityLayout->addLayout(priorityLayout, 1);
    layout->addLayout(dateTimePriorityLayout);

    QHBoxLayout *studyDurationLayout = new QHBoxLayout;
    studyDurationLayout->addWidget(new QLabel("Study Duration"));
    studyDurationLayout->addWidget(studyDurationPresetBox, 1);
    studyDurationLayout->addWidget(customDurationSpin, 1);
    layout->addLayout(studyDurationLayout);

    struct StagedAttachment
    {
        QString sourcePath;
        QString fileName;
    };

    QVector<TaskAttachment> remainingAttachments = existingTask
        ? existingTask->attachments
        : QVector<TaskAttachment>();
    QVector<TaskAttachment> removedAttachments;
    QVector<StagedAttachment> stagedAttachments;

    layout->addWidget(new QLabel("Attachments"));
    QPushButton *addAttachmentsButton = new QPushButton("Add Attachments");
    layout->addWidget(addAttachmentsButton);

    QScrollArea *attachmentScroll = new QScrollArea;
    attachmentScroll->setWidgetResizable(true);
    attachmentScroll->setMaximumHeight(150);
    QWidget *attachmentList = new QWidget;
    QVBoxLayout *attachmentListLayout = new QVBoxLayout(attachmentList);
    attachmentListLayout->setContentsMargins(0, 0, 0, 0);
    attachmentListLayout->setSpacing(4);
    attachmentScroll->setWidget(attachmentList);
    layout->addWidget(attachmentScroll);

    std::function<void()> refreshAttachmentList;
    refreshAttachmentList = [&]() {
        while (attachmentListLayout->count() > 0) {
            QLayoutItem *item = attachmentListLayout->takeAt(0);
            if (item->widget())
                item->widget()->deleteLater();
            delete item;
        }

        if (remainingAttachments.isEmpty() && stagedAttachments.isEmpty()) {
            QLabel *emptyLabel = new QLabel("No attachments selected.");
            emptyLabel->setStyleSheet("color: #777777;");
            attachmentListLayout->addWidget(emptyLabel);
        }

        for (const TaskAttachment &attachment : remainingAttachments) {
            QWidget *row = new QWidget;
            QHBoxLayout *rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(4, 2, 4, 2);

            const QString absolutePath = attachmentAbsolutePath(attachment.relativePath);
            const bool missing = absolutePath.isEmpty() || !QFileInfo::exists(absolutePath);
            QLabel *nameLabel = new QLabel(attachment.fileName + (missing ? " (missing)" : QString()));
            nameLabel->setWordWrap(true);

            QPushButton *openButton = new QPushButton("Open");
            QPushButton *removeButton = new QPushButton("Remove");

            openButton->setMinimumWidth(70);
            removeButton->setMinimumWidth(70);

            rowLayout->addWidget(nameLabel, 1);
            rowLayout->addWidget(openButton);
            rowLayout->addWidget(removeButton);
            attachmentListLayout->addWidget(row);

            connect(openButton, &QPushButton::clicked, this, [=]() {
                const QString openPath = attachmentAbsolutePath(attachment.relativePath);
                if (openPath.isEmpty() || !QFileInfo::exists(openPath)) {
                    QMessageBox::warning(this, "Attachment Missing",
                                         "The attachment could not be found:\n" + attachment.fileName);
                    return;
                }

                if (!QDesktopServices::openUrl(QUrl::fromLocalFile(openPath))) {
                    QMessageBox::warning(this, "Attachment Could Not Be Opened",
                                         "No compatible application could open:\n" + attachment.fileName);
                }
            });

            connect(removeButton, &QPushButton::clicked, this, [&, attachment]() {
                for (int i = 0; i < remainingAttachments.size(); ++i) {
                    if (remainingAttachments[i].id == attachment.id) {
                        removedAttachments.append(remainingAttachments[i]);
                        remainingAttachments.removeAt(i);
                        break;
                    }
                }
                refreshAttachmentList();
            });
        }

        for (int stagedIndex = 0; stagedIndex < stagedAttachments.size(); ++stagedIndex) {
            const StagedAttachment staged = stagedAttachments[stagedIndex];

            QWidget *row = new QWidget;
            QHBoxLayout *rowLayout = new QHBoxLayout(row);
            rowLayout->setContentsMargins(4, 2, 4, 2);

            QLabel *nameLabel = new QLabel(staged.fileName);
            nameLabel->setWordWrap(true);
            QPushButton *removeButton = new QPushButton("Remove");
            removeButton->setMinimumWidth(70);

            rowLayout->addWidget(nameLabel, 1);
            rowLayout->addWidget(removeButton);
            attachmentListLayout->addWidget(row);

            connect(removeButton, &QPushButton::clicked, this, [&, stagedIndex]() {
                if (stagedIndex >= 0 && stagedIndex < stagedAttachments.size())
                    stagedAttachments.removeAt(stagedIndex);
                refreshAttachmentList();
            });
        }

        attachmentListLayout->addStretch();
    };

    connect(addAttachmentsButton, &QPushButton::clicked, this, [=, &dialog, &stagedAttachments]() {
        const QStringList selectedFiles = QFileDialog::getOpenFileNames(
            &dialog,
            "Select PDF or PNG Attachments",
            QString(),
            "Supported files (*.pdf *.PDF *.png *.PNG);;PDF files (*.pdf *.PDF);;PNG files (*.png *.PNG)");

        for (const QString &selectedFile : selectedFiles) {
            if (!isSupportedAttachmentType(selectedFile)) {
                QMessageBox::warning(&dialog, "Unsupported Attachment",
                                     "Only valid PDF and PNG files can be attached:\n" +
                                     QFileInfo(selectedFile).fileName());
                continue;
            }

            StagedAttachment staged;
            staged.sourcePath = selectedFile;
            staged.fileName = QFileInfo(selectedFile).fileName();
            stagedAttachments.append(staged);
        }

        refreshAttachmentList();
    });

    refreshAttachmentList();

    QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString title = titleEdit->text().trimmed();
    if (title.isEmpty()) {
        QMessageBox::warning(this, "Task Title Required", "Please enter a task title.");
        showTaskDialog(existingTask);
        return;
    }

    const bool customDuration = studyDurationPresetBox->currentData().toInt() == 0;
    const int durationMinutes = customDuration
        ? customDurationSpin->value()
        : studyDurationPresetBox->currentData().toInt();

    if (durationMinutes < 1 || durationMinutes > 180) {
        QMessageBox::warning(this, "Invalid Study Duration",
                             "Study duration must be between 1 and 180 minutes.");
        return;
    }

    Task task;
    task.id = existingTask ? existingTask->id : QUuid::createUuid().toString(QUuid::WithoutBraces);
    task.title = title;
    task.description = descriptionEdit->toPlainText().trimmed();
    task.dueDate = dueDateEdit->date() == dueDateEdit->minimumDate()
        ? QDate()
        : dueDateEdit->date();
    task.dueTime = dueTimeCheck->isChecked() ? dueTimeEdit->time() : QTime();
    task.studyDurationPreset = customDuration ? "custom" : QString::number(durationMinutes);
    task.studyDurationSeconds = durationMinutes * 60;
    task.priority = priorityBox->currentText();
    task.completed = existingTask ? existingTask->completed : false;
    task.attachments = remainingAttachments;

    QVector<TaskAttachment> newlyCopiedAttachments;
    for (const StagedAttachment &staged : stagedAttachments) {
        TaskAttachment attachment;
        if (!copyAttachmentToStorage(staged.sourcePath, task.id, attachment)) {
            for (const TaskAttachment &copied : newlyCopiedAttachments)
                removeAttachmentFile(copied);

            QMessageBox::warning(this, "Attachment Copy Failed",
                                 "The attachment could not be copied safely:\n" + staged.fileName);
            return;
        }
        newlyCopiedAttachments.append(attachment);
    }
    task.attachments.append(newlyCopiedAttachments);

    const bool saved = existingTask
        ? taskStore.updateTask(task)
        : taskStore.addTask(task);

    if (!saved) {
        for (const TaskAttachment &copied : newlyCopiedAttachments)
            removeAttachmentFile(copied);

        QMessageBox::warning(this, "Task Not Saved",
                             "The task could not be saved. Please check available storage and try again.");
        return;
    }

    if (existingTask) {
        for (const TaskAttachment &removed : removedAttachments)
            removeAttachmentFile(removed);
    }

    refreshTaskCards();
    refreshTimerTaskChoices();
    updateDashboardStats();
}

void MainWindow::refreshTimerTaskChoices()
{
    if (!timerTaskCombo)
        return;

    const QString selectedTaskId = timerTaskCombo->currentData().toString();
    timerTaskCombo->blockSignals(true);
    timerTaskCombo->clear();
    timerTaskCombo->addItem("No task", QString());

    for (const Task &task : taskStore.tasks()) {
        if (!task.completed)
            timerTaskCombo->addItem(task.title, task.id);
    }

    int selectedIndex = timerTaskCombo->findData(selectedTaskId);
    if (selectedIndex >= 0)
        timerTaskCombo->setCurrentIndex(selectedIndex);
    else
        timerTaskCombo->setCurrentIndex(0);

    timerTaskCombo->blockSignals(false);

    if (!timer->isActive() && activeSessionStartTime.isNull())
        loadSelectedTaskDuration();
}

void MainWindow::loadSelectedTaskDuration()
{
    const QString taskId = timerTaskCombo->currentData().toString();

    timerDurationCombo->blockSignals(true);

    for (int i = timerDurationCombo->count() - 1; i >= 0; --i) {
        if (timerDurationCombo->itemData(i, Qt::UserRole + 1).toBool())
            timerDurationCombo->removeItem(i);
    }

    if (taskId.isEmpty()) {
        int index = timerDurationCombo->findData(previouslySelectedGlobalTimerDurationSeconds);
        if (index < 0)
            index = timerDurationCombo->findData(25 * 60);
        if (index >= 0) {
            timerDurationCombo->setCurrentIndex(index);
            remainingMilliseconds = static_cast<qint64>(
                timerDurationCombo->itemData(index).toInt()) * 1000;
        }
        timerDurationCombo->blockSignals(false);
        updateTimerDisplay();
        return;
    }

    for (const Task &task : taskStore.tasks()) {
        if (task.id == taskId) {
            const int duration = task.studyDurationSeconds;
            if (duration >= 60 && duration <= 180 * 60) {
                int index = timerDurationCombo->findData(duration);
                if (index < 0) {
                    timerDurationCombo->addItem(
                        QString("%1 minute%2 (task)")
                            .arg(duration / 60)
                            .arg(duration == 60 ? "" : "s"),
                        duration);
                    index = timerDurationCombo->count() - 1;
                    timerDurationCombo->setItemData(index, true, Qt::UserRole + 1);
                }

                timerDurationCombo->setCurrentIndex(index);
                remainingMilliseconds = static_cast<qint64>(duration) * 1000;
            }
            break;
        }
    }

    timerDurationCombo->blockSignals(false);
    updateTimerDisplay();
}

// ================= UNSAVED SESSION RECOVERY =================

void MainWindow::clearUnsavedSessionAfterSuccess()
{
    unsavedCompletedSessionPending = false;
    unsavedCompletedSession = StudySession();
    activeSessionStartTime = QDateTime();
    sessionEndTime = QDateTime();
    timer->stop();

    activeTimerTaskId.clear();
    activeTimerTaskTitle.clear();
    activeTimerSubjectId.clear();
    activeTimerSubjectName.clear();

    remainingMilliseconds = 0;
    timerDurationCombo->setEnabled(true);
    timerTaskCombo->setEnabled(true);
    timerSubjectCombo->setEnabled(true);
    timerLabel->setText("Time's Up!");

    updateDashboardStats();
    if (studyReportPage)
        studyReportPage->refresh();
}

bool MainWindow::retryUnsavedCompletedSession()
{
    if (!unsavedCompletedSessionPending)
        return true;

    if (!studySessionStore.addSession(unsavedCompletedSession))
        return false;

    clearUnsavedSessionAfterSuccess();
    return true;
}

void MainWindow::showUnsavedSessionRecovery(const QString &requestedAction)
{
    if (!unsavedCompletedSessionPending)
        return;

    while (unsavedCompletedSessionPending) {
        QMessageBox recovery(this);
        recovery.setIcon(QMessageBox::Warning);
        recovery.setWindowTitle("Completed Session Not Saved");
        recovery.setText("The completed study session could not be saved.");
        recovery.setInformativeText(
            "Choose Retry Save to try again, Discard to explicitly remove this unsaved session, "
            "or Cancel to keep it pending. The application will not start another session until this is resolved.");
        recovery.setStandardButtons(QMessageBox::Retry | QMessageBox::Discard | QMessageBox::Cancel);
        recovery.setDefaultButton(QMessageBox::Retry);

        const int choice = recovery.exec();
        if (choice == QMessageBox::Retry) {
            if (retryUnsavedCompletedSession()) {
                if (requestedAction == "reset the timer")
                    resetTimer();
                return;
            }

            QMessageBox::warning(this, "Save Still Failed",
                                 "The completed session could not be saved again. It remains pending and will not be duplicated by retry.");
            // Loop repeats to show the recovery dialog again; no nested dialogs are created.
            continue;
        }

        if (choice == QMessageBox::Discard) {
            QMessageBox confirmation(this);
            confirmation.setIcon(QMessageBox::Warning);
            confirmation.setWindowTitle("Discard Unsaved Session");
            confirmation.setText("Discard the completed study session permanently?");
            confirmation.setInformativeText("This session has not been saved and cannot be recovered after discarding.");
            confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            confirmation.setDefaultButton(QMessageBox::No);

            if (confirmation.exec() == QMessageBox::Yes) {
                unsavedCompletedSessionPending = false;
                unsavedCompletedSession = StudySession();
                activeSessionStartTime = QDateTime();
                sessionEndTime = QDateTime();
                timer->stop();

                activeTimerTaskId.clear();
                activeTimerTaskTitle.clear();
                activeTimerSubjectId.clear();
                activeTimerSubjectName.clear();

                activeSessionDurationSeconds = timerDurationCombo->currentData().toInt();
                if (activeSessionDurationSeconds <= 0)
                    activeSessionDurationSeconds = previouslySelectedGlobalTimerDurationSeconds;
                remainingMilliseconds = static_cast<qint64>(activeSessionDurationSeconds) * 1000;

                timerDurationCombo->setEnabled(true);
                timerTaskCombo->setEnabled(true);
                timerSubjectCombo->setEnabled(true);
                updateTimerDisplay();
            }
            return;
        }

        // Cancel: leave the unsaved session pending and exit without nesting dialogs.
        return;
    }
}