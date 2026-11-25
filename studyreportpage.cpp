#include "studyreportpage.h"

#include <QDate>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPainter>
#include <QPushButton>
#include <QScrollArea>
#include <QVBoxLayout>

class WeeklyProgressWidget : public QWidget
{
public:
    explicit WeeklyProgressWidget(QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(220);
        setMaximumHeight(260);
    }

    void setMinutes(const QList<int> &minutes)
    {
        m_minutes = minutes;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        painter.fillRect(rect(), QColor("#FFFFFF"));

        int maxMinutes = 1;
        for (int value : m_minutes)
            maxMinutes = qMax(maxMinutes, value);

        const int left = 35;
        const int right = 25;
        const int top = 25;
        const int bottom = 45;
        const int chartWidth = qMax(100, width() - left - right);
        const int chartHeight = qMax(100, height() - top - bottom);
        const int slotWidth = chartWidth / 7;

        painter.setPen(QPen(QColor("#E5E0FF"), 1));
        for (int i = 0; i <= 4; ++i) {
            int y = top + chartHeight * i / 4;
            painter.drawLine(left, y, left + chartWidth, y);
        }

        const QStringList days = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
        for (int i = 0; i < 7; ++i) {
            int value = i < m_minutes.size() ? m_minutes[i] : 0;
            int barHeight = value > 0 ? chartHeight * value / maxMinutes : 0;
            int x = left + i * slotWidth + slotWidth / 4;
            int y = top + chartHeight - barHeight;
            int barWidth = slotWidth / 2;

            painter.setPen(Qt::NoPen);
            painter.setBrush(value > 0 ? QColor("#6C63FF") : QColor("#E5E0FF"));
            painter.drawRoundedRect(x, y, barWidth, barHeight, 6, 6);

            painter.setPen(QColor("#777777"));
            painter.drawText(x - 8, top + chartHeight + 24, barWidth + 16, 20,
                             Qt::AlignCenter, days[i]);

            if (value > 0) {
                painter.drawText(x - 12, qMax(2, y - 20), barWidth + 24, 18,
                                 Qt::AlignCenter, QString::number(value));
            }
        }
    }

private:
    QList<int> m_minutes;
};

static QString formatDuration(int seconds)
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;

    if (hours > 0)
        return QString("%1h %2m").arg(hours).arg(minutes);
    return QString("%1m").arg(minutes);
}

