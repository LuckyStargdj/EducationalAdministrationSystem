#ifndef STUDENTFORM_H
#define STUDENTFORM_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QDateEdit>
#include <QPushButton>
#include <QLineEdit>
#include "UserManager.h"
#include "DatabaseManager.h"
#include "FileUploadWidget.h"

class StudentForm : public QWidget {
    Q_OBJECT
public:
    explicit StudentForm(UserManager* userMgr, DatabaseManager* dbMgr, QWidget* parent = nullptr);
    void refresh();

private slots:
    void addStudent();
    void editStudent();
    void deleteStudent();
    void searchStudents();
    void exportStudents();
    void onSelectionChanged();
    void clearForm();

private:
    void setupUI();
    void populateForm(const QString& studentId);

    UserManager* m_userMgr;
    DatabaseManager* m_dbMgr;

    // Form fields
    QLineEdit* m_idEdit;
    QLineEdit* m_nameEdit;
    QComboBox* m_genderCombo;
    QLineEdit* m_classEdit;
    QLineEdit* m_yearEdit;
    FileUploadWidget* m_fileUpload;
    QPushButton* m_saveButton;
    QPushButton* m_clearButton;

    // List view
    QTableView* m_studentView;
    QLineEdit* m_searchEdit;

    // Buttons
    QPushButton* m_addButton;
    QPushButton* m_editButton;
    QPushButton* m_deleteButton;
    QPushButton* m_searchButton;
    QPushButton* m_exportButton;
};

#endif // STUDENTFORM_H
