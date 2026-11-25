#include "subjectsmanagerpage.h"

#include <QColorDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QUuid>

SubjectsManagerPage::SubjectsManagerPage(SubjectStore *store, QWidget *parent)
    : QWidget(parent),
      m_store(store),
      m_cardsContainer(nullptr)
{
    QVBoxLayout *pageLayout = new QVBoxLayout(this);

    QPushButton *backButton = new QPushButton("← Back to Dashboard");

    QLabel *title = new QLabel("Subjects Manager");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("font-size: 32px; font-weight: bold; color: #6C63FF;");

    QLabel *description = new QLabel("Organize subjects, teachers and course details.");
    description->setAlignment(Qt::AlignCenter);
    description->setStyleSheet("font-size: 16px; color: gray;");

    QPushButton *addButton = new QPushButton("+ Add New Subject");
    addButton->setMinimumHeight(45);
    addButton->setMinimumWidth(180);

    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addWidget(backButton);
    headerLayout->addStretch();

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);

    m_cardsContainer = new QWidget;
    QVBoxLayout *cardsLayout = new QVBoxLayout(m_cardsContainer);
    cardsLayout->setContentsMargins(10, 10, 10, 10);
    cardsLayout->setSpacing(12);
    cardsLayout->addStretch();
    scrollArea->setWidget(m_cardsContainer);

    pageLayout->addLayout(headerLayout);
    pageLayout->addWidget(title);
    pageLayout->addWidget(description);
    pageLayout->addSpacing(10);

    QHBoxLayout *addLayout = new QHBoxLayout;
    addLayout->addStretch();
    addLayout->addWidget(addButton);
    addLayout->addStretch();
    pageLayout->addLayout(addLayout);
    pageLayout->addSpacing(10);
    pageLayout->addWidget(scrollArea);

    connect(backButton, &QPushButton::clicked, this, &SubjectsManagerPage::backRequested);
    connect(addButton, &QPushButton::clicked, this, [=]() {
        showSubjectDialog();
    });

    refreshCards();
}

void SubjectsManagerPage::refreshCards()
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout *>(m_cardsContainer->layout());

    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
        if (item->widget())
            item->widget()->deleteLater();
        delete item;
    }

    const QVector<Subject> subjects = m_store->subjects();

    if (subjects.isEmpty()) {
        QLabel *emptyLabel = new QLabel("No subjects yet. Click \"Add New Subject\" to get started.");
        emptyLabel->setAlignment(Qt::AlignCenter);
        emptyLabel->setStyleSheet("font-size: 16px; color: #777777; padding: 35px;");
        layout->addWidget(emptyLabel);
    }

    for (const Subject &subject : subjects) {
        QFrame *card = new QFrame;
        card->setStyleSheet(
            "QFrame { background-color: white; border: 1px solid #E5E0FF; border-radius: 15px; }"
        );

        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(18, 15, 18, 15);
        cardLayout->setSpacing(16);

        QLabel *colorBadge = new QLabel;
        colorBadge->setFixedSize(28, 28);
        colorBadge->setStyleSheet(
            QString("background-color: %1; border-radius: 14px;").arg(subject.color)
        );

        QVBoxLayout *textLayout = new QVBoxLayout;

        QLabel *nameLabel = new QLabel(subject.name);
        nameLabel->setStyleSheet("font-size: 18px; font-weight: bold; color: #25243A;");

        QLabel *codeLabel = new QLabel(subject.code.isEmpty()
                                       ? "No subject code"
                                       : subject.code);
        codeLabel->setStyleSheet("font-size: 13px; font-weight: bold; color: #6C63FF;");

        QLabel *teacherLabel = new QLabel(subject.teacher.isEmpty()
                                          ? "No teacher assigned"
                                          : "Teacher: " + subject.teacher);
        teacherLabel->setStyleSheet("font-size: 14px; color: #555555;");

        QLabel *descriptionLabel = new QLabel(subject.description.isEmpty()
                                              ? "No description"
                                              : subject.description);
        descriptionLabel->setWordWrap(true);
        descriptionLabel->setStyleSheet("font-size: 14px; color: #777777;");

        textLayout->addWidget(nameLabel);
        textLayout->addWidget(codeLabel);
        textLayout->addWidget(teacherLabel);
        textLayout->addWidget(descriptionLabel);

        QPushButton *editButton = new QPushButton("Edit");
        QPushButton *deleteButton = new QPushButton("Delete");
        editButton->setMinimumWidth(90);
        deleteButton->setMinimumWidth(90);

        QVBoxLayout *actionsLayout = new QVBoxLayout;
        actionsLayout->addWidget(editButton);
        actionsLayout->addWidget(deleteButton);

        cardLayout->addWidget(colorBadge);
        cardLayout->addLayout(textLayout, 1);
        cardLayout->addLayout(actionsLayout);

        layout->addWidget(card);

        connect(editButton, &QPushButton::clicked, this, [=]() {
            Subject editable = subject;
            showSubjectDialog(&editable);
        });

        connect(deleteButton, &QPushButton::clicked, this, [=]() {
            QMessageBox confirmation;
            confirmation.setWindowTitle("Delete Subject");
            confirmation.setText("Delete \"" + subject.name + "\"?");
            confirmation.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            confirmation.setDefaultButton(QMessageBox::No);

            if (confirmation.exec() == QMessageBox::Yes) {
                m_store->removeSubject(subject.id);
                refreshCards();
                emit subjectsChanged();
            }
        });
    }

    layout->addStretch();
}

