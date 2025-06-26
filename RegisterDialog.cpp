#include "RegisterDialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QRegularExpression>
#include <QRandomGenerator>
#include <QPushButton>
#include <QIntValidator>
#include <QFont>

RegisterDialog::RegisterDialog(UserManager* userMgr, QWidget* parent)
    : QDialog(parent), m_userManager(userMgr), m_captchaValue(0) {
    setWindowTitle("�������ϵͳ - ע��");
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setFixedSize(500, 500);
    setupUI();
    generateCaptcha();
}

void RegisterDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // ����
    QLabel* titleLabel = new QLabel("�û�ע��", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    QGroupBox* formGroup = new QGroupBox(this);
    QFormLayout* formLayout = new QFormLayout(formGroup);

    // ѧ��/����
    m_usernameEdit = new QLineEdit(this);
    m_usernameEdit->setPlaceholderText("������ѧ��/���ţ�10λ���֣�");
    formLayout->addRow("ѧ��/����:", m_usernameEdit);

    // ����
    m_nameEdit = new QLineEdit(this);
    formLayout->addRow("����:", m_nameEdit);

    // ��ɫ
    m_roleCombo = new QComboBox(this);
    m_roleCombo->addItem("ѧ��", UserManager::Student);
    m_roleCombo->addItem("��ʦ", UserManager::Teacher);
    m_roleCombo->addItem("����Ա", UserManager::Admin);
    formLayout->addRow("��ɫ:", m_roleCombo);

    // �༶
    m_classEdit = new QLineEdit(this);
    formLayout->addRow("�༶:", m_classEdit);

    // ��ѧ���
    m_yearEdit = new QLineEdit(this);
    QIntValidator* yearValidator = new QIntValidator(2000, QDate::currentDate().year(), this);
    m_yearEdit->setValidator(yearValidator);
    m_yearEdit->setPlaceholderText("������4λ�������");
    formLayout->addRow("��ѧ���:", m_yearEdit);

    // ����
    m_passwordEdit = new QLineEdit(this);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("����:", m_passwordEdit);

    // ȷ������
    m_confirmPasswordEdit = new QLineEdit(this);
    m_confirmPasswordEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("ȷ������:", m_confirmPasswordEdit);

    // ��֤��
    QHBoxLayout* captchaLayout = new QHBoxLayout();
    m_captchaLabel = new QLabel(this);
    m_captchaLabel->setFixedSize(100, 40);
    m_captchaLabel->setStyleSheet("background-color: white; border: 1px solid gray;");
    m_captchaLabel->setAlignment(Qt::AlignCenter);

    m_captchaEdit = new QLineEdit(this);
    m_captchaEdit->setFixedWidth(100);

    QPushButton* refreshBtn = new QPushButton("ˢ��", this);
    connect(refreshBtn, &QPushButton::clicked, this, &RegisterDialog::generateCaptcha);

    captchaLayout->addWidget(m_captchaLabel);
    captchaLayout->addWidget(m_captchaEdit);
    captchaLayout->addWidget(refreshBtn);
    formLayout->addRow("��֤��:", captchaLayout);

    formGroup->setLayout(formLayout);
    mainLayout->addWidget(formGroup);

    // ע�ᰴť
    m_registerButton = new QPushButton("ע��", this);
    m_registerButton->setFixedHeight(40);
    connect(m_registerButton, &QPushButton::clicked, this, &RegisterDialog::onRegisterClicked);
    mainLayout->addWidget(m_registerButton);
}

void RegisterDialog::generateCaptcha() {
    m_captchaValue = QRandomGenerator::global()->bounded(1000, 10000);
    m_captchaLabel->setText(QString::number(m_captchaValue));
    m_captchaLabel->setFont(QFont("Arial", 20, QFont::Bold));
}

void RegisterDialog::onRegisterClicked() {
    QString username = m_usernameEdit->text().trimmed();
    QString name = m_nameEdit->text().trimmed();
    int roleIndex = m_roleCombo->currentIndex();
    UserManager::Role role = static_cast<UserManager::Role>(m_roleCombo->itemData(roleIndex).toInt());
    QString password = m_passwordEdit->text();
    QString confirmPassword = m_confirmPasswordEdit->text();
    QString className = m_classEdit->text().trimmed();
    int enrollmentYear = m_yearEdit->text().toInt();
    QString captcha = m_captchaEdit->text();

    // ������֤
    if (username.isEmpty() || name.isEmpty()) {
        QMessageBox::warning(this, "�������", "ѧ��/���ź���������Ϊ��");
        return;
    }

    if (password.isEmpty() || confirmPassword.isEmpty()) {
        QMessageBox::warning(this, "�������", "�����ȷ�����벻��Ϊ��");
        return;
    }

    if (password != confirmPassword) {
        QMessageBox::warning(this, "�������", "������������벻һ��");
        return;
    }

    if (role == UserManager::Student && className.isEmpty()) {
        QMessageBox::warning(this, "�������", "ѧ��������д�༶");
        return;
    }

    if (role == UserManager::Student && enrollmentYear == 0) {
        QMessageBox::warning(this, "�������", "ѧ��������д��ѧ���");
        return;
    }

    if (captcha.isEmpty() || captcha != QString::number(m_captchaValue)) {
        QMessageBox::warning(this, "��֤�����", "��֤���������������");
        generateCaptcha();
        m_captchaEdit->clear();
        return;
    }

    // ע���û�
    if (m_userManager->registerUser(name, role, username, className, enrollmentYear, password)) {
        m_username = username;
        accept();
        QMessageBox::information(this, "ע��ɹ�", "�û�ע��ɹ������¼");
    }
    else {
        QMessageBox::warning(this, "ע��ʧ��", "�û�ע��ʧ�ܣ�������");
        generateCaptcha();
    }
}