#include "quizgeneratorpage.h"

#include "taskstore.h"
#include "task.h"
#include "quizgenerator.h"
#include "quizstore.h"
#include "quizattemptstore.h"
#include "quizattempt.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>
#include <QProgressBar>
#include <QTextEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QMessageBox>
#include <QScrollArea>
#include <QFrame>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QUuid>
#include <QFile>
#include <QStandardPaths>

QuizGeneratorPage::QuizGeneratorPage(
    TaskStore *taskStore,
    QuizGenerator *quizGenerator,
    QuizTextExtractor *textExtractor,
    QuizStore *quizStore,
    QuizAttemptStore *attemptStore,
    QWidget *parent
)
    : QWidget(parent),
      m_taskStore(taskStore),
      m_quizGenerator(quizGenerator),
      m_textExtractor(textExtractor),
      m_quizStore(quizStore),
      m_attemptStore(attemptStore)
{
    setupUI();

    if (m_quizGenerator) {
        connect(
            m_quizGenerator,
            &QuizGenerator::finished,
            this,
            &QuizGeneratorPage::onGenerationFinished
        );
    }

    populateTaskSelector();
}

QuizGeneratorPage::~QuizGeneratorPage()
{
    if (m_quizGenerator && m_generationInProgress) {
        m_quizGenerator->cancel();
    }
}

void QuizGeneratorPage::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(12);

    QLabel *heading = new QLabel("AI Quiz Generator", this);
    QFont headingFont = heading->font();
    headingFont.setPointSize(20);
    headingFont.setBold(true);
    heading->setFont(headingFont);

    QLabel *description = new QLabel(
        "Generate multiple-choice quizzes from your task attachments.",
        this
    );
