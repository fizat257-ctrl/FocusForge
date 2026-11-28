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

bool TaskStore::addTask(const Task &task)
{
    m_tasks.append(task);
    if (save())
        return true;

    m_tasks.removeLast();
    return false;
}

bool TaskStore::updateTask(const Task &task)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == task.id) {
            const Task previousTask = m_tasks[i];
            m_tasks[i] = task;
            if (save())
                return true;

            m_tasks[i] = previousTask;
            return false;
        }
    }
    return false;
}

bool TaskStore::removeTask(const QString &id)
{
    for (int i = 0; i < m_tasks.size(); ++i) {
        if (m_tasks[i].id == id) {
            const Task previousTask = m_tasks[i];
            m_tasks.removeAt(i);
            if (save())
                return true;

            m_tasks.insert(i, previousTask);
            return false;
        }
    }
    return false;
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

        const QString dueTimeText = settings.value("dueTime").toString().trimmed();
        task.dueTime = dueTimeText.isEmpty()
            ? QTime()
            : QTime::fromString(dueTimeText, Qt::ISODate);

        task.priority = settings.value("priority", "Medium").toString();
        task.completed = settings.value("completed", false).toBool();

        const QString preset = settings.value("studyDurationPreset", "25").toString();
        int durationSeconds = settings.value("studyDurationSeconds", 25 * 60).toInt();
        if (durationSeconds < 60 || durationSeconds > 180 * 60)
            durationSeconds = 25 * 60;

        task.studyDurationSeconds = durationSeconds;
        task.studyDurationPreset = (preset == "custom")
            ? "custom"
            : QString::number(durationSeconds / 60);

        const int attachmentCount = settings.beginReadArray("Attachments");
        for (int attachmentIndex = 0; attachmentIndex < attachmentCount; ++attachmentIndex) {
            settings.setArrayIndex(attachmentIndex);

            TaskAttachment attachment;
            attachment.id = settings.value("id").toString();
            attachment.fileName = settings.value("fileName").toString();
            attachment.relativePath = settings.value("relativePath").toString();

            if (!attachment.id.isEmpty() &&
                !attachment.fileName.isEmpty() &&
                !attachment.relativePath.isEmpty()) {
                task.attachments.append(attachment);
            }
        }
        settings.endArray();

        if (!task.id.isEmpty()) {
            m_tasks.append(task);
        }
    }

    settings.endArray();
}

bool TaskStore::save() const
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
        settings.setValue("dueTime", task.dueTime.isValid()
                             ? task.dueTime.toString(Qt::ISODate)
                             : QString());
        settings.setValue("priority", task.priority);
        settings.setValue("completed", task.completed);
        settings.setValue("studyDurationPreset", task.studyDurationPreset);
        settings.setValue("studyDurationSeconds", task.studyDurationSeconds);

        settings.beginWriteArray("Attachments", task.attachments.size());
        for (int attachmentIndex = 0; attachmentIndex < task.attachments.size(); ++attachmentIndex) {
            settings.setArrayIndex(attachmentIndex);
            const TaskAttachment &attachment = task.attachments[attachmentIndex];
            settings.setValue("id", attachment.id);
            settings.setValue("fileName", attachment.fileName);
            settings.setValue("relativePath", attachment.relativePath);
        }
        settings.endArray();
    }

    settings.endArray();
    settings.sync();
    return settings.status() == QSettings::NoError;
}
