#include "CourseForm.h"
#include <QFormLayout>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QSqlQueryModel>
#include <QMessageBox>
#include <QSqlRecord>
#include <QDoubleValidator>
#include <QDate>

CourseForm::CourseForm(UserManager* userMgr, CourseManager* courseMgr, QWidget* parent)
    : QWidget(parent), m_userMgr(userMgr), m_courseMgr(courseMgr) {
    setupUI();
}

void CourseForm::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 表单部分
    QGroupBox* formGroup = new QGroupBox("课程信息");
    QFormLayout* formLayout = new QFormLayout;

    m_idEdit = new QLineEdit;
    m_nameEdit = new QLineEdit;
    m_creditEdit = new QLineEdit;
    m_creditEdit->setValidator(new QDoubleValidator(0.1, 10.0, 1, this));
    m_teacherIdEdit = new QLineEdit;
    m_startDateEdit = new QDateEdit(QDate::currentDate());
    m_semesterEdit = new QLineEdit;

    QPushButton* addButton = new QPushButton("添加");
    QPushButton* editButton = new QPushButton("编辑");
    QPushButton* deleteButton = new QPushButton("删除");

    connect(addButton, &QPushButton::clicked, this, &CourseForm::addCourse);
    connect(editButton, &QPushButton::clicked, this, &CourseForm::editCourse);
    connect(deleteButton, &QPushButton::clicked, this, &CourseForm::deleteCourse);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);

    formLayout->addRow("课程编号:", m_idEdit);
    formLayout->addRow("课程名称:", m_nameEdit);
    formLayout->addRow("学分:", m_creditEdit);
    formLayout->addRow("教师ID:", m_teacherIdEdit);
    formLayout->addRow("开课日期:", m_startDateEdit);
    formLayout->addRow("学期:", m_semesterEdit);
    formLayout->addRow(buttonLayout);

    formGroup->setLayout(formLayout);
    mainLayout->addWidget(formGroup);

    // 列表部分
    QGroupBox* listGroup = new QGroupBox("课程列表");
    QVBoxLayout* listLayout = new QVBoxLayout;

    m_courseView = new QTableView;
    m_courseView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_courseView->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_courseView->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &CourseForm::onSelectionChanged);

    listLayout->addWidget(m_courseView);
    listGroup->setLayout(listLayout);
    mainLayout->addWidget(listGroup);

    // 初始化数据
    refresh();
}