m_backButton = new QPushButton("← Back to Dashboard", this);
m_backButton->setFixedWidth(180);
mainLayout->addWidget(m_backButton);
    mainLayout->addWidget(heading);
    mainLayout->addWidget(description);

    

    // Widget that holds all the scrollable content
    QWidget *contentWidget = new QWidget(this);
    QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);

    // Main content scroll area – allows the page to scroll when content
    // exceeds the available window height, while keeping the title accessible.
    QScrollArea *contentScrollArea = new QScrollArea(this);
    contentScrollArea->setWidgetResizable(true);
    contentScrollArea->setWidget(contentWidget);
    mainLayout->addWidget(contentScrollArea);

    QGroupBox *sourceGroup = new QGroupBox("Select Study Material", this);
    QVBoxLayout *sourceLayout = new QVBoxLayout(sourceGroup);
    

    m_taskSelector = new QComboBox(sourceGroup);
    m_taskSelector->setMinimumHeight(35);

    sourceLayout->addWidget(new QLabel("Select Task:", sourceGroup));
    sourceLayout->addWidget(m_taskSelector);

    m_attachmentContainer = new QWidget(sourceGroup);
    m_attachmentLayout = new QVBoxLayout(m_attachmentContainer);
    m_attachmentLayout->setContentsMargins(0, 0, 0, 0);

    sourceLayout->addWidget(new QLabel("Attachments:", sourceGroup));
    sourceLayout->addWidget(m_attachmentContainer);

    m_extractButton = new QPushButton("Extract Text", sourceGroup);
    sourceLayout->addWidget(m_extractButton);

    m_extractionPreview = new QTextEdit(
        "No attachment selected.",
        sourceGroup
    );
    m_extractionPreview->setReadOnly(true);
    m_extractionPreview->setLineWrapMode(QTextEdit::WidgetWidth);
    m_extractionPreview->setMaximumHeight(200);
    m_extractionPreview->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_extractionPreview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_extractionPreview->setTextInteractionFlags(Qt::TextSelectableByMouse);

    sourceLayout->addWidget(m_extractionPreview);
    contentLayout->addWidget(sourceGroup);

    QGroupBox *generationGroup = new QGroupBox(
        "Quiz Settings",
        this
    );

    QHBoxLayout *generationLayout = new QHBoxLayout(generationGroup);

    generationLayout->addWidget(
        new QLabel("Number of Questions:", generationGroup)
    );

    m_questionCountSelector = new QSpinBox(generationGroup);
    m_questionCountSelector->setRange(1, 20);
    m_questionCountSelector->setValue(5);

    generationLayout->addWidget(m_questionCountSelector);

    m_generateButton = new QPushButton(
        "Generate Quiz",
        generationGroup
    );

    generationLayout->addWidget(m_generateButton);
    contentLayout->addWidget(generationGroup);

    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 0);
    m_progressBar->setVisible(false);
   contentLayout->addWidget(m_progressBar);

    m_statusLabel = new QLabel("Ready.", this);
    m_statusLabel->setWordWrap(true);
   contentLayout->addWidget(m_statusLabel);

    QGroupBox *quizGroup = new QGroupBox(
        "Generated Quiz",
        this
    );

    QVBoxLayout *quizGroupLayout = new QVBoxLayout(quizGroup);

    QScrollArea *scrollArea = new QScrollArea(quizGroup);
    scrollArea->setWidgetResizable(true);

    m_generatedQuizWidget = new QWidget(scrollArea);
    m_generatedQuizLayout = new QVBoxLayout(m_generatedQuizWidget);

    scrollArea->setWidget(m_generatedQuizWidget);
    quizGroupLayout->addWidget(scrollArea);

    QHBoxLayout *actionLayout = new QHBoxLayout();

    m_saveButton = new QPushButton("Save Quiz", quizGroup);
    m_attemptButton = new QPushButton("Start Quiz", quizGroup);
    m_submitAttemptButton = new QPushButton(
        "Submit Answers",
        quizGroup
    );

    actionLayout->addWidget(m_saveButton);
    actionLayout->addWidget(m_attemptButton);
    actionLayout->addWidget(m_submitAttemptButton);

    quizGroupLayout->addLayout(actionLayout);

    contentLayout->addWidget(quizGroup);
    

    m_saveButton->setEnabled(false);
    m_attemptButton->setEnabled(false);
    m_submitAttemptButton->setEnabled(false);
    m_generateButton->setEnabled(false);
    m_extractButton->setEnabled(false);

    connect(
        m_taskSelector,
        &QComboBox::currentTextChanged,
        this,
        [this]() {
            const QString taskId = selectedTaskId();
            onTaskSelected(taskId);
        }
    );

    connect(
        m_extractButton,
        &QPushButton::clicked,
        this,
        &QuizGeneratorPage::onExtractText
    );

    connect(
        m_generateButton,
        &QPushButton::clicked,
        this,
        &QuizGeneratorPage::onGenerateQuiz
    );

    connect(
        m_saveButton,
        &QPushButton::clicked,
        this,
        &QuizGeneratorPage::onSaveQuiz
    );

    connect(
        m_attemptButton,
        &QPushButton::clicked,
        this,
        &QuizGeneratorPage::onAttemptQuiz
    );

    connect(
        m_submitAttemptButton,
        &QPushButton::clicked,
        this,
        &QuizGeneratorPage::onAttemptSubmitted
    );
    connect(m_backButton, &QPushButton::clicked, this, &QuizGeneratorPage::pageClosed);
}
// ================= TASK AND ATTACHMENT FUNCTIONS =================

void QuizGeneratorPage::refresh()
{
    populateTaskSelector();
}

QString QuizGeneratorPage::selectedTaskId() const
{
    if (!m_taskSelector)
        return QString();

    return m_taskSelector->currentData().toString();
}

QString QuizGeneratorPage::selectedAttachmentId() const
{
    return m_selectedAttachmentId;
}

QString QuizGeneratorPage::selectedAttachmentPath() const
{
    return m_selectedAttachmentPath;
}

void QuizGeneratorPage::populateTaskSelector()
{
    if (!m_taskSelector || !m_taskStore)
        return;

    const QString previousTaskId = selectedTaskId();

    m_taskSelector->blockSignals(true);
    m_taskSelector->clear();

    const QVector<Task> tasks = m_taskStore->tasks();

    for (const Task &task : tasks) {
        m_taskSelector->addItem(task.title, task.id);
    }

    int selectedIndex = m_taskSelector->findData(previousTaskId);

    if (selectedIndex < 0 && m_taskSelector->count() > 0)
        selectedIndex = 0;

    if (selectedIndex >= 0)
        m_taskSelector->setCurrentIndex(selectedIndex);

    m_taskSelector->blockSignals(false);

    onTaskSelected(selectedTaskId());
}

