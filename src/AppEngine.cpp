#include "AppEngine.h"
#include <QDir>
#include <QStandardPaths>

AppEngine& AppEngine::instance() {
    static AppEngine inst;
    return inst;
}

bool AppEngine::initialize() {
    // 确保数据目录存在
    QDir::current().mkpath("data/cards");
    QDir::current().mkpath("data/stats");

    m_cardManager = std::make_unique<CardManager>("data/cards");
    m_statsManager = std::make_unique<StatisticsManager>("data/stats");

    if (!m_cardManager->load()) return false;
    m_statsManager->load();
    return true;
}