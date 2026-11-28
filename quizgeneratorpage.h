#ifndef QUIZGENERATORPAGE_H
#define QUIZGENERATORPAGE_H

#include <QWidget>
#include <QVector>
#include <QString>
#include <QStringList>
#include <QDateTime>

#include "quiz.h"
#include "quizgeneratorresult.h"
#include "quiztextractor.h"

class TaskStore;
class QuizGenerator;
class QuizStore;
class QuizAttemptStore;

class QComboBox;
class QPushButton;
class QLabel;
class QSpinBox;
class QProgressBar;
class QVBoxLayout;
class QButtonGroup;
class QRadioButton;
class QGroupBox;
class QWidget;
class QTextEdit;

class QuizGeneratorPage : public QWidget
{
    Q_OBJECT

public:
    explicit QuizGeneratorPage(
        TaskStore *taskStore,
        QuizGenerator *quizGenerator,
        QuizTextExtractor *textExtractor,
        QuizStore *quizStore,
        QuizAttemptStore *attemptStore,
        QWidget *parent = nullptr
    );

    ~QuizGeneratorPage() override;

    void refresh();

signals:
    void pageClosed();

private slots:
    void onTaskSelected(const QString &taskId);
    void onAttachmentSelected();
    void onExtractText();
    void onGenerateQuiz();
    void onGenerationFinished(QuizGenerationResult result);
    void onSaveQuiz();
    void onAttemptQuiz();
    void onAttemptSubmitted();

private:
    void setupUI();

    void populateTaskSelector();
    void populateAttachmentList(const QString &taskId);

    void clearGeneratedQuiz();
    void showGenerationLoading(bool loading);
    void showQuiz(const QuizGenerationResult &result);
    void showAttemptResults();

    QString selectedTaskId() const;
    QString selectedAttachmentId() const;
    QString selectedAttachmentPath() const;

    void saveCurrentQuiz();
    void startAttempt();

    TaskStore *m_taskStore = nullptr;
    QuizGenerator *m_quizGenerator = nullptr;
    QuizTextExtractor *m_textExtractor = nullptr;
    QuizStore *m_quizStore = nullptr;
    QuizAttemptStore *m_attemptStore = nullptr;

    QComboBox *m_taskSelector = nullptr;

    QWidget *m_attachmentContainer = nullptr;
    QVBoxLayout *m_attachmentLayout = nullptr;

    QPushButton *m_extractButton = nullptr;
    QPushButton *m_backButton = nullptr;
    QPushButton *m_generateButton = nullptr;
    QPushButton *m_saveButton = nullptr;
    QPushButton *m_attemptButton = nullptr;
    QPushButton *m_submitAttemptButton = nullptr;

    QTextEdit *m_extractionPreview = nullptr;
    QLabel *m_statusLabel = nullptr;

    QSpinBox *m_questionCountSelector = nullptr;
    QProgressBar *m_progressBar = nullptr;

    QWidget *m_generatedQuizWidget = nullptr;
    QVBoxLayout *m_generatedQuizLayout = nullptr;

    QVector<QGroupBox *> m_questionGroups;
    QVector<QButtonGroup *> m_answerGroups;

    QString m_selectedTaskId;
    QString m_selectedAttachmentId;
    QString m_selectedAttachmentPath;

    QString m_extractedText;

    QuizExtractionResult m_lastExtractionResult;
    QuizGenerationResult m_cachedResult;

    Quiz m_currentQuiz;
    QDateTime m_attemptStartTime;

    bool m_generationInProgress = false;
    bool m_attemptInProgress = false;

    QVector<int> m_selectedOptions;
};

#endif // QUIZGENERATORPAGE_H