void QuizGeneratorPage::onTaskSelected(const QString &taskId)
{
    m_selectedTaskId = taskId;

    m_selectedAttachmentId.clear();
    m_selectedAttachmentPath.clear();
    m_extractedText.clear();

    m_lastExtractionResult = QuizExtractionResult();

    if (m_extractionPreview) {
        m_extractionPreview->setText(
            "Select an attachment and extract its text."
        );
    }

    if (m_generateButton)
        m_generateButton->setEnabled(false);

    clearGeneratedQuiz();

    populateAttachmentList(taskId);
}

void QuizGeneratorPage::populateAttachmentList(const QString &taskId)
{
    if (!m_attachmentLayout || !m_attachmentContainer)
        return;

    while (QLayoutItem *item = m_attachmentLayout->takeAt(0)) {
        if (QWidget *widget = item->widget())
            widget->deleteLater();

        delete item;
    }

    m_selectedAttachmentId.clear();
    m_selectedAttachmentPath.clear();

    if (!m_taskStore || taskId.isEmpty()) {
        m_attachmentLayout->addWidget(
            new QLabel("Please select a task.", m_attachmentContainer)
        );
        return;
    }

    const QVector<Task> tasks = m_taskStore->tasks();

    const Task *selectedTask = nullptr;

    for (const Task &task : tasks) {
        if (task.id == taskId) {
            selectedTask = &task;
            break;
        }
    }

    if (!selectedTask || selectedTask->attachments.isEmpty()) {
        m_attachmentLayout->addWidget(
            new QLabel(
                "No PDF or PNG attachments found for this task.",
                m_attachmentContainer
            )
        );
        return;
    }

    QButtonGroup *buttonGroup = new QButtonGroup(
        m_attachmentContainer
    );

    buttonGroup->setExclusive(true);

    const QString appDataPath =
        QStandardPaths::writableLocation(
            QStandardPaths::AppDataLocation
        );

    const QString attachmentRoot =
        QDir(appDataPath).filePath("attachments");

    for (const TaskAttachment &attachment : selectedTask->attachments) {

        QString absolutePath;

        if (QFileInfo(attachment.relativePath).isAbsolute()) {
            absolutePath = QDir::cleanPath(
                attachment.relativePath
            );
        } else {
            absolutePath = QDir::cleanPath(
                QDir(attachmentRoot).filePath(
                    attachment.relativePath
                )
            );
        }

        QRadioButton *radioButton = new QRadioButton(
            attachment.fileName,
            m_attachmentContainer
        );

        radioButton->setProperty(
            "attachmentId",
            attachment.id
        );

        radioButton->setProperty(
            "attachmentPath",
            absolutePath
        );

        buttonGroup->addButton(radioButton);
        m_attachmentLayout->addWidget(radioButton);

        connect(
            radioButton,
            &QRadioButton::clicked,
            this,
            &QuizGeneratorPage::onAttachmentSelected
        );
    }

    m_attachmentLayout->addStretch();
}

void QuizGeneratorPage::onAttachmentSelected()
{
    QRadioButton *radioButton =
        qobject_cast<QRadioButton *>(sender());

    if (!radioButton)
        return;

    m_selectedAttachmentId =
        radioButton->property("attachmentId").toString();

    m_selectedAttachmentPath =
        radioButton->property("attachmentPath").toString();

    m_extractedText.clear();

    if (m_extractionPreview) {
        m_extractionPreview->setText(
            "Selected: " + radioButton->text() +
            "\nClick Extract Text to read this attachment."
        );
    }

    if (m_generateButton)
        m_generateButton->setEnabled(false);

    if (m_extractButton)
        m_extractButton->setEnabled(true);

    clearGeneratedQuiz();
}
// ================= TEXT EXTRACTION =================

