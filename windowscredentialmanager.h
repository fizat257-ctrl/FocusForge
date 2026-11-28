#ifndef WINDOWSCREDENTIALMANAGER_H
#define WINDOWSCREDENTIALMANAGER_H

#include <QString>

// Thin wrapper over the Windows Credential Manager API (CredWrite/CredRead/CredDelete
// from Advapi32). Used to store the OpenAI API key securely, never in QSettings,
// source code, logs, or ordinary configuration files.
class WindowsCredentialManager
{
public:
    // Saves a credential to Windows Credential Manager.
    static bool saveCredential(const QString &targetName,
                               const QString &userName,
                               const QString &credential);

    // Retrieves a credential from Windows Credential Manager.
    // Returns an empty QString if the credential does not exist.
    static QString readCredential(const QString &targetName,
                                  const QString &userName);

    // Deletes a credential from Windows Credential Manager.
    static bool deleteCredential(const QString &targetName,
                                 const QString &userName);

    // Human-readable description of the last Win32 error.
    static QString lastError();

    // Fixed target/user used by FocusForge for the OpenAI API key.
    static constexpr const char *OpenAITarget = "FocusForge/OpenAI API Key";
    static constexpr const char *OpenAIUser = "FocusForge";
};

#endif // WINDOWSCREDENTIALMANAGER_H
