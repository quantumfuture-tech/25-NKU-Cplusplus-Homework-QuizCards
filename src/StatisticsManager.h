// FlashcardConsole/src/StatisticsManager.h
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 单张卡片的统计记录（一次会话中）
struct CardStats {
    int totalAttempts = 0;
    int correctCount = 0;
    bool lastResult = false;
    std::vector<bool> roundHistory;
};

// 一次抽查会话的完整数据集
struct StatisticsDataset {
    std::string name;                   // 会话名称，例如时间戳
    std::vector<int> cardIds;           // 参与的所有卡片 ID
    std::unordered_map<int, CardStats> stats; // 每个卡片的统计

    json toJson() const;
    static StatisticsDataset fromJson(const json& j);

    // 聚合两个数据集（用于合并文件夹）
    static StatisticsDataset merge(const std::vector<StatisticsDataset>& datasets);
};

// 管理统计树（data/stats/）
class StatisticsManager {
public:
    StatisticsManager(const std::filesystem::path& statsRoot);
    ~StatisticsManager();

    // 加载统计树（扫描目录）
    bool load();

    // 保存一个会话到文件（自动生成文件名）
    void saveSession(const StatisticsDataset& session);

    // 根据文件路径加载会话
    std::optional<StatisticsDataset> loadSession(const std::filesystem::path& sessionPath);

    // 获取统计树显示信息（简化：列出所有会话文件）
    void listAllSessions() const;

    // 聚合某个文件夹（或根目录）下的所有会话统计
    StatisticsDataset aggregateFolder(const std::filesystem::path& folderPath);

    // 获取根目录路径
    std::filesystem::path rootPath() const { return m_statsRoot; }

private:
    std::filesystem::path m_statsRoot;
    std::vector<std::filesystem::path> m_sessionFiles; // 缓存所有 .session.json 文件
};