void QuizGeneratorPage::onExtractText()
{
    if (!m_textExtractor) {
        QMessageBox::warning(
            this,
            "Error",
            "Text extractor is not available."
        );
        return;
    }

    if (m_selectedAttachmentId.isEmpty() ||
        m_selectedAttachmentPath.isEmpty()) {

        QMessageBox::warning(
            this,
            "Select Attachment",
            "Please select a PDF or PNG attachment first."
        );
        return;
    }

    m_extractButton->setEnabled(false);
    m_generateButton->setEnabled(false);

    m_statusLabel->setText("Extracting text...");

    m_lastExtractionResult =
        m_textExtractor->extract(m_selectedAttachmentPath);

    m_extractButton->setEnabled(true);

    if (m_lastExtractionResult.status !=
        QuizExtractionResult::Status::Success) {

        m_extractedText.clear();

        m_extractionPreview->setText(
            m_lastExtractionResult.userMessage
        );

        m_statusLabel->setText("Text extraction failed.");

        QMessageBox::warning(
            this,
            "Extraction Failed",
            m_lastExtractionResult.userMessage
        );

        return;
    }

    m_extractedText =
        m_lastExtractionResult.extractedText;

    QString preview = m_extractedText.left(1500);

    if (m_extractedText.size() > 1500)
        preview += "\n\n... (Preview truncated)";

    m_extractionPreview->setText(
        "Text extracted successfully.\n\n" + preview
    );

    m_generateButton->setEnabled(true);

    m_statusLabel->setText(
        m_lastExtractionResult.userMessage
    );

    clearGeneratedQuiz();
}

// ================= QUIZ GENERATION =================

void QuizGeneratorPage::onGenerateQuiz()
{
    if (!m_quizGenerator) {
        QMessageBox::warning(
            this,
            "Error",
            "Quiz generator is not available."
        );
        return;
    }

    if (m_selectedTaskId.isEmpty()) {
        QMessageBox::warning(
            this,
            "Select Task",
            "Please select a task first."
        );
        return;
    }

    if (m_selectedAttachmentId.isEmpty()) {
        QMessageBox::warning(
            this,
            "Select Attachment",
            "Please select an attachment first."
        );
        return;
    }

    if (m_extractedText.trimmed().isEmpty()) {
        QMessageBox::warning(
            this,
            "No Extracted Text",
            "Please extract text before generating a quiz."
        );
        return;
    }

    if (m_quizGenerator->isBusy()) {
        QMessageBox::information(
            this,
            "Please Wait",
            "A quiz generation request is already running."
        );
        return;
    }

    const int questionCount =
        m_questionCountSelector->value();

    const QStringList attachmentIds = {
        m_selectedAttachmentId
    };

    clearGeneratedQuiz();

    m_generationInProgress = true;

    showGenerationLoading(true);

    m_statusLabel->setText(
        "Generating quiz using AI. Please wait..."
    );

    m_quizGenerator->generateQuiz(
        m_extractedText,
        questionCount,
        m_selectedTaskId,
        attachmentIds
    );
}

void QuizGeneratorPage::showGenerationLoading(bool loading)
{
    m_generationInProgress = loading;

    m_progressBar->setVisible(loading);

    m_generateButton->setEnabled(!loading);
    m_extractButton->setEnabled(!loading);
    m_taskSelector->setEnabled(!loading);
    m_questionCountSelector->setEnabled(!loading);

    if (loading) {
        m_progressBar->setRange(0, 0);
    } else {
        m_progressBar->setRange(0, 100);
        m_progressBar->setValue(0);
    }
}

// ================= GENERATION RESULT =================

void QuizGeneratorPage::onGenerationFinished(
    QuizGenerationResult result
)
{
    showGenerationLoading(false);

    m_cachedResult = result;

    if (result.status != QuizGenerationResult::Status::Success) {

        m_statusLabel->setText("Quiz generation failed.");

        QMessageBox::warning(
            this,
            "Quiz Generation Failed",
            result.errorMessage.isEmpty()
                ? "Unable to generate quiz."
                : result.errorMessage
        );

        return;
    }

    if (result.questions.isEmpty()) {
        m_statusLabel->setText(
            "AI returned no questions."
        );

        QMessageBox::warning(
            this,
            "Empty Quiz",
            "No questions were returned."
        );

        return;
    }

    showQuiz(result);

    m_statusLabel->setText(
        QString("Successfully generated %1 questions.")
            .arg(result.questions.size())
    );
}
// ================= CLEAR GENERATED QUIZ =================

void QuizGeneratorPage::clearGeneratedQuiz()
{
    if (!m_generatedQuizLayout)
        return;

    while (QLayoutItem *item =
               m_generatedQuizLayout->takeAt(0)) {

        if (QWidget *widget = item->widget())
            delete widget;

        delete item;
    }

    m_questionGroups.clear();
    m_answerGroups.clear();
    m_selectedOptions.clear();

    m_saveButton->setEnabled(false);
    m_attemptButton->setEnabled(false);
    m_submitAttemptButton->setEnabled(false);

    m_attemptInProgress = false;
    m_currentQuiz = Quiz();
}

