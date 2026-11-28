#ifndef APIKEYEDITORDIALOG_H
#define APIKEYEDITORDIALOG_H

#include <QDialog>
#include <QString>

class QLineEdit;

class ApiKeyEditorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ApiKeyEditorDialog(QWidget *parent = nullptr);

    QString apiKey() const;

private slots:
    void onSave();
    void onDelete();
    void onCancel();

private:
    QLineEdit *m_apiKeyEdit;
};

#endif // APIKEYEDITORDIALOG_H