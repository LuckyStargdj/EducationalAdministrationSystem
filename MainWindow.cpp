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

    // 加载保存的窗口状态
    QSettings settings;
    restoreGeometry(settings.value("windowGeometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());

    // 如果用户已登录，更新UI
    if (m_userMgr->currentRole() != UserManager::Role(-1)) {
        onLoginSuccess();
    }
}

MainWindow::~MainWindow() {
    // 保存状态
    QSettings settings;
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::setupUI() {
    setWindowTitle("教务管理系统");
    resize(1024, 768);

    // 创建堆叠窗口
    m_stackedWidget = new QStackedWidget(this);
    setCentralWidget(m_stackedWidget);

    // 创建仪表盘页面
    m_dashboard = new QWidget(this);
    QVBoxLayout* dashboardLayout = new QVBoxLayout(m_dashboard);

    // 标题标签
    QLabel* titleLabel = new QLabel("教务管理系统", m_dashboard);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(24);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);

    // 添加部件到布局
    dashboardLayout->addStretch();
    dashboardLayout->addWidget(titleLabel);
    dashboardLayout->addStretch();

    m_dashboard->setLayout(dashboardLayout);
    m_stackedWidget->addWidget(m_dashboard);

    // 创建状态栏
    m_statusBar = statusBar();
    m_userInfoLabel = new QLabel("未登录");
    m_dbStatusLabel = new QLabel("数据库状态: 未知");
    m_lastLoginLabel = new QLabel("最后登录: 无记录");

    // 在标签之间添加分隔符
    QLabel* separator1 = new QLabel("|");
    QLabel* separator2 = new QLabel("|");

    // 添加到状态栏
    m_statusBar->addPermanentWidget(m_userInfoLabel);
    m_statusBar->addPermanentWidget(separator1);
    m_statusBar->addPermanentWidget(m_dbStatusLabel);
    m_statusBar->addPermanentWidget(separator2);
    m_statusBar->addPermanentWidget(m_lastLoginLabel);

    // 更新状态栏
    updateStatusBar();

    // 创建菜单和工具栏
    setupMenu();
    setupToolbar();
}

void MainWindow::setupMenu() {
    // 文件菜单
    QMenu* fileMenu = menuBar()->addMenu("文件");

    QAction* exportAction = new QAction(QIcon("./icons/export.png"), "导出数据", this);
    QAction* importAction = new QAction(QIcon("./icons/import.png"), "导入数据", this);
    QAction* logoutAction = new QAction("注销", this);
    QAction* exitAction = new QAction("退出", this);

    fileMenu->addAction(exportAction);
    fileMenu->addAction(importAction);
    fileMenu->addSeparator();
    fileMenu->addAction(logoutAction);
    fileMenu->addSeparator();
    fileMenu->addAction(exitAction);

    // 管理菜单
    QMenu* manageMenu = menuBar()->addMenu("管理");

    QAction* studentAction = new QAction(QIcon("./icons/students.png"), "学生管理", this);
    QAction* courseAction = new QAction(QIcon("./icons/courses.png"), "课程管理", this);
    QAction* scoreAction = new QAction(QIcon("./icons/scores.png"), "成绩管理", this);
    QAction* reportAction = new QAction(QIcon("./icons/reports.png"), "报表统计", this);

    manageMenu->addAction(studentAction);
    manageMenu->addAction(courseAction);
    manageMenu->addAction(scoreAction);
    manageMenu->addSeparator();
    manageMenu->addAction(reportAction);

    // 系统菜单
    QMenu* sysMenu = menuBar()->addMenu("系统");

    QAction* adminAction = new QAction(QIcon("./icons/system.png"), "系统管理", this);
    QAction* aboutAction = new QAction(QIcon("./icons/about.png"), "关于", this);

    sysMenu->addAction(adminAction);
    sysMenu->addSeparator();
    sysMenu->addAction(aboutAction);

    // 连接信号槽
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

    // 根据用户角色禁用菜单
    adminAction->setEnabled(false);
}

void MainWindow::setupToolbar() {
    QToolBar* toolbar = addToolBar("主工具栏");

    QAction* profileAction = new QAction(QIcon("./icons/profile.png"), "个人资料", this);
    QAction* studentAction = new QAction(QIcon("./icons/students.png"), "学生管理", this);
    QAction* courseAction = new QAction(QIcon("./icons/courses.png"), "课程管理", this);
    QAction* scoreAction = new QAction(QIcon("./icons/scores.png"), "成绩管理", this);
    QAction* reportAction = new QAction(QIcon("./icons/reports.png"), "报表统计", this);

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
        // 已登录状态
        QString roleText;
        switch (m_userMgr->currentRole()) {
        case UserManager::Admin: roleText = "管理员"; break;
        case UserManager::Teacher: roleText = "教师"; break;
        case UserManager::Student: roleText = "学生"; break;
        }

        QString userInfo = QString("用户: %1 (%2)").arg(m_userMgr->currentUserName(), roleText);
        m_userInfoLabel->setText(userInfo);

        QString dbStatus = m_dbMgr->isConnected() ? "数据库: 已连接" : "数据库: 未连接";
        m_dbStatusLabel->setText(dbStatus);

        QString lastLogin = QString("最后登录: %1").arg(m_userMgr->getLastLoginTime(m_userMgr->currentUserId()));
        m_lastLoginLabel->setText(lastLogin);
    }
    else {
        // 未登录状态
        m_userInfoLabel->setText("未登录");
        m_dbStatusLabel->setText(QString("数据库: %1").arg(
            m_dbMgr->isConnected() ? "已连接" : "未连接"));
        m_lastLoginLabel->setText("最后登录: 无记录");
    }
}

