#include "quizgenerator.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkRequest>
#include <QUrl>

#include "windowscredentialmanager.h"

QuizGenerator::QuizGenerator(QObject *parent)
    : QObject(parent)
{
    m_timeoutTimer.setSingleShot(true);
    connect(&m_timeoutTimer, &QTimer::timeout, this, &QuizGenerator::handleTimeout);
}

QuizGenerator::~QuizGenerator()
{
    cancel();
}

void QuizGenerator::generateQuiz(const QString &extractedText,
                                 int questionCount,
                                 const QString &taskId,
                                 const QStringList &sourceAttachmentIds)
{
    if (isBusy()) {
        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::NetworkError;
        result.errorMessage = "A generation request is already in progress.";
        emit finished(result);
        return;
    }

    m_apiKey = retrieveApiKey();
    if (m_apiKey.isEmpty()) {
        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::ApiKeyMissing;
        result.errorMessage =
            "OpenAI API key not found. Please save it in the application settings first.";
        emit finished(result);
        return;
    }

    if (questionCount < kMinQuestions || questionCount > kMaxQuestions) {
        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::ValidationError;
        result.errorMessage =
            QString("Question count must be between %1 and %2.")
                .arg(kMinQuestions)
                .arg(kMaxQuestions);
        emit finished(result);
        return;
    }

    if (extractedText.trimmed().isEmpty()) {
        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::ValidationError;
        result.errorMessage = "No text provided for quiz generation.";
        emit finished(result);
        return;
    }

    m_taskId = taskId;
    m_sourceAttachmentIds = sourceAttachmentIds;

    startRequest(extractedText, questionCount);
}

void QuizGenerator::cancel()
{
    m_timeoutTimer.stop();
    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    m_responseEmitted = true;
    clearApiKeySecurely();
}

bool QuizGenerator::isBusy() const
{
    return m_currentReply != nullptr;
}

void QuizGenerator::startRequest(const QString &extractedText, int questionCount)
{
    QByteArray body = buildRequestBody(extractedText, questionCount);

    QNetworkRequest request(QUrl(QString::fromLatin1(kApiEndpoint)));
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/json"));

    // Construct Authorization header as QByteArray to avoid intermediate QString
    QByteArray authHeader = QByteArrayLiteral("Bearer ") + m_apiKey.toUtf8();
    request.setRawHeader(QByteArrayLiteral("Authorization"), authHeader);

    // Best-effort clear of temporary QByteArray containing the API key
    authHeader.clear();

    m_responseBuffer.clear();
    m_responseEmitted = false;

    m_currentReply = m_networkAccessManager.post(request, body);

    connect(m_currentReply, &QNetworkReply::finished, this, &QuizGenerator::handleFinished);
    connect(m_currentReply, &QNetworkReply::errorOccurred, this,
            &QuizGenerator::handleNetworkError);
    connect(m_currentReply, &QNetworkReply::readyRead, this,
            &QuizGenerator::handleReadyRead);

    m_timeoutTimer.start(kTimeoutMs);
}

void QuizGenerator::handleNetworkError()
{
    m_timeoutTimer.stop();
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply || reply != m_currentReply)
        return;

    QString errorString = reply->errorString();
    m_currentReply = nullptr;
    reply->deleteLater();

    // Prevent double emission if readyRead() already handled an over-size response.
    if (m_responseEmitted) {
        return;
    }

    m_responseEmitted = true;

    QuizGenerationResult result;
    result.status = QuizGenerationResult::Status::NetworkError;
    result.errorMessage = QString("Network error: %1").arg(errorString);
    emit finished(result);
}

void QuizGenerator::handleFinished()
{
    m_timeoutTimer.stop();

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply || reply != m_currentReply) {
        if (reply)
            reply->deleteLater();
        return;
    }

    // Capture status code before the reply is deleted.
    QVariant statusCodeVar = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    int statusCode = statusCodeVar.isValid() ? statusCodeVar.toInt() : 0;

    // Read any remaining bytes into the bounded buffer.
    m_responseBuffer += reply->readAll();

    m_currentReply = nullptr;
    reply->deleteLater();

    // Prevent double emission if readyRead() already handled an over-size response.
    if (m_responseEmitted) {
        return;
    }

    // Enforce the 2 MB response limit on the complete body.
    if (m_responseBuffer.size() > kMaxResponseBytes) {
        m_responseEmitted = true;

        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::ResponseTooLarge;
        result.errorMessage =
            QString("Response exceeded the %1 MB limit.")
                .arg(kMaxResponseBytes / (1024 * 1024));
        emit finished(result);
        return;
    }

    clearApiKeySecurely();

    if (statusCode == 200) {
        QuizGenerationResult result = parseResponse(m_responseBuffer);
        emit finished(result);
    } else {
        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::HttpError;
        result.errorMessage =
            QString("HTTP %1: %2").arg(statusCode)
                .arg(QString::fromUtf8(m_responseBuffer.constData(), m_responseBuffer.size()));
        emit finished(result);
    }
}

