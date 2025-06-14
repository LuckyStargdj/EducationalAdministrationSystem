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

    // 参数区域
    QGroupBox* paramGroup = new QGroupBox("报告参数");
    QFormLayout* paramLayout = new QFormLayout;

    m_courseCombo = new QComboBox;
    QVector<QVariantMap> courses = m_courseMgr->getAllCourses();
    for (const QVariantMap& course : courses) {
        m_courseCombo->addItem(course["name"].toString(), course["id"]);
    }

    m_classCombo = new QComboBox;
    // 添加班级选项（实际中应从数据库获取）
    m_classCombo->addItem("计算机一班", "计算机一班");
    m_classCombo->addItem("计算机二班", "计算机二班");
    m_classCombo->addItem("软件工程一班", "软件工程一班");

    m_reportTypeCombo = new QComboBox;
    m_reportTypeCombo->addItem("课程统计", "course");
    m_reportTypeCombo->addItem("班级统计", "class");
    m_reportTypeCombo->addItem("学生成绩单", "transcript");

    m_generateButton = new QPushButton("生成报告");
    connect(m_generateButton, &QPushButton::clicked, this, &ReportForm::generateReport);

    paramLayout->addRow("课程:", m_courseCombo);
    paramLayout->addRow("班级:", m_classCombo);
    paramLayout->addRow("报告类型:", m_reportTypeCombo);
    paramLayout->addRow(m_generateButton);

    paramGroup->setLayout(paramLayout);
    mainLayout->addWidget(paramGroup);

    // 报告显示区域
    m_reportView = new QTableView;
    mainLayout->addWidget(m_reportView);
}

void ReportForm::generateReport() {
    QString reportType = m_reportTypeCombo->currentData().toString();

    QStandardItemModel* model = new QStandardItemModel(this);

    if (reportType == "course") {
        QString courseId = m_courseCombo->currentData().toString();
        QMap<QString, double> stats = m_courseMgr->calculateCourseStats(courseId);

        model->setHorizontalHeaderLabels({ "统计项", "数值" });
        model->appendRow({ new QStandardItem("平均分"), new QStandardItem(QString::number(stats["average"])) });
        model->appendRow({ new QStandardItem("标准差"), new QStandardItem(QString::number(stats["stddev"])) });
        model->appendRow({ new QStandardItem("及格率"), new QStandardItem(QString::number(stats["pass_rate"] * 100) + "%") });

    }
    else if (reportType == "class") {
        QString className = m_classCombo->currentData().toString();
        QMap<QString, double> stats = m_courseMgr->calculateClassStats(className);

        model->setHorizontalHeaderLabels({ "统计项", "数值" });
        model->appendRow({ new QStandardItem("平均分"), new QStandardItem(QString::number(stats["average"])) });
        model->appendRow({ new QStandardItem("标准差"), new QStandardItem(QString::number(stats["stddev"])) });
        model->appendRow({ new QStandardItem("及格率"), new QStandardItem(QString::number(stats["pass_rate"] * 100) + "%") });

    }
    else if (reportType == "transcript") {
        // 学生成绩单实现（略）
    }

    m_reportView->setModel(model);
    m_reportView->horizontalHeader()->setStretchLastSection(true);
}
