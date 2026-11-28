#include "quizattemptstore.h"

#include <QSettings>

QuizAttemptStore::QuizAttemptStore()
{
    load();
}

QVector<QuizAttempt> QuizAttemptStore::attempts() const
{
    return m_attempts;
}

QVector<QuizAttempt> QuizAttemptStore::attemptsForQuiz(const QString &quizId) const
{
    QVector<QuizAttempt> result;
    for (const QuizAttempt &attempt : m_attempts) {
        if (attempt.quizId == quizId)
            result.append(attempt);
    }
    return result;
}

QVector<QuizAttempt> QuizAttemptStore::attemptsForTask(const QString &taskId) const
{
    QVector<QuizAttempt> result;
    for (const QuizAttempt &attempt : m_attempts) {
        if (attempt.taskId == taskId)
            result.append(attempt);
    }
    return result;
}
bool QuizAttemptStore::addAttempt(const QuizAttempt &attempt)
{
    if (!isValid(attempt))
        return false;

    // Prevent duplicate attempt IDs.
    for (const QuizAttempt &existing : m_attempts) {
        if (existing.id == attempt.id)
            return false;
    }

    m_attempts.append(attempt);
    if (save())
        return true;

    m_attempts.removeLast();
    return false;
}

bool QuizAttemptStore::updateAttempt(const QuizAttempt &attempt)
{
    if (!isValid(attempt))
        return false;

    for (int i = 0; i < m_attempts.size(); ++i) {
        if (m_attempts[i].id == attempt.id) {
            const QuizAttempt previousAttempt = m_attempts[i];
            m_attempts[i] = attempt;
            if (save())
                return true;

            m_attempts[i] = previousAttempt;
            return false;
        }
    }
    return false;
}

bool QuizAttemptStore::removeAttempt(const QString &id)
{
    for (int i = 0; i < m_attempts.size(); ++i) {
        if (m_attempts[i].id == id) {
            const QuizAttempt previousAttempt = m_attempts[i];
            m_attempts.removeAt(i);
            if (save())
                return true;

            m_attempts.insert(i, previousAttempt);
            return false;
        }
    }
    return false;
}

bool QuizAttemptStore::removeAttemptsForQuiz(const QString &quizId)
{
    bool anythingRemoved = false;
    for (int i = m_attempts.size() - 1; i >= 0; --i) {
        if (m_attempts[i].quizId == quizId) {
            m_attempts.removeAt(i);
            anythingRemoved = true;
        }
    }
    if (!anythingRemoved)
        return true;
    return save();
}

bool QuizAttemptStore::removeAttemptsForTask(const QString &taskId)
{
    bool anythingRemoved = false;
    for (int i = m_attempts.size() - 1; i >= 0; --i) {
        if (m_attempts[i].taskId == taskId) {
            m_attempts.removeAt(i);
            anythingRemoved = true;
        }
    }
    if (!anythingRemoved)
        return true;
    return save();
}
void QuizAttemptStore::load()
{
    m_attempts.clear();

    QSettings settings("FocusForge", "FocusForge");
    const int count = settings.beginReadArray("QuizAttempts");

    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);

        QuizAttempt attempt;
        attempt.id = settings.value("id").toString();
        attempt.quizId = settings.value("quizId").toString();
        attempt.taskId = settings.value("taskId").toString();
        attempt.startTime = QDateTime::fromString(settings.value("startTime").toString(), Qt::ISODate);
        attempt.endTime = QDateTime::fromString(settings.value("endTime").toString(), Qt::ISODate);
        attempt.correctAnswerCount = settings.value("correctAnswerCount", 0).toInt();
        attempt.totalQuestionCount = settings.value("totalQuestionCount", 0).toInt();

        const int selectionCount = settings.beginReadArray("SelectedOptionIndexes");
        for (int selectionIndex = 0; selectionIndex < selectionCount; ++selectionIndex) {
            settings.setArrayIndex(selectionIndex);
            attempt.selectedOptionIndexes.append(settings.value("index").toInt());
        }
        settings.endArray();

        if (isValid(attempt))
            m_attempts.append(attempt);
    }

    settings.endArray();
}
bool QuizAttemptStore::save() const
{
    QSettings settings("FocusForge", "FocusForge");
    settings.beginWriteArray("QuizAttempts", m_attempts.size());

    for (int i = 0; i < m_attempts.size(); ++i) {
        settings.setArrayIndex(i);
        const QuizAttempt &attempt = m_attempts[i];

        settings.setValue("id", attempt.id);
        settings.setValue("quizId", attempt.quizId);
        settings.setValue("taskId", attempt.taskId);
        settings.setValue("startTime", attempt.startTime.isValid()
                                         ? attempt.startTime.toString(Qt::ISODate)
                                         : QString());
        settings.setValue("endTime", attempt.endTime.isValid()
                                     ? attempt.endTime.toString(Qt::ISODate)
                                     : QString());
        settings.setValue("correctAnswerCount", attempt.correctAnswerCount);
        settings.setValue("totalQuestionCount", attempt.totalQuestionCount);

        settings.beginWriteArray("SelectedOptionIndexes",
                                 attempt.selectedOptionIndexes.size());
        for (int selectionIndex = 0;
             selectionIndex < attempt.selectedOptionIndexes.size();
             ++selectionIndex) {
            settings.setArrayIndex(selectionIndex);
            settings.setValue("index", attempt.selectedOptionIndexes[selectionIndex]);
        }
        settings.endArray();
    }

    settings.endArray();
    settings.sync();

    if (settings.status() != QSettings::NoError) {
        qWarning("Failed to save quiz attempts to settings.");
        return false;
    }

    return true;
}

bool QuizAttemptStore::isValid(const QuizAttempt &attempt) const
{
    if (attempt.id.isEmpty() || attempt.quizId.isEmpty() || attempt.taskId.isEmpty())
        return false;
    if (!attempt.startTime.isValid())
        return false;
    if (attempt.totalQuestionCount < 0)
        return false;
    if (attempt.correctAnswerCount < 0
        || attempt.correctAnswerCount > attempt.totalQuestionCount)
        return false;
    // Selected option indexes must match the question count and be in range [0, 3].
    if (static_cast<int>(attempt.selectedOptionIndexes.size())
        != attempt.totalQuestionCount)
        return false;
    for (int index : attempt.selectedOptionIndexes) {
        if (index < 0 || index > 3)
            return false;
    }

    return true;
}



