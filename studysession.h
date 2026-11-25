#ifndef STUDYSESSION_H
#define STUDYSESSION_H

#include <QDateTime>
#include <QString>

struct StudySession
{
    QString id;
    QDateTime startTime;
    QDateTime endTime;
    int plannedSeconds = 25 * 60;
    int actualSeconds = 25 * 60;
    QString subjectId;
    QString subjectName;
    QString taskId;
    QString taskTitle;
};

#endif
