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

    // ������
    QGroupBox* formGroup = new QGroupBox("ѧ����Ϣ");
    QFormLayout* formLayout = new QFormLayout;

    m_idEdit = new QLineEdit(this);
    m_idEdit->setReadOnly(true);

    m_nameEdit = new QLineEdit(this);

    m_genderCombo = new QComboBox(this);
    m_genderCombo->addItem("��");
    m_genderCombo->addItem("Ů");

    m_classEdit = new QLineEdit(this);

    m_yearEdit = new QLineEdit(this);
    m_yearEdit->setValidator(new QIntValidator(2000, QDate::currentDate().year(), this));

    m_fileUpload = new FileUploadWidget(this);

    // ��ť����
    QHBoxLayout* buttonLayout = new QHBoxLayout;
    m_addButton = new QPushButton("���", this);
    m_editButton = new QPushButton("�༭", this);
    m_deleteButton = new QPushButton("ɾ��", this);
    m_saveButton = new QPushButton("����", this);
    m_clearButton = new QPushButton("���", this);

    buttonLayout->addWidget(m_addButton);
    buttonLayout->addWidget(m_editButton);
    buttonLayout->addWidget(m_deleteButton);
    buttonLayout->addWidget(m_saveButton);
    buttonLayout->addWidget(m_clearButton);

    // ��ӱ���
    formLayout->addRow("ѧ��:", m_idEdit);
    formLayout->addRow("����:", m_nameEdit);
    formLayout->addRow("�Ա�:", m_genderCombo);
    formLayout->addRow("�༶:", m_classEdit);
    formLayout->addRow("��ѧ���:", m_yearEdit);
    formLayout->addRow("���ӵ���:", m_fileUpload);
    formLayout->addRow(buttonLayout);

    formGroup->setLayout(formLayout);

    // �б���
    QGroupBox* listGroup = new QGroupBox("ѧ���б�", this);
    QVBoxLayout* listLayout = new QVBoxLayout;

    m_studentView = new QTableView(this);
    m_studentView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_studentView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // ��������
    QHBoxLayout* searchLayout = new QHBoxLayout;
    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setPlaceholderText("����ѧ�š�������ƴ������ĸ");
    m_searchButton = new QPushButton("����", this);
    m_exportButton = new QPushButton("����", this);

    searchLayout->addWidget(m_searchEdit);
    searchLayout->addWidget(m_searchButton);
    searchLayout->addWidget(m_exportButton);
    searchLayout->addStretch();

    listLayout->addLayout(searchLayout);
    listLayout->addWidget(m_studentView);

    listGroup->setLayout(listLayout);

    // ������
    mainLayout->addWidget(formGroup);
    mainLayout->addWidget(listGroup);

    // �����źŲ�
    connect(m_addButton, &QPushButton::clicked, this, &StudentForm::addStudent);
    connect(m_editButton, &QPushButton::clicked, this, &StudentForm::editStudent);
    connect(m_deleteButton, &QPushButton::clicked, this, &StudentForm::deleteStudent);
    connect(m_saveButton, &QPushButton::clicked, this, &StudentForm::addStudent);
    connect(m_clearButton, &QPushButton::clicked, this, &StudentForm::clearForm);
    connect(m_searchButton, &QPushButton::clicked, this, &StudentForm::searchStudents);
    connect(m_exportButton, &QPushButton::clicked, this, &StudentForm::exportStudents);
    connect(m_studentView->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &StudentForm::onSelectionChanged);

    // ��ʼ����ѧ���б�
    refresh();
}

void StudentForm::refresh() {
    QSqlQueryModel* model = new QSqlQueryModel(this);
    QString sql = "SELECT student_id AS 'ѧ��', name AS '����', "
        "CASE gender WHEN 0 THEN '��' ELSE 'Ů' END AS '�Ա�', "
        "class_name AS '�༶', enrollment_year AS '��ѧ���' "
        "FROM users WHERE role = 2";

    model->setQuery(sql, m_dbMgr->getDataBase());
    if (model->lastError().isValid()) {
        qWarning() << "StudentForm::refresh error:" << model->lastError();
    }

    m_studentView->setModel(model);
    m_studentView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // ���ѡ��
    m_studentView->selectionModel()->clearSelection();
    clearForm();
}

