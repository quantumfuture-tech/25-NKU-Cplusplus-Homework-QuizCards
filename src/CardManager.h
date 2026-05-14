// FlashcardConsole/src/CardManager.h
#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <memory>
#include <optional>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 单张卡片
struct Card {
    int id = 0;
    std::string question;
    std::string answer;
    bool active = true;

    bool operator==(const Card& other) const {
        return question == other.question && answer == other.answer;
    }
};

// 树节点类型
enum class TreeNodeType { Folder, Unit };

// 树节点，用 vector 存储所有节点，根节点索引为 0
struct TreeNode {
    TreeNodeType type = TreeNodeType::Folder;
    std::string name;
    bool active = true;

    // Folder 专用
    std::vector<int> childrenIndices;   // 子节点在 m_nodes 中的索引

    // Unit 专用
    std::vector<int> cardIds;           // 此单元包含的卡片 ID

    // 用于序列化到 .tree 或 .unit.json
    json toJson() const;
    static TreeNode fromJson(const json& j, TreeNodeType t);
};

// 管理全局卡片和题库树
class CardManager {
public:
    CardManager(const std::filesystem::path& cardsRoot);
    ~CardManager();

    // 加载整个题库（扫描目录，重建树和卡片列表）
    bool load();

    // 保存某个节点的属性（.tree 文件）或单元内容（.unit.json）
    bool saveNode(int nodeIdx);

    // 获取根节点索引
    int rootIndex() const { return 0; }

    // 根据索引获取节点
    const TreeNode& getNode(int idx) const { return m_nodes.at(idx); }
    TreeNode& getNode(int idx) { return m_nodes.at(idx); }

    // 获取全局卡片
    const std::vector<Card>& getAllCards() const { return m_cards; }
    const Card* getCardById(int id) const;

    // 添加新卡片到某个单元（会立即保存单元文件）
    int addCardToUnit(int unitIdx, const std::string& question, const std::string& answer, bool active = true);

    // 从单元删除卡片
    bool removeCardFromUnit(int unitIdx, int cardId);

    // 更新卡片内容
    bool updateCard(int cardId, const std::string& newQuestion, const std::string& newAnswer, bool newActive);

    // 在指定文件夹下创建新单元
    int createUnit(int parentFolderIdx, const std::string& unitName);

    // 在指定文件夹下创建新文件夹
    int createFolder(int parentFolderIdx, const std::string& folderName);

    // 获取当前激活的所有卡片（所有激活单元中的激活卡片）
    std::vector<int> getActiveCardIds() const;

    // 调试打印树结构
    void printTree(int nodeIdx = -1, int depth = 0) const;

private:
    std::filesystem::path m_cardsRoot;
    std::vector<TreeNode> m_nodes;      // 存储所有节点，索引即 ID
    std::vector<Card> m_cards;          // 全局卡片列表，id 从 1 开始递增
    std::unordered_map<int, int> m_cardToNode; // 卡片ID -> 所属单元节点索引（用于快速定位）

    // 遍历目录构建树 (递归)
    bool scanDirectory(const std::filesystem::path& dirPath, int parentNodeIdx);

    // 加载单个 .unit.json 文件
    bool loadUnitFile(const std::filesystem::path& unitPath, int parentIdx);

    // 保存单元的所有卡片到 .unit.json
    bool saveUnitToFile(int unitIdx);

    // 保存文件夹属性到 .tree 文件
    bool saveFolderTreeFile(int folderIdx);

    // 生成唯一卡片 ID
    int generateNewCardId() const { return m_cards.empty() ? 1 : m_cards.back().id + 1; }

    // 去重：如果已有相同问题和答案的卡片，返回其 ID，否则返回 -1
    int findDuplicateCard(const std::string& question, const std::string& answer) const;

    // 更新全局卡片列表（当添加卡片时调用）
    void addCardToGlobal(const Card& card);

    // 重新建立 cardToNode 映射
    void rebuildCardToNodeMap();
};