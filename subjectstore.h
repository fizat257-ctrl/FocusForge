#ifndef SUBJECTSTORE_H
#define SUBJECTSTORE_H

#include "subject.h"

#include <QVector>

class SubjectStore
{
public:
    SubjectStore();

    QVector<Subject> subjects() const;
    void addSubject(const Subject &subject);
    void updateSubject(const Subject &subject);
    void removeSubject(const QString &id);

private:
    QVector<Subject> m_subjects;

    void load();
    void save() const;
};

#endif
