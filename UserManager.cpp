#include "UserManager.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QRegularExpression>
#include <QDateTime>

UserManager::UserManager(QObject* parent)
    : QObject(parent), m_dbManager(&DatabaseManager::instance()) {}

UserManager::UserManager(int userid, const QString& username, Role role, QObject* parent)
    : QObject(parent), m_dbManager(&DatabaseManager::instance()) {
    m_currentUserId = userid;
    m_currentUserName = username;
    m_currentRole = role;
}

UserManager::~UserManager() {}

bool UserManager::validateStudentId(const QString& studentId) const {
    QRegularExpression regex("^\\d{10}$|^T\\d{4}$");
    return regex.match(studentId).hasMatch();
}

bool UserManager::validatePassword(const QString& password) const {
    return password.length() >= 6;
}

bool UserManager::validateEnrollmentYear(int year) const {
    return year >= 2000 && year <= QDate::currentDate().year();
}

bool UserManager::registerUser(const QString& name, Role role, const QString& studentId,
    const QString& className, int enrollmentYear, const QString& password) {
    // 验证输入
    if (!validateStudentId(studentId)) {
        emit registerStatus(false, "学号格式错误");
        return false;
    }

    if (!validatePassword(password)) {
        emit registerStatus(false, "密码长度至少为6位");
        return false;
    }

    if (role == Student && !validateEnrollmentYear(enrollmentYear)) {
        emit registerStatus(false, "入学年份无效");
        return false;
    }

    // 检查学号唯一性
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE student_id = :studentId");
    query.bindValue(":studentId", studentId);
    if (!query.exec() || !query.next()) {
        emit registerStatus(false, "数据库查询失败");
        return false;
    }

    if (query.value(0).toInt() > 0) {
        emit registerStatus(false, "该学号已注册");
        return false;
    }

    // 生成拼音首字母
    QString pinyin = m_dbManager->getPinyinInitials(name);

    // 准备数据
    QMap<QString, QVariant> values;
    values["name"] = name;
    values["pinyin"] = pinyin;
    values["role"] = static_cast<int>(role);
    values["student_id"] = studentId;
    values["password"] = password;
    if (role == Student) {
        values["class_name"] = className;
        values["enrollment_year"] = enrollmentYear;
    }

    // 插入数据库
    if (m_dbManager->insertRecord("users", values) > 0) {
        emit registerStatus(true, "注册成功");
        return true;
    }

    emit registerStatus(false, "注册失败");
    return false;
}

bool UserManager::login(const QString& username, const QString& password) {
    if (username.isEmpty() || password.isEmpty()) {
        emit loginStatusChanged(false, "用户名和密码不能为空");
        return false;
    }

    QSqlQuery query;
    query.prepare("SELECT id, name, role, student_id, password, status, class_name, enrollment_year, last_login "
        "FROM users WHERE student_id = :username");
    query.bindValue(":username", username);

    if (!query.exec() || !query.next()) {
        emit loginStatusChanged(false, "用户不存在");
        return false;
    }

    // 检查账户状态
    if (query.value("status").toInt() == 0) {
        emit loginStatusChanged(false, "账户已冻结");
        return false;
    }

    // 验证密码
    QString savedPassword = query.value("password").toString();
    if (password != savedPassword) {
        emit loginStatusChanged(false, "密码错误");
        return false;
    }

    // 更新用户信息
    m_currentUserId = query.value("id").toInt();
    m_currentUserName = query.value("name").toString();
    m_currentRole = static_cast<Role>(query.value("role").toInt());
    m_currentStudentId = query.value("student_id").toString();
    m_currentClassName = query.value("class_name").toString();
    m_currentEnrollmentYear = query.value("enrollment_year").toInt();
    m_lastLoginTime = QDateTime::currentDateTime();

    // 更新最后登录时间
    updateLastLoginTime(m_currentUserId);

    emit loginStatusChanged(true, "登录成功");
    return true;
}

void UserManager::updateLastLoginTime(int userId) {
    QString now = QDateTime::currentDateTime().toString(Qt::ISODate);

    QSqlQuery query;
    query.prepare("UPDATE users SET last_login = :time WHERE id = :id");
    query.bindValue(":time", now);
    query.bindValue(":id", userId);
    query.exec();
}

