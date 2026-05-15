#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QObject>
#include <QFuture>
#include <functional>
#include <filesystem>

class FileUtils : public QObject {
    Q_OBJECT
public:
    static void asyncWriteJson(const std::filesystem::path& path, const std::string& content,
                               std::function<void(bool)> callback = nullptr);
    static std::string readJsonString(const std::filesystem::path& path);
    static bool writeJson(const std::filesystem::path& path, const std::string& content);
    static bool ensureDirectoryExists(const std::filesystem::path& path);
};

#endif // FILEUTILS_H