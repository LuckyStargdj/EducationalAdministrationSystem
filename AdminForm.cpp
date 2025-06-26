#include "AdminForm.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QSqlQueryModel>
#include <QFileDialog>
#include <QTextStream>
#include <QLabel>
#include <QStandardItemModel>

AdminForm::AdminForm(UserManager* userMgr, DatabaseManager* dbMgr, QWidget* parent)
    : QWidget(parent), m_userMgr(userMgr), m_dbMgr(dbMgr) {
    setupUI();
}

void AdminForm::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 过滤区域
    QGroupBox* filterGroup = new QGroupBox("用户过滤");
    QHBoxLayout* filterLayout = new QHBoxLayout;

    m_roleCombo = new QComboBox;
    m_roleCombo->addItem("所有用户", -1);
    m_roleCombo->addItem("管理员", UserManager::Admin);
    m_roleCombo->addItem("教师", UserManager::Teacher);
    m_roleCombo->addItem("学生", UserManager::Student);
    connect(m_roleCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &AdminForm::refreshUsers);

    QPushButton* refreshButton = new QPushButton("刷新");
    connect(refreshButton, &QPushButton::clicked, this, &AdminForm::refreshUsers);

    filterLayout->addWidget(new QLabel("角色:"));
    filterLayout->addWidget(m_roleCombo);
    filterLayout->addWidget(refreshButton);
    filterLayout->addStretch();

    filterGroup->setLayout(filterLayout);
    mainLayout->addWidget(filterGroup);

    // 用户列表
    QGroupBox* userGroup = new QGroupBox("用户列表");
    QVBoxLayout* userLayout = new QVBoxLayout;

    m_userView = new QTableView;
    userLayout->addWidget(m_userView);
    userGroup->setLayout(userLayout);
    mainLayout->addWidget(userGroup, 1);

    // 操作按钮
    QHBoxLayout* actionLayout = new QHBoxLayout;

    m_freezeButton = new QPushButton("冻结账户");
    connect(m_freezeButton, &QPushButton::clicked, this, [this] { manageUserStatus(true); });

    m_activateButton = new QPushButton("激活账户");
    connect(m_activateButton, &QPushButton::clicked, this, [this] { manageUserStatus(false); });

    m_exportButton = new QPushButton("导出数据");
    connect(m_exportButton, &QPushButton::clicked, this, &AdminForm::exportData);

    actionLayout->addWidget(m_freezeButton);
    actionLayout->addWidget(m_activateButton);
    actionLayout->addWidget(m_exportButton);
    actionLayout->addStretch();

    mainLayout->addLayout(actionLayout);

    // 初始加载
    refreshUsers();
}

void AdminForm::refreshUsers() {
    int role = m_roleCombo->currentData().toInt();
    QVector<QVariantMap> users = m_userMgr->listUsers(static_cast<UserManager::Role>(role));

    QStandardItemModel* model = new QStandardItemModel(users.size(), 6, this);
    model->setHorizontalHeaderLabels({ "ID", "姓名", "角色", "学号/工号", "状态", "最后登录" });

    for (int i = 0; i < users.size(); i++) {
        QVariantMap user = users[i];
        model->setItem(i, 0, new QStandardItem(user["id"].toString()));
        model->setItem(i, 1, new QStandardItem(user["name"].toString()));

        QString roleStr;
        switch (user["role"].toInt()) {
        case UserManager::Admin: roleStr = "管理员"; break;
        case UserManager::Teacher: roleStr = "教师"; break;
        case UserManager::Student: roleStr = "学生"; break;
        }
        model->setItem(i, 2, new QStandardItem(roleStr));
        model->setItem(i, 3, new QStandardItem(user["student_id"].toString()));
        model->setItem(i, 4, new QStandardItem(user["status"].toInt() == 1 ? "激活" : "冻结"));
        model->setItem(i, 5, new QStandardItem(user["last_login"].toString()));
    }

    m_userView->setModel(model);
    m_userView->horizontalHeader()->setStretchLastSection(true);
}

void AdminForm::manageUserStatus(bool freeze) {
    QModelIndexList selected = m_userView->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择用户");
        return;
    }

    for (const QModelIndex& index : selected) {
        int userId = m_userView->model()->data(m_userView->model()->index(index.row(), 0)).toInt();

        if (freeze) {
            m_userMgr->freezeAccount(userId);
        }
        else {
            m_userMgr->activateAccount(userId);
        }
    }

    refreshUsers();
}

void AdminForm::exportData() {
    QString fileName = QFileDialog::getSaveFileName(this, "导出用户数据", "", "CSV 文件 (*.csv)");
    if (fileName.isEmpty()) return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法创建文件");
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    // 写入表头
    out << "ID,姓名,角色,学号/工号,状态,最后登录\n";

    // 写入数据
    QSqlQuery query;
    query.exec("SELECT id, name, role, student_id, status, last_login FROM users");

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

    file.close();
    QMessageBox::information(this, "导出成功", "用户数据已导出到CSV文件");
}