bool UserManager::resetPassword(int userId, const QString& newPassword, Role currentRole) {
    if (currentRole != Admin) {
        qDebug() << "Only admin can reset passwords";
        return false;
    }

    if (!validatePassword(newPassword)) {
        return false;
    }

    QMap<QString, QVariant> values;
    values["password"] = newPassword;

    return m_dbManager->updateRecord("users", values, QString("id = %1").arg(userId));
}

bool UserManager::freezeAccount(int userId) {
    QMap<QString, QVariant> values;
    values["status"] = 0;

    bool success = m_dbManager->updateRecord("users", values, QString("id = %1").arg(userId));
    if (success) emit userDataChanged();
    return success;
}

bool UserManager::activateAccount(int userId) {
    QMap<QString, QVariant> values;
    values["status"] = 1;

    bool success = m_dbManager->updateRecord("users", values, QString("id = %1").arg(userId));
    if (success) emit userDataChanged();
    return success;
}

bool UserManager::updateUserInfo(int userId, const QString& field, const QVariant& value) {
    QStringList allowedFields = { "name", "class_name", "enrollment_year", "gender" };
    if (!allowedFields.contains(field)) return false;

    QMap<QString, QVariant> values;
    values[field] = value;

    bool success = m_dbManager->updateRecord("users", values, QString("id = %1").arg(userId));
    if (success) emit userDataChanged();
    return success;
}

QString UserManager::getLastLoginTime(int userId) const {
    QSqlQuery query;
    query.prepare("SELECT last_login FROM users WHERE id = :id");
    query.bindValue(":id", userId);

    if (query.exec() && query.next()) {
        return query.value(0).toString();
    }
    return "从未登录";
}

int UserManager::currentUserId() const {
    return m_currentUserId;
}

QString UserManager::currentUserName() const {
    return m_currentUserName;
}

UserManager::Role UserManager::currentRole() const {
    return m_currentRole;
}

QString UserManager::currentClass() const {
    return m_currentClassName;
}

QString UserManager::currentStudentId() const {
    return m_currentStudentId;
}

QVariantMap UserManager::getCurrentUserInfo() const {
    QVariantMap info;
    info["id"] = m_currentUserId;
    info["name"] = m_currentUserName;
    info["role"] = m_currentRole;
    info["student_id"] = m_currentStudentId;
    info["class_name"] = m_currentClassName;
    info["enrollment_year"] = m_currentEnrollmentYear;
    info["last_login"] = getLastLoginTime(m_currentUserId);
    return info;
}

void UserManager::setCurrentUserId(int id) {
    m_currentUserId = id;
}

void UserManager::setCurrentUserName(const QString& username) {
    m_currentUserName = username;
}

void UserManager::setCurrentRole(Role role) {
    m_currentRole = role;
}

void UserManager::setCurrentClassName(const QString& classname) {
    m_currentClassName = classname;
}

void UserManager::setCurrentStudentId(int id) {
    m_currentStudentId = id;
}

QVector<QVariantMap> UserManager::listUsers(Role roleFilter) {
    QVector<QVariantMap> users;

    QString sql = "SELECT id, name, role, student_id, status, class_name, enrollment_year, last_login "
        "FROM users";

    if (roleFilter != (Role)-1) {
        sql += " WHERE role = :role";
    }

    QSqlQuery query;
    query.prepare(sql);

    if (roleFilter != (Role)-1) {
        query.bindValue(":role", static_cast<int>(roleFilter));
    }

    if (query.exec()) {
        while (query.next()) {
            QVariantMap user;
            user["id"] = query.value("id");
            user["name"] = query.value("name");
            user["role"] = query.value("role");
            user["student_id"] = query.value("student_id");
            user["status"] = query.value("status");
            user["class_name"] = query.value("class_name");
            user["enrollment_year"] = query.value("enrollment_year");
            user["last_login"] = query.value("last_login");
            users.append(user);
        }
    }

    return users;
}

bool UserManager::deleteUser(int userId) {
    if (userId == m_currentUserId) {
        qWarning() << "Cannot delete current user";
        return false;
    }

    return m_dbManager->deleteRecord("users", QString("id = %1").arg(userId));
}