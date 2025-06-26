#include <QApplication>
#include "DatabaseManager.h"
#include "UserManager.h"
#include "CourseManager.h"
#include "LoginDialog.h"
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // 初始化数据库
    DatabaseManager& dbMgr = DatabaseManager::instance();
    dbMgr.openDatabase();

    // 用户管理
    UserManager userMgr;

    // 显示登录对话框
    LoginDialog loginDlg(&userMgr);
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;
    }

    // 课程管理
    CourseManager courseMgr(&dbMgr);

    // 显示主窗口
    MainWindow mainWin(&userMgr, &dbMgr, &courseMgr);
    mainWin.show();

    return app.exec();
}