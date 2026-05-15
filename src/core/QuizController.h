#ifndef QUIZCONTROLLER_H
#define QUIZCONTROLLER_H

#include <QObject>
#include <vector>
#include <unordered_map>
#include <optional>
#include "CardManager.h"
#include "StatisticsManager.h"

enum class QuizMode { RandomGroup, MasteryRecursive };
enum class DivideMode { Bycards, Bygroups, Bypercentage };

struct QuizConfig {
    QuizMode quizmode = QuizMode::RandomGroup;
    DivideMode dividemode = DivideMode::Bycards;
    int requiredValue = 2;
    std::vector<int> breakPoint;
    std::vector<int> sourceCardIds;
};

class QuizSession : public QObject {
    Q_OBJECT
public:
    QuizSession(const QuizConfig& config, CardManager* cardMgr, QObject* parent = nullptr);
    ~QuizSession();

    bool start();

    std::optional<Card> currentCard() const;
    //返回是否结束
    bool recordResult(bool correct);

    // 进度: (当前轮, 总轮数, 本轮已答, 本轮总数)
    std::tuple<int, int, int, int> getProgress() const;
    StatisticsDataset finalize();

private:
    QuizConfig m_config;
    CardManager* m_cardMgr;

    // 模式一
    std::vector<std::vector<int>> m_groups;
    int m_currentGroupIdx = 0;
    int m_currentCardIdxInGroup = 0;
    // 模式二
    std::vector<int> m_currentRoundCards;
    std::unordered_map<int, int> m_cardConsecutiveCorrect;
    std::unordered_map<int, bool> m_cardMastered;

    std::unordered_map<int, CardStats> m_tempStats;

    void setupRandomGroups();
    void setupMasteryFirstRound();
    //返回记录后是否结束
    bool recordForMode1(bool correct);
    bool recordForMode2(bool correct);
};

#endif // QUIZCONTROLLER_H