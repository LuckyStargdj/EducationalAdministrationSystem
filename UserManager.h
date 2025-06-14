#ifndef USERMANAGER_H
#define USERMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QString>
#include <QVariant>
#include <QRegularExpression>
#include <QDateTime>
#include <QMap>

class DatabaseManager;

class UserManager : public QObject {
    Q_OBJECT
public:
    enum Role { Admin = 0, Teacher = 1, Student = 2 };
    Q_ENUM(Role)

    explicit UserManager(QObject* parent = nullptr);
    explicit UserManager(int userId, const QString& username, Role role, QObject* parent = nullptr);
    virtual ~UserManager();

    // 用户管理接口
    bool registerUser(const QString& name, Role role, const QString& studentId,
        const QString& className, int enrollmentYear, const QString& password);
    bool login(const QString& username, const QString& password);
    bool resetPassword(int userId, const QString& newPassword, Role currentRole);
    bool freezeAccount(int userId);
    bool activateAccount(int userId);
    bool updateUserInfo(int userId, const QString& field, const QVariant& value);
    QString getLastLoginTime(int userId) const;
    void updateLastLoginTime(int userId);

    // 获取当前用户信息
    int currentUserId() const;
    QString currentUserName() const;
    Role currentRole() const;
    QString currentClass() const;
    QString currentStudentId() const;
    QVariantMap getCurrentUserInfo() const;

    // 设置当前用户信息
    void setCurrentUserId(int id);
    void setCurrentUserName(const QString& username);
    void setCurrentRole(Role role);
    void setCurrentClassName(const QString& classname);
    void setCurrentStudentId(int id);

    // 用户管理（管理员权限）
    QVector<QVariantMap> listUsers(Role roleFilter = (Role)-1);
    bool deleteUser(int userId);

signals:
    void loginStatusChanged(bool success, const QString& message);
    void registerStatus(bool success, const QString& message);
    void userDataChanged();

private:
    bool validatePassword(const QString& password) const;
    bool validateStudentId(const QString& studentId) const;
    bool validateEnrollmentYear(int year) const;

    int m_currentUserId = -1;
    QString m_currentUserName;
    Role m_currentRole = Student;
    QString m_currentStudentId;
    QString m_currentClassName;
    int m_currentEnrollmentYear;
    QDateTime m_lastLoginTime;
    DatabaseManager* m_dbManager;
};

#endif // USERMANAGER_H