// ================= DISPLAY GENERATED QUIZ =================

void QuizGeneratorPage::showQuiz(
    const QuizGenerationResult &result
)
{
    clearGeneratedQuiz();

    m_currentQuiz.id =
        QUuid::createUuid().toString(QUuid::WithoutBraces);

    m_currentQuiz.taskId = m_selectedTaskId;
    m_currentQuiz.title = "AI Generated Study Quiz";
    m_currentQuiz.sourceDescription =
        QFileInfo(m_selectedAttachmentPath).fileName();

    m_currentQuiz.sourceAttachmentIds =
        QStringList{m_selectedAttachmentId};

    m_currentQuiz.createdAt = QDateTime::currentDateTime();
    m_currentQuiz.questions = result.questions;

    for (int i = 0; i < m_currentQuiz.questions.size(); ++i) {

        const QuizQuestion &question =
            m_currentQuiz.questions[i];

        QGroupBox *questionGroup = new QGroupBox(
            QString("Question %1").arg(i + 1),
            m_generatedQuizWidget
        );

        QVBoxLayout *questionLayout =
            new QVBoxLayout(questionGroup);

        QLabel *questionLabel = new QLabel(
            question.text,
            questionGroup
        );

        questionLabel->setWordWrap(true);
        questionLayout->addWidget(questionLabel);

        QButtonGroup *answerGroup =
            new QButtonGroup(questionGroup);

        answerGroup->setExclusive(true);

        for (int optionIndex = 0;
             optionIndex < question.options.size();
             ++optionIndex) {

            const QuizOption &option =
                question.options[optionIndex];

            QRadioButton *radioButton = new QRadioButton(
                option.text,
                questionGroup
            );

            answerGroup->addButton(
                radioButton,
                optionIndex
            );

            radioButton->setEnabled(false);

            questionLayout->addWidget(radioButton);
        }

        m_generatedQuizLayout->addWidget(questionGroup);

        m_questionGroups.append(questionGroup);
        m_answerGroups.append(answerGroup);
    }

    m_saveButton->setEnabled(true);
    m_attemptButton->setEnabled(true);

    m_generatedQuizLayout->addStretch();
}

// ================= SAVE QUIZ =================

void QuizGeneratorPage::saveCurrentQuiz()
{
    if (!m_quizStore || m_currentQuiz.questions.isEmpty())
        return;

    bool alreadySaved = false;

    const QVector<Quiz> existingQuizzes =
        m_quizStore->quizzes();

    for (const Quiz &quiz : existingQuizzes) {
        if (quiz.id == m_currentQuiz.id) {
            alreadySaved = true;
            break;
        }
    }

    if (alreadySaved)
        return;

    if (!m_quizStore->addQuiz(m_currentQuiz)) {
        QMessageBox::warning(
            this,
            "Save Failed",
            "The quiz could not be saved."
        );
        return;
    }

    m_statusLabel->setText("Quiz saved successfully.");
}

void QuizGeneratorPage::onSaveQuiz()
{
    if (m_currentQuiz.questions.isEmpty()) {
        QMessageBox::warning(
            this,
            "No Quiz",
            "Generate a quiz before saving it."
        );
        return;
    }

    saveCurrentQuiz();

    if (m_quizStore) {
        const QVector<Quiz> savedQuizzes =
            m_quizStore->quizzes();

        for (const Quiz &quiz : savedQuizzes) {
            if (quiz.id == m_currentQuiz.id) {
                QMessageBox::information(
                    this,
                    "Quiz Saved",
                    "Your quiz has been saved successfully."
                );

                m_saveButton->setEnabled(false);
                return;
            }
        }
    }
}

// ================= START QUIZ ATTEMPT =================

void QuizGeneratorPage::startAttempt()
{
    if (m_currentQuiz.questions.isEmpty())
        return;

    m_selectedOptions.clear();

    for (int i = 0;
         i < m_currentQuiz.questions.size();
         ++i) {

        m_selectedOptions.append(-1);

        if (i < m_answerGroups.size()) {

            QButtonGroup *group = m_answerGroups[i];

            for (QAbstractButton *button : group->buttons())
                button->setEnabled(true);
        }
    }

    m_attemptStartTime = QDateTime::currentDateTime();
    m_attemptInProgress = true;

    m_attemptButton->setEnabled(false);
    m_saveButton->setEnabled(false);
    m_submitAttemptButton->setEnabled(true);

    m_statusLabel->setText(
        "Quiz started. Select one answer for each question."
    );
}

