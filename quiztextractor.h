#ifndef QUIZTEXTRACTOR_H
#define QUIZTEXTRACTOR_H

#include <QString>

// Structured result of a text-extraction operation. Extracted text is held
// in memory only and is never persisted to QSettings, task metadata, quiz
// stores, logs, or temporary files.
struct QuizExtractionResult
{
    enum class Status
    {
        Success,                   // Text extracted successfully
        ToolNotFound,              // Required external tool (pdftotext) missing
        InvalidInput,              // Validation failed (missing, ext, size, directory)
        ProcessError,              // QProcess failed to start or exited non-zero
        ProcessTimeout,            // Extraction exceeded the timeout
        ScannedPdfOcrUnavailable,  // PDF appears scanned/image-based; OCR not available
        PngOcrUnavailable,         // PNG OCR requested but Tesseract not installed
        OutputTooLarge,            // Extracted text exceeded the output cap
        EmptyOutput,               // Extraction ran but produced no text
    };

    Status status = Status::InvalidInput;
    QString extractedText;   // In-memory extracted text (may be empty)
    QString userMessage;     // Human-readable explanation of the outcome
};

// QuizTextExtractor is a QProcess-based service that extracts text from
// task attachments (PDF and PNG). It is intentionally independent of quiz
// UI and AI API integration. Extracted text is returned in memory only.
class QuizTextExtractor
{
public:
    QuizTextExtractor();

    // Extracts text from a PDF or PNG attachment.
    // The input must be an absolute path inside FocusForge's managed
    // attachment directory. Returns a structured result.
    QuizExtractionResult extract(const QString &filePath);

private:
    QString attachmentRootPath() const;
    bool isInsideAttachmentDirectory(const QString &absolutePath) const;
    bool isSupportedExtension(const QString &filePath) const;
    QString locateTool(const QString &toolName) const;

    QuizExtractionResult extractPdf(const QString &filePath, const QString &toolPath) const;
    QuizExtractionResult extractPng(const QString &filePath, const QString &toolPath) const;
    QuizExtractionResult runProcess(const QString &program,
                                    const QStringList &arguments,
                                    int timeoutMs,
                                    QString &outputText) const;

    static constexpr qint64 kMaxFileSizeBytes = 25 * 1024 * 1024;   // 25 MB
    static constexpr int kProcessTimeoutMs = 30 * 1000;             // 30 s
    static constexpr qint64 kMaxOutputBytes = 2 * 1024 * 1024;      // 2 MB
    static constexpr int kMinPdfTextChars = 20;                     // scanned-PDF heuristic
};

#endif // QUIZTEXTRACTOR_H
