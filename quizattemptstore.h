#ifndef QUIZATTEMPTSTORE_H
#define QUIZATTEMPTSTORE_H

#include "quizattempt.h"

#include <QVector>

class QuizAttemptStore
{
public:
    QuizAttemptStore();

    QVector<QuizAttempt> attempts() const;
    QVector<QuizAttempt> attemptsForQuiz(const QString &quizId) const;
    QVector<QuizAttempt> attemptsForTask(const QString &taskId) const;

    bool addAttempt(const QuizAttempt &attempt);
    bool updateAttempt(const QuizAttempt &attempt);
    bool removeAttempt(const QString &id);
    bool removeAttemptsForQuiz(const QString &quizId);
    bool removeAttemptsForTask(const QString &taskId);

private:
    QVector<QuizAttempt> m_attempts;

    void load();
    bool save() const;
    bool isValid(const QuizAttempt &attempt) const;
};

#endif // QUIZATTEMPTSTORE_H
