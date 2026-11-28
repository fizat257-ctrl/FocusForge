#ifndef QUIZATTEMPT_H
#define QUIZATTEMPT_H

#include <QDateTime>
#include <QString>
#include <QVector>

struct QuizAttempt
{
    QString id;
    QString quizId;
    QString taskId;
    QDateTime startTime;
    QDateTime endTime;
    QVector<int> selectedOptionIndexes;
    int correctAnswerCount = 0;
    int totalQuestionCount = 0;
};

#endif // QUIZATTEMPT_H
