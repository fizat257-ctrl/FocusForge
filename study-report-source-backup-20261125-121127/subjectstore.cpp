#include "subjectstore.h"

#include <QSettings>

SubjectStore::SubjectStore()
{
    load();
}

QVector<Subject> SubjectStore::subjects() const
{
    return m_subjects;
}

void SubjectStore::addSubject(const Subject &subject)
{
    m_subjects.append(subject);
    save();
}

void SubjectStore::updateSubject(const Subject &subject)
{
    for (int i = 0; i < m_subjects.size(); ++i) {
        if (m_subjects[i].id == subject.id) {
            m_subjects[i] = subject;
            save();
            return;
        }
    }
}

void SubjectStore::removeSubject(const QString &id)
{
    for (int i = 0; i < m_subjects.size(); ++i) {
        if (m_subjects[i].id == id) {
            m_subjects.removeAt(i);
            save();
            return;
        }
    }
}

void SubjectStore::load()
{
    m_subjects.clear();

    QSettings settings("FocusForge", "FocusForge");
    const int count = settings.beginReadArray("Subjects");

    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);

        Subject subject;
        subject.id = settings.value("id").toString();
        subject.name = settings.value("name").toString();
        subject.code = settings.value("code").toString();
        subject.teacher = settings.value("teacher").toString();
        subject.description = settings.value("description").toString();
        subject.color = settings.value("color", "#6C63FF").toString();

        if (!subject.id.isEmpty())
            m_subjects.append(subject);
    }

    settings.endArray();
}

void SubjectStore::save() const
{
    QSettings settings("FocusForge", "FocusForge");
    settings.beginWriteArray("Subjects", m_subjects.size());

    for (int i = 0; i < m_subjects.size(); ++i) {
        settings.setArrayIndex(i);
        const Subject &subject = m_subjects[i];

        settings.setValue("id", subject.id);
        settings.setValue("name", subject.name);
        settings.setValue("code", subject.code);
        settings.setValue("teacher", subject.teacher);
        settings.setValue("description", subject.description);
        settings.setValue("color", subject.color);
    }

    settings.endArray();
    settings.sync();
}
