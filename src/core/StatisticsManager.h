#ifndef STATISTICSMANAGER_H
#define STATISTICSMANAGER_H

#include <QObject>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <optional>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

struct CardStats {
    int totalAttempts = 0;
    int correctCount = 0;
    bool lastResult = false;
    std::vector<bool> roundHistory;
};

struct StatisticsDataset {
    std::string name;
    std::vector<int> cardIds;
    std::unordered_map<int, CardStats> stats;

    json toJson() const;
    static StatisticsDataset fromJson(const json& j);
    static StatisticsDataset merge(const std::vector<StatisticsDataset>& datasets);
};

class StatisticsManager : public QObject {
    Q_OBJECT
public:
    explicit StatisticsManager(const std::filesystem::path& statsRoot, QObject* parent = nullptr);
    ~StatisticsManager();

    bool load();
    void saveSession(const StatisticsDataset& session);
    std::optional<StatisticsDataset> loadSession(const std::filesystem::path& sessionPath);
    void listAllSessions() const;
    StatisticsDataset aggregateFolder(const std::filesystem::path& folderPath);
    std::filesystem::path rootPath() const { return m_statsRoot; }

    const std::vector<std::filesystem::path>& sessionFiles() const { return m_sessionFiles; }

private:
    std::filesystem::path m_statsRoot;
    std::vector<std::filesystem::path> m_sessionFiles;
};

#endif // STATISTICSMANAGER_H