void StudentForm::addStudent() {
    QString studentId = m_idEdit->text().trimmed();
    QString name = m_nameEdit->text().trimmed();
    int gender = m_genderCombo->currentIndex(); // 0:��, 1:Ů
    QString className = m_classEdit->text().trimmed();
    QString yearText = m_yearEdit->text().trimmed();

    // ��֤����
    if (name.isEmpty() || className.isEmpty() || yearText.isEmpty()) {
        QMessageBox::warning(this, "�������", "����д���б����ֶ�");
        return;
    }

    bool ok;
    int year = yearText.toInt(&ok);
    if (!ok || year < 2000 || year > QDate::currentDate().year()) {
        QMessageBox::warning(this, "�������", "��ѧ�����Ч");
        return;
    }

    QMap<QString, QVariant> values;
    values["name"] = name;
    values["gender"] = gender;
    values["class_name"] = className;
    values["enrollment_year"] = year;
    values["role"] = UserManager::Student;

    // ����������ѧ��
    if (studentId.isEmpty()) {
        // ����ѧ��: ���+5λ�����
        studentId = QString::number(year) + QString::number(QRandomGenerator::global()->bounded(10000, 99999));
        values["student_id"] = studentId;
        values["pinyin"] = m_dbMgr->getPinyinInitials(name);

        if (m_dbMgr->insertRecord("users", values) > 0) {
            // �ϴ�ѧ������
            QString fileName = m_fileUpload->uploadFile(studentId);
            if (!fileName.isEmpty()) {
                QMap<QString, QVariant> fileValues;
                fileValues["student_id"] = studentId;
                fileValues["file_path"] = fileName;
                m_dbMgr->insertRecord("student_files", fileValues);
            }

            QMessageBox::information(this, "�ɹ�", "ѧ����ӳɹ�");
            refresh();
        }
        else {
            QMessageBox::critical(this, "����", "���ѧ��ʧ��: " + m_dbMgr->getDataBase().lastError().text());
        }
    }
    // ����Ǳ༭����ѧ��
    else {
        values["pinyin"] = m_dbMgr->getPinyinInitials(name);

        if (m_dbMgr->updateRecord("users", values, QString("student_id = '%1'").arg(studentId))) {
            // ���µ���
            QString fileName = m_fileUpload->uploadFile(studentId);
            if (!fileName.isEmpty()) {
                QMap<QString, QVariant> fileValues;
                fileValues["file_path"] = fileName;

                // ����Ƿ����е�����¼
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

            QMessageBox::information(this, "�ɹ�", "ѧ����Ϣ���³ɹ�");
            refresh();
        }
        else {
            QMessageBox::critical(this, "����", "����ѧ����Ϣʧ��: " + m_dbMgr->getDataBase().lastError().text());
        }
    }
}

void StudentForm::editStudent() {
    QModelIndexList selected = m_studentView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "����", "��ѡ��һ��ѧ��");
        return;
    }

    // ��ȡ��һ�У�ѧ�ţ�������
    int row = selected.first().row();
    QString studentId = m_studentView->model()->index(row, 0).data().toString();

    // ����
    populateForm(studentId);
}

void StudentForm::deleteStudent() {
    QModelIndexList selected = m_studentView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "����", "��ѡ��Ҫɾ����ѧ��");
        return;
    }

    // ��ȡ��һ�У�ѧ�ţ�������
    int row = selected.first().row();
    QString studentId = m_studentView->model()->index(row, 0).data().toString();

    if (studentId == m_userMgr->currentStudentId()) {
        QMessageBox::warning(this, "����", "����ɾ����ǰ��¼�û�");
        return;
    }

    // ����ȷ��
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "ȷ��ɾ��",
        QString("ȷ��Ҫɾ��ѧ��Ϊ %1 ��ѧ���𣿴˲������ɻָ���").arg(studentId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // ɾ���û��͵���
        if (m_dbMgr->deleteRecord("users", QString("student_id = '%1'").arg(studentId)) &&
            m_dbMgr->deleteRecord("student_files", QString("student_id = '%1'").arg(studentId))) {
            QMessageBox::information(this, "�ɹ�", "ѧ����ɾ��");
            refresh();
        }
        else {
            QMessageBox::critical(this, "����", "ɾ��ʧ��: " + m_dbMgr->getDataBase().lastError().text());
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
    QString sql = "SELECT student_id AS 'ѧ��', name AS '����', "
        "CASE gender WHEN 0 THEN '��' ELSE 'Ů' END AS '�Ա�', "
        "class_name AS '�༶', enrollment_year AS '��ѧ���' "
        "FROM users WHERE role = 2 ";

    // �ж���ѧ�Ż�������
    if (QRegularExpression("^\\d+$").match(keyword).hasMatch()) {
        sql += QString("AND student_id LIKE '%%1%'").arg(keyword);
    }
    else {
        // ֧��������ƴ������ĸ
        sql += QString("AND (name LIKE '%%1%' OR pinyin LIKE '%%2%')").arg(keyword, keyword.toLower());
    }

    model->setQuery(sql, m_dbMgr->getDataBase());
    if (model->lastError().isValid()) {
        qWarning() << "StudentForm::searchStudents error:" << model->lastError();
    }

    m_studentView->setModel(model);
    m_studentView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // �����
    clearForm();
}

void StudentForm::exportStudents() {
    QString fileName = QFileDialog::getSaveFileName(this, "����ѧ������", "", "CSV �ļ� (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "����", "�޷������ļ�: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // ��ͷ
    out << "ѧ��,����,�Ա�,�༶,��ѧ���\n";

    // ����
    QSqlQuery query("SELECT student_id, name, gender, class_name, enrollment_year "
        "FROM users WHERE role = 2");

    while (query.next()) {
        QString gender = query.value("gender").toInt() == 0 ? "��" : "Ů";
        out << query.value("student_id").toString() << ","
            << query.value("name").toString() << ","
            << gender << ","
            << query.value("class_name").toString() << ","
            << query.value("enrollment_year").toString() << "\n";
    }

    file.close();
    QMessageBox::information(this, "�����ɹ�", QString("�ѵ���%1��ѧ����¼").arg(query.size()));
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

        // ���õ�����ʾ
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