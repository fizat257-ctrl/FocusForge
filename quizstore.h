#ifndef QUIZSTORE_H
#define QUIZSTORE_H

#include "quiz.h"

#include <QVector>

class QuizStore
{
public:
    QuizStore();

    QVector<Quiz> quizzes() const;
    QVector<Quiz> quizzesForTask(const QString &taskId) const;

    bool addQuiz(const Quiz &quiz);
    bool updateQuiz(const Quiz &quiz);
    bool removeQuiz(const QString &id);
    bool removeQuizzesForTask(const QString &taskId);

private:
    QVector<Quiz> m_quizzes;

    void load();
    bool save() const;
    bool isValid(const Quiz &quiz) const;
};

#endif // QUIZSTORE_H
