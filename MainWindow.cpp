#include "MainWindow.h"
#include <QMenuBar>
#include <QToolBar>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QGroupBox>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QInputDialog>
#include <QTextStream>

MainWindow::MainWindow(UserManager* userMgr, DatabaseManager* dbMgr,
    CourseManager* courseMgr, QWidget* parent)
    : QMainWindow(parent),
    m_userMgr(userMgr),
    m_dbMgr(dbMgr),
    m_courseMgr(courseMgr) {
    setupUI();
    connect(m_dbMgr, &DatabaseManager::connectionChanged, this, &MainWindow::onDbConnectionChanged);

    // ���ر���Ĵ���״̬
    QSettings settings;
    restoreGeometry(settings.value("windowGeometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    // ����û��ѵ�¼������UI
    if (m_userMgr->currentRole() != UserManager::Role(-1)) {
        onLoginSuccess();
    }
}

MainWindow::~MainWindow() {
    // ����״̬
    QSettings settings;
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::setupUI() {
    setWindowTitle("�������ϵͳ");
    resize(1024, 768);

    // �����ѵ�����
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // �����Ǳ���ҳ��
    m_dashboard = new QWidget(this);
    QVBoxLayout* dashboardLayout = new QVBoxLayout(m_dashboard);

    // �����ǩ
    QLabel* titleLabel = new QLabel("�������ϵͳ", m_dashboard);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // ��Ӳ���������
    dashboardLayout->addStretch();
    dashboardLayout->addWidget(titleLabel);
    dashboardLayout->addStretch();

    m_dashboard->setLayout(dashboardLayout);
    m_stackedWidget->addWidget(m_dashboard);

    // ����״̬��
    m_statusBar = statusBar();
    m_userInfoLabel = new QLabel("δ��¼");
    m_dbStatusLabel = new QLabel("���ݿ�״̬: δ֪");
    m_lastLoginLabel = new QLabel("����¼: �޼�¼");

    // �ڱ�ǩ֮����ӷָ���
    QLabel* separator1 = new QLabel("|");
    QLabel* separator2 = new QLabel("|");

    // ��ӵ�״̬��
    m_statusBar->addPermanentWidget(m_userInfoLabel);
    m_statusBar->addPermanentWidget(separator1);
    m_statusBar->addPermanentWidget(m_dbStatusLabel);
    m_statusBar->addPermanentWidget(separator2);
    m_statusBar->addPermanentWidget(m_lastLoginLabel);

    // ����״̬��
    updateStatusBar();

    // �����˵��͹�����
    setupMenu();
    setupToolbar();
}

void MainWindow::setupMenu() {
    // �ļ��˵�
    QMenu* fileMenu = menuBar()->addMenu("�ļ�");

    QAction* exportAction = new QAction(QIcon("./icons/export.png"), "��������", this);
    QAction* importAction = new QAction(QIcon("./icons/import.png"), "��������", this);
    QAction* logoutAction = new QAction(QIcon("./icons/logout.png"), "ע��", this);
    QAction* exitAction = new QAction(QIcon("./icons/exit.png"), "�˳�", this);

    fileMenu->addAction(exportAction);
    fileMenu->addAction(importAction);
    fileMenu->addSeparator();
    fileMenu->addAction(logoutAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    // ����˵�
    QMenu* manageMenu = menuBar()->addMenu("����");

    QAction* studentAction = new QAction(QIcon("./icons/students.png"), "ѧ������", this);
    QAction* courseAction = new QAction(QIcon("./icons/courses.png"), "�γ̹���", this);
    QAction* scoreAction = new QAction(QIcon("./icons/scores.png"), "�ɼ�����", this);
    QAction* reportAction = new QAction(QIcon("./icons/reports.png"), "����ͳ��", this);

    manageMenu->addAction(studentAction);
    manageMenu->addAction(courseAction);
    manageMenu->addAction(scoreAction);
    manageMenu->addSeparator();
    manageMenu->addAction(reportAction);

    // ϵͳ�˵�
    QMenu* sysMenu = menuBar()->addMenu("ϵͳ");

    QAction* adminAction = new QAction(QIcon("./icons/system.png"), "ϵͳ����", this);
    QAction* aboutAction = new QAction(QIcon("./icons/about.png"), "����", this);

    sysMenu->addAction(adminAction);
    sysMenu->addSeparator();
    sysMenu->addAction(aboutAction);

    // �����źŲ�
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportData);
    connect(importAction, &QAction::triggered, this, &MainWindow::importData);
    connect(logoutAction, &QAction::triggered, this, &MainWindow::onLogout);
    connect(exitAction, &QAction::triggered, this, &QMainWindow::close);

    connect(studentAction, &QAction::triggered, this, &MainWindow::showStudentManagement);
    connect(courseAction, &QAction::triggered, this, &MainWindow::showCourseManagement);
    connect(scoreAction, &QAction::triggered, this, &MainWindow::showScoreManagement);
    connect(reportAction, &QAction::triggered, this, &MainWindow::showReportGeneration);

    connect(adminAction, &QAction::triggered, this, &MainWindow::showAdminPanel);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::about);

    // �����û���ɫ���ò˵�
    adminAction->setEnabled(false);
}

void MainWindow::setupToolbar() {
    QToolBar* toolbar = addToolBar("��������");

    QAction* profileAction = new QAction(QIcon("./icons/profile.png"), "��������", this);
    QAction* studentAction = new QAction(QIcon("./icons/students.png"), "ѧ������", this);
    QAction* courseAction = new QAction(QIcon("./icons/courses.png"), "�γ̹���", this);
    QAction* scoreAction = new QAction(QIcon("./icons/scores.png"), "�ɼ�����", this);
    QAction* reportAction = new QAction(QIcon("./icons/reports.png"), "����ͳ��", this);

    toolbar->addAction(profileAction);
    toolbar->addSeparator();
    toolbar->addAction(studentAction);
    toolbar->addAction(courseAction);
    toolbar->addAction(scoreAction);
    toolbar->addAction(reportAction);

    connect(profileAction, &QAction::triggered, this, &MainWindow::showUserProfile);
    connect(studentAction, &QAction::triggered, this, &MainWindow::showStudentManagement);
    connect(courseAction, &QAction::triggered, this, &MainWindow::showCourseManagement);
    connect(scoreAction, &QAction::triggered, this, &MainWindow::showScoreManagement);
    connect(reportAction, &QAction::triggered, this, &MainWindow::showReportGeneration);
}

void MainWindow::updateStatusBar() {
    if (m_userMgr->currentRole() != UserManager::Role(-1)) {
        // �ѵ�¼״̬
        QString roleText;
        switch (m_userMgr->currentRole()) {
        case UserManager::Admin: roleText = "����Ա"; break;
        case UserManager::Teacher: roleText = "��ʦ"; break;
        case UserManager::Student: roleText = "ѧ��"; break;
        }

        QString userInfo = QString("�û�: %1 (%2)").arg(m_userMgr->currentUserName(), roleText);
        m_userInfoLabel->setText(userInfo);

        QString dbStatus = m_dbMgr->isConnected() ? "���ݿ�: ������" : "���ݿ�: δ����";
        m_dbStatusLabel->setText(dbStatus);

        QString lastLogin = QString("����¼: %1").arg(m_userMgr->getLastLoginTime(m_userMgr->currentUserId()));
        m_lastLoginLabel->setText(lastLogin);
    }
    else {
        // δ��¼״̬
        m_userInfoLabel->setText("δ��¼");
        m_dbStatusLabel->setText(QString("���ݿ�: %1").arg(
            m_dbMgr->isConnected() ? "������" : "δ����"));
        m_lastLoginLabel->setText("����¼: �޼�¼");
    }
}

void MainWindow::onDbConnectionChanged(bool connected) {
    updateStatusBar();
}

void MainWindow::onLoginSuccess() {
    // �����Ǳ���
    QString welcome = QString("��ӭ %1! (%2)").arg(
        m_userMgr->currentUserName(),
        m_userMgr->currentRole() == UserManager::Admin ? "����Ա" :
        m_userMgr->currentRole() == UserManager::Teacher ? "��ʦ" : "ѧ��");

    QLabel* label = m_dashboard->findChild<QLabel*>();
    if (label) {
        label->setText(welcome);
    }

    // ���ò˵�ѡ��
    QAction* adminAction = menuBar()->actions().at(2)->menu()->actions().at(0);
    adminAction->setEnabled(m_userMgr->currentRole() == UserManager::Admin);

    // ����״̬��
    updateStatusBar();
}

void MainWindow::onLogout() {
    // �����û�״̬
    m_userMgr = new UserManager(this);

    // ������й���ҳ��
    delete m_studentPage; m_studentPage = nullptr;
    delete m_coursePage; m_coursePage = nullptr;
    delete m_scorePage; m_scorePage = nullptr;
    delete m_reportPage; m_reportPage = nullptr;
    delete m_adminPage; m_adminPage = nullptr;

    // �����Ǳ���
    m_dashboard->findChild<QLabel*>()->setText("�������ϵͳ");

    // ���ù���Ա�˵�
    QAction* adminAction = menuBar()->actions().at(2)->menu()->actions().at(0);
    adminAction->setEnabled(false);

    // ���ص�¼����
    m_stackedWidget->setCurrentWidget(m_dashboard);
    updateStatusBar();
}

void MainWindow::showUserProfile() {
    if (m_userMgr->currentRole() == UserManager::Role(-1)) {
        QMessageBox::information(this, "δ��¼", "���ȵ�¼ϵͳ");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("��������");
    dialog.setFixedSize(400, 300);

    QFormLayout layout(&dialog);

    // ����
    QLineEdit* nameEdit = new QLineEdit(m_userMgr->currentUserName(), &dialog);

    // ѧ��/���ţ�ֻ����
    QLineEdit* idEdit = new QLineEdit(m_userMgr->currentStudentId(), &dialog);
    idEdit->setReadOnly(true);

    // ��ɫ��ֻ����
    QLineEdit* roleEdit = new QLineEdit(&dialog);
    roleEdit->setReadOnly(true);
    switch (m_userMgr->currentRole()) {
    case UserManager::Admin: roleEdit->setText("����Ա"); break;
    case UserManager::Teacher: roleEdit->setText("��ʦ"); break;
    case UserManager::Student: roleEdit->setText("ѧ��"); break;
    }

    // �༶
    QLineEdit* classEdit = new QLineEdit(m_userMgr->currentClass(), &dialog);

    // ����¼
    QLineEdit* lastLoginEdit = new QLineEdit(&dialog);
    lastLoginEdit->setReadOnly(true);
    lastLoginEdit->setText(m_userMgr->getLastLoginTime(m_userMgr->currentUserId()));

    // ��ť
    QPushButton* saveButton = new QPushButton("����", &dialog);

    layout.addRow("����:", nameEdit);
    layout.addRow("ѧ��/����:", idEdit);
    layout.addRow("��ɫ:", roleEdit);
    layout.addRow("�༶:", classEdit);
    layout.addRow("����¼:", lastLoginEdit);
    layout.addRow(saveButton);

    connect(saveButton, &QPushButton::clicked, [&] {
        // �����û���Ϣ
        m_userMgr->updateUserInfo(m_userMgr->currentUserId(), "name", nameEdit->text());
        m_userMgr->updateUserInfo(m_userMgr->currentUserId(), "class_name", classEdit->text());

        // ���µ�ǰ�û����Ͱ༶
        UserManager* um = const_cast<UserManager*>(m_userMgr);
        um->setCurrentUserName(nameEdit->text());
        um->setCurrentClassName(classEdit->text());

        updateStatusBar();
        QMessageBox::information(this, "�ɹ�", "������Ϣ�Ѹ���");
        dialog.accept();
        });

    dialog.exec();
}

void MainWindow::showStudentManagement() {
    if (m_userMgr->currentRole() == UserManager::Role(-1)) {
        QMessageBox::information(this, "δ��¼", "���ȵ�¼ϵͳ");
        return;
    }

    if (!m_studentPage) {
        createStudentManagementPage();
    }
    m_stackedWidget->setCurrentWidget(m_studentPage);
}

void MainWindow::createStudentManagementPage() {
    m_studentPage = new StudentForm(m_userMgr, m_dbMgr, this);
    m_stackedWidget->addWidget(m_studentPage);
}

void MainWindow::showCourseManagement() {
    if (m_userMgr->currentRole() == UserManager::Role(-1)) {
        QMessageBox::information(this, "δ��¼", "���ȵ�¼ϵͳ");
        return;
    }

    if (!m_coursePage) {
        createCourseManagementPage();
    }
    m_stackedWidget->setCurrentWidget(m_coursePage);
}

void MainWindow::createCourseManagementPage() {
    m_coursePage = new CourseForm(m_userMgr, m_courseMgr, this);
    m_stackedWidget->addWidget(m_coursePage);
}

void MainWindow::showScoreManagement() {
    if (m_userMgr->currentRole() == UserManager::Role(-1)) {
        QMessageBox::information(this, "δ��¼", "���ȵ�¼ϵͳ");
        return;
    }

    if (!m_scorePage) {
        createScoreManagementPage();
    }
    m_stackedWidget->setCurrentWidget(m_scorePage);
}

void MainWindow::createScoreManagementPage() {
    m_scorePage = new ScoreForm(m_userMgr, m_courseMgr, this);
    m_stackedWidget->addWidget(m_scorePage);
}

void MainWindow::showReportGeneration() {
    if (m_userMgr->currentRole() == UserManager::Role(-1)) {
        QMessageBox::information(this, "δ��¼", "���ȵ�¼ϵͳ");
        return;
    }

    if (!m_reportPage) {
        createReportPage();
    }
    m_stackedWidget->setCurrentWidget(m_reportPage);
}

void MainWindow::createReportPage() {
    m_reportPage = new ReportForm(m_dbMgr, m_courseMgr, this);
    m_stackedWidget->addWidget(m_reportPage);
}

void MainWindow::showAdminPanel() {
    if (m_userMgr->currentRole() != UserManager::Admin) {
        QMessageBox::warning(this, "Ȩ�޲���", "ֻ�й���Ա���Է���ϵͳ����");
        return;
    }

    if (!m_adminPage) {
        createAdminPage();
    }
    m_stackedWidget->setCurrentWidget(m_adminPage);
}

void MainWindow::createAdminPage() {
    m_adminPage = new AdminForm(m_userMgr, m_dbMgr, this);
    m_stackedWidget->addWidget(m_adminPage);
}

void MainWindow::exportData() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "��������",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "CSV �ļ� (*.csv);;�����ļ� (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "����", "�޷������ļ�: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // ���ݵ�ǰ�ҳ�浼����ͬ����
    QWidget* currentWidget = m_stackedWidget->currentWidget();

    if (currentWidget == m_studentPage) {
        // ����ѧ������
        out << "ѧ��,����,�Ա�,�༶,��ѧ���" << "\n";

        QSqlQuery query("SELECT student_id, name, gender, class_name, enrollment_year "
            "FROM users WHERE role = 2");
        while (query.next()) {
            out << query.value(0).toString() << ","
                << query.value(1).toString() << ","
                << (query.value(2).toInt() == 0 ? "��" : "Ů") << ","
                << query.value(3).toString() << ","
                << query.value(4).toString() << "\n";
        }
    }
    else if (currentWidget == m_coursePage) {
        // �����γ�����
        out << "�γ̱��,�γ�����,ѧ��,��ʦID,��������,ѧ��" << "\n";

        QSqlQuery query("SELECT * FROM courses");
        while (query.next()) {
            out << query.value("id").toString() << ","
                << query.value("name").toString() << ","
                << query.value("credit").toString() << ","
                << query.value("teacher_id").toString() << ","
                << query.value("start_date").toString() << ","
                << query.value("semester").toString() << "\n";
        }
    }
    else if (currentWidget == m_scorePage || currentWidget == m_reportPage) {
        // �����ɼ�����
        out << "ѧ��,�γ̱��,����" << "\n";

        QSqlQuery query("SELECT * FROM scores");
        while (query.next()) {
            out << query.value("student_id").toString() << ","
                << query.value("course_id").toString() << ","
                << query.value("score").toString() << "\n";
        }
    }
    else {
        // ����ȫ���û�����
        out << "ID,����,��ɫ,ѧ��/����,״̬,����¼" << "\n";

        QSqlQuery query("SELECT id, name, role, student_id, status, last_login FROM users");
        while (query.next()) {
            QString roleStr;
            switch (query.value("role").toInt()) {
            case UserManager::Admin: roleStr = "����Ա"; break;
            case UserManager::Teacher: roleStr = "��ʦ"; break;
            case UserManager::Student: roleStr = "ѧ��"; break;
            }

            out << query.value("id").toString() << ","
                << query.value("name").toString() << ","
                << roleStr << ","
                << query.value("student_id").toString() << ","
                << (query.value("status").toInt() == 1 ? "����" : "����") << ","
                << query.value("last_login").toString() << "\n";
        }
    }

    file.close();
    QMessageBox::information(this, "�����ɹ�", "�����ѳɹ�������: " + fileName);
}

void MainWindow::importData() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "��������",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "CSV �ļ� (*.csv);;�����ļ� (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "����", "�޷����ļ�: " + file.errorString());
        return;
    }

    // TODO: ʵ�־���ĵ����߼�
    // ����ֻ��һ�����ʵ��

    int rowCount = 0;
    QTextStream in(&file);
    in.setCodec("UTF-8");

    // ����������
    if (!in.atEnd()) in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine();
        // ����ÿ������
        rowCount++;
    }

    file.close();
    QMessageBox::information(this, "�������",
        QString("�ɹ����� %1 ������").arg(rowCount));

    // ˢ�µ�ǰҳ��
    if (m_studentPage && m_stackedWidget->currentWidget() == m_studentPage) {
        m_studentPage->refresh();
    }
    else if (m_coursePage && m_stackedWidget->currentWidget() == m_coursePage) {
        m_coursePage->refresh();
    }
}

void MainWindow::about() {
    QString aboutText = QString(
        "<h2>�������ϵͳ</h2>"
        "<p>�汾: 1.0.0</p>"
        "<p>����: ����ϵͳ�����Ŷ�</p>"
        "<p>������ʱ��: %1 %2</p>"
        "<p>����: ��ϵͳ֧��ѧ������ʦ�͹���Ա���ֽ�ɫ���ṩѧ�������γ̹���"
        "�ɼ������ͳ�Ʊ���ȹ��ܡ�</p>"
        "<p>֧��: ��ϵ���� support@administration.edu</p>"
    ).arg(__DATE__, __TIME__);

    QMessageBox::about(this, "���ڽ������ϵͳ", aboutText);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // ���浱ǰ״̬
    QSettings settings;
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("windowState", saveState());

    QMainWindow::closeEvent(event);
}