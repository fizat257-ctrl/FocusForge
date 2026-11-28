#include "quiztextractor.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

QuizTextExtractor::QuizTextExtractor()
{
}

QString QuizTextExtractor::attachmentRootPath() const
{
    const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (appDataPath.isEmpty())
        return QString();
    return QDir::cleanPath(appDataPath + QLatin1String("/attachments"));
}

bool QuizTextExtractor::isInsideAttachmentDirectory(const QString &absolutePath) const
{
    const QString root = attachmentRootPath();
    if (root.isEmpty() || absolutePath.isEmpty())
        return false;

    const QString absolute = QDir::cleanPath(absolutePath);
    const QString rootPrefix = QDir::cleanPath(root) + QLatin1Char('/');
    return absolute == root || absolute.startsWith(rootPrefix, Qt::CaseInsensitive);
}

bool QuizTextExtractor::isSupportedExtension(const QString &filePath) const
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    return suffix == QLatin1String("pdf") || suffix == QLatin1String("png");
}

QString QuizTextExtractor::locateTool(const QString &toolName) const
{
    // 1. Prefer lookup via QStandardPaths (respects the system PATH).
    const QString found = QStandardPaths::findExecutable(toolName);
    if (!found.isEmpty())
        return found;

    // 2. Verified fallback for the Git-for-Windows / Poppler toolchain on
    //    this development machine. Never constructs a shell command.
    if (toolName == QLatin1String("pdftotext"))
        return QStringLiteral("C:/Program Files/Git/ucrt64/bin/pdftotext.exe");

    return QString();
}

QuizExtractionResult QuizTextExtractor::extract(const QString &filePath)
{
    QuizExtractionResult result;

    // ---------- Input validation ----------
    if (filePath.isEmpty()) {
        result.status = QuizExtractionResult::Status::InvalidInput;
        result.userMessage = "No file path provided.";
        return result;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        result.status = QuizExtractionResult::Status::InvalidInput;
        result.userMessage = "The attachment file does not exist or is not a regular file.";
        return result;
    }

    if (!isSupportedExtension(filePath)) {
        result.status = QuizExtractionResult::Status::InvalidInput;
        result.userMessage = "Only PDF and PNG attachments are supported.";
        return result;
    }

    if (fileInfo.size() > kMaxFileSizeBytes) {
        result.status = QuizExtractionResult::Status::InvalidInput;
        result.userMessage = QString("Attachment exceeds the %1 MB size limit.")
                                 .arg(kMaxFileSizeBytes / (1024 * 1024));
        return result;
    }

    const QString absolutePath = QDir::cleanPath(filePath);
    if (!isInsideAttachmentDirectory(absolutePath)) {
        result.status = QuizExtractionResult::Status::InvalidInput;
        result.userMessage = "The file is outside FocusForge's managed attachment directory.";
        return result;
    }

    const QString suffix = fileInfo.suffix().toLower();
    if (suffix == QLatin1String("pdf")) {
        const QString toolPath = locateTool(QStringLiteral("pdftotext"));
        if (toolPath.isEmpty()) {
            result.status = QuizExtractionResult::Status::ToolNotFound;
            result.userMessage = "PDF text extraction is unavailable: pdftotext was not found.";
            return result;
        }
        return extractPdf(absolutePath, toolPath);
    }

    // PNG
    {
        const QString toolPath = locateTool(QStringLiteral("tesseract"));
        if (toolPath.isEmpty()) {
            result.status = QuizExtractionResult::Status::PngOcrUnavailable;
            result.userMessage = "PNG OCR is unavailable: Tesseract OCR is not installed.";
            return result;
        }
        return extractPng(absolutePath, toolPath);
    }

    result.status = QuizExtractionResult::Status::InvalidInput;
    result.userMessage = "Unsupported file type.";
    return result;
}

