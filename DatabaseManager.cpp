#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <QRegularExpression>
#include <QStringList>
#include <QFile>
#include <QSqlRecord>

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager instance;
    return instance;
}

QSqlDatabase DatabaseManager::getDataBase() const {
    return m_db;
}

DatabaseManager::DatabaseManager(QObject* parent)
    : QObject(parent), m_connected(false) {
    m_db = QSqlDatabase::addDatabase("QSQLITE");

    // 存放数据库文件的目录
    QString dataDir = "./data";
    if (dataDir.isEmpty()) {
        qWarning() << "Failed to get app data location";
        dataDir = ".";
    }

    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    QString dbPath = dir.absoluteFilePath("educational_administration.db");
    m_db.setDatabaseName(dbPath);

    connect(&m_connectionCheckTimer, &QTimer::timeout, this, &DatabaseManager::checkConnection);
    m_connectionCheckTimer.start(5000);
}

bool DatabaseManager::openDatabase() {
    if (m_connected) return true;
    if (!m_db.open()) {
        qDebug() << "Database Error: " << m_db.lastError().text();
        setConnected(false);
        return false;
    }

    initDatabase();
    return true;
}

void DatabaseManager::initDatabase() {
    QSqlQuery query;

    // 创建用户表
    query.exec("CREATE TABLE IF NOT EXISTS users ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "name TEXT NOT NULL,"
        "pinyin TEXT NOT NULL,"  // 拼音首字母字段
        "role INTEGER NOT NULL,"
        "student_id TEXT UNIQUE NOT NULL,"
        "password TEXT NOT NULL,"
        "class_name TEXT,"
        "enrollment_year INTEGER,"
        "gender INTEGER DEFAULT 0," // 0:男, 1:女
        "status INTEGER DEFAULT 1,"
        "last_login TEXT)");

    // 创建学生档案表
    query.exec("CREATE TABLE IF NOT EXISTS student_files ("
        "student_id TEXT PRIMARY KEY,"
        "file_path TEXT)");

    // 创建课程表
    query.exec("CREATE TABLE IF NOT EXISTS courses ("
        "id TEXT PRIMARY KEY,"
        "name TEXT NOT NULL,"
        "credit REAL NOT NULL,"
        "teacher_id TEXT NOT NULL,"
        "start_date TEXT,"
        "semester TEXT)");

    // 创建成绩表
    query.exec("CREATE TABLE IF NOT EXISTS scores ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT,"
        "student_id TEXT NOT NULL,"
        "course_id TEXT NOT NULL,"
        "score REAL,"
        "UNIQUE(student_id, course_id))");

    // 添加测试数据
    if (query.exec("SELECT COUNT(*) FROM users") && query.next() && query.value(0).toInt() == 0) {
        query.exec("INSERT INTO users (name, pinyin, role, student_id, password, class_name, enrollment_year) VALUES "
            "('Admin', 'admin', 0, '1000000000', 'admin123', 'AdminClass', 2024),"
            "('张三', 'zs', 2, '2024000001', 'zhangsan', '计算机一班', 2024),"
            "('李四', 'ls', 2, '2024000002', 'lisi', '计算机一班', 2024),"
            "('王老师', 'wls', 1, 'T1001', 'wanglaoshi', '', 0)");
    }

    setConnected(true);
}

bool DatabaseManager::isConnected() const {
    return m_connected;
}

void DatabaseManager::setConnected(bool connected) {
    if (m_connected != connected) {
        m_connected = connected;
        emit connectionChanged(connected);
    }
}

void DatabaseManager::checkConnection() {
    bool currentlyConnected = m_db.isOpen();
    if (currentlyConnected) {
        QSqlQuery query("SELECT 1");
        currentlyConnected = query.isActive();
    }
    if (currentlyConnected != m_connected) {
        setConnected(currentlyConnected);
    }
}

QSqlQuery DatabaseManager::execQuery(const QString& sql, const QMap<QString, QVariant>& params) {
    QSqlQuery query;
    query.prepare(sql);

    for (auto it = params.constBegin(); it != params.constEnd(); ++it) {
        query.bindValue(it.key(), it.value());
    }

    if (!query.exec()) {
        qWarning() << "Query failed:" << query.lastError() << "\nSQL:" << sql;
    }
    return query;
}