void QuizGenerator::handleTimeout()
{
    if (m_responseEmitted)
        return;

    if (m_currentReply) {
        m_currentReply->abort();
        m_currentReply->deleteLater();
        m_currentReply = nullptr;
    }

    m_responseEmitted = true;

    QuizGenerationResult result;
    result.status = QuizGenerationResult::Status::Timeout;
    result.errorMessage = "Request timed out after 90 seconds.";
    emit finished(result);
}

void QuizGenerator::handleReadyRead()
{
    if (m_responseEmitted || !m_currentReply)
        return;

    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());
    if (!reply || reply != m_currentReply)
        return;

    m_responseBuffer += reply->readAll();

    if (m_responseBuffer.size() > kMaxResponseBytes) {
        m_timeoutTimer.stop();
        m_responseEmitted = true;

        m_currentReply = nullptr;
        reply->abort();
        reply->deleteLater();

        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::ResponseTooLarge;
        result.errorMessage =
            QString("Response exceeded the %1 MB limit.")
                .arg(kMaxResponseBytes / (1024 * 1024));
        emit finished(result);
    }
}
void QuizGenerator::clearApiKeySecurely()
{
    if (!m_apiKey.isEmpty()) {
        m_apiKey.fill(QLatin1Char('\0'));
        m_apiKey.clear();
    }
}

QuizGenerationResult QuizGenerator::parseResponse(const QByteArray &data) const


QuizGenerationResult QuizGenerator::parseResponse(const QByteArray &data) const
{
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull() || doc.isEmpty()) {
        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::InvalidJson;
        result.errorMessage = "Invalid JSON response from API.";
        return result;
    }

    QJsonObject root = doc.object();

    if (root.contains("error")) {
        QJsonObject errorObj = root.value("error").toObject();
        QString message = errorObj.value("message").toString();
        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::ApiError;
        result.errorMessage = QString("API error: %1").arg(message);
        return result;
    }

    QString extractedText;
    QJsonArray outputArray = root.value("output").toArray();
    for (const QJsonValue &outputValue : outputArray) {
        if (!outputValue.isObject())
            continue;
        QJsonObject outputObj = outputValue.toObject();

        if (outputObj.value("type").toString() == "refusal") {
            QuizGenerationResult result;
            result.status = QuizGenerationResult::Status::Refusal;
            result.errorMessage = "The model refused to generate a quiz.";
            return result;
        }

        if (outputObj.value("type").toString() != "message")
            continue;

        QJsonArray contentArray = outputObj.value("content").toArray();
        for (const QJsonValue &contentValue : contentArray) {
            if (!contentValue.isObject())
                continue;
            QJsonObject contentObj = contentValue.toObject();
            if (contentObj.value("type").toString() == "output_text")
                extractedText += contentObj.value("text").toString();
        }
    }

    if (extractedText.trimmed().isEmpty()) {
        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::IncompleteResponse;
        result.errorMessage = "No text content in API response.";
        return result;
    }

    QJsonDocument quizDoc = QJsonDocument::fromJson(extractedText.toUtf8());
    if (quizDoc.isNull() || quizDoc.isEmpty() || !quizDoc.isObject()) {
        QuizGenerationResult result;
        result.status = QuizGenerationResult::Status::InvalidJson;
        result.errorMessage = "Model did not return valid JSON.";
        return result;
    }

    return buildResultFromQuizJson(quizDoc);
}


