#ifndef TASK_H
#define TASK_H

#include <QDate>
#include <QString>

struct Task
{
    QString id;
    QString title;
    QString description;
    QDate dueDate;
    QString priority;
    bool completed = false;
};

#endif
