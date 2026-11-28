#ifndef TASK_H
#define TASK_H

#include <QDate>
#include <QTime>
#include <QString>
#include <QVector>

struct TaskAttachment
{
    QString id;
    QString fileName;
    QString relativePath;
};

struct Task
{
    QString id;
    QString title;
    QString description;
    QDate dueDate;
    QTime dueTime;
    int studyDurationSeconds = 25 * 60;
    QString studyDurationPreset = "25";
    QString priority;
    bool completed = false;
    QVector<TaskAttachment> attachments;
};

#endif
