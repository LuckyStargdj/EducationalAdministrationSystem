#include "CourseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QtMath>
#include <QDebug>

CourseManager::CourseManager(DatabaseManager* dbMgr, QObject* parent)
    : QObject(parent), m_dbManager(dbMgr) {}

bool CourseManager::addCourse(const QString& id, const QString& name, double credit,
    const QString& teacherId, const QDate& startDate, const QString& semester) {
    QMap<QString, QVariant> values;
    values["id"] = id;
    values["name"] = name;
    values["credit"] = credit;
    values["teacher_id"] = teacherId;
    values["start_date"] = startDate.toString(Qt::ISODate);
    values["semester"] = semester;

    bool success = m_dbManager->insertRecord("courses", values) > 0;
    if (success) emit courseDataChanged();
    return success;
}

bool CourseManager::deleteCourse(const QString& courseId) {
    // 先删除相关成绩
    if (!m_dbManager->deleteRecord("scores", QString("course_id = '%1'").arg(courseId))) {
        return false;
    }

    // 删除课程
    bool success = m_dbManager->deleteRecord("courses", QString("id = '%1'").arg(courseId));
    if (success) emit courseDataChanged();
    return success;
}

bool CourseManager::updateCourse(const QString& courseId, const QMap<QString, QVariant>& updates) {
    bool success = m_dbManager->updateRecord("courses", updates, QString("id = '%1'").arg(courseId));
    if (success) emit courseDataChanged();
    return success;
}

QVector<QVariantMap> CourseManager::getAllCourses() const {
    QVector<QVariantMap> courses;

    QSqlQuery query("SELECT * FROM courses");
    while (query.next()) {
        QSqlRecord record = query.record();
        QVariantMap course;
        for (int i = 0; i < record.count(); i++) {
            course[record.fieldName(i)] = record.value(i);
        }
        courses.append(course);
    }
    return courses;
}

QVector<QVariantMap> CourseManager::getCoursesByTeacher(const QString& teacherId) const {
    QVector<QVariantMap> courses;

    QSqlQuery query;
    query.prepare("SELECT * FROM courses WHERE teacher_id = :teacherId");
    query.bindValue(":teacherId", teacherId);

    if (query.exec()) {
        while (query.next()) {
            QSqlRecord record = query.record();
            QVariantMap course;
            for (int i = 0; i < record.count(); i++) {
                course[record.fieldName(i)] = record.value(i);
            }
            courses.append(course);
        }
    }
    return courses;
}

bool CourseManager::addScore(const QString& studentId, const QString& courseId, double score) {
    QMap<QString, QVariant> values;
    values["student_id"] = studentId;
    values["course_id"] = courseId;
    values["score"] = score;

    bool success = m_dbManager->insertRecord("scores", values) > 0;
    if (success) emit scoreDataChanged();
    return success;
}

bool CourseManager::updateScore(const QString& studentId, const QString& courseId, double score) {
    QMap<QString, QVariant> values;
    values["score"] = score;

    QString where = QString("student_id = '%1' AND course_id = '%2'").arg(studentId, courseId);
    bool success = m_dbManager->updateRecord("scores", values, where);
    if (success) emit scoreDataChanged();
    return success;
}

double CourseManager::getScore(const QString& studentId, const QString& courseId) const {
    QSqlQuery query;
    query.prepare("SELECT score FROM scores WHERE student_id = :studentId AND course_id = :courseId");
    query.bindValue(":studentId", studentId);
    query.bindValue(":courseId", courseId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }
    return -1; // 无效分数
}

QVector<QVariantMap> CourseManager::getScoresByStudent(const QString& studentId) const {
    QVector<QVariantMap> scores;

    QSqlQuery query;
    query.prepare("SELECT course_id, score FROM scores WHERE student_id = :studentId");
    query.bindValue(":studentId", studentId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap score;
            score["course_id"] = query.value("course_id");
            score["score"] = query.value("score");
            scores.append(score);
        }
    }
    return scores;
}

QVector<QVariantMap> CourseManager::getScoresByCourse(const QString& courseId) const {
    QVector<QVariantMap> scores;

    QSqlQuery query;
    query.prepare("SELECT student_id, score FROM scores WHERE course_id = :courseId");
    query.bindValue(":courseId", courseId);

    if (query.exec()) {
        while (query.next()) {
            QVariantMap score;
            score["student_id"] = query.value("student_id");
            score["score"] = query.value("score");
            scores.append(score);
        }
    }
    return scores;
}

QMap<QString, double> CourseManager::calculateCourseStats(const QString& courseId) const {
    QMap<QString, double> stats;
    stats["average"] = 0;
    stats["stddev"] = 0;
    stats["pass_rate"] = 0;

    QSqlQuery query;
    query.prepare("SELECT AVG(score), STDDEV_POP(score), "
        "COUNT(CASE WHEN score >= 60 THEN 1 END) * 1.0 / COUNT(*) "
        "FROM scores WHERE course_id = :courseId");
    query.bindValue(":courseId", courseId);

    if (query.exec() && query.next()) {
        stats["average"] = query.value(0).toDouble();
        stats["stddev"] = query.value(1).toDouble();
        stats["pass_rate"] = query.value(2).toDouble();
    }

    return stats;
}

QMap<QString, double> CourseManager::calculateClassStats(const QString& className) const {
    QMap<QString, double> stats;
    stats["average"] = 0;
    stats["stddev"] = 0;
    stats["pass_rate"] = 0;

    QSqlQuery query;
    query.prepare("SELECT AVG(score), STDDEV_POP(score), "
        "COUNT(CASE WHEN score >= 60 THEN 1 END) * 1.0 / COUNT(*) "
        "FROM scores s "
        "JOIN users u ON s.student_id = u.student_id "
        "WHERE u.class_name = :className");
    query.bindValue(":className", className);

    if (query.exec() && query.next()) {
        stats["average"] = query.value(0).toDouble();
        stats["stddev"] = query.value(1).toDouble();
        stats["pass_rate"] = query.value(2).toDouble();
    }

    return stats;
}
