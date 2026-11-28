#include "apikeyeditordialog.h"
#include "windowscredentialmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

ApiKeyEditorDialog::ApiKeyEditorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle("API Key Management");
    resize(450, 160);

    m_apiKeyEdit = new QLineEdit(this);
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText("Enter OpenAI API key");

    QPushButton *saveBtn = new QPushButton("Save", this);
    QPushButton *updateBtn = new QPushButton("Update", this);
    QPushButton *deleteBtn = new QPushButton("Delete Saved Key", this);
    QPushButton *cancelBtn = new QPushButton("Cancel", this);

    connect(saveBtn, &QPushButton::clicked,
            this, &ApiKeyEditorDialog::onSave);

    connect(updateBtn, &QPushButton::clicked,
            this, &ApiKeyEditorDialog::onSave);

    connect(deleteBtn, &QPushButton::clicked,
            this, &ApiKeyEditorDialog::onDelete);

    connect(cancelBtn, &QPushButton::clicked,
            this, &ApiKeyEditorDialog::onCancel);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow("API Key:", m_apiKeyEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(saveBtn);
    buttonLayout->addWidget(updateBtn);
    buttonLayout->addWidget(deleteBtn);
    buttonLayout->addWidget(cancelBtn);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(buttonLayout);
}

QString ApiKeyEditorDialog::apiKey() const
{
    return m_apiKeyEdit->text();
}

void ApiKeyEditorDialog::onSave()
{
    const QString key = m_apiKeyEdit->text().trimmed();

    if (key.isEmpty()) {
        QMessageBox::warning(
            this,
            "Invalid Input",
            "Please enter an API key."
        );
        return;
    }

    const bool success = WindowsCredentialManager::saveCredential(
        WindowsCredentialManager::OpenAITarget,
        WindowsCredentialManager::OpenAIUser,
        key
    );

    if (success) {
        QMessageBox::information(
            this,
            "Success",
            "API key saved successfully."
        );
        accept();
    } else {
        QMessageBox::critical(
            this,
            "Error",
            "Failed to save API key. " +
                WindowsCredentialManager::lastError()
        );
    }
}

void ApiKeyEditorDialog::onDelete()
{
    const bool success = WindowsCredentialManager::deleteCredential(
        WindowsCredentialManager::OpenAITarget,
        WindowsCredentialManager::OpenAIUser
    );

    if (success) {
        QMessageBox::information(
            this,
            "Success",
            "Saved API key deleted."
        );
        m_apiKeyEdit->clear();
    } else {
        QMessageBox::critical(
            this,
            "Error",
            "Failed to delete API key. " +
                WindowsCredentialManager::lastError()
        );
    }
}

void ApiKeyEditorDialog::onCancel()
{
    reject();
}