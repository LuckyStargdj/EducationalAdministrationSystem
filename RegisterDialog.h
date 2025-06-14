#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H
#pragma execution_character_set("utf-8")

#include <QDialog>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include "UserManager.h"

class RegisterDialog : public QDialog {
    Q_OBJECT
public:
    explicit RegisterDialog(UserManager* userMgr, QWidget* parent = nullptr);
    QString username() const { return m_username; }

private slots:
    void onRegisterClicked();

private:
    void setupUI();
    void generateCaptcha();

    UserManager* m_userManager;
    QLineEdit* m_usernameEdit;
    QLineEdit* m_nameEdit;
    QComboBox* m_roleCombo;
    QLineEdit* m_passwordEdit;
    QLineEdit* m_confirmPasswordEdit;
    QLineEdit* m_classEdit;
    QLineEdit* m_yearEdit;
    QLabel* m_captchaLabel;
    QLineEdit* m_captchaEdit;
    QPushButton* m_registerButton;

    QString m_username;
    int m_captchaValue = 0;
};

#endif // REGISTERDIALOG_H
