#include "taskstore.h"

#include <QSettings>

TaskStore::TaskStore()
{
    load();
}

QVector<Task> TaskStore::tasks() const
{
    return m_tasks;
}

void TaskStore::addTask(const Task &task)
{
    m_tasks.append(task);
    save();
}

void TaskStore::updateTask(const Task &task)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == task.id) {
            m_tasks[i] = task;
            save();
            return;
        }
    }
}

void TaskStore::removeTask(const QString &id)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == id) {
            m_tasks.removeAt(i);
            save();
            return;
        }
    }
}

void TaskStore::load()
{
    m_tasks.clear();

    QSettings settings("FocusForge", "FocusForge");
    const int count = settings.beginReadArray("Tasks");

    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);

        Task task;
        task.id = settings.value("id").toString();
        task.title = settings.value("title").toString();
        task.description = settings.value("description").toString();
        task.dueDate = QDate::fromString(settings.value("dueDate").toString(), Qt::ISODate);
        task.priority = settings.value("priority", "Medium").toString();
        task.completed = settings.value("completed", false).toBool();

        if (!task.id.isEmpty()) {
            m_tasks.append(task);
        }
    }

    settings.endArray();
}

void TaskStore::save() const
{
    QSettings settings("FocusForge", "FocusForge");
    settings.beginWriteArray("Tasks", m_tasks.size());

    for (int i = 0; i < m_tasks.size(); ++i) {
        settings.setArrayIndex(i);
        const Task &task = m_tasks[i];

        settings.setValue("id", task.id);
        settings.setValue("title", task.title);
        settings.setValue("description", task.description);
        settings.setValue("dueDate", task.dueDate.isValid()
                             ? task.dueDate.toString(Qt::ISODate)
                             : QString());
        settings.setValue("priority", task.priority);
        settings.setValue("completed", task.completed);
    }

    settings.endArray();
    settings.sync();
}