void SubjectsManagerPage::showSubjectDialog(Subject *existingSubject)
{
    QDialog dialog(this);
    dialog.setWindowTitle(existingSubject ? "Edit Subject" : "Add New Subject");
    dialog.resize(480, 430);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    QLineEdit *nameEdit = new QLineEdit;
    QLineEdit *codeEdit = new QLineEdit;
    QLineEdit *teacherEdit = new QLineEdit;
    QTextEdit *descriptionEdit = new QTextEdit;
    QPushButton *colorButton = new QPushButton;

    nameEdit->setPlaceholderText("Subject name");
    codeEdit->setPlaceholderText("Subject code");
    teacherEdit->setPlaceholderText("Teacher name");
    descriptionEdit->setPlaceholderText("Subject description");
    descriptionEdit->setMinimumHeight(100);

    QColor selectedColor = existingSubject ? QColor(existingSubject->color) : QColor("#6C63FF");
    if (!selectedColor.isValid())
        selectedColor = QColor("#6C63FF");

    colorButton->setText("Choose Color");
    colorButton->setFixedHeight(38);
    colorButton->setStyleSheet(
        QString("background-color: %1; color: white; border: none; border-radius: 8px;")
        .arg(selectedColor.name())
    );

    if (existingSubject) {
        nameEdit->setText(existingSubject->name);
        codeEdit->setText(existingSubject->code);
        teacherEdit->setText(existingSubject->teacher);
        descriptionEdit->setPlainText(existingSubject->description);
    }

    layout->addWidget(new QLabel("Subject Name *"));
    layout->addWidget(nameEdit);
    layout->addWidget(new QLabel("Subject Code"));
    layout->addWidget(codeEdit);
    layout->addWidget(new QLabel("Teacher Name"));
    layout->addWidget(teacherEdit);
    layout->addWidget(new QLabel("Description"));
    layout->addWidget(descriptionEdit);
    layout->addWidget(new QLabel("Color"));
    layout->addWidget(colorButton);

    connect(colorButton, &QPushButton::clicked, this, [=, &selectedColor, &dialog]() mutable {
        QColor chosen = QColorDialog::getColor(selectedColor, &dialog, "Choose Subject Color");
        if (chosen.isValid()) {
            selectedColor = chosen;
            colorButton->setStyleSheet(
                QString("background-color: %1; color: white; border: none; border-radius: 8px;")
                .arg(selectedColor.name())
            );
        }
    });

    QDialogButtonBox *buttons = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel
    );
    layout->addWidget(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() != QDialog::Accepted)
        return;

    const QString name = nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Subject Name Required", "Please enter a subject name.");
        showSubjectDialog(existingSubject);
        return;
    }

    Subject subject;
    subject.id = existingSubject
        ? existingSubject->id
        : QUuid::createUuid().toString(QUuid::WithoutBraces);
    subject.name = name;
    subject.code = codeEdit->text().trimmed();
    subject.teacher = teacherEdit->text().trimmed();
    subject.description = descriptionEdit->toPlainText().trimmed();
    subject.color = selectedColor.name();

    if (existingSubject)
        m_store->updateSubject(subject);
    else
        m_store->addSubject(subject);

    refreshCards();
    emit subjectsChanged();
}