void CourseForm::refresh() {
    QSqlQueryModel* model = new QSqlQueryModel;

    // 教师只能看到自己教的课程
    if (m_userMgr->currentRole() == UserManager::Teacher) {
        QString teacherId = m_userMgr->currentStudentId();
        model->setQuery(QString("SELECT id AS '课程编号', name AS '课程名称', "
            "credit AS '学分', teacher_id AS '教师ID', "
            "start_date AS '开课日期', semester AS '学期' "
            "FROM courses WHERE teacher_id = '%1'").arg(teacherId));
    }
    else {
        model->setQuery("SELECT id AS '课程编号', name AS '课程名称', "
            "credit AS '学分', teacher_id AS '教师ID', "
            "start_date AS '开课日期', semester AS '学期' "
            "FROM courses");
    }

    m_courseView->setModel(model);
    m_courseView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void CourseForm::addCourse() {
    QString id = m_idEdit->text().trimmed();
    QString name = m_nameEdit->text().trimmed();
    double credit = m_creditEdit->text().toDouble();
    QString teacherId = m_teacherIdEdit->text().trimmed();
    QDate startDate = m_startDateEdit->date();
    QString semester = m_semesterEdit->text().trimmed();

    if (id.isEmpty() || name.isEmpty() || teacherId.isEmpty() || semester.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写所有必填字段");
        return;
    }

    // 如果是教师，只能添加自己为授课教师的课程
    if (m_userMgr->currentRole() == UserManager::Teacher) {
        QString currentTeacherId = m_userMgr->currentStudentId();
        if (teacherId != currentTeacherId) {
            QMessageBox::warning(this, "权限错误", "您只能添加自己授课的课程");
            return;
        }
    }

    if (m_courseMgr->addCourse(id, name, credit, teacherId, startDate, semester)) {
        QMessageBox::information(this, "成功", "课程添加成功");
        refresh();
        clearForm();
    }
    else {
        QMessageBox::critical(this, "错误", "添加课程失败");
    }
}

void CourseForm::editCourse() {
    QString id = m_idEdit->text().trimmed();
    if (id.isEmpty()) return;

    QMap<QString, QVariant> updates;
    if (!m_nameEdit->text().isEmpty()) updates["name"] = m_nameEdit->text();
    if (!m_creditEdit->text().isEmpty()) updates["credit"] = m_creditEdit->text().toDouble();
    if (!m_teacherIdEdit->text().isEmpty()) updates["teacher_id"] = m_teacherIdEdit->text();
    if (!m_semesterEdit->text().isEmpty()) updates["semester"] = m_semesterEdit->text();
    if (m_startDateEdit->date() != QDate::currentDate()) {
        updates["start_date"] = m_startDateEdit->date().toString(Qt::ISODate);
    }

    if (updates.isEmpty()) {
        QMessageBox::warning(this, "提示", "没有需要更新的信息");
        return;
    }

    // 检查教师权限
    QString currentTeacherId;
    QModelIndexList selected = m_courseView->selectionModel()->selectedRows(3); // 教师ID列
    if (!selected.isEmpty()) {
        QModelIndex index = selected.first();
        currentTeacherId = m_courseView->model()->data(index).toString();
    }

    if (m_userMgr->currentRole() == UserManager::Teacher) {
        QString teacherId = m_userMgr->currentStudentId();
        if (currentTeacherId != teacherId) {
            QMessageBox::warning(this, "权限错误", "您只能修改自己授课的课程");
            return;
        }
    }

    if (m_courseMgr->updateCourse(id, updates)) {
        QMessageBox::information(this, "成功", "课程信息更新成功");
        refresh();
    }
    else {
        QMessageBox::critical(this, "错误", "更新课程信息失败");
    }
}

void CourseForm::deleteCourse() {
    QModelIndexList selected = m_courseView->selectionModel()->selectedRows(0); // ID列
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要删除的课程");
        return;
    }

    QModelIndex index = selected.first();
    QString courseId = m_courseView->model()->data(index).toString();

    // 检查教师权限
    QString teacherId;
    selected = m_courseView->selectionModel()->selectedRows(3); // 教师ID列
    if (!selected.isEmpty()) {
        QModelIndex index = selected.first();
        teacherId = m_courseView->model()->data(index).toString();
    }

    if (m_userMgr->currentRole() == UserManager::Teacher) {
        QString currentTeacherId = m_userMgr->currentStudentId();
        if (teacherId != currentTeacherId) {
            QMessageBox::warning(this, "权限错误", "您只能删除自己授课的课程");
            return;
        }
    }

    // 二次确认
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
        QString("确定要删除课程 '%1' 吗？此操作不可恢复。").arg(courseId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_courseMgr->deleteCourse(courseId)) {
            QMessageBox::information(this, "成功", "课程已删除");
            refresh();
        }
        else {
            QMessageBox::critical(this, "错误", "删除课程失败");
        }
    }
}

void CourseForm::onSelectionChanged() {
    QModelIndexList selected = m_courseView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        clearForm();
        return;
    }

    QModelIndex index = selected.first();
    QString courseId = m_courseView->model()->data(m_courseView->model()->index(index.row(), 0)).toString();
    populateForm(courseId);
}

void CourseForm::populateForm(const QString& courseId) {
    QVector<QVariantMap> courses = m_courseMgr->getAllCourses();
    for (const QVariantMap& course : courses) {
        if (course["id"].toString() == courseId) {
            m_idEdit->setText(courseId);
            m_nameEdit->setText(course["name"].toString());
            m_creditEdit->setText(QString::number(course["credit"].toDouble()));
            m_teacherIdEdit->setText(course["teacher_id"].toString());
            m_startDateEdit->setDate(QDate::fromString(course["start_date"].toString(), Qt::ISODate));
            m_semesterEdit->setText(course["semester"].toString());
            break;
        }
    }
}

void CourseForm::clearForm() {
    m_idEdit->clear();
    m_nameEdit->clear();
    m_creditEdit->clear();
    m_teacherIdEdit->clear();
    m_startDateEdit->setDate(QDate::currentDate());
    m_semesterEdit->clear();
}