#include "FileUploadWidget.h"
#include <QHBoxLayout>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QStandardPaths>
#include <QLabel>
#include <QPushButton>
#include <QDebug>

FileUploadWidget::FileUploadWidget(QWidget* parent) : QWidget(parent) {
    QHBoxLayout* layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    m_fileNameLabel = new QLabel("未选择文件", this);
    m_fileNameLabel->setMinimumWidth(200);
    m_fileNameLabel->setFrameStyle(QFrame::Box | QFrame::Sunken);

    QPushButton* browseButton = new QPushButton("选择文件...", this);
    connect(browseButton, &QPushButton::clicked, this, &FileUploadWidget::browseFile);

    layout->addWidget(m_fileNameLabel);
    layout->addWidget(browseButton);
}

void FileUploadWidget::browseFile() {
    QString filter = "所有文件 (*);;图片文件 (*.jpg *.png);;PDF文件 (*.pdf)";
    QString file = QFileDialog::getOpenFileName(this, "选择学生档案", "", filter);

    if (!file.isEmpty()) {
        m_filePath = file;
        QFileInfo fileInfo(file);
        m_fileNameLabel->setText(fileInfo.fileName());
    }
}

void FileUploadWidget::clear() {
    m_filePath.clear();
    m_fileNameLabel->setText("未选择文件");
}

void FileUploadWidget::setFile(const QString& fileName) {
    QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/student_files/";
    m_filePath = appDataDir + fileName;
    m_fileNameLabel->setText(fileName);
}

QString FileUploadWidget::uploadFile(const QString& studentId) {
    if (m_filePath.isEmpty()) {
        return "";
    }

    QFileInfo sourceFileInfo(m_filePath);
    if (!sourceFileInfo.exists()) {
        qWarning() << "Source file does not exist:" << m_filePath;
        return "";
    }

    // 创建存放目录
    QString destDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/student_files/";
    QDir dir(destDir);
    if (!dir.exists() && !dir.mkpath(".")) {
        qWarning() << "Failed to create directory:" << destDir;
        return "";
    }

    // 生成唯一文件名
    QString destFileName = QString("%1_%2.%3").arg(studentId)
        .arg(QDateTime::currentDateTime().toString("yyyyMMddhhmmss"))
        .arg(sourceFileInfo.suffix());
    QString destPath = destDir + destFileName;

    // 复制文件
    if (QFile::copy(m_filePath, destPath)) {
        return destFileName;
    }
    else {
        qWarning() << "Failed to copy file from" << m_filePath << "to" << destPath;
        return "";
    }
}
