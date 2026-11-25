#ifndef SUBJECTSMANAGERPAGE_H
#define SUBJECTSMANAGERPAGE_H

#include "subjectstore.h"

#include <QWidget>

class SubjectsManagerPage : public QWidget
{
    Q_OBJECT

public:
    explicit SubjectsManagerPage(SubjectStore *store, QWidget *parent = nullptr);

signals:
    void backRequested();
    void subjectsChanged();

private:
    SubjectStore *m_store;
    QWidget *m_cardsContainer;

    void refreshCards();
    void showSubjectDialog(Subject *existingSubject = nullptr);
};

#endif
