#ifndef REPORTFORM_H
#define REPORTFORM_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QTableView>
#include <QHeaderView>
#include <QComboBox>
#include <QComboBox>
#include <QPushButton>
#include "DatabaseManager.h"
#include "CourseManager.h"

class ReportForm : public QWidget {
    Q_OBJECT
public:
    explicit ReportForm(DatabaseManager* dbMgr, CourseManager* courseMgr, QWidget* parent = nullptr);

private slots:
    void generateReport();

private:
    void setupUI();

    DatabaseManager* m_dbMgr;
    CourseManager* m_courseMgr;

    QComboBox* m_courseCombo;
    QComboBox* m_classCombo;
    QComboBox* m_reportTypeCombo;
    QPushButton* m_generateButton;
    QTableView* m_reportView;
};

#endif // REPORTFORM_H
