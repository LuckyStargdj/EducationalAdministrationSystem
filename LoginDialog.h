#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H
#pragma execution_character_set("utf-8")

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "UserManager.h"

class LoginDialog : public QDialog {
    Q_OBJECT
public:
    explicit LoginDialog(UserManager* userMgr, QWidget* parent = nullptr);
    explicit LoginDialog(UserManager* userMgr, const QString& username, QWidget* parent = nullptr);
    QString username() const { return m_username; }

private slots:
    void onLoginClicked();
    void onRegisterClicked();
    void handleLoginResult(bool success, const QString& message);

private:
    void setupUI();
    void generateCaptcha();

    QLineEdit* m_usernameEdit;
    QLineEdit* m_passwordEdit;
    QLabel* m_captchaLabel;
    QLineEdit* m_captchaEdit;
    QPushButton* m_loginButton;
    QPushButton* m_registerButton;

    QString m_username;
    int m_captchaValue = 0;
    int m_failCount = 0;
    UserManager* m_userManager;
};

#endif // LOGINDIALOG_H

