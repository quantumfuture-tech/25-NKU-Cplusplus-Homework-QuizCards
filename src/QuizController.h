// FlashcardConsole/src/QuizController.h
#pragma once

#include <vector>
#include <functional>
#include <optional>
#include "CardManager.h"
#include "StatisticsManager.h"

// 抽查模式参数
enum class QuizMode { RandomGroup, MasteryRecursive };

struct QuizConfig {
    QuizMode mode = QuizMode::RandomGroup;
    // 每一轮的断点（该轮末索引+1）（模式一）
    std::vector<int> breakPoint;   
    // 达标所需连续正确次数（模式二）
    int masteryRequired = 2;       
    // 要抽查的卡片ID列表（从题库或抽查筐获取）
    std::vector<int> sourceCardIds; 
};

// 一个进行中的抽查会话
class QuizSession {
public:
    QuizSession(const QuizConfig& config, CardManager* cardMgr);
    ~QuizSession();

    // 开始抽查，返回是否成功
    bool start();

    // 当前是否进行中
    bool isActive() const { return m_active; }

    // 获取当前卡片（问题）
    std::optional<Card> currentCard() const;

    // 记录当前卡片的答题结果（true=正确，false=错误）
    void recordResult(bool correct);

    // 是否已完成所有轮次
    bool isFinished() const;

    // 提前结束
    void forceFinish();

    // 获取进度信息 (当前轮, 总轮数, 本轮已答数量, 本轮总卡片数)
    std::tuple<int, int, int, int> getProgress() const;

    // 获取最终统计数据集
    StatisticsDataset finalize();

private:
    bool m_active = false;
    QuizConfig m_config;
    CardManager* m_cardMgr = nullptr;

    // 模式一：分组列表，每个分组是卡片ID列表
    std::vector<std::vector<int>> m_groups;
    int m_currentGroupIdx = 0;      // 当前进行到第几组（轮）
    int m_currentCardIdxInGroup = 0; // 当前组内第几张卡片
    std::vector<int> m_currentGroupCards;

    // 模式二：当前轮次需要测试的卡片列表
    std::vector<int> m_currentRoundCards;
    std::unordered_map<int, int> m_cardConsecutiveCorrect; // 连续正确次数
    std::unordered_map<int, bool> m_cardMastered;          // 是否已达标

    // 统计结果暂存
    std::unordered_map<int, CardStats> m_tempStats;

    void setupRandomGroups();
    void setupMasteryFirstRound();
    void advanceToNextCard();
    void advanceToNextRoundOrFinish();
    void recordForMode1(bool correct);
    void recordForMode2(bool correct);
};