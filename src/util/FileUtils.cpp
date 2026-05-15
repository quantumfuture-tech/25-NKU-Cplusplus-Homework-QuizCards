#include "FileUtils.h"
#include <QtConcurrent/QtConcurrent>
#include <QFile>
#include <QTextStream>
#include <QDebug>

void FileUtils::asyncWriteJson(const std::filesystem::path& path, const std::string& content,
                               std::function<void(bool)> callback) {
    QtConcurrent::run([path, content, callback]() {
        bool ok = writeJson(path, content);
        if (callback) callback(ok);
    });
}

std::string FileUtils::readJsonString(const std::filesystem::path& path) {
    QFile file(QString::fromStdString(path.string()));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return {};
    QTextStream stream(&file);
    return stream.readAll().toStdString();
}

bool FileUtils::writeJson(const std::filesystem::path& path, const std::string& content) {
    QFile file(QString::fromStdString(path.string()));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream stream(&file);
    stream << QString::fromStdString(content);
    return true;
}

bool FileUtils::ensureDirectoryExists(const std::filesystem::path& path) {
    if (std::filesystem::exists(path))
        return std::filesystem::is_directory(path);
    return std::filesystem::create_directories(path);
}