#ifndef QUIZGENERATOR_H
#define QUIZGENERATOR_H

#include "quizgeneratorresult.h"

#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QStringList>
#include <QTimer>
#include <QVector>

// Asynchronous AI quiz-generation service using the OpenAI Responses API
// with Structured Outputs (strict JSON Schema). Uses QNetworkAccessManager
// with signals/slots — never blocks the UI thread.
class QuizGenerator : public QObject
{
    Q_OBJECT

public:
    explicit QuizGenerator(QObject *parent = nullptr);
    ~QuizGenerator() override;

    // Starts an asynchronous generation request.
    // questionCount must be between 1 and 20 inclusive.
    void generateQuiz(const QString &extractedText,
                      int questionCount,
                      const QString &taskId,
                      const QStringList &sourceAttachmentIds);

    // Cancels any in-flight request.
    void cancel();
    bool isBusy() const;

signals:
    void finished(QuizGenerationResult result);

private:
    void startRequest(const QString &extractedText, int questionCount);

    void handleNetworkError();
    void handleFinished();
    void handleTimeout();
    void handleReadyRead();

    void clearApiKeySecurely();

    QuizGenerationResult parseResponse(const QByteArray &data) const;
    QuizGenerationResult buildResultFromQuizJson(
        const QJsonDocument &doc) const;

    QByteArray buildRequestBody(const QString &extractedText,
                                int questionCount) const;

    QString retrieveApiKey() const;

    QNetworkAccessManager m_networkAccessManager;
    QNetworkReply *m_currentReply = nullptr;

    QTimer m_timeoutTimer;
    QByteArray m_responseBuffer;

    bool m_responseEmitted = false;

    QString m_apiKey;
    QString m_taskId;
    QStringList m_sourceAttachmentIds;

    static constexpr int kTimeoutMs = 90 * 1000;
    static constexpr qint64 kMaxResponseBytes = 2 * 1024 * 1024;
    static constexpr int kMaxInputChars = 12000;

    static constexpr int kMinQuestions = 1;
    static constexpr int kMaxQuestions = 20;

    static constexpr const char *kApiEndpoint =
        "https://api.openai.com/v1/responses";

    static constexpr const char *kModel = "gpt-4o-mini";
};

#endif // QUIZGENERATOR_H