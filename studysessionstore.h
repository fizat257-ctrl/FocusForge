#ifndef STUDYSESSIONSTORE_H
#define STUDYSESSIONSTORE_H

#include "studysession.h"

#include <QVector>

class StudySessionStore
{
public:
    StudySessionStore();

    QVector<StudySession> sessions() const;
    bool addSession(const StudySession &session);

private:
    QVector<StudySession> m_sessions;

    void load();
    bool save() const;
};

#endif
