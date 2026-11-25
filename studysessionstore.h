#ifndef STUDYSESSIONSTORE_H
#define STUDYSESSIONSTORE_H

#include "studysession.h"

#include <QVector>

class StudySessionStore
{
public:
    StudySessionStore();

    QVector<StudySession> sessions() const;
    void addSession(const StudySession &session);

private:
    QVector<StudySession> m_sessions;

    void load();
    void save() const;
};

#endif
