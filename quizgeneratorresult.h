#ifndef QUIZGENERATORRESULT_H
#define QUIZGENERATORRESULT_H

#include "quiz.h"

#include <QVector>
#include <QString>

// Structured result of an AI quiz-generation operation.
// Validated QuizQuestion objects are returned to the future UI layer;
// the service never saves quizzes or API keys itself.
struct QuizGenerationResult
{
    enum class Status
    {
        Success,              // Questions generated and validated
        ApiKeyMissing,        // No OpenAI API key in Windows Credential Manager
        NetworkError,         // QNetworkReply network error
        HttpError,            // Non-2xx HTTP status
        ApiError,             // OpenAI API returned an error in the body
        Timeout,              // Request exceeded the 90-second timeout
        Cancelled,            // Request was cancelled by the caller
        InvalidJson,          // Response body was not valid JSON
        ValidationError,      // Input parameters failed validation
        Refusal,              // Model refused to generate
        IncompleteResponse,   // Response missing expected content
        ResponseTooLarge      // Response exceeded the 2 MB limit
    };

    Status status = Status::ApiKeyMissing;
    QVector<QuizQuestion> questions;
    QString errorMessage;
};

#endif // QUIZGENERATORRESULT_H
