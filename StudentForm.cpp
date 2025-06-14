#include "StudentForm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlError>
#include <QIntValidator>
#include <QRegularExpression>
#include <QDateTime>
#include <QRandomGenerator>
#include <QTextStream>
#include <QDebug>

StudentForm::StudentForm(UserManager* userMgr, DatabaseManager* dbMgr, QWidget* parent)
    : QWidget(parent), m_userMgr(userMgr), m_dbMgr(dbMgr) {
    setupUI();
}

void StudentForm::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 表单部分
    QGroupBox* formGroup = new QGroupBox("学生信息");
    QFormLayout* formLayout = new QFormLayout;

    m_idEdit = new QLineEdit(this);
    m_idEdit->setReadOnly(true);

    m_nameEdit = new QLineEdit(this);

    m_genderCombo = new QComboBox(this);
    m_genderCombo->addItem("男");
    m_genderCombo->addItem("女");

    m_classEdit = new QLineEdit(this);

    m_yearEdit = new QLineEdit(this);
    m_yearEdit->setValidator(new QIntValidator(2000, QDate::currentDate().year(), this));

    m_fileUpload = new FileUploadWidget(this);

    // 按钮区域
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_addButton = new QPushButton("添加", this);
    m_editButton = new QPushButton("编辑", this);
    m_deleteButton = new QPushButton("删除", this);
    m_saveButton = new QPushButton("保存", this);
    m_clearButton = new QPushButton("清空", this);

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_clearButton);

    // 添加表单项
    formLayout->addRow("学号:", m_idEdit);
    formLayout->addRow("姓名:", m_nameEdit);
    formLayout->addRow("性别:", m_genderCombo);
    formLayout->addRow("班级:", m_classEdit);
    formLayout->addRow("入学年份:", m_yearEdit);
    formLayout->addRow("电子档案:", m_fileUpload);
    formLayout->addRow(buttonLayout);

    formGroup->setLayout(formLayout);

    // 列表部分
    QGroupBox* listGroup = new QGroupBox("学生列表", this);
    QVBoxLayout* listLayout = new QVBoxLayout;

    m_studentView = new QTableView(this);
    m_studentView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_studentView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 搜索区域
    QHBoxLayout* searchLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("输入学号、姓名或拼音首字母");
    m_searchButton = new QPushButton("搜索", this);
    m_exportButton = new QPushButton("导出", this);

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addWidget(m_exportButton);
    searchLayout->addStretch();

    listLayout->addLayout(searchLayout);
    listLayout->addWidget(m_studentView);

    listGroup->setLayout(listLayout);

    // 主布局
    mainLayout->addWidget(formGroup);
    mainLayout->addWidget(listGroup);

    // 连接信号槽
    connect(m_addButton, &QPushButton::clicked, this, &StudentForm::addStudent);
    connect(m_editButton, &QPushButton::clicked, this, &StudentForm::editStudent);
    connect(m_deleteButton, &QPushButton::clicked, this, &StudentForm::deleteStudent);
    connect(m_saveButton, &QPushButton::clicked, this, &StudentForm::addStudent);
    connect(m_clearButton, &QPushButton::clicked, this, &StudentForm::clearForm);
    connect(m_searchButton, &QPushButton::clicked, this, &StudentForm::searchStudents);
    connect(m_exportButton, &QPushButton::clicked, this, &StudentForm::exportStudents);
    connect(m_studentView->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &StudentForm::onSelectionChanged);

    // 初始加载学生列表
    refresh();
}

void StudentForm::refresh() {
    QSqlQueryModel* model = new QSqlQueryModel(this);
    QString sql = "SELECT student_id AS '学号', name AS '姓名', "
        "CASE gender WHEN 0 THEN '男' ELSE '女' END AS '性别', "
        "class_name AS '班级', enrollment_year AS '入学年份' "
        "FROM users WHERE role = 2";

    model->setQuery(sql, m_dbMgr->getDataBase());
    if (model->lastError().isValid()) {
        qWarning() << "StudentForm::refresh error:" << model->lastError();
    }

    m_studentView->setModel(model);
    m_studentView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 清除选择
    m_studentView->selectionModel()->clearSelection();
    clearForm();
}

