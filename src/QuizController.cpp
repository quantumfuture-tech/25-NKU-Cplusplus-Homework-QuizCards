// FlashcardConsole/src/QuizController.cpp
#include "QuizController.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>

QuizSession::QuizSession(const QuizConfig& config, CardManager* cardMgr)
    : m_config(config), m_cardMgr(cardMgr) {}

QuizSession::~QuizSession() {}

bool QuizSession::start() {
    if (m_config.sourceCardIds.empty()) {
        std::cout << "错误：没有选择任何卡片进行抽查。" << std::endl;
        return false;
    }
    if (m_config.mode == QuizMode::RandomGroup) {
        setupRandomGroups();
        if (m_groups.empty()) return false;
        m_currentGroupIdx = 0;
        m_currentGroupCards = m_groups[0];
        m_currentCardIdxInGroup = 0;
    } else if (m_config.mode == QuizMode::MasteryRecursive) {
        setupMasteryFirstRound();
        if (m_currentRoundCards.empty()) return false;
        m_currentCardIdxInGroup = 0;
    }
    m_active = true;
    return true;
}

void QuizSession::setupRandomGroups() {
    std::cout << "洗牌中…" << std::endl;
    // 打乱卡片顺序
    std::vector<int> shuffled = m_config.sourceCardIds;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(shuffled.begin(), shuffled.end(), std::default_random_engine(seed));

    // 按照breakPoint分组
    m_groups.clear();
    size_t total = shuffled.size();
    std::cout << "断点设置：" << std::endl;
    for (auto bpt : m_config.breakPoint) {
        static int start = 0;
        //此函数左闭右开
        std::vector<int> group(shuffled.begin() + start, shuffled.begin() + bpt);
        m_groups.push_back(group);
        start = bpt;
        std::cout << bpt << " ";
    }
    std::cout << std::endl;
    
}

void QuizSession::setupMasteryFirstRound() {
    m_currentRoundCards = m_config.sourceCardIds;
    for (int cid : m_currentRoundCards) {
        m_cardConsecutiveCorrect[cid] = 0;
        m_cardMastered[cid] = false;
    }
}

std::optional<Card> QuizSession::currentCard() const {
    if (!m_active) return std::nullopt;
    int cardId = -1;
    if (m_config.mode == QuizMode::RandomGroup) {
        if (m_currentGroupIdx >= static_cast<int>(m_groups.size())) return std::nullopt;
        const auto& group = m_groups[m_currentGroupIdx];
        if (m_currentCardIdxInGroup >= static_cast<int>(group.size())) return std::nullopt;
        cardId = group[m_currentCardIdxInGroup];
    } else {
        if (m_currentCardIdxInGroup >= static_cast<int>(m_currentRoundCards.size())) return std::nullopt;
        cardId = m_currentRoundCards[m_currentCardIdxInGroup];
    }
    const Card* c = m_cardMgr->getCardById(cardId);
    if (c) return *c;
    return std::nullopt;
}

void QuizSession::recordResult(bool correct) {
    if (!m_active) return;
    if (m_config.mode == QuizMode::RandomGroup) {
        recordForMode1(correct);
    } else {
        recordForMode2(correct);
    }
}

void QuizSession::recordForMode1(bool correct) {
    int cardId = m_groups[m_currentGroupIdx][m_currentCardIdxInGroup];
    CardStats& stat = m_tempStats[cardId];
    stat.totalAttempts++;
    if (correct) stat.correctCount++;
    stat.lastResult = correct;
    stat.roundHistory.push_back(correct);

    m_currentCardIdxInGroup++;
    if (m_currentCardIdxInGroup >= static_cast<int>(m_groups[m_currentGroupIdx].size())) {
        // 当前组结束，进入下一组
        m_currentGroupIdx++;
        if (m_currentGroupIdx < static_cast<int>(m_groups.size())) {
            m_currentCardIdxInGroup = 0;
        } else {
            m_active = false; // 所有轮次结束
        }
    }
}

void QuizSession::recordForMode2(bool correct) {
    int cardId = m_currentRoundCards[m_currentCardIdxInGroup];
    CardStats& stat = m_tempStats[cardId];
    stat.totalAttempts++;
    if (correct) stat.correctCount++;
    stat.lastResult = correct;
    stat.roundHistory.push_back(correct);

    if (correct) {
        m_cardConsecutiveCorrect[cardId]++;
        if (m_cardConsecutiveCorrect[cardId] >= m_config.masteryRequired) {
            m_cardMastered[cardId] = true;
        }
    } else {
        m_cardConsecutiveCorrect[cardId] = 0;
    }

    m_currentCardIdxInGroup++;
    if (m_currentCardIdxInGroup >= static_cast<int>(m_currentRoundCards.size())) {
        // 本轮结束，构建下一轮未达标的卡片
        std::vector<int> nextRound;
        for (int cid : m_currentRoundCards) {
            if (!m_cardMastered[cid]) nextRound.push_back(cid);
        }
        if (nextRound.empty()) {
            m_active = false;
        } else {
            m_currentRoundCards = nextRound;
            m_currentCardIdxInGroup = 0;
        }
    }
}

bool QuizSession::isFinished() const {
    return !m_active;
}

void QuizSession::forceFinish() {
    m_active = false;
}

std::tuple<int, int, int, int> QuizSession::getProgress() const {
    if (m_config.mode == QuizMode::RandomGroup) {
        int totalGroups = static_cast<int>(m_groups.size());
        int currentGroup = m_currentGroupIdx;
        int curGroupTotal = (currentGroup < totalGroups) ? static_cast<int>(m_groups[currentGroup].size()) : 0;
        int curAnswered = m_currentCardIdxInGroup;
        return {currentGroup + 1, totalGroups, curAnswered, curGroupTotal};
    } else {
        int total = static_cast<int>(m_currentRoundCards.size());
        int answered = m_currentCardIdxInGroup;
        return {1, 1, answered, total}; // 模式二简化展示
    }
}

StatisticsDataset QuizSession::finalize() {
    StatisticsDataset ds;
    ds.name = "抽查会话";
    for (int cid : m_config.sourceCardIds) {
        ds.cardIds.push_back(cid);
        if (m_tempStats.find(cid) != m_tempStats.end()) {
            ds.stats[cid] = m_tempStats[cid];
        } else {
            // 未出现的卡片统计置零
            CardStats empty;
            ds.stats[cid] = empty;
        }
    }
    return ds;
}