#include "ReportForm.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QStandardItemModel>
#include <QMessageBox>

ReportForm::ReportForm(DatabaseManager* dbMgr, CourseManager* courseMgr, QWidget* parent)
    : QWidget(parent), m_dbMgr(dbMgr), m_courseMgr(courseMgr) {
    setupUI();
}

void ReportForm::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ��������
    QGroupBox* paramGroup = new QGroupBox("�������");
    QFormLayout* paramLayout = new QFormLayout;

    m_courseCombo = new QComboBox;
    QVector<QVariantMap> courses = m_courseMgr->getAllCourses();
    for (const QVariantMap& course : courses) {
        m_courseCombo->addItem(course["name"].toString(), course["id"]);
    }

    m_classCombo = new QComboBox;
    // ��Ӱ༶ѡ�ʵ����Ӧ�����ݿ��ȡ��
    m_classCombo->addItem("�����һ��", "�����һ��");
    m_classCombo->addItem("���������", "���������");
    m_classCombo->addItem("�������һ��", "�������һ��");

    m_reportTypeCombo = new QComboBox;
    m_reportTypeCombo->addItem("�γ�ͳ��", "course");
    m_reportTypeCombo->addItem("�༶ͳ��", "class");
    m_reportTypeCombo->addItem("ѧ���ɼ���", "transcript");

    m_generateButton = new QPushButton("���ɱ���");
    connect(m_generateButton, &QPushButton::clicked, this, &ReportForm::generateReport);

    paramLayout->addRow("�γ�:", m_courseCombo);
    paramLayout->addRow("�༶:", m_classCombo);
    paramLayout->addRow("��������:", m_reportTypeCombo);
    paramLayout->addRow(m_generateButton);

    paramGroup->setLayout(paramLayout);
    mainLayout->addWidget(paramGroup);

    // ������ʾ����
    m_reportView = new QTableView;
    mainLayout->addWidget(m_reportView);
}

void ReportForm::generateReport() {
    QString reportType = m_reportTypeCombo->currentData().toString();

    QStandardItemModel* model = new QStandardItemModel(this);

    if (reportType == "course") {
        QString courseId = m_courseCombo->currentData().toString();
        QMap<QString, double> stats = m_courseMgr->calculateCourseStats(courseId);

        model->setHorizontalHeaderLabels({ "ͳ����", "��ֵ" });
        model->appendRow({ new QStandardItem("ƽ����"), new QStandardItem(QString::number(stats["average"])) });
        model->appendRow({ new QStandardItem("��׼��"), new QStandardItem(QString::number(stats["stddev"])) });
        model->appendRow({ new QStandardItem("������"), new QStandardItem(QString::number(stats["pass_rate"] * 100) + "%") });

    }
    else if (reportType == "class") {
        QString className = m_classCombo->currentData().toString();
        QMap<QString, double> stats = m_courseMgr->calculateClassStats(className);

        model->setHorizontalHeaderLabels({ "ͳ����", "��ֵ" });
        model->appendRow({ new QStandardItem("ƽ����"), new QStandardItem(QString::number(stats["average"])) });
        model->appendRow({ new QStandardItem("��׼��"), new QStandardItem(QString::number(stats["stddev"])) });
        model->appendRow({ new QStandardItem("������"), new QStandardItem(QString::number(stats["pass_rate"] * 100) + "%") });

    }
    else if (reportType == "transcript") {
        // ѧ���ɼ���ʵ�֣��ԣ�
    }

    m_reportView->setModel(model);
    m_reportView->horizontalHeader()->setStretchLastSection(true);
}
