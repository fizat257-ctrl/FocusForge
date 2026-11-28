#include "quizstore.h"

#include <QSettings>

QuizStore::QuizStore()
{
    load();
}

QVector<Quiz> QuizStore::quizzes() const
{
    return m_quizzes;
}

QVector<Quiz> QuizStore::quizzesForTask(const QString &taskId) const
{
    QVector<Quiz> result;
    for (const Quiz &quiz : m_quizzes) {
        if (quiz.taskId == taskId)
            result.append(quiz);
    }
    return result;
}
bool QuizStore::addQuiz(const Quiz &quiz)
{
    if (!isValid(quiz))
        return false;

    // Keep IDs stable and unique: never create a duplicate.
    for (const Quiz &existing : m_quizzes) {
        if (existing.id == quiz.id)
            return false;
    }

    m_quizzes.append(quiz);
    if (save())
        return true;

    m_quizzes.removeLast();
    return false;
}

bool QuizStore::updateQuiz(const Quiz &quiz)
{
    if (!isValid(quiz))
        return false;

    for (int i = 0; i < m_quizzes.size(); ++i) {
        if (m_quizzes[i].id == quiz.id) {
            const Quiz previousQuiz = m_quizzes[i];
            m_quizzes[i] = quiz;
            if (save())
                return true;

            m_quizzes[i] = previousQuiz;
            return false;
        }
    }
    return false;
}

bool QuizStore::removeQuiz(const QString &id)
{
    for (int i = 0; i < m_quizzes.size(); ++i) {
        if (m_quizzes[i].id == id) {
            const Quiz previousQuiz = m_quizzes[i];
            m_quizzes.removeAt(i);
            if (save())
                return true;

            m_quizzes.insert(i, previousQuiz);
            return false;
        }
    }
    return false;
}

bool QuizStore::removeQuizzesForTask(const QString &taskId)
{
    bool anythingRemoved = false;
    // Iterate backwards so indices stay valid after removal.
    for (int i = m_quizzes.size() - 1; i >= 0; --i) {
        if (m_quizzes[i].taskId == taskId) {
            m_quizzes.removeAt(i);
            anythingRemoved = true;
        }
    }
    if (!anythingRemoved)
        return true; // Nothing to do is not an error.
    return save();
}
void QuizStore::load()
{
    m_quizzes.clear();

    QSettings settings("FocusForge", "FocusForge");
    const int count = settings.beginReadArray("Quizzes");

    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);

        Quiz quiz;
        quiz.id = settings.value("id").toString();
        quiz.taskId = settings.value("taskId").toString();
        quiz.title = settings.value("title").toString();
        quiz.sourceDescription = settings.value("sourceDescription").toString();
        quiz.createdAt = QDateTime::fromString(settings.value("createdAt").toString(), Qt::ISODate);

        // Source attachment IDs are metadata only; no binary PDF/PNG content.
        const int attachmentCount = settings.beginReadArray("SourceAttachments");
        for (int attachmentIndex = 0; attachmentIndex < attachmentCount; ++attachmentIndex) {
            settings.setArrayIndex(attachmentIndex);
            const QString attachmentId = settings.value("id").toString();
            if (!attachmentId.isEmpty())
                quiz.sourceAttachmentIds.append(attachmentId);
        }
        settings.endArray();

        const int questionCount = settings.beginReadArray("Questions");
        for (int questionIndex = 0; questionIndex < questionCount; ++questionIndex) {
            settings.setArrayIndex(questionIndex);

            QuizQuestion question;
            question.id = settings.value("id").toString();
            question.text = settings.value("text").toString();
            question.correctOptionIndex = settings.value("correctOptionIndex", -1).toInt();
            question.explanation = settings.value("explanation").toString();

            const int optionCount = settings.beginReadArray("Options");
            for (int optionIndex = 0; optionIndex < optionCount; ++optionIndex) {
                settings.setArrayIndex(optionIndex);
                QuizOption option;
                option.id = settings.value("id").toString();
                option.text = settings.value("text").toString();
                if (!option.text.isEmpty())
                    question.options.append(option);
            }
            settings.endArray();

            // Keep only well-formed questions; skip malformed ones safely.
            if (question.id.isEmpty() || question.text.isEmpty())
                continue;
            if (question.options.size() != 4)
                continue;
            if (question.correctOptionIndex < 0 || question.correctOptionIndex >= 4)
                continue;

            quiz.questions.append(question);
        }

        if (!quiz.id.isEmpty() && !quiz.taskId.isEmpty() && !quiz.title.isEmpty())
            m_quizzes.append(quiz);
    }

    settings.endArray();
}
bool QuizStore::save() const
{
    QSettings settings("FocusForge", "FocusForge");
    settings.beginWriteArray("Quizzes", m_quizzes.size());

    for (int i = 0; i < m_quizzes.size(); ++i) {
        settings.setArrayIndex(i);
        const Quiz &quiz = m_quizzes[i];

        settings.setValue("id", quiz.id);
        settings.setValue("taskId", quiz.taskId);
        settings.setValue("title", quiz.title);
        settings.setValue("sourceDescription", quiz.sourceDescription);
        settings.setValue("createdAt", quiz.createdAt.isValid()
                                         ? quiz.createdAt.toString(Qt::ISODate)
                                         : QString());

        settings.beginWriteArray("SourceAttachments", quiz.sourceAttachmentIds.size());
        for (int attachmentIndex = 0; attachmentIndex < quiz.sourceAttachmentIds.size(); ++attachmentIndex) {
            settings.setArrayIndex(attachmentIndex);
            settings.setValue("id", quiz.sourceAttachmentIds[attachmentIndex]);
        }
        settings.endArray();

        settings.beginWriteArray("Questions", quiz.questions.size());
        for (int questionIndex = 0; questionIndex < quiz.questions.size(); ++questionIndex) {
            settings.setArrayIndex(questionIndex);
            const QuizQuestion &question = quiz.questions[questionIndex];

            settings.setValue("id", question.id);
            settings.setValue("text", question.text);
            settings.setValue("correctOptionIndex", question.correctOptionIndex);
            settings.setValue("explanation", question.explanation);

            settings.beginWriteArray("Options", question.options.size());
            for (int optionIndex = 0; optionIndex < question.options.size(); ++optionIndex) {
                settings.setArrayIndex(optionIndex);
                const QuizOption &option = question.options[optionIndex];
                settings.setValue("id", option.id);
                settings.setValue("text", option.text);
            }
            settings.endArray();
        }
        settings.endArray();
    }

    settings.endArray();
    settings.sync();

    if (settings.status() != QSettings::NoError) {
        qWarning("Failed to save quizzes to settings.");
        return false;
    }

    return true;
}

bool QuizStore::isValid(const Quiz &quiz) const
{
    if (quiz.id.isEmpty() || quiz.taskId.isEmpty() || quiz.title.isEmpty())
        return false;
    if (quiz.questions.isEmpty())
        return false;

    for (const QuizQuestion &question : quiz.questions) {
        if (question.id.isEmpty() || question.text.isEmpty())
            return false;
        if (question.options.size() != 4)
            return false;

        bool allOptionsValid = true;
        for (const QuizOption &option : question.options) {
            if (option.text.isEmpty()) {
                allOptionsValid = false;
                break;
            }
        }
        if (!allOptionsValid)
            return false;

        if (question.correctOptionIndex < 0 || question.correctOptionIndex >= 4)
            return false;
    }

    return true;
}



