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

    // ��ѯ��������
    QGroupBox* queryGroup = new QGroupBox("�ɼ���ѯ");
    QFormLayout* queryLayout = new QFormLayout;

    m_courseCombo = new QComboBox;
    connect(m_courseCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &ScoreForm::onCourseSelectionChanged);

    m_studentIdEdit = new QLineEdit;
    connect(m_studentIdEdit, &QLineEdit::textChanged,
        this, &ScoreForm::onStudentSelectionChanged);

    m_scoreEdit = new QLineEdit;
    m_scoreEdit->setValidator(new QDoubleValidator(0, 100, 2, this));

    m_queryButton = new QPushButton("��ѯ");
    connect(m_queryButton, &QPushButton::clicked, this, &ScoreForm::queryScores);

    QPushButton* addButton = new QPushButton("��ӳɼ�");
    connect(addButton, &QPushButton::clicked, this, &ScoreForm::addScore);

    QPushButton* updateButton = new QPushButton("���³ɼ�");
    connect(updateButton, &QPushButton::clicked, this, &ScoreForm::updateScore);

    QPushButton* deleteButton = new QPushButton("ɾ���ɼ�");
    connect(deleteButton, &QPushButton::clicked, this, &ScoreForm::deleteScore);

    queryLayout->addRow("�γ�:", m_courseCombo);
    queryLayout->addRow("ѧ��:", m_studentIdEdit);
    queryLayout->addRow("�ɼ�:", m_scoreEdit);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    buttonLayout->addWidget(m_queryButton);
    buttonLayout->addWidget(addButton);
    buttonLayout->addWidget(updateButton);
    buttonLayout->addWidget(deleteButton);
    queryLayout->addRow(buttonLayout);

    queryGroup->setLayout(queryLayout);
    mainLayout->addWidget(queryGroup);

    // �ɼ����
    m_scoreView = new QTableView;
    mainLayout->addWidget(m_scoreView);

    // ��ʼ���γ��б�
    loadCoursesForTeacher();
}

void ScoreForm::loadCoursesForTeacher() {
    m_courseCombo->clear();

    // ��ʦֻ�ܿ����Լ��̵Ŀγ�
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
        // ����Ա���Կ������пγ�
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
        QMessageBox::warning(this, "����", "��ѡ��γ�");
        return;
    }

    if (!studentId.isEmpty()) {
        // ��ѯ�ض�ѧ���ڸÿγ̵ĳɼ�
        double score = m_courseMgr->getScore(studentId, courseId);
        m_scoreEdit->setText(score >= 0 ? QString::number(score) : "");

        // ͬʱ��ѯ��ѧ�����гɼ�
        QVector<QVariantMap> scores = m_courseMgr->getScoresByStudent(studentId);
        QSqlQueryModel* model = new QSqlQueryModel;
        // ��Ҫ��scoresת��Ϊ���ģ�ͣ�ʵ��ʵ����Ӧ���ģ�����ݣ�
        m_scoreView->setModel(model);
    }
    else {
        // ��ѯ�ÿγ�����ѧ���ɼ�
        QVector<QVariantMap> scores = m_courseMgr->getScoresByCourse(courseId);
        QSqlQueryModel* model = new QSqlQueryModel;
        // ���ģ������
        m_scoreView->setModel(model);
    }

    m_scoreView->horizontalHeader()->setStretchLastSection(true);
}

void ScoreForm::addScore() {
    QString courseId = m_courseCombo->currentData().toString();
    QString studentId = m_studentIdEdit->text().trimmed();
    double score = m_scoreEdit->text().toDouble();

    if (courseId.isEmpty() || studentId.isEmpty()) {
        QMessageBox::warning(this, "�������", "��ѡ��γ̲�����ѧ��");
        return;
    }

    // ��鵱ǰ�û��Ƿ���Ȩ�ޣ���ʦֻ��Ϊ�Լ��Ŀγ���ӳɼ���
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
            QMessageBox::warning(this, "Ȩ�޴���", "������Ϊ���ſγ���ӳɼ�");
            return;
        }
    }

    if (m_courseMgr->addScore(studentId, courseId, score)) {
        QMessageBox::information(this, "�ɹ�", "�ɼ���ӳɹ�");
        queryScores(); // ˢ�²�ѯ���
    }
    else {
        QMessageBox::critical(this, "����", "��ӳɼ�ʧ��");
    }
}

void ScoreForm::updateScore() {
    QString courseId = m_courseCombo->currentData().toString();
    QString studentId = m_studentIdEdit->text().trimmed();
    double newScore = m_scoreEdit->text().toDouble();

    if (courseId.isEmpty() || studentId.isEmpty()) {
        QMessageBox::warning(this, "�������", "��ѡ��γ̲�����ѧ��");
        return;
    }

    // ��鵱ǰ�û��Ƿ���Ȩ�ޣ���ʦֻ��Ϊ�Լ��Ŀγ̸��³ɼ���
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
            QMessageBox::warning(this, "Ȩ�޴���", "�������޸����ſγ̵ĳɼ�");
            return;
        }
    }

    if (m_courseMgr->updateScore(studentId, courseId, newScore)) {
        QMessageBox::information(this, "�ɹ�", "�ɼ����³ɹ�");
        queryScores(); // ˢ�²�ѯ���
    }
    else {
        QMessageBox::critical(this, "����", "���³ɼ�ʧ��");
    }
}

void ScoreForm::deleteScore() {
    QModelIndexList selected = m_scoreView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "����", "��ѡ��Ҫɾ���ĳɼ�");
        return;
    }

    QModelIndex index = selected.first();
    QString studentId = m_scoreView->model()->data(m_scoreView->model()->index(index.row(), 0)).toString();
    QString courseId = m_courseCombo->currentData().toString();

    // ���Ȩ��
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
            QMessageBox::warning(this, "Ȩ�޴���", "������ɾ�����ſγ̵ĳɼ�");
            return;
        }
    }

    // ����ȷ��
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "ȷ��ɾ��",
        QString("ȷ��Ҫɾ��ѧ��Ϊ %1 �ĳɼ���").arg(studentId),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        if (m_courseMgr->updateScore(studentId, courseId, -1)) { // ʹ��-1��ʾɾ��
            QMessageBox::information(this, "�ɹ�", "�ɼ���ɾ��");
            queryScores(); // ˢ�²�ѯ���
        }
        else {
            QMessageBox::critical(this, "����", "ɾ���ɼ�ʧ��");
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