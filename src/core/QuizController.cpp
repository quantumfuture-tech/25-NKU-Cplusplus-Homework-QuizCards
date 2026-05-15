#include "QuizController.h"
#include "Debug.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <iostream>
#include <QDebug>
QuizSession::QuizSession(const QuizConfig& config, CardManager* cardMgr, QObject* parent)
    : QObject(parent), m_config(config), m_cardMgr(cardMgr) {}

QuizSession::~QuizSession() {}

bool QuizSession::start() {
    if (m_config.sourceCardIds.empty()) return false;
    if (m_config.quizmode == QuizMode::RandomGroup) {
        setupRandomGroups();
        if (m_groups.empty()) return false;
        m_currentGroupIdx = 0;
        m_currentCardIdxInGroup = 0;
    } else {
        setupMasteryFirstRound();
        if (m_currentRoundCards.empty()) return false;
        m_currentCardIdxInGroup = 0;
    }
    return true;
}

void QuizSession::setupRandomGroups() {
    std::vector<int> shuffled = m_config.sourceCardIds;
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::shuffle(shuffled.begin(), shuffled.end(), std::default_random_engine(seed));

    m_groups.clear();
    size_t total = shuffled.size();
    int start = 0;
    for (auto bpt : m_config.breakPoint) {
        std::vector<int> group(shuffled.begin() + start, shuffled.begin() + bpt);
        m_groups.push_back(group);
        start = bpt;
    }
}

void QuizSession::setupMasteryFirstRound() {
    m_currentRoundCards = m_config.sourceCardIds;
    for (int cid : m_currentRoundCards) {
        m_cardConsecutiveCorrect[cid] = 0;
        m_cardMastered[cid] = false;
    }
}

std::optional<Card> QuizSession::currentCard() const {
    //根据当前进度获取卡片
    int cardId = -1;
    if (m_config.quizmode == QuizMode::RandomGroup) {
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
//返回是否结束
bool QuizSession::recordResult(bool correct) {
    myDebug()<<"获取结果";
    if (m_config.quizmode == QuizMode::RandomGroup) return recordForMode1(correct);
    else return recordForMode2(correct);
}
//返回是否结束
bool QuizSession::recordForMode1(bool correct) {
    qDebug() << "尝试记录结果并更新进度" ;
    int cardId = m_groups[m_currentGroupIdx][m_currentCardIdxInGroup];
    CardStats& stat = m_tempStats[cardId];
    stat.totalAttempts++;
    if (correct) stat.correctCount++;
    stat.lastResult = correct;
    stat.roundHistory.push_back(correct);
    m_currentCardIdxInGroup++;
    if (m_currentCardIdxInGroup >= static_cast<int>(m_groups[m_currentGroupIdx].size())) {
        m_currentGroupIdx++;
        if (m_currentGroupIdx < static_cast<int>(m_groups.size())) {
            m_currentCardIdxInGroup = 0;
        } else {
            m_currentGroupIdx--;
            qDebug() << "检测到抽查结束！";
            return false;
        }
    }
    qDebug() << "成功记录！";
    return true;
}

bool QuizSession::recordForMode2(bool correct) {
    int cardId = m_currentRoundCards[m_currentCardIdxInGroup];
    CardStats& stat = m_tempStats[cardId];
    stat.totalAttempts++;
    if (correct) stat.correctCount++;
    stat.lastResult = correct;
    stat.roundHistory.push_back(correct);

    if (correct) {
        m_cardConsecutiveCorrect[cardId]++;
        if (m_cardConsecutiveCorrect[cardId] >= m_config.requiredValue) {
            m_cardMastered[cardId] = true;
        }
    } else {
        m_cardConsecutiveCorrect[cardId] = 0;
    }

    m_currentCardIdxInGroup++;
    if (m_currentCardIdxInGroup >= static_cast<int>(m_currentRoundCards.size())) {
        std::vector<int> nextRound;
        for (int cid : m_currentRoundCards) {
            if (!m_cardMastered[cid]) nextRound.push_back(cid);
        }
        if (nextRound.empty()) {
            return false;
        } else {
            m_currentRoundCards = nextRound;
            m_currentCardIdxInGroup = 0;
        }
    }
    return true;
}


std::tuple<int, int, int, int> QuizSession::getProgress() const {
    qDebug()<<"进度条请求获取进度";
    if (m_config.quizmode == QuizMode::RandomGroup) {
        int totalGroups = static_cast<int>(m_groups.size());
        int currentGroup = m_currentGroupIdx;
        int curGroupTotal = (currentGroup < totalGroups) ? static_cast<int>(m_groups[currentGroup].size()) : 0;
        int curAnswered = m_currentCardIdxInGroup;
        return {currentGroup + 1, totalGroups, curAnswered, curGroupTotal};
    } else {
        int total = static_cast<int>(m_currentRoundCards.size());
        int answered = m_currentCardIdxInGroup;
        return {1, 1, answered, total};
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
            ds.stats[cid] = CardStats();
        }
    }
    return ds;
}