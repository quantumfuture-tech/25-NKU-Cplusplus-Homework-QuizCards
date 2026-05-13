// FlashcardConsole/src/main.cpp
#include "CardManager.h"
#include "StatisticsManager.h"
#include "QuizController.h"
#include <iostream>
#include <string>
#include <limits>
#include <cctype>

// 辅助：打印卡片列表（用于抽查配置）
void printCards(const std::vector<int>& cardIds, CardManager& cardMgr) {
    for (int cid : cardIds) {
        const Card* c = cardMgr.getCardById(cid);
        if (c) {
            std::cout << "  [" << cid << "] " << c->question.substr(0, 40) << " (激活:" << (c->active ? "是" : "否") << ")" << std::endl;
        }
    }
}

// 演示抽查流程
void runQuiz(CardManager& cardMgr, StatisticsManager& statMgr) {
    // 获取激活卡片
    std::vector<int> activeCards = cardMgr.getActiveCardIds();
    if (activeCards.empty()) {
        std::cout << "没有激活的卡片，无法抽查。请先在题库中激活一些卡片。" << std::endl;
        return;
    }

    std::cout << "\n===== 开始抽查 =====" << std::endl;
    std::cout << "当前共有 " << activeCards.size() << " 张激活卡片可用于抽查。" << std::endl;
    std::cout << "选择抽查模式: 1-随机分组模式  2-达标递归模式: ";
    int modeChoice;
    std::cin >> modeChoice;
    std::cin.ignore();

    QuizConfig config;
    config.sourceCardIds = activeCards;
    if (modeChoice == 1) {
        config.mode = QuizMode::RandomGroup;
        std::cout << "每组卡片数量 (默认5): ";
        std::cin >> config.groupSize;
        std::cout << "总轮数 (默认3): ";
        std::cin >> config.totalRounds;
        std::cin.ignore();
    } else {
        config.mode = QuizMode::MasteryRecursive;
        std::cout << "达标所需连续正确次数 (默认2): ";
        std::cin >> config.masteryRequired;
        std::cin.ignore();
    }

    QuizSession session(config, &cardMgr);
    if (!session.start()) {
        std::cout << "启动抽查失败。" << std::endl;
        return;
    }

    while (session.isActive()) {
        auto curCardOpt = session.currentCard();
        if (!curCardOpt) break;
        const Card& card = *curCardOpt;
        auto [round, totalRounds, answered, roundTotal] = session.getProgress();
        std::cout << "\n--- 进度: 轮次 " << round << "/" << totalRounds
                  << "  卡片 " << answered + 1 << "/" << roundTotal << " ---" << std::endl;
        std::cout << "问题: " << card.question << std::endl;
        std::cout << "按任意键显示答案...";
        std::cin.get();
        std::cout << "答案: " << card.answer << std::endl;
        std::cout << "回答正确吗? (y/n): ";
        char ans;
        std::cin >> ans;
        std::cin.ignore();
        bool correct = (ans == 'y' || ans == 'Y');
        session.recordResult(correct);
    }

    StatisticsDataset result = session.finalize();
    statMgr.saveSession(result);

    // 简单显示统计
    std::cout << "\n抽查结束，统计摘要:" << std::endl;
    for (int cid : result.cardIds) {
        const Card* c = cardMgr.getCardById(cid);
        if (c && result.stats.count(cid)) {
            const auto& stat = result.stats.at(cid);
            int correct = stat.correctCount;
            int total = stat.totalAttempts;
            int percent = total > 0 ? (correct * 100 / total) : 0;
            std::cout << "  [" << cid << "] " << c->question.substr(0, 30) << " ... 正确率: " << percent << "% (" << correct << "/" << total << ")" << std::endl;
        }
    }
}