void StudentForm::addStudent() {
    QString studentId = m_idEdit->text().trimmed();
    QString name = m_nameEdit->text().trimmed();
    int gender = m_genderCombo->currentIndex(); // 0:男, 1:女
    QString className = m_classEdit->text().trimmed();
    QString yearText = m_yearEdit->text().trimmed();

    // 验证输入
    if (name.isEmpty() || className.isEmpty() || yearText.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请填写所有必填字段");
        return;
    }

    bool ok;
    int year = yearText.toInt(&ok);
    if (!ok || year < 2000 || year > QDate::currentDate().year()) {
        QMessageBox::warning(this, "输入错误", "入学年份无效");
        return;
    }

    QMap<QString, QVariant> values;
    values["name"] = name;
    values["gender"] = gender;
    values["class_name"] = className;
    values["enrollment_year"] = year;
    values["role"] = UserManager::Student;

    // 如果是添加新学生
    if (studentId.isEmpty()) {
        // 生成学号: 年份+5位随机数
        studentId = QString::number(year) + QString::number(QRandomGenerator::global()->bounded(10000, 99999));
        values["student_id"] = studentId;
        values["pinyin"] = m_dbMgr->getPinyinInitials(name);

        if (m_dbMgr->insertRecord("users", values) > 0) {
            // 上传学生档案
            QString fileName = m_fileUpload->uploadFile(studentId);
            if (!fileName.isEmpty()) {
                QMap<QString, QVariant> fileValues;
                fileValues["student_id"] = studentId;
                fileValues["file_path"] = fileName;
                m_dbMgr->insertRecord("student_files", fileValues);
            }

            QMessageBox::information(this, "成功", "学生添加成功");
            refresh();
        }
        else {
            QMessageBox::critical(this, "错误", "添加学生失败: " + m_dbMgr->getDataBase().lastError().text());
        }
    }
    // 如果是编辑现有学生
    else {
        values["pinyin"] = m_dbMgr->getPinyinInitials(name);

        if (m_dbMgr->updateRecord("users", values, QString("student_id = '%1'").arg(studentId))) {
            // 更新档案
            QString fileName = m_fileUpload->uploadFile(studentId);
            if (!fileName.isEmpty()) {
                QMap<QString, QVariant> fileValues;
                fileValues["file_path"] = fileName;

                // 检查是否已有档案记录
                QSqlQuery query;
                query.prepare("SELECT COUNT(*) FROM student_files WHERE student_id = :studentId");
                query.bindValue(":studentId", studentId);
                if (query.exec() && query.next() && query.value(0).toInt() > 0) {
                    m_dbMgr->updateRecord("student_files", fileValues,
                        QString("student_id = '%1'").arg(studentId));
                }
                else {
                    fileValues["student_id"] = studentId;
                    m_dbMgr->insertRecord("student_files", fileValues);
                }
            }

            QMessageBox::information(this, "成功", "学生信息更新成功");
            refresh();
        }
        else {
            QMessageBox::critical(this, "错误", "更新学生信息失败: " + m_dbMgr->getDataBase().lastError().text());
        }
    }
}

void StudentForm::editStudent() {
    QModelIndexList selected = m_studentView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择一个学生");
        return;
    }

    // 获取第一列（学号）的数据
    int row = selected.first().row();
    QString studentId = m_studentView->model()->index(row, 0).data().toString();

    // 填充表单
    populateForm(studentId);
}