void MainWindow::onDbConnectionChanged(bool connected) {
    updateStatusBar();
}

void MainWindow::onLoginSuccess() {
    // 更新仪表盘
    QString welcome = QString("欢迎 %1! (%2)").arg(
        m_userMgr->currentUserName(),
        m_userMgr->currentRole() == UserManager::Admin ? "管理员" :
        m_userMgr->currentRole() == UserManager::Teacher ? "教师" : "学生");

    QLabel* label = m_dashboard->findChild<QLabel*>();
    if (label) {
        label->setText(welcome);
    }

    // 启用菜单选项
    QAction* adminAction = menuBar()->actions().at(2)->menu()->actions().at(0);
    adminAction->setEnabled(m_userMgr->currentRole() == UserManager::Admin);

    // 更新状态栏
    updateStatusBar();
}

void MainWindow::onLogout() {
    // 重置用户状态
    m_userMgr = new UserManager(this);

    // 清除所有管理页面
    delete m_studentPage; m_studentPage = nullptr;
    delete m_coursePage; m_coursePage = nullptr;
    delete m_scorePage; m_scorePage = nullptr;
    delete m_reportPage; m_reportPage = nullptr;
    delete m_adminPage; m_adminPage = nullptr;

    // 重置仪表盘
    m_dashboard->findChild<QLabel*>()->setText("教务管理系统");

    // 禁用管理员菜单
    QAction* adminAction = menuBar()->actions().at(2)->menu()->actions().at(0);
    adminAction->setEnabled(false);

    // 返回登录界面
    m_stackedWidget->setCurrentWidget(m_dashboard);
    updateStatusBar();
}

void MainWindow::showUserProfile() {
    if (m_userMgr->currentRole() == UserManager::Role(-1)) {
        QMessageBox::information(this, "未登录", "请先登录系统");
        return;
    }

    QDialog dialog(this);
    dialog.setWindowTitle("个人资料");
    dialog.setFixedSize(400, 300);

    QFormLayout layout(&dialog);

    // 姓名
    QLineEdit* nameEdit = new QLineEdit(m_userMgr->currentUserName(), &dialog);

    // 学号/工号（只读）
    QLineEdit* idEdit = new QLineEdit(m_userMgr->currentStudentId(), &dialog);
    idEdit->setReadOnly(true);

    // 角色（只读）
    QLineEdit* roleEdit = new QLineEdit(&dialog);
    roleEdit->setReadOnly(true);
    switch (m_userMgr->currentRole()) {
    case UserManager::Admin: roleEdit->setText("管理员"); break;
    case UserManager::Teacher: roleEdit->setText("教师"); break;
    case UserManager::Student: roleEdit->setText("学生"); break;
    }

    // 班级
    QLineEdit* classEdit = new QLineEdit(m_userMgr->currentClass(), &dialog);

    // 最后登录
    QLineEdit* lastLoginEdit = new QLineEdit(&dialog);
    lastLoginEdit->setReadOnly(true);
    lastLoginEdit->setText(m_userMgr->getLastLoginTime(m_userMgr->currentUserId()));

    // 按钮
    QPushButton* saveButton = new QPushButton("保存", &dialog);

    layout.addRow("姓名:", nameEdit);
    layout.addRow("学号/工号:", idEdit);
    layout.addRow("角色:", roleEdit);
    layout.addRow("班级:", classEdit);
    layout.addRow("最后登录:", lastLoginEdit);
    layout.addRow(saveButton);

    connect(saveButton, &QPushButton::clicked, [&] {
        // 更新用户信息
        m_userMgr->updateUserInfo(m_userMgr->currentUserId(), "name", nameEdit->text());
        m_userMgr->updateUserInfo(m_userMgr->currentUserId(), "class_name", classEdit->text());

        // 更新当前用户名和班级
        UserManager* um = const_cast<UserManager*>(m_userMgr);
        um->setCurrentUserName(nameEdit->text());
        um->setCurrentClassName(classEdit->text());

        updateStatusBar();
        QMessageBox::information(this, "成功", "个人信息已更新");
        dialog.accept();
        });

    dialog.exec();
}

