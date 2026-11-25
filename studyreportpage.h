#ifndef STUDYREPORTPAGE_H
#define STUDYREPORTPAGE_H

#include "studysessionstore.h"
#include "subjectstore.h"
#include "taskstore.h"

#include <QWidget>

class QLabel;
class WeeklyProgressWidget;

class StudyReportPage : public QWidget
{
    Q_OBJECT

public:
    StudyReportPage(StudySessionStore *sessionStore,
                    TaskStore *taskStore,
                    SubjectStore *subjectStore,
                    QWidget *parent = nullptr);

public slots:
    void refresh();

signals:
    void backRequested();

private:
    StudySessionStore *m_sessionStore;
    TaskStore *m_taskStore;
    SubjectStore *m_subjectStore;
    QLabel *m_totalLabel;
    QLabel *m_todayLabel;
    QLabel *m_weekLabel;
    QLabel *m_sessionsLabel;
    QLabel *m_subjectStats;
    QLabel *m_taskStats;
    WeeklyProgressWidget *m_chart;

    void updateReport();
};

#endif
