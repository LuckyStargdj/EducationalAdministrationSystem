#ifndef SCOREFORM_H
#define SCOREFORM_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QTableView>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include "UserManager.h"
#include "CourseManager.h"

class ScoreForm : public QWidget {
    Q_OBJECT
public:
    explicit ScoreForm(UserManager* userMgr, CourseManager* courseMgr, QWidget* parent = nullptr);

private slots:
    void queryScores();
    void addScore();
    void updateScore();
    void deleteScore();
    void onCourseSelectionChanged();
    void onStudentSelectionChanged();

private:
    void setupUI();
    void loadCoursesForTeacher();

    UserManager* m_userMgr;
    CourseManager* m_courseMgr;

    // 查询控件
    QComboBox* m_courseCombo;
    QLineEdit* m_studentIdEdit;
    QLineEdit* m_scoreEdit;
    QPushButton* m_queryButton;

    // 成绩表格
    QTableView* m_scoreView;
};

#endif // SCOREFORM_H