// 浏览统计历史
void browseStatistics(StatisticsManager& statMgr) {
    statMgr.load();
    statMgr.listAllSessions();
    std::cout << "输入会话文件名查看详情（如 2026-05-12_10-30.session.json）或按回车返回: ";
    std::string filename;
    std::getline(std::cin, filename);
    if (!filename.empty()) {
        auto ds = statMgr.loadSession(statMgr.rootPath() / filename);
        if (ds) {
            std::cout << "会话: " << ds->name << std::endl;
            for (int cid : ds->cardIds) {
                if (ds->stats.count(cid)) {
                    const auto& stat = ds->stats.at(cid);
                    std::cout << "  卡片ID " << cid << " 尝试次数:" << stat.totalAttempts
                              << " 正确次数:" << stat.correctCount << std::endl;
                }
            }
        } else {
            std::cout << "加载失败。" << std::endl;
        }
    }
}

// 题库管理简单菜单
void manageLibrary(CardManager& cardMgr) {
    int choice;
    do {
        std::cout << "\n===== 题库管理 =====" << std::endl;
        std::cout << "1. 显示题库树" << std::endl;
        std::cout << "2. 添加新单元（根目录）" << std::endl;
        std::cout << "3. 为单元添加卡片" << std::endl;
        std::cout << "4. 从单元删除卡片" << std::endl;
        std::cout << "5. 返回主菜单" << std::endl;
        std::cout << "请选择: ";
        std::cin >> choice;
        std::cin.ignore();

        if (choice == 1) {
            cardMgr.printTree();
        } else if (choice == 2) {
            std::string unitName;
            std::cout << "单元名称: ";
            std::getline(std::cin, unitName);
            int idx = cardMgr.createUnit(0, unitName);
            if (idx != -1) std::cout << "单元创建成功。" << std::endl;
            else std::cout << "创建失败。" << std::endl;
        } else if (choice == 3) {
            cardMgr.printTree();
            std::cout << "请输入单元索引（从0开始，根据打印顺序）: ";
            int unitIdx;
            std::cin >> unitIdx;
            std::cin.ignore();
            if (unitIdx < 0 || unitIdx >= static_cast<int>(cardMgr.getNode(unitIdx).type == TreeNodeType::Unit ? 1 : 0)) {
                std::cout << "无效索引。" << std::endl;
                continue;
            }
            std::string question, answer;
            std::cout << "问题: ";
            std::getline(std::cin, question);
            std::cout << "答案: ";
            std::getline(std::cin, answer);
            int newId = cardMgr.addCardToUnit(unitIdx, question, answer);
            if (newId != -1) std::cout << "卡片添加成功，ID=" << newId << std::endl;
        } else if (choice == 4) {
            cardMgr.printTree();
            std::cout << "单元索引: ";
            int unitIdx;
            std::cin >> unitIdx;
            std::cin.ignore();
            std::cout << "卡片ID: ";
            int cid;
            std::cin >> cid;
            std::cin.ignore();
            if (cardMgr.removeCardFromUnit(unitIdx, cid)) {
                std::cout << "删除成功。" << std::endl;
            } else {
                std::cout << "删除失败。" << std::endl;
            }
        }
    } while (choice != 5);
}

int main() {
    std::cout << "抽认卡学习应用 (控制台原型)" << std::endl;
    // 确保数据目录存在
    std::filesystem::create_directories("data/cards");
    std::filesystem::create_directories("data/stats");

    CardManager cardMgr("data/cards");
    StatisticsManager statMgr("data/stats");

    // 加载题库
    if (!cardMgr.load()) {
        std::cout << "加载题库失败，请检查 data/cards 目录。" << std::endl;
        return 1;
    }
    std::cout << "题库加载成功。" << std::endl;

    int mainChoice;
    do {
        std::cout << "\n===== 主菜单 =====" << std::endl;
        std::cout << "1. 题库管理" << std::endl;
        std::cout << "2. 开始抽查" << std::endl;
        std::cout << "3. 浏览统计历史" << std::endl;
        std::cout << "0. 退出" << std::endl;
        std::cout << "请选择: ";
        std::cin >> mainChoice;
        std::cin.ignore();

        switch (mainChoice) {
            case 1: manageLibrary(cardMgr); break;
            case 2: runQuiz(cardMgr, statMgr); break;
            case 3: browseStatistics(statMgr); break;
            case 0: std::cout << "再见。" << std::endl; break;
            default: std::cout << "无效输入。" << std::endl;
        }
    } while (mainChoice != 0);

    return 0;
}