void QuizGeneratorPage::onAttemptQuiz()
{
    if (m_currentQuiz.questions.isEmpty()) {
        QMessageBox::warning(
            this,
            "No Quiz",
            "Please generate a quiz first."
        );
        return;
    }

    if (m_attemptInProgress)
        return;

    startAttempt();
}

// ================= SUBMIT QUIZ ATTEMPT =================

void QuizGeneratorPage::onAttemptSubmitted()
{
    if (!m_attemptInProgress)
        return;

    int correctAnswers = 0;

    m_selectedOptions.clear();

    for (int i = 0;
         i < m_answerGroups.size();
         ++i) {

        QButtonGroup *group = m_answerGroups[i];

        const int selectedIndex = group->checkedId();

        if (selectedIndex < 0) {
            QMessageBox::warning(
                this,
                "Incomplete Quiz",
                "Please answer every question before submitting."
            );
            return;
        }

        m_selectedOptions.append(selectedIndex);

        if (selectedIndex ==
            m_currentQuiz.questions[i].correctOptionIndex) {

            ++correctAnswers;
        }
    }

    QuizAttempt attempt;

    attempt.id =
        QUuid::createUuid().toString(QUuid::WithoutBraces);

    attempt.quizId = m_currentQuiz.id;
    attempt.taskId = m_currentQuiz.taskId;

    attempt.startTime = m_attemptStartTime;
    attempt.endTime = QDateTime::currentDateTime();

    attempt.selectedOptionIndexes = m_selectedOptions;

    attempt.correctAnswerCount = correctAnswers;
    attempt.totalQuestionCount =
        m_currentQuiz.questions.size();

    if (!m_attemptStore ||
        !m_attemptStore->addAttempt(attempt)) {

        QMessageBox::warning(
            this,
            "Save Error",
            "Your quiz result could not be saved."
        );

        return;
    }

    m_attemptInProgress = false;

    for (QButtonGroup *group : m_answerGroups) {
        for (QAbstractButton *button : group->buttons())
            button->setEnabled(false);
    }

    m_submitAttemptButton->setEnabled(false);

    showAttemptResults();
}

// ================= SHOW QUIZ RESULTS =================

void QuizGeneratorPage::showAttemptResults()
{
    const int totalQuestions =
        m_currentQuiz.questions.size();

    int correctAnswers = 0;

    for (int i = 0;
         i < totalQuestions &&
         i < m_selectedOptions.size();
         ++i) {

        const QuizQuestion &question =
            m_currentQuiz.questions[i];

        const int selectedIndex = m_selectedOptions[i];

        if (selectedIndex == question.correctOptionIndex)
            ++correctAnswers;

        QLabel *resultLabel = new QLabel(
            m_questionGroups[i]
        );

        if (selectedIndex == question.correctOptionIndex) {
            resultLabel->setText(
                "Correct Answer!"
            );
        } else {
            resultLabel->setText(
                "Incorrect. Correct answer: " +
                question.options[
                    question.correctOptionIndex
                ].text
            );
        }

        resultLabel->setWordWrap(true);

        m_questionGroups[i]->layout()->addWidget(
            resultLabel
        );

        if (!question.explanation.isEmpty()) {
            QLabel *explanationLabel = new QLabel(
                "Explanation: " + question.explanation,
                m_questionGroups[i]
            );

            explanationLabel->setWordWrap(true);

            m_questionGroups[i]->layout()->addWidget(
                explanationLabel
            );
        }
    }

    const double percentage =
        totalQuestions > 0
            ? (correctAnswers * 100.0) / totalQuestions
            : 0.0;

    QMessageBox::information(
        this,
        "Quiz Results",
        QString(
            "Quiz Completed!\n\n"
            "Correct Answers: %1 / %2\n"
            "Score: %3%"
        )
            .arg(correctAnswers)
            .arg(totalQuestions)
            .arg(QString::number(percentage, 'f', 1))
    );

    m_statusLabel->setText(
        QString("Quiz completed. Score: %1 / %2")
            .arg(correctAnswers)
            .arg(totalQuestions)
    );
}