QuizExtractionResult QuizTextExtractor::extractPdf(const QString &filePath,
                                                   const QString &toolPath) const
{
    QuizExtractionResult result;

    // pdftotext <file> -   (dash writes extracted text to stdout)
    QString outputText;
    const QuizExtractionResult runResult =
        runProcess(toolPath, QStringList() << filePath << QStringLiteral("-"),
                   kProcessTimeoutMs, outputText);
    const QuizExtractionResult::Status runStatus = runResult.status;

    switch (runStatus) {
    case QuizExtractionResult::Status::ProcessTimeout:
        result.status = QuizExtractionResult::Status::ProcessTimeout;
        result.userMessage = "PDF extraction timed out after 30 seconds.";
        return result;
    case QuizExtractionResult::Status::ProcessError:
        result.status = QuizExtractionResult::Status::ProcessError;
        result.userMessage = "PDF extraction process failed.";
        return result;
    default:
        break;
    }

    // Apply output cap to avoid unbounded memory growth.
    if (outputText.size() > kMaxOutputBytes) {
        outputText.truncate(kMaxOutputBytes);
        result.status = QuizExtractionResult::Status::OutputTooLarge;
        result.userMessage = "Extracted PDF text was truncated to 2 MB.";
        result.extractedText = outputText;
        return result;
    }

    result.extractedText = outputText;

    // Scanned-PDF heuristic: empty or very short output suggests an
    // image-based PDF that pdftotext cannot read.
    if (outputText.trimmed().size() < kMinPdfTextChars) {
        result.status = QuizExtractionResult::Status::ScannedPdfOcrUnavailable;
        result.userMessage = "This PDF appears to be scanned or image-based. "
                             "OCR is not available in this version.";
        return result;
    }

    if (outputText.isEmpty()) {
        result.status = QuizExtractionResult::Status::EmptyOutput;
        result.userMessage = "No text could be extracted from the PDF.";
        return result;
    }

    result.status = QuizExtractionResult::Status::Success;
    result.userMessage = QString("Extracted %1 characters from PDF.")
                             .arg(outputText.size());
    return result;
}
QuizExtractionResult QuizTextExtractor::extractPng(const QString &filePath,
                                                   const QString &toolPath) const
{
    QuizExtractionResult result;

    // tesseract <file> stdout   (writes recognized text to stdout)
    QString outputText;
    const QuizExtractionResult runResult =
        runProcess(toolPath, QStringList() << filePath << QStringLiteral("stdout"),
                   kProcessTimeoutMs, outputText);
    const QuizExtractionResult::Status runStatus = runResult.status;

    switch (runStatus) {
    case QuizExtractionResult::Status::ProcessTimeout:
        result.status = QuizExtractionResult::Status::ProcessTimeout;
        result.userMessage = "PNG OCR timed out after 30 seconds.";
        return result;
    case QuizExtractionResult::Status::ProcessError:
        result.status = QuizExtractionResult::Status::ProcessError;
        result.userMessage = "PNG OCR process failed.";
        return result;
    default:
        break;
    }

    if (outputText.size() > kMaxOutputBytes) {
        outputText.truncate(kMaxOutputBytes);
        result.status = QuizExtractionResult::Status::OutputTooLarge;
        result.userMessage = "Extracted PNG text was truncated to 2 MB.";
        result.extractedText = outputText;
        return result;
    }

    result.extractedText = outputText;

    if (outputText.trimmed().isEmpty()) {
        result.status = QuizExtractionResult::Status::EmptyOutput;
        result.userMessage = "No text could be recognized from the PNG image.";
        return result;
    }

    result.status = QuizExtractionResult::Status::Success;
    result.userMessage = QString("Recognized %1 characters from PNG.")
                             .arg(outputText.size());
    return result;
}

QuizExtractionResult QuizTextExtractor::runProcess(const QString &program,
                                                   const QStringList &arguments,
                                                   int timeoutMs,
                                                   QString &outputText) const
{
    QProcess process;
    process.setProgram(program);
    process.setArguments(arguments);   // arguments passed separately; no shell command
    process.start();

    if (!process.waitForStarted()) {
        return QuizExtractionResult{QuizExtractionResult::Status::ProcessError};
    }

    const bool finished = process.waitForFinished(timeoutMs);
    if (!finished) {
        process.kill();
        process.waitForFinished(5000); // allow the process to terminate
        return QuizExtractionResult{QuizExtractionResult::Status::ProcessTimeout};
    }

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        outputText = QString::fromLocal8Bit(process.readAllStandardOutput());
        return QuizExtractionResult{QuizExtractionResult::Status::ProcessError};
    }

    outputText = QString::fromLocal8Bit(process.readAllStandardOutput());
    return QuizExtractionResult{QuizExtractionResult::Status::Success};
}


