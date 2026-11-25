#include "mainwindow.h"
#include "studyreportpage.h"

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
#include <QScrollArea>
#include <QTextEdit>
#include <QUuid>

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
      taskCardsContainer(nullptr),
      studyReportPage(nullptr),
      timerSubjectCombo(nullptr),
      timerTaskCombo(nullptr),
      remainingSeconds(25 * 60)
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

    tasksCard = new QLabel(QString("Total Tasks\n%1").arg(taskStore.tasks().size()));
    completedCard = new QLabel("Completed\n0");
    QLabel *hoursCard = new QLabel("Study Hours\n3.5");
    subjectsCard = new QLabel(QString("Subjects\n%1").arg(subjectStore.subjects().size()));

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

    QHBoxLayout *selectionLayout = new QHBoxLayout;
    QVBoxLayout *subjectSelectionLayout = new QVBoxLayout;
    QVBoxLayout *taskSelectionLayout = new QVBoxLayout;
    subjectSelectionLayout->addWidget(subjectSelectionLabel);
    subjectSelectionLayout->addWidget(timerSubjectCombo);
    taskSelectionLayout->addWidget(taskSelectionLabel);
    taskSelectionLayout->addWidget(timerTaskCombo);
    selectionLayout->addLayout(subjectSelectionLayout);
    selectionLayout->addLayout(taskSelectionLayout);

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

    refreshTaskCards();
    updateDashboardStats();
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
    if (remainingSeconds <= 0)
        return;

    if (activeSessionStartTime.isNull())
        activeSessionStartTime = QDateTime::currentDateTime();

    sessionEndTime = QDateTime::currentDateTime().addSecs(remainingSeconds);
    timer->start();
}

void MainWindow::pauseTimer()
{
    if (timer->isActive()) {
        remainingSeconds = qMax(0,
            QDateTime::currentDateTime().secsTo(sessionEndTime));
        sessionEndTime = QDateTime();
    }
    timer->stop();
    updateTimerDisplay();
}

void MainWindow::resetTimer()
{
    timer->stop();
    sessionEndTime = QDateTime();
    activeSessionStartTime = QDateTime();
    remainingSeconds = 25 * 60;
    updateTimerDisplay();
}

void MainWindow::completeFocusSession()
{
    if (activeSessionStartTime.isNull())
        return;

    StudySession session;
    session.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
    session.startTime = activeSessionStartTime;
    session.endTime = QDateTime::currentDateTime();
    session.plannedSeconds = 25 * 60;
    session.actualSeconds = 25 * 60;

    int subjectIndex = timerSubjectCombo ? timerSubjectCombo->currentIndex() : -1;
    if (subjectIndex > 0) {
        session.subjectId = timerSubjectCombo->currentData().toString();
        session.subjectName = timerSubjectCombo->currentText();
    } else {
        session.subjectName = "Uncategorized";
    }

    int taskIndex = timerTaskCombo ? timerTaskCombo->currentIndex() : -1;
    if (taskIndex > 0) {
        session.taskId = timerTaskCombo->currentData().toString();
        session.taskTitle = timerTaskCombo->currentText();
    }

    studySessionStore.addSession(session);
    activeSessionStartTime = QDateTime();

    if (studyReportPage)
        studyReportPage->refresh();
}

// ================= TASKS FUNCTIONS =================

void MainWindow::updateDashboardStats()
{
    const QVector<Task> tasks = taskStore.tasks();
    int completed = 0;

    for (const Task &task : tasks) {
        if (task.completed)
            ++completed;
    }

    tasksCard->setText(QString("Total Tasks\n%1").arg(tasks.size()));
    completedCard->setText(QString("Completed\n%1").arg(completed));
    subjectsCard->setText(QString("Subjects\n%1").arg(subjectStore.subjects().size()));
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
        QLabel *detailsLabel = new QLabel("Due: " + dueText + "  •  Priority: " + task.priority);
        detailsLabel->setStyleSheet("font-size: 13px; color: #6C63FF;");

        textLayout->addWidget(titleLabel);
        textLayout->addWidget(descriptionLabel);
        textLayout->addWidget(detailsLabel);

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
            taskStore.updateTask(updated);
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
                taskStore.removeTask(task.id);
                refreshTaskCards();
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
    dialog.resize(480, 390);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLineEdit *titleEdit = new QLineEdit;
    QTextEdit *descriptionEdit = new QTextEdit;
    QDateEdit *dueDateEdit = new QDateEdit;
    QComboBox *priorityBox = new QComboBox;

    titleEdit->setPlaceholderText("Task title");
    descriptionEdit->setPlaceholderText("Task description");
    descriptionEdit->setMinimumHeight(110);

    dueDateEdit->setCalendarPopup(true);
    dueDateEdit->setMinimumDate(QDate::currentDate().addYears(-1));
    dueDateEdit->setMaximumDate(QDate::currentDate().addYears(10));
    priorityBox->addItems({"Low", "Medium", "High"});

    if (existingTask) {
        titleEdit->setText(existingTask->title);
        descriptionEdit->setPlainText(existingTask->description);
        dueDateEdit->setDate(existingTask->dueDate.isValid() ? existingTask->dueDate : QDate::currentDate());
        priorityBox->setCurrentText(existingTask->priority);
    } else {
        dueDateEdit->setDate(QDate::currentDate());
        priorityBox->setCurrentText("Medium");
    }

    layout->addWidget(new QLabel("Title *"));
    layout->addWidget(titleEdit);
    layout->addWidget(new QLabel("Description"));
    layout->addWidget(descriptionEdit);

    QHBoxLayout *datePriorityLayout = new QHBoxLayout;
    QVBoxLayout *dateLayout = new QVBoxLayout;
    QVBoxLayout *priorityLayout = new QVBoxLayout;
    dateLayout->addWidget(new QLabel("Due Date"));
    dateLayout->addWidget(dueDateEdit);
    priorityLayout->addWidget(new QLabel("Priority"));
    priorityLayout->addWidget(priorityBox);
    datePriorityLayout->addLayout(dateLayout, 1);
    datePriorityLayout->addLayout(priorityLayout, 1);
    layout->addLayout(datePriorityLayout);

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

    Task task;
    task.id = existingTask ? existingTask->id : QUuid::createUuid().toString(QUuid::WithoutBraces);
    task.title = title;
    task.description = descriptionEdit->toPlainText().trimmed();
    task.dueDate = dueDateEdit->date();
    task.priority = priorityBox->currentText();
    task.completed = existingTask ? existingTask->completed : false;

    if (existingTask)
        taskStore.updateTask(task);
    else
        taskStore.addTask(task);

    refreshTaskCards();
    updateDashboardStats();
}