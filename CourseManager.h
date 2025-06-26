#ifndef COURSEMANAGER_H
#define COURSEMANAGER_H
#pragma execution_character_set("utf-8")

#include <QObject>
#include <QMap>
#include <QVector>
#include <QVariant>
#include <QDate>
#include "DatabaseManager.h"

class CourseManager : public QObject {
    Q_OBJECT
public:
    explicit CourseManager(DatabaseManager* dbMgr, QObject* parent = nullptr);

    // 课程管理
    bool addCourse(const QString& id, const QString& name, double credit,
        const QString& teacherId, const QDate& startDate, const QString& semester);
    bool deleteCourse(const QString& courseId);
    bool updateCourse(const QString& courseId, const QMap<QString, QVariant>& updates);
    QVector<QVariantMap> getAllCourses() const;
    QVector<QVariantMap> getCoursesByTeacher(const QString& teacherId) const;

    // 成绩管理
    bool addScore(const QString& studentId, const QString& courseId, double score);
    bool updateScore(const QString& studentId, const QString& courseId, double score);
    double getScore(const QString& studentId, const QString& courseId) const;
    QVector<QVariantMap> getScoresByStudent(const QString& studentId) const;
    QVector<QVariantMap> getScoresByCourse(const QString& courseId) const;

    // 统计功能
    QMap<QString, double> calculateCourseStats(const QString& courseId) const;
    QMap<QString, double> calculateClassStats(const QString& className) const;

signals:
    void courseDataChanged();
    void scoreDataChanged();

private:
    DatabaseManager* m_dbManager;
};

#endif // COURSEMANAGER_H

