#include "ScoreForm.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QHeaderView>
#include <QDoubleValidator>

ScoreForm::ScoreForm(UserManager* userMgr, CourseManager* courseMgr, QWidget* parent)
    : QWidget(parent), m_userMgr(userMgr), m_courseMgr(courseMgr) {
    setupUI();
}

void ScoreForm::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 查询条件区域
    QGroupBox* queryGroup = new QGroupBox("成绩查询");
    QFormLayout* queryLayout = new QFormLayout;

    m_courseCombo = new QComboBox;
    connect(m_courseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ScoreForm::onCourseSelectionChanged);

    m_studentIdEdit = new QLineEdit;
    connect(m_studentIdEdit, &QLineEdit::textChanged,
        this, &ScoreForm::onStudentSelectionChanged);

    m_scoreEdit = new QLineEdit;
    m_scoreEdit->setValidator(new QDoubleValidator(0, 100, 2, this));

    m_queryButton = new QPushButton("查询");
    connect(m_queryButton, &QPushButton::clicked, this, &ScoreForm::queryScores);

    QPushButton* addButton = new QPushButton("添加成绩");
    connect(addButton, &QPushButton::clicked, this, &ScoreForm::addScore);

    QPushButton* updateButton = new QPushButton("更新成绩");
    connect(updateButton, &QPushButton::clicked, this, &ScoreForm::updateScore);

    QPushButton* deleteButton = new QPushButton("删除成绩");
    connect(deleteButton, &QPushButton::clicked, this, &ScoreForm::deleteScore);

    queryLayout->addRow("课程:", m_courseCombo);
    queryLayout->addRow("学号:", m_studentIdEdit);
    queryLayout->addRow("成绩:", m_scoreEdit);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_queryButton);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(deleteButton);
    queryLayout->addRow(buttonLayout);

    queryGroup->setLayout(queryLayout);
    mainLayout->addWidget(queryGroup);

    // 成绩表格
    m_scoreView = new QTableView;
    mainLayout->addWidget(m_scoreView);

    // 初始化课程列表
    loadCoursesForTeacher();
}

void ScoreForm::loadCoursesForTeacher() {
    m_courseCombo->clear();

    // 教师只能看到自己教的课程
    if (m_userMgr->currentRole() == UserManager::Teacher) {
        QString teacherId = m_userMgr->currentStudentId();
        QVector<QVariantMap> courses = m_courseMgr->getCoursesByTeacher(teacherId);

        for (const QVariantMap& course : courses) {
            m_courseCombo->addItem(
                QString("%1 (%2)").arg(course["name"].toString(), course["id"].toString()),
                course["id"]
            );
        }
    }
    else {
        // 管理员可以看到所有课程
        QVector<QVariantMap> courses = m_courseMgr->getAllCourses();
        for (const QVariantMap& course : courses) {
            m_courseCombo->addItem(
                QString("%1 (%2)").arg(course["name"].toString(), course["id"].toString()),
                course["id"]
            );
        }
    }
}

void ScoreForm::queryScores() {
    QString courseId = m_courseCombo->currentData().toString();
    QString studentId = m_studentIdEdit->text().trimmed();

    if (courseId.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择课程");
        return;
    }

    if (!studentId.isEmpty()) {
        // 查询特定学生在该课程的成绩
        double score = m_courseMgr->getScore(studentId, courseId);
        m_scoreEdit->setText(score >= 0 ? QString::number(score) : "");

        // 同时查询该学生所有成绩
        QVector<QVariantMap> scores = m_courseMgr->getScoresByStudent(studentId);
        QSqlQueryModel* model = new QSqlQueryModel;
        // 需要将scores转换为表格模型（实际实现中应填充模型数据）
        m_scoreView->setModel(model);
    }
    else {
        // 查询该课程所有学生成绩
        QVector<QVariantMap> scores = m_courseMgr->getScoresByCourse(courseId);
        QSqlQueryModel* model = new QSqlQueryModel;
        // 填充模型数据
        m_scoreView->setModel(model);
    }

    m_scoreView->horizontalHeader()->setStretchLastSection(true);
}

