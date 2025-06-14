#ifndef ADMINFORM_H
#define ADMINFORM_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QPushButton>
#include "DatabaseManager.h"
#include "UserManager.h"

class AdminForm : public QWidget {
    Q_OBJECT
public:
    explicit AdminForm(UserManager* userMgr, DatabaseManager* dbMgr, QWidget* parent = nullptr);

private slots:
    void manageUserStatus(bool freeze);
    void refreshUsers();
    void exportData();

private:
    void setupUI();

    UserManager* m_userMgr;
    DatabaseManager* m_dbMgr;

    QComboBox* m_roleCombo;
    QTableView* m_userView;
    QPushButton* m_freezeButton;
    QPushButton* m_activateButton;
    QPushButton* m_exportButton;
};

#endif // ADMINFORM_H