QuizGenerationResult QuizGenerator::buildResultFromQuizJson(const QJsonDocument &doc) const
{
    QuizGenerationResult result;
    QJsonObject root = doc.object();

    QJsonArray questionsArray = root.value("questions").toArray();
    if (questionsArray.isEmpty()) {
        result.status = QuizGenerationResult::Status::ValidationError;
        result.errorMessage = "Response contains no questions.";
        return result;
    }

    QVector<QuizQuestion> questions;
    for (const QJsonValue &qValue : questionsArray) {
        if (!qValue.isObject())
            continue;
        QJsonObject qObj = qValue.toObject();

        QString questionText = qObj.value("question").toString().trimmed();
        if (questionText.isEmpty())
            continue;

        QJsonArray optionsArray = qObj.value("options").toArray();
        if (optionsArray.size() != 4)
            continue;

        QStringList options;
        bool allOptionsValid = true;
        for (const QJsonValue &optValue : optionsArray) {
            QString opt = optValue.toString().trimmed();
            if (opt.isEmpty()) {
                allOptionsValid = false;
                break;
            }
            options.append(opt);
        }
        if (!allOptionsValid)
            continue;

        int correctIndex = qObj.value("correctOptionIndex").toInt();
        if (correctIndex < 0 || correctIndex > 3)
            continue;

        QString explanation = qObj.value("explanation").toString();

        QuizQuestion question;
        question.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        question.text = questionText;
        question.options.clear();
        for (const QString &opt : options) {
            QuizOption option;
            option.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
            option.text = opt;
            question.options.append(option);
        }
        question.correctOptionIndex = correctIndex;
        question.explanation = explanation;

        questions.append(question);
    }

    if (questions.isEmpty()) {
        result.status = QuizGenerationResult::Status::ValidationError;
        result.errorMessage = "No valid questions could be constructed from the response.";
        return result;
    }

    result.status = QuizGenerationResult::Status::Success;
    result.questions = questions;
    result.errorMessage = QString("Generated %1 question(s).").arg(questions.size());
    return result;
}

QByteArray QuizGenerator::buildRequestBody(const QString &extractedText,
                                           int questionCount) const
{
    QString inputText = extractedText;
    if (inputText.size() > kMaxInputChars)
        inputText.truncate(kMaxInputChars);

    QJsonObject questionSchema;
    questionSchema["type"] = "object";
    questionSchema["additionalProperties"] = false;
    QJsonArray questionRequired;
    questionRequired << "question" << "options" << "correctOptionIndex" << "explanation";
    questionSchema["required"] = questionRequired;

    QJsonObject optionsSchema;
    optionsSchema["type"] = "array";
    optionsSchema["items"] = QJsonObject{{"type", "string"}};
    optionsSchema["minItems"] = 4;
    optionsSchema["maxItems"] = 4;

    QJsonObject properties;
    properties["question"] = QJsonObject{{"type", "string"}};
    properties["options"] = optionsSchema;
    QJsonObject correctIndexSchema;
    correctIndexSchema["type"] = "integer";
    correctIndexSchema["minimum"] = 0;
    correctIndexSchema["maximum"] = 3;
    properties["correctOptionIndex"] = correctIndexSchema;
    properties["explanation"] = QJsonObject{{"type", "string"}};
    questionSchema["properties"] = properties;

    QJsonObject questionsProp;
    questionsProp["type"] = "array";
    questionsProp["items"] = questionSchema;
    questionsProp["minItems"] = kMinQuestions;
    questionsProp["maxItems"] = kMaxQuestions;

    QJsonObject schema;
    schema["type"] = "object";
    schema["additionalProperties"] = false;
    QJsonArray rootRequired;
    rootRequired << "questions";
    schema["required"] = rootRequired;
    QJsonObject rootProperties;
    rootProperties["questions"] = questionsProp;
    schema["properties"] = rootProperties;

    QJsonObject jsonSchemaObj;
    jsonSchemaObj["name"] = "mcq_quiz";
    jsonSchemaObj["strict"] = true;
    jsonSchemaObj["schema"] = schema;

    QJsonObject textFormat;
    textFormat["type"] = "json_schema";
    textFormat["json_schema"] = jsonSchemaObj;

    QJsonArray inputArray;
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] =
        "You are an expert quiz generator. Generate multiple-choice questions based on the "
        "provided text. Each question must have exactly four options, a correct option index "
        "(0-3), and an explanation.";
    inputArray.append(systemMsg);

    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] =
        QString("Generate %1 multiple-choice questions from the following text:\n\n%2")
            .arg(questionCount)
            .arg(inputText);
    inputArray.append(userMsg);

    QJsonObject root;
    root["model"] = QString::fromLatin1(kModel);
    root["input"] = inputArray;

    QJsonObject textObj;
    textObj["format"] = textFormat;
    root["text"] = textObj;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Compact);
}

QString QuizGenerator::retrieveApiKey() const
{
    return WindowsCredentialManager::readCredential(
        QString::fromLatin1(WindowsCredentialManager::OpenAITarget),
        QString::fromLatin1(WindowsCredentialManager::OpenAIUser));
}