void StudentForm::deleteStudent() {
    QModelIndexList selected = m_studentView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择要删除的学生");
        return;
    }

    // 获取第一列（学号）的数据
    int row = selected.first().row();
    QString studentId = m_studentView->model()->index(row, 0).data().toString();

    if (studentId == m_userMgr->currentStudentId()) {
        QMessageBox::warning(this, "警告", "不能删除当前登录用户");
        return;
    }

    // 二次确认
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
        QString("确定要删除学号为 %1 的学生吗？此操作不可恢复。").arg(studentId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 删除用户和档案
        if (m_dbMgr->deleteRecord("users", QString("student_id = '%1'").arg(studentId)) &&
            m_dbMgr->deleteRecord("student_files", QString("student_id = '%1'").arg(studentId))) {
            QMessageBox::information(this, "成功", "学生已删除");
            refresh();
        }
        else {
            QMessageBox::critical(this, "错误", "删除失败: " + m_dbMgr->getDataBase().lastError().text());
        }
    }
}

void StudentForm::searchStudents() {
    QString keyword = m_searchEdit->text().trimmed();
    if (keyword.isEmpty()) {
        refresh();
        return;
    }

    QSqlQueryModel* model = new QSqlQueryModel(this);
    QString sql = "SELECT student_id AS '学号', name AS '姓名', "
        "CASE gender WHEN 0 THEN '男' ELSE '女' END AS '性别', "
        "class_name AS '班级', enrollment_year AS '入学年份' "
        "FROM users WHERE role = 2 ";

    // 判断是学号还是姓名
    if (QRegularExpression("^\\d+$").match(keyword).hasMatch()) {
        sql += QString("AND student_id LIKE '%%1%'").arg(keyword);
    }
    else {
        // 支持姓名或拼音首字母
        sql += QString("AND (name LIKE '%%1%' OR pinyin LIKE '%%2%')").arg(keyword, keyword.toLower());
    }

    model->setQuery(sql, m_dbMgr->getDataBase());
    if (model->lastError().isValid()) {
        qWarning() << "StudentForm::searchStudents error:" << model->lastError();
    }

    m_studentView->setModel(model);
    m_studentView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 清除表单
    clearForm();
}

void StudentForm::exportStudents() {
    QString fileName = QFileDialog::getSaveFileName(this, "导出学生数据", "", "CSV 文件 (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // 表头
    out << "学号,姓名,性别,班级,入学年份\n";

    // 数据
    QSqlQuery query("SELECT student_id, name, gender, class_name, enrollment_year "
        "FROM users WHERE role = 2");

    while (query.next()) {
        QString gender = query.value("gender").toInt() == 0 ? "男" : "女";
        out << query.value("student_id").toString() << ","
            << query.value("name").toString() << ","
            << gender << ","
            << query.value("class_name").toString() << ","
            << query.value("enrollment_year").toString() << "\n";
    }

    file.close();
    QMessageBox::information(this, "导出成功", QString("已导出%1条学生记录").arg(query.size()));
}

void StudentForm::onSelectionChanged() {
    QModelIndexList selected = m_studentView->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        int row = selected.first().row();
        QString studentId = m_studentView->model()->index(row, 0).data().toString();
        populateForm(studentId);
    }
}

void StudentForm::populateForm(const QString& studentId) {
    QSqlQuery query;
    query.prepare("SELECT users.student_id, users.name, users.gender, "
        "users.class_name, users.enrollment_year, student_files.file_path "
        "FROM users "
        "LEFT JOIN student_files ON users.student_id = student_files.student_id "
        "WHERE users.student_id = :studentId");
    query.bindValue(":studentId", studentId);

    if (query.exec() && query.next()) {
        m_idEdit->setText(query.value("student_id").toString());
        m_nameEdit->setText(query.value("name").toString());
        m_genderCombo->setCurrentIndex(query.value("gender").toInt());
        m_classEdit->setText(query.value("class_name").toString());
        m_yearEdit->setText(query.value("enrollment_year").toString());

        // 设置档案显示
        QString filePath = query.value("file_path").toString();
        if (!filePath.isEmpty()) {
            m_fileUpload->setFile(filePath);
        }
    }
}

void StudentForm::clearForm() {
    m_idEdit->clear();
    m_nameEdit->clear();
    m_genderCombo->setCurrentIndex(0);
    m_classEdit->clear();
    m_yearEdit->clear();
    m_fileUpload->clear();
}