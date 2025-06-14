#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QSqlDatabase>
#include <QTimer>
#include <QMap>
#include <QSqlQuery>
#include <QVector>
#include <QPair>

class DatabaseManager : public QObject {
    Q_OBJECT
public:
    static DatabaseManager& instance();
    QSqlDatabase getDataBase() const;
    bool openDatabase();
    bool isConnected() const;
    void checkConnection();

    QSqlQuery execQuery(const QString& sql, const QMap<QString, QVariant>& params = {});
    int insertRecord(const QString& table, const QMap<QString, QVariant>& values);
    bool updateRecord(const QString& table, const QMap<QString, QVariant>& values, const QString& where);
    bool deleteRecord(const QString& table, const QString& where);

    // 拼音处理
    QString getPinyinInitials(const QString& name);

    // 成绩统计
    QMap<QString, double> calculateClassStats(const QString& className);
    QVector<QPair<QString, double>> calculatePassRate(const QString& courseId);

signals:
    void connectionChanged(bool connected);

private:
    DatabaseManager(QObject* parent = nullptr);
    void setConnected(bool connected);
    void initDatabase();

    QSqlDatabase m_db;
    bool m_connected = false;
    QTimer m_connectionCheckTimer;
};

#endif // DATABASEMANAGER_H