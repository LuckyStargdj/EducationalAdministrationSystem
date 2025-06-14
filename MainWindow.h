#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#pragma execution_character_set("utf-8")

#include <QMainWindow>
#include <QStackedWidget>
#include <QStatusBar>
#include <QSqlTableModel>
#include "UserManager.h"
#include "DatabaseManager.h"
#include "CourseManager.h"
#include "StudentForm.h"
#include "CourseForm.h"
#include "ScoreForm.h"
#include "ReportForm.h"
#include "AdminForm.h"

class QTableView;
class QPushButton;
class QComboBox;
class QLineEdit;
class QLabel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(UserManager* userMgr, DatabaseManager* dbMgr, CourseManager* courseMgr, QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onLoginSuccess();
    void onDbConnectionChanged(bool connected);
    void onLogout();
    void showUserProfile();
    void showStudentManagement();
    void showCourseManagement();
    void showScoreManagement();
    void showReportGeneration();
    void showAdminPanel();
    void exportData();
    void importData();
    void about();

private:
    void setupUI();
    void setupMenu();
    void setupToolbar();
    void updateStatusBar();

    void createStudentManagementPage();
    void createCourseManagementPage();
    void createScoreManagementPage();
    void createReportPage();
    void createAdminPage();

    UserManager* m_userMgr;
    DatabaseManager* m_dbMgr;
    CourseManager* m_courseMgr;

    // UI components
    QStackedWidget* m_stackedWidget;
    QStatusBar* m_statusBar;

    // Pages
    QWidget* m_dashboard;
    StudentForm* m_studentPage = nullptr;
    CourseForm* m_coursePage = nullptr;
    ScoreForm* m_scorePage = nullptr;
    ReportForm* m_reportPage = nullptr;
    AdminForm* m_adminPage = nullptr;

    // Status bar labels
    QLabel* m_userInfoLabel;
    QLabel* m_dbStatusLabel;
    QLabel* m_lastLoginLabel;

    // Models
    QSqlTableModel* m_studentModel = nullptr;
    QSqlTableModel* m_courseModel = nullptr;
    QSqlTableModel* m_scoreModel = nullptr;
};

#endif // MAINWINDOW_H