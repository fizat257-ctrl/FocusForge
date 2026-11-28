#ifndef QUIZ_H
#define QUIZ_H

#include <QDateTime>
#include <QString>
#include <QVector>
#include <QStringList>

struct QuizOption
{
    QString id;
    QString text;
};

struct QuizQuestion
{
    QString id;
    QString text;
    QVector<QuizOption> options;
    int correctOptionIndex = -1;
    QString explanation;
};

struct Quiz
{
    QString id;
    QString taskId;
    QString title;
    QString sourceDescription;
    QStringList sourceAttachmentIds;
    QDateTime createdAt;
    QVector<QuizQuestion> questions;
};

#endif // QUIZ_H
