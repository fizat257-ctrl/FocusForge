#include "studysessionstore.h"

#include <QSettings>

StudySessionStore::StudySessionStore()
{
    load();
}

QVector<StudySession> StudySessionStore::sessions() const
{
    return m_sessions;
}

void StudySessionStore::addSession(const StudySession &session)
{
    m_sessions.append(session);
    save();
}

void StudySessionStore::load()
{
    m_sessions.clear();

    QSettings settings("FocusForge", "FocusForge");
    const int count = settings.beginReadArray("StudySessions");

    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);

        StudySession session;
        session.id = settings.value("id").toString();
        session.startTime = QDateTime::fromString(settings.value("startTime").toString(), Qt::ISODate);
        session.endTime = QDateTime::fromString(settings.value("endTime").toString(), Qt::ISODate);
        session.plannedSeconds = settings.value("plannedSeconds", 25 * 60).toInt();
        session.actualSeconds = settings.value("actualSeconds", 25 * 60).toInt();
        session.subjectId = settings.value("subjectId").toString();
        session.subjectName = settings.value("subjectName", "Uncategorized").toString();
        session.taskId = settings.value("taskId").toString();
        session.taskTitle = settings.value("taskTitle").toString();

        if (!session.id.isEmpty() && session.actualSeconds > 0)
            m_sessions.append(session);
    }

    settings.endArray();
}

void StudySessionStore::save() const
{
    QSettings settings("FocusForge", "FocusForge");
    settings.beginWriteArray("StudySessions", m_sessions.size());

    for (int i = 0; i < m_sessions.size(); ++i) {
        settings.setArrayIndex(i);
        const StudySession &session = m_sessions[i];

        settings.setValue("id", session.id);
        settings.setValue("startTime", session.startTime.toString(Qt::ISODate));
        settings.setValue("endTime", session.endTime.toString(Qt::ISODate));
        settings.setValue("plannedSeconds", session.plannedSeconds);
        settings.setValue("actualSeconds", session.actualSeconds);
        settings.setValue("subjectId", session.subjectId);
        settings.setValue("subjectName", session.subjectName);
        settings.setValue("taskId", session.taskId);
        settings.setValue("taskTitle", session.taskTitle);
    }

    settings.endArray();
    settings.sync();
}
