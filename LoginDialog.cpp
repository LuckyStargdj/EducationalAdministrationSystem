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
    setWindowTitle("�������ϵͳ - ��¼");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(500, 400);
    setupUI();
    connect(m_userManager, &UserManager::loginStatusChanged, this, &LoginDialog::handleLoginResult);
    generateCaptcha();
}

LoginDialog::LoginDialog(UserManager* userMgr, const QString& username, QWidget* parent)
    : QDialog(parent), m_userManager(userMgr), m_username(username), m_captchaValue(0), m_failCount(0) {
    setWindowTitle("�������ϵͳ - ��¼");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(500, 400);
    setupUI();
    m_usernameEdit->setText(m_username);
    connect(m_userManager, &UserManager::loginStatusChanged, this, &LoginDialog::handleLoginResult);
    generateCaptcha();
}

void LoginDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ����
    QLabel* titleLabel = new QLabel("�������ϵͳ", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    QGroupBox* formGroup = new QGroupBox("�û���¼", this);
    QFormLayout* formLayout = new QFormLayout(formGroup);
    formLayout->setSpacing(20);

    // �û���
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("������ѧ��/����");
    formLayout->addRow("�û���:", m_usernameEdit);

    // ����
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setPlaceholderText("����������");
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("����:", m_passwordEdit);

    // ��֤��
    QHBoxLayout* captchaLayout = new QHBoxLayout();
    m_captchaLabel = new QLabel(this);
    m_captchaLabel->setFixedSize(100, 40);
    m_captchaLabel->setStyleSheet("background-color: white; border: 1px solid gray;");
    m_captchaLabel->setAlignment(Qt::AlignCenter);

    m_captchaEdit = new QLineEdit(this);
    m_captchaEdit->setFixedWidth(100);

    QPushButton* refreshBtn = new QPushButton("ˢ��", this);
    connect(refreshBtn, &QPushButton::clicked, this, &LoginDialog::generateCaptcha);

    captchaLayout->addWidget(m_captchaLabel);
    captchaLayout->addWidget(m_captchaEdit);
    captchaLayout->addWidget(refreshBtn);
    formLayout->addRow("��֤��:", captchaLayout);

    formGroup->setLayout(formLayout);
    mainLayout->addWidget(formGroup);

    // ��ť����
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    m_loginButton = new QPushButton("��¼", this);
    m_loginButton->setFixedHeight(40);
    connect(m_loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);

    m_registerButton = new QPushButton("ע��", this);
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
        QMessageBox::warning(this, "�������", "�û��������벻��Ϊ��");
        return;
    }

    if (captcha.isEmpty() || captcha != QString::number(m_captchaValue)) {
        QMessageBox::warning(this, "��֤�����", "��֤���������������");
        generateCaptcha();
        m_captchaEdit->clear();
        return;
    }

    // ���Ե�¼
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
        accept(); // ��¼�ɹ����رնԻ���
    }
    else {
        m_failCount++;
        generateCaptcha();
        m_captchaEdit->clear();

        if (m_failCount >= 5) {
            QMessageBox::critical(this, "�˻�����", "��ε�¼ʧ�ܣ��˻���������");
            reject(); // �رյ�¼�Ի���
        }
        else {
            QMessageBox::warning(this, "��¼ʧ��", message);
        }
    }
}
