#include "windowscredentialmanager.h"

#include <QByteArray>
#include <QString>

#include <string>
#include <windows.h>
#include <wincred.h>

QString WindowsCredentialManager::lastError()
{
    const DWORD errorCode = GetLastError();
    return QString("Win32 error %1").arg(errorCode);
}

bool WindowsCredentialManager::saveCredential(
    const QString &targetName,
    const QString &userName,
    const QString &credential)
{
    if (targetName.isEmpty() ||
        userName.isEmpty() ||
        credential.isEmpty()) {
        return false;
    }

    const std::wstring wTarget = targetName.toStdWString();
    const std::wstring wUser = userName.toStdWString();
    const QByteArray credentialBytes = credential.toUtf8();

    CREDENTIALW cred = {};

    cred.TargetName = const_cast<LPWSTR>(wTarget.c_str());
    cred.Type = CRED_TYPE_GENERIC;
    cred.UserName = const_cast<LPWSTR>(wUser.c_str());
    cred.CredentialBlob = reinterpret_cast<LPBYTE>(
        const_cast<char *>(credentialBytes.constData())
    );
    cred.CredentialBlobSize = static_cast<DWORD>(
        credentialBytes.size()
    );
    cred.Persist = CRED_PERSIST_LOCAL_MACHINE;

    return CredWriteW(&cred, 0) != FALSE;
}

QString WindowsCredentialManager::readCredential(
    const QString &targetName,
    const QString &userName)
{
    if (targetName.isEmpty() || userName.isEmpty()) {
        return QString();
    }

    const std::wstring wTarget = targetName.toStdWString();

    PCREDENTIALW cred = nullptr;

    if (!CredReadW(
            wTarget.c_str(),
            CRED_TYPE_GENERIC,
            0,
            &cred)) {
        return QString();
    }

    QString result;

    if (cred && cred->UserName &&
        QString::fromWCharArray(cred->UserName) == userName &&
        cred->CredentialBlob &&
        cred->CredentialBlobSize > 0) {

        result = QString::fromUtf8(
            reinterpret_cast<const char *>(cred->CredentialBlob),
            static_cast<qsizetype>(cred->CredentialBlobSize)
        );
    }

    if (cred) {
        CredFree(cred);
    }

    return result;
}

bool WindowsCredentialManager::deleteCredential(
    const QString &targetName,
    const QString &userName)
{
    if (targetName.isEmpty() || userName.isEmpty()) {
        return false;
    }

    const std::wstring wTarget = targetName.toStdWString();

    return CredDeleteW(
        wTarget.c_str(),
        CRED_TYPE_GENERIC,
        0
    ) != FALSE;
}