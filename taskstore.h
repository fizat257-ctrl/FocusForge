#ifndef TASKSTORE_H
#define TASKSTORE_H

#include "task.h"

#include <QVector>

class TaskStore
{
public:
    TaskStore();

    QVector<Task> tasks() const;
    void addTask(const Task &task);
    void updateTask(const Task &task);
    void removeTask(const QString &id);

private:
    QVector<Task> m_tasks;

    void load();
    void save() const;
};

#endif