StudyReportPage::StudyReportPage(StudySessionStore *sessionStore,
                                 TaskStore *taskStore,
                                 SubjectStore *subjectStore,
                                 QWidget *parent)
    : QWidget(parent),
      m_sessionStore(sessionStore),
      m_taskStore(taskStore),
      m_subjectStore(subjectStore)
{
    QVBoxLayout *pageLayout = new QVBoxLayout(this);

    QPushButton *backButton = new QPushButton("← Back to Dashboard");

    QLabel *title = new QLabel("Study Report");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 32px; font-weight: bold; color: #6C63FF;");

    QLabel *subtitle = new QLabel("Analyze your focus sessions and productivity.");
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("font-size: 16px; color: gray;");

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addWidget(backButton);
    headerLayout->addStretch();

    pageLayout->addLayout(headerLayout);
    pageLayout->addWidget(title);
    pageLayout->addWidget(subtitle);

    QScrollArea *scroll = new QScrollArea;
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);

    QWidget *content = new QWidget;
    QVBoxLayout *contentLayout = new QVBoxLayout(content);
    contentLayout->setSpacing(15);

    QHBoxLayout *summaryLayout = new QHBoxLayout;

    QLabel *totalLabel = new QLabel;
    QLabel *todayLabel = new QLabel;
    QLabel *weekLabel = new QLabel;
    QLabel *sessionsLabel = new QLabel;

    QLabel *summaryLabels[] = {totalLabel, todayLabel, weekLabel, sessionsLabel};
    for (QLabel *label : summaryLabels) {
        label->setAlignment(Qt::AlignCenter);
        label->setMinimumHeight(105);
        label->setStyleSheet(
            "background-color: white;"
            "border: 1px solid #E5E0FF;"
            "border-radius: 15px;"
            "padding: 12px;"
            "font-size: 17px;"
            "font-weight: bold;"
            "color: #6C63FF;"
        );
        summaryLayout->addWidget(label);
    }

    contentLayout->addLayout(summaryLayout);

    QFrame *chartCard = new QFrame;
    chartCard->setStyleSheet("QFrame { background-color: white; border: 1px solid #E5E0FF; border-radius: 15px; }");
    QVBoxLayout *chartLayout = new QVBoxLayout(chartCard);

    QLabel *chartTitle = new QLabel("Weekly Progress");
    chartTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #25243A;");
    WeeklyProgressWidget *chart = new WeeklyProgressWidget;
    chartLayout->addWidget(chartTitle);
    chartLayout->addWidget(chart);

    contentLayout->addWidget(chartCard);

    QHBoxLayout *detailsLayout = new QHBoxLayout;

    QFrame *subjectCard = new QFrame;
    subjectCard->setStyleSheet("QFrame { background-color: white; border: 1px solid #E5E0FF; border-radius: 15px; }");
    QVBoxLayout *subjectLayout = new QVBoxLayout(subjectCard);

    QLabel *subjectTitle = new QLabel("Subject-wise Study Time");
    subjectTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #25243A;");
    subjectLayout->addWidget(subjectTitle);

    QLabel *subjectStats = new QLabel;
    subjectStats->setWordWrap(true);
    subjectStats->setStyleSheet("font-size: 14px; color: #555555;");
    subjectLayout->addWidget(subjectStats);

    QFrame *taskCard = new QFrame;
    taskCard->setStyleSheet("QFrame { background-color: white; border: 1px solid #E5E0FF; border-radius: 15px; }");
    QVBoxLayout *taskLayout = new QVBoxLayout(taskCard);

    QLabel *taskTitle = new QLabel("Task Completion");
    taskTitle->setStyleSheet("font-size: 20px; font-weight: bold; color: #25243A;");
    taskLayout->addWidget(taskTitle);

    QLabel *taskStats = new QLabel;
    taskStats->setWordWrap(true);
    taskStats->setStyleSheet("font-size: 14px; color: #555555;");
    taskLayout->addWidget(taskStats);

    detailsLayout->addWidget(subjectCard, 1);
    detailsLayout->addWidget(taskCard, 1);
    contentLayout->addLayout(detailsLayout);

    scroll->setWidget(content);
    pageLayout->addWidget(scroll);

    connect(backButton, &QPushButton::clicked, this, &StudyReportPage::backRequested);

    const auto refreshContent = [=]() {
        const QVector<StudySession> sessions = m_sessionStore->sessions();
        const QDate today = QDate::currentDate();
        const QDate weekStart = today.addDays(-(today.dayOfWeek() - 1));

        int totalSeconds = 0;
        int todaySeconds = 0;
        int weekSeconds = 0;
        QList<int> weekMinutes(7, 0);
        QMap<QString, int> subjectSeconds;

        for (const StudySession &session : sessions) {
            totalSeconds += session.actualSeconds;

            QDate sessionDate = session.endTime.isValid()
                ? session.endTime.date()
                : session.startTime.date();

            if (sessionDate == today)
                todaySeconds += session.actualSeconds;

            if (sessionDate >= weekStart && sessionDate <= today) {
                weekSeconds += session.actualSeconds;
                int dayIndex = sessionDate.dayOfWeek() - 1;
                weekMinutes[dayIndex] += session.actualSeconds / 60;
            }

            QString subjectName = session.subjectName.isEmpty()
                ? "Uncategorized"
                : session.subjectName;
            subjectSeconds[subjectName] += session.actualSeconds;
        }

        totalLabel->setText("Total Study Time\n" + formatDuration(totalSeconds));
        todayLabel->setText("Today's Study Time\n" + formatDuration(todaySeconds));
        weekLabel->setText("Weekly Study Time\n" + formatDuration(weekSeconds));
        sessionsLabel->setText("Completed Sessions\n" + QString::number(sessions.size()));
        chart->setMinutes(weekMinutes);

        if (subjectSeconds.isEmpty()) {
            subjectStats->setText("No completed focus sessions yet.");
        } else {
            QString text;
            for (auto it = subjectSeconds.begin(); it != subjectSeconds.end(); ++it)
                text += it.key() + ": " + formatDuration(it.value()) + "\n";
            subjectStats->setText(text.trimmed());
        }

        const QVector<Task> tasks = m_taskStore->tasks();
        int completedTasks = 0;
        for (const Task &task : tasks) {
            if (task.completed)
                ++completedTasks;
        }

        int percentage = tasks.isEmpty()
            ? 0
            : (completedTasks * 100 / tasks.size());

        taskStats->setText(
            "Total Tasks: " + QString::number(tasks.size()) + "\n" +
            "Completed: " + QString::number(completedTasks) + "\n" +
            "Completion: " + QString::number(percentage) + "%"
        );
    };

    refreshContent();

    connect(this, &StudyReportPage::backRequested, this, [=]() {});
}

void StudyReportPage::refresh()
{
    hide();
    show();
}
