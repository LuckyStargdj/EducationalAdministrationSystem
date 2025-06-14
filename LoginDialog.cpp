#include "LoginDialog.h"
#include "RegisterDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QFont>
#include <QPalette>

LoginDialog::LoginDialog(UserManager* userMgr, QWidget* parent)
    : QDialog(parent), m_userManager(userMgr), m_captchaValue(0), m_failCount(0) {
    setWindowTitle("教务管理系统 - 登录");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(500, 400);
    setupUI();
    connect(m_userManager, &UserManager::loginStatusChanged, this, &LoginDialog::handleLoginResult);
    generateCaptcha();
}

LoginDialog::LoginDialog(UserManager* userMgr, const QString& username, QWidget* parent)
    : QDialog(parent), m_userManager(userMgr), m_username(username), m_captchaValue(0), m_failCount(0) {
    setWindowTitle("教务管理系统 - 登录");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(500, 400);
    setupUI();
    m_usernameEdit->setText(m_username);
    connect(m_userManager, &UserManager::loginStatusChanged, this, &LoginDialog::handleLoginResult);
    generateCaptcha();
}

void LoginDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 标题
    QLabel* titleLabel = new QLabel("教务管理系统", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    QGroupBox* formGroup = new QGroupBox("用户登录", this);
    QFormLayout* formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(20);

    // 用户名
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("请输入学号/工号");
    formLayout->addRow("用户名:", m_usernameEdit);

    // 密码
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("请输入密码");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("密码:", m_passwordEdit);

    // 验证码
    QHBoxLayout* captchaLayout = new QHBoxLayout();
    m_captchaLabel = new QLabel(this);
    m_captchaLabel->setFixedSize(100, 40);
    m_captchaLabel->setStyleSheet("background-color: white; border: 1px solid gray;");
    m_captchaLabel->setAlignment(Qt::AlignCenter);

    m_captchaEdit = new QLineEdit(this);
    m_captchaEdit->setFixedWidth(100);

    QPushButton* refreshBtn = new QPushButton("刷新", this);
    connect(refreshBtn, &QPushButton::clicked, this, &LoginDialog::generateCaptcha);

    captchaLayout->addWidget(m_captchaLabel);
    captchaLayout->addWidget(m_captchaEdit);
    captchaLayout->addWidget(refreshBtn);
    formLayout->addRow("验证码:", captchaLayout);

    formGroup->setLayout(formLayout);
    mainLayout->addWidget(formGroup);

    // 按钮布局
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_loginButton = new QPushButton("登录", this);
    m_loginButton->setFixedHeight(40);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);

    m_registerButton = new QPushButton("注册", this);
    m_registerButton->setFixedHeight(40);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginDialog::onRegisterClicked);

    buttonLayout->addWidget(m_loginButton);
    buttonLayout->addWidget(m_registerButton);
    mainLayout->addLayout(buttonLayout);
}

void LoginDialog::generateCaptcha() {
    m_captchaValue = QRandomGenerator::global()->bounded(1000, 10000);
    m_captchaLabel->setText(QString::number(m_captchaValue));
    m_captchaLabel->setFont(QFont("Arial", 20, QFont::Bold));
}

void LoginDialog::onLoginClicked() {
    m_username = m_usernameEdit->text().trimmed();
    QString password = m_passwordEdit->text();
    QString captcha = m_captchaEdit->text();

    if (m_username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "用户名和密码不能为空");
        return;
    }

    if (captcha.isEmpty() || captcha != QString::number(m_captchaValue)) {
        QMessageBox::warning(this, "验证码错误", "验证码输入错误，请重试");
        generateCaptcha();
        m_captchaEdit->clear();
        return;
    }

    // 尝试登录
    m_userManager->login(m_username, password);
}

void LoginDialog::onRegisterClicked() {
    RegisterDialog dlg(m_userManager, this);
    if (dlg.exec() == QDialog::Accepted) {
        m_usernameEdit->setText(dlg.username());
        m_passwordEdit->setFocus();
    }
}

void LoginDialog::handleLoginResult(bool success, const QString& message) {
    if (success) {
        m_failCount = 0;
        accept(); // 登录成功，关闭对话框
    }
    else {
        m_failCount++;
        generateCaptcha();
        m_captchaEdit->clear();

        if (m_failCount >= 5) {
            QMessageBox::critical(this, "账户锁定", "多次登录失败，账户已锁定！");
            reject(); // 关闭登录对话框
        }
        else {
            QMessageBox::warning(this, "登录失败", message);
        }
    }
}
