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

    // ������
    QGroupBox* formGroup = new QGroupBox("�γ���Ϣ");
    QFormLayout* formLayout = new QFormLayout;

    m_idEdit = new QLineEdit;
    m_nameEdit = new QLineEdit;
    m_creditEdit = new QLineEdit;
    m_creditEdit->setValidator(new QDoubleValidator(0.1, 10.0, 1, this));
    m_teacherIdEdit = new QLineEdit;
    m_startDateEdit = new QDateEdit(QDate::currentDate());
    m_semesterEdit = new QLineEdit;

    QPushButton* addButton = new QPushButton("���");
    QPushButton* editButton = new QPushButton("�༭");
    QPushButton* deleteButton = new QPushButton("ɾ��");

    connect(addButton, &QPushButton::clicked, this, &CourseForm::addCourse);
    connect(editButton, &QPushButton::clicked, this, &CourseForm::editCourse);
    connect(deleteButton, &QPushButton::clicked, this, &CourseForm::deleteCourse);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(editButton);
    buttonLayout->addWidget(deleteButton);

    formLayout->addRow("�γ̱��:", m_idEdit);
    formLayout->addRow("�γ�����:", m_nameEdit);
    formLayout->addRow("ѧ��:", m_creditEdit);
    formLayout->addRow("��ʦID:", m_teacherIdEdit);
    formLayout->addRow("��������:", m_startDateEdit);
    formLayout->addRow("ѧ��:", m_semesterEdit);
    formLayout->addRow(buttonLayout);

    formGroup->setLayout(formLayout);
    mainLayout->addWidget(formGroup);

    // �б���
    QGroupBox* listGroup = new QGroupBox("�γ��б�");
    QVBoxLayout* listLayout = new QVBoxLayout;

    m_courseView = new QTableView;
    m_courseView->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_courseView->setSelectionMode(QAbstractItemView::SingleSelection);

    connect(m_courseView->selectionModel(), &QItemSelectionModel::selectionChanged,
        this, &CourseForm::onSelectionChanged);

    listLayout->addWidget(m_courseView);
    listGroup->setLayout(listLayout);
    mainLayout->addWidget(listGroup);

    // ��ʼ������
    refresh();
}

void CourseForm::refresh() {
    QSqlQueryModel* model = new QSqlQueryModel;

    // ��ʦֻ�ܿ����Լ��̵Ŀγ�
    if (m_userMgr->currentRole() == UserManager::Teacher) {
        QString teacherId = m_userMgr->currentStudentId();
        model->setQuery(QString("SELECT id AS '�γ̱��', name AS '�γ�����', "
            "credit AS 'ѧ��', teacher_id AS '��ʦID', "
            "start_date AS '��������', semester AS 'ѧ��' "
            "FROM courses WHERE teacher_id = '%1'").arg(teacherId));
    }
    else {
        model->setQuery("SELECT id AS '�γ̱��', name AS '�γ�����', "
            "credit AS 'ѧ��', teacher_id AS '��ʦID', "
            "start_date AS '��������', semester AS 'ѧ��' "
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
        QMessageBox::warning(this, "�������", "����д���б����ֶ�");
        return;
    }

    // ����ǽ�ʦ��ֻ������Լ�Ϊ�ڿν�ʦ�Ŀγ�
    if (m_userMgr->currentRole() == UserManager::Teacher) {
        QString currentTeacherId = m_userMgr->currentStudentId();
        if (teacherId != currentTeacherId) {
            QMessageBox::warning(this, "Ȩ�޴���", "��ֻ������Լ��ڿεĿγ�");
            return;
        }
    }

    if (m_courseMgr->addCourse(id, name, credit, teacherId, startDate, semester)) {
        QMessageBox::information(this, "�ɹ�", "�γ���ӳɹ�");
        refresh();
        clearForm();
    }
    else {
        QMessageBox::critical(this, "����", "��ӿγ�ʧ��");
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
        QMessageBox::warning(this, "��ʾ", "û����Ҫ���µ���Ϣ");
        return;
    }

    // ����ʦȨ��
    QString currentTeacherId;
    QModelIndexList selected = m_courseView->selectionModel()->selectedRows(3); // ��ʦID��
    if (!selected.isEmpty()) {
        QModelIndex index = selected.first();
        currentTeacherId = m_courseView->model()->data(index).toString();
    }

    if (m_userMgr->currentRole() == UserManager::Teacher) {
        QString teacherId = m_userMgr->currentStudentId();
        if (currentTeacherId != teacherId) {
            QMessageBox::warning(this, "Ȩ�޴���", "��ֻ���޸��Լ��ڿεĿγ�");
            return;
        }
    }

    if (m_courseMgr->updateCourse(id, updates)) {
        QMessageBox::information(this, "�ɹ�", "�γ���Ϣ���³ɹ�");
        refresh();
    }
    else {
        QMessageBox::critical(this, "����", "���¿γ���Ϣʧ��");
    }
}

void CourseForm::deleteCourse() {
    QModelIndexList selected = m_courseView->selectionModel()->selectedRows(0); // ID��
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "����", "��ѡ��Ҫɾ���Ŀγ�");
        return;
    }

    QModelIndex index = selected.first();
    QString courseId = m_courseView->model()->data(index).toString();

    // ����ʦȨ��
    QString teacherId;
    selected = m_courseView->selectionModel()->selectedRows(3); // ��ʦID��
    if (!selected.isEmpty()) {
        QModelIndex index = selected.first();
        teacherId = m_courseView->model()->data(index).toString();
    }

    if (m_userMgr->currentRole() == UserManager::Teacher) {
        QString currentTeacherId = m_userMgr->currentStudentId();
        if (teacherId != currentTeacherId) {
            QMessageBox::warning(this, "Ȩ�޴���", "��ֻ��ɾ���Լ��ڿεĿγ�");
            return;
        }
    }

    // ����ȷ��
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "ȷ��ɾ��",
        QString("ȷ��Ҫɾ���γ� '%1' �𣿴˲������ɻָ���").arg(courseId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_courseMgr->deleteCourse(courseId)) {
            QMessageBox::information(this, "�ɹ�", "�γ���ɾ��");
            refresh();
        }
        else {
            QMessageBox::critical(this, "����", "ɾ���γ�ʧ��");
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