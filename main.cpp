#include <QApplication>
#include "DatabaseManager.h"
#include "UserManager.h"
#include "CourseManager.h"
#include "LoginDialog.h"
#include "MainWindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    // ��ʼ�����ݿ�
    DatabaseManager& dbMgr = DatabaseManager::instance();
    dbMgr.openDatabase();

    // �û�����
    UserManager userMgr;

    // ��ʾ��¼�Ի���
    LoginDialog loginDlg(&userMgr);
    if (loginDlg.exec() != QDialog::Accepted) {
        return 0;
    }

    // �γ̹���
    CourseManager courseMgr(&dbMgr);

    // ��ʾ������
    MainWindow mainWin(&userMgr, &dbMgr, &courseMgr);
    mainWin.show();

    return app.exec();
}