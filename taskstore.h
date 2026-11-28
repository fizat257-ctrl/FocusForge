#ifndef TASKSTORE_H
#define TASKSTORE_H

#include "task.h"

#include <QVector>

class TaskStore
{
public:
    TaskStore();

    QVector<Task> tasks() const;
    bool addTask(const Task &task);
    bool updateTask(const Task &task);
    bool removeTask(const QString &id);

private:
    QVector<Task> m_tasks;

    void load();
    bool save() const;
};

#endif