void MainWindow::showStudentManagement() {
    if (m_userMgr->currentRole() == UserManager::Role(-1)) {
        QMessageBox::information(this, "未登录", "请先登录系统");
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
        QMessageBox::information(this, "未登录", "请先登录系统");
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
        QMessageBox::information(this, "未登录", "请先登录系统");
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
        QMessageBox::information(this, "未登录", "请先登录系统");
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
        QMessageBox::warning(this, "权限不足", "只有管理员可以访问系统管理");
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
        this, "导出数据",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "CSV 文件 (*.csv);;所有文件 (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件: " + file.errorString());
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // 根据当前活动页面导出不同数据
    QWidget* currentWidget = m_stackedWidget->currentWidget();

    if (currentWidget == m_studentPage) {
        // 导出学生数据
        out << "学号,姓名,性别,班级,入学年份" << "\n";

        QSqlQuery query("SELECT student_id, name, gender, class_name, enrollment_year "
            "FROM users WHERE role = 2");
        while (query.next()) {
            out << query.value(0).toString() << ","
                << query.value(1).toString() << ","
                << (query.value(2).toInt() == 0 ? "男" : "女") << ","
                << query.value(3).toString() << ","
                << query.value(4).toString() << "\n";
        }
    }
    else if (currentWidget == m_coursePage) {
        // 导出课程数据
        out << "课程编号,课程名称,学分,教师ID,开课日期,学期" << "\n";

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
        // 导出成绩数据
        out << "学号,课程编号,分数" << "\n";

        QSqlQuery query("SELECT * FROM scores");
        while (query.next()) {
            out << query.value("student_id").toString() << ","
                << query.value("course_id").toString() << ","
                << query.value("score").toString() << "\n";
        }
    }
    else {
        // 导出全部用户数据
        out << "ID,姓名,角色,学号/工号,状态,最后登录" << "\n";

        QSqlQuery query("SELECT id, name, role, student_id, status, last_login FROM users");
        while (query.next()) {
            QString roleStr;
            switch (query.value("role").toInt()) {
            case UserManager::Admin: roleStr = "管理员"; break;
            case UserManager::Teacher: roleStr = "教师"; break;
            case UserManager::Student: roleStr = "学生"; break;
            }

            out << query.value("id").toString() << ","
                << query.value("name").toString() << ","
                << roleStr << ","
                << query.value("student_id").toString() << ","
                << (query.value("status").toInt() == 1 ? "激活" : "冻结") << ","
                << query.value("last_login").toString() << "\n";
        }
    }

    file.close();
    QMessageBox::information(this, "导出成功", "数据已成功导出到: " + fileName);
}

void MainWindow::importData() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "导入数据",
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        "CSV 文件 (*.csv);;所有文件 (*)");

    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件: " + file.errorString());
        return;
    }

    // TODO: 实现具体的导入逻辑
    // 这里只是一个框架实现

    int rowCount = 0;
    QTextStream in(&file);
    in.setCodec("UTF-8");

    // 跳过标题行
    if (!in.atEnd()) in.readLine();

    while (!in.atEnd()) {
        QString line = in.readLine();
        // 处理每行数据
        rowCount++;
    }

    file.close();
    QMessageBox::information(this, "导入完成",
        QString("成功导入 %1 条数据").arg(rowCount));

    // 刷新当前页面
    if (m_studentPage && m_stackedWidget->currentWidget() == m_studentPage) {
        m_studentPage->refresh();
    }
    else if (m_coursePage && m_stackedWidget->currentWidget() == m_coursePage) {
        m_coursePage->refresh();
    }
}

void MainWindow::about() {
    QString aboutText = QString(
        "<h2>教务管理系统</h2>"
        "<p>版本: 1.0.0</p>"
        "<p>作者: 管理系统开发团队</p>"
        "<p>最后编译时间: %1 %2</p>"
        "<p>描述: 本系统支持学生、教师和管理员三种角色，提供学生管理、课程管理、"
        "成绩管理和统计报表等功能。</p>"
        "<p>支持: 联系邮箱 support@administration.edu</p>"
    ).arg(__DATE__, __TIME__);

    QMessageBox::about(this, "关于教务管理系统", aboutText);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // 保存当前状态
    QSettings settings;
    settings.setValue("windowGeometry", saveGeometry());
    settings.setValue("windowState", saveState());

    QMainWindow::closeEvent(event);
}