void ScoreForm::addScore() {
    QString courseId = m_courseCombo->currentData().toString();
    QString studentId = m_studentIdEdit->text().trimmed();
    double score = m_scoreEdit->text().toDouble();

    if (courseId.isEmpty() || studentId.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择课程并输入学号");
        return;
    }

    // 检查当前用户是否有权限（教师只能为自己的课程添加成绩）
    if (m_userMgr->currentRole() == UserManager::Teacher) {
        bool validCourse = false;
        QString teacherId = m_userMgr->currentStudentId();
        QVector<QVariantMap> courses = m_courseMgr->getCoursesByTeacher(teacherId);

        for (const QVariantMap& course : courses) {
            if (course["id"].toString() == courseId) {
                validCourse = true;
                break;
            }
        }

        if (!validCourse) {
            QMessageBox::warning(this, "权限错误", "您不能为这门课程添加成绩");
            return;
        }
    }

    if (m_courseMgr->addScore(studentId, courseId, score)) {
        QMessageBox::information(this, "成功", "成绩添加成功");
        queryScores(); // 刷新查询结果
    }
    else {
        QMessageBox::critical(this, "错误", "添加成绩失败");
    }
}

void ScoreForm::updateScore() {
    QString courseId = m_courseCombo->currentData().toString();
    QString studentId = m_studentIdEdit->text().trimmed();
    double newScore = m_scoreEdit->text().toDouble();

    if (courseId.isEmpty() || studentId.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请选择课程并输入学号");
        return;
    }

    // 检查当前用户是否有权限（教师只能为自己的课程更新成绩）
    if (m_userMgr->currentRole() == UserManager::Teacher) {
        bool validCourse = false;
        QString teacherId = m_userMgr->currentStudentId();
        QVector<QVariantMap> courses = m_courseMgr->getCoursesByTeacher(teacherId);

        for (const QVariantMap& course : courses) {
            if (course["id"].toString() == courseId) {
                validCourse = true;
                break;
            }
        }

        if (!validCourse) {
            QMessageBox::warning(this, "权限错误", "您不能修改这门课程的成绩");
            return;
        }
    }

    if (m_courseMgr->updateScore(studentId, courseId, newScore)) {
        QMessageBox::information(this, "成功", "成绩更新成功");
        queryScores(); // 刷新查询结果
    }
    else {
        QMessageBox::critical(this, "错误", "更新成绩失败");
    }
}

void ScoreForm::deleteScore() {
    QModelIndexList selected = m_scoreView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要删除的成绩");
        return;
    }

    QModelIndex index = selected.first();
    QString studentId = m_scoreView->model()->data(m_scoreView->model()->index(index.row(), 0)).toString();
    QString courseId = m_courseCombo->currentData().toString();

    // 检查权限
    if (m_userMgr->currentRole() == UserManager::Teacher) {
        bool validCourse = false;
        QString teacherId = m_userMgr->currentStudentId();
        QVector<QVariantMap> courses = m_courseMgr->getCoursesByTeacher(teacherId);

        for (const QVariantMap& course : courses) {
            if (course["id"].toString() == courseId) {
                validCourse = true;
                break;
            }
        }

        if (!validCourse) {
            QMessageBox::warning(this, "权限错误", "您不能删除这门课程的成绩");
            return;
        }
    }

    // 二次确认
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
        QString("确定要删除学号为 %1 的成绩吗？").arg(studentId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_courseMgr->updateScore(studentId, courseId, -1)) { // 使用-1表示删除
            QMessageBox::information(this, "成功", "成绩已删除");
            queryScores(); // 刷新查询结果
        }
        else {
            QMessageBox::critical(this, "错误", "删除成绩失败");
        }
    }
}

void ScoreForm::onCourseSelectionChanged() {
    if (!m_studentIdEdit->text().isEmpty()) {
        queryScores();
    }
}

void ScoreForm::onStudentSelectionChanged() {
    if (!m_studentIdEdit->text().isEmpty() && !m_courseCombo->currentData().isNull()) {
        queryScores();
    }
}