int DatabaseManager::insertRecord(const QString& table, const QMap<QString, QVariant>& values) {
    if (table.isEmpty() || values.isEmpty()) return -1;

    QString columns;
    QString placeholders;
    QMapIterator<QString, QVariant> it(values);

    while (it.hasNext()) {
        it.next();
        if (!columns.isEmpty()) {
            columns += ", ";
            placeholders += ", ";
        }
        columns += it.key();
        placeholders += ":" + it.key();
    }

    QString sql = QString("INSERT INTO %1 (%2) VALUES (%3)").arg(table).arg(columns).arg(placeholders);
    QSqlQuery query = execQuery(sql, values);

    if (query.lastError().isValid()) return -1;
    return query.lastInsertId().toInt();
}

bool DatabaseManager::updateRecord(const QString& table, const QMap<QString, QVariant>& values, const QString& where) {
    if (table.isEmpty() || values.isEmpty() || where.isEmpty()) return false;

    QString setClause;
    QMapIterator<QString, QVariant> it(values);

    while (it.hasNext()) {
        it.next();
        if (!setClause.isEmpty()) setClause += ", ";
        setClause += it.key() + " = :" + it.key();
    }

    QString sql = QString("UPDATE %1 SET %2 WHERE %3").arg(table).arg(setClause).arg(where);
    QSqlQuery query = execQuery(sql, values);

    return !query.lastError().isValid();
}

bool DatabaseManager::deleteRecord(const QString& table, const QString& where) {
    if (table.isEmpty() || where.isEmpty()) return false;

    QString sql = QString("DELETE FROM %1 WHERE %2").arg(table).arg(where);
    QSqlQuery query = execQuery(sql);

    return !query.lastError().isValid();
}

QString DatabaseManager::getPinyinInitials(const QString& name) {
    // 简化的拼音首字母转换
    static QMap<QString, QString> charMap = {
        {"张", "z"}, {"王", "w"}, {"李", "l"}, {"刘", "l"}, {"陈", "c"},
        {"杨", "y"}, {"赵", "z"}, {"黄", "h"}, {"周", "z"}, {"吴", "w"},
        {"三", "s"}, {"四", "s"}, {"五", "w"}, {"六", "l"}, {"七", "q"},
        {"八", "b"}, {"九", "j"}, {"十", "s"}, {"明", "m"}, {"华", "h"}
    };

    QString initials;
    for (int i = 0; i < name.length(); i++) {
        QString ch = name.mid(i, 1);
        if (charMap.contains(ch)) {
            initials += charMap[ch];
        }
        else {
            initials += ch.toLower();
        }
    }
    return initials;
}

QMap<QString, double> DatabaseManager::calculateClassStats(const QString& className) {
    QMap<QString, double> stats;

    QSqlQuery query;
    query.prepare("SELECT AVG(score), STDDEV_POP(score) "
        "FROM scores JOIN users ON scores.student_id = users.student_id "
        "WHERE users.class_name = :class");
    query.bindValue(":class", className);

    if (query.exec() && query.next()) {
        stats["average"] = query.value(0).toDouble();
        stats["stddev"] = query.value(1).toDouble();
    }

    return stats;
}

QVector<QPair<QString, double>> DatabaseManager::calculatePassRate(const QString& courseId) {
    QVector<QPair<QString, double>> results;

    QSqlQuery query;
    query.prepare("SELECT class_name, "
        "COUNT(CASE WHEN score >= 60 THEN 1 END) * 1.0 / COUNT(score) as pass_rate "
        "FROM scores JOIN users ON scores.student_id = users.student_id "
        "WHERE course_id = :courseId "
        "GROUP BY class_name");
    query.bindValue(":courseId", courseId);

    if (query.exec()) {
        while (query.next()) {
            results.append(qMakePair(query.value(0).toString(), query.value(1).toDouble()));
        }
    }

    return results;
}