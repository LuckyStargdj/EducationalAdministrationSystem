#ifndef FILEUPLOADWIDGET_H
#define FILEUPLOADWIDGET_H
#pragma execution_character_set("utf-8")

#include <QWidget>
#include <QLabel>
#include <QPushButton>

class QLabel;
class QPushButton;

class FileUploadWidget : public QWidget {
    Q_OBJECT
public:
    explicit FileUploadWidget(QWidget* parent = nullptr);
    void clear();
    void setFile(const QString& fileName);
    QString uploadFile(const QString& studentId);

private slots:
    void browseFile();

private:
    QString m_filePath;
    QLabel* m_fileNameLabel;
};

#endif // FILEUPLOADWIDGET_H
