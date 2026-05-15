#include "StatisticsManager.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <set>

json StatisticsDataset::toJson() const {
    json j;
    j["name"] = name;
    j["cardIds"] = cardIds;
    json statsJson;
    for (const auto& [cid, stat] : stats) {
        json statJson;
        statJson["totalAttempts"] = stat.totalAttempts;
        statJson["correctCount"] = stat.correctCount;
        statJson["lastResult"] = stat.lastResult;
        std::vector<int> historyInt;
        for (bool b : stat.roundHistory) historyInt.push_back(b ? 1 : 0);
        statJson["roundHistory"] = historyInt;
        statsJson[std::to_string(cid)] = statJson;
    }
    j["stats"] = statsJson;
    return j;
}

StatisticsDataset StatisticsDataset::fromJson(const json& j) {
    StatisticsDataset ds;
    ds.name = j.value("name", "");
    if (j.contains("cardIds")) ds.cardIds = j["cardIds"].get<std::vector<int>>();
    if (j.contains("stats")) {
        for (auto& [key, val] : j["stats"].items()) {
            int cid = std::stoi(key);
            CardStats stat;
            stat.totalAttempts = val.value("totalAttempts", 0);
            stat.correctCount = val.value("correctCount", 0);
            stat.lastResult = val.value("lastResult", false);
            if (val.contains("roundHistory")) {
                for (int v : val["roundHistory"]) stat.roundHistory.push_back(v != 0);
            }
            ds.stats[cid] = stat;
        }
    }
    return ds;
}

StatisticsDataset StatisticsDataset::merge(const std::vector<StatisticsDataset>& datasets) {
    StatisticsDataset result;
    result.name = "聚合统计";
    std::unordered_map<int, CardStats> mergedStats;
    std::set<int> allCardIds;
    for (const auto& ds : datasets) {
        for (int cid : ds.cardIds) allCardIds.insert(cid);
        for (const auto& [cid, stat] : ds.stats) {
            CardStats& target = mergedStats[cid];
            target.totalAttempts += stat.totalAttempts;
            target.correctCount += stat.correctCount;
            target.lastResult = stat.lastResult;
            for (bool res : stat.roundHistory) target.roundHistory.push_back(res);
        }
    }
    result.cardIds.assign(allCardIds.begin(), allCardIds.end());
    result.stats = mergedStats;
    return result;
}

StatisticsManager::StatisticsManager(const std::filesystem::path& statsRoot, QObject* parent)
    : QObject(parent), m_statsRoot(statsRoot) {
    if (!std::filesystem::exists(m_statsRoot)) std::filesystem::create_directories(m_statsRoot);
}

StatisticsManager::~StatisticsManager() {}

bool StatisticsManager::load() {
    m_sessionFiles.clear();
    if (!std::filesystem::exists(m_statsRoot)) return false;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_statsRoot)) {
        if (entry.is_regular_file() && entry.path().extension() == ".session.json") {
            m_sessionFiles.push_back(entry.path());
        }
    }
    return true;
}

void StatisticsManager::saveSession(const StatisticsDataset& session) {
    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm bt;
#ifdef _WIN32
    localtime_s(&bt, &in_time_t);
#else
    localtime_r(&in_time_t, &bt);
#endif
    std::ostringstream oss;
    oss << std::put_time(&bt, "%Y-%m-%d_%H-%M-%S");
    std::string filename = oss.str() + ".session.json";
    std::filesystem::path filepath = m_statsRoot / filename;
    json j = session.toJson();
    std::ofstream ofs(filepath);
    ofs << j.dump(4);
    m_sessionFiles.push_back(filepath);
}

std::optional<StatisticsDataset> StatisticsManager::loadSession(const std::filesystem::path& sessionPath) {
    std::ifstream ifs(sessionPath);
    if (!ifs) return std::nullopt;
    json j;
    try { ifs >> j; } catch (...) { return std::nullopt; }
    return StatisticsDataset::fromJson(j);
}

void StatisticsManager::listAllSessions() const {
    // 仅调试
}

StatisticsDataset StatisticsManager::aggregateFolder(const std::filesystem::path& folderPath) {
    std::vector<StatisticsDataset> datasets;
    for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
        if (entry.is_regular_file() && entry.path().extension() == ".session.json") {
            auto ds = loadSession(entry.path());
            if (ds) datasets.push_back(*ds);
        }
    }
    return StatisticsDataset::merge(datasets);
}