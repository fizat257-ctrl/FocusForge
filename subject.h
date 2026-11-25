#ifndef SUBJECT_H
#define SUBJECT_H

#include <QString>

struct Subject
{
    QString id;
    QString name;
    QString code;
    QString teacher;
    QString description;
    QString color = "#6C63FF";
};

#endif
