#ifndef COURSEFORM_H
#define COURSEFORM_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include "UserManager.h"
#include "CourseManager.h"

class CourseForm : public QWidget {
    Q_OBJECT
public:
    explicit CourseForm(UserManager* userMgr, CourseManager* courseMgr, QWidget* parent = nullptr);
    void refresh();

private slots:
    void addCourse();
    void editCourse();
    void deleteCourse();
    void onSelectionChanged();

private:
    void setupUI();
    void populateForm(const QString& courseId);
    void clearForm();

    UserManager* m_userMgr;
    CourseManager* m_courseMgr;

    // Form fields
    QLineEdit* m_idEdit;
    QLineEdit* m_nameEdit;
    QLineEdit* m_creditEdit;
    QLineEdit* m_teacherIdEdit;
    QDateEdit* m_startDateEdit;
    QLineEdit* m_semesterEdit;

    // List view
    QTableView* m_courseView;
};

#endif // COURSEFORM_H

