#ifndef CARDMANAGER_H
#define CARDMANAGER_H

#include <QObject>
#include <filesystem>
#include <vector>
#include <unordered_map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// 卡片结构
struct Card {
    int id = 0;
    std::string question;
    std::string answer;
    bool active = true;

    bool operator==(const Card& other) const {
        return question == other.question && answer == other.answer;
    }
};

enum class TreeNodeType { Folder, Unit };

struct TreeNode {
    TreeNodeType type = TreeNodeType::Folder;
    std::string name;
    bool active = true;

    std::vector<int> childrenIndices;   // Folder 专用
    std::vector<int> cardIds;           // Unit 专用

    json toJson() const;
    static TreeNode fromJson(const json& j, TreeNodeType t);
};

class CardManager : public QObject {
    Q_OBJECT
public:
    explicit CardManager(const std::filesystem::path& cardsRoot, QObject* parent = nullptr);
    ~CardManager();

    bool load();
    bool saveNode(int nodeIdx);

    int rootIndex() const { return 0; }
    const TreeNode& getNode(int idx) const { return m_nodes.at(idx); }
    TreeNode& getNode(int idx) { return m_nodes.at(idx); }
    const std::vector<Card>& getAllCards() const { return m_cards; }
    const Card* getCardById(int id) const;

    // 递归设置节点及其所有后代的激活状态（包括单元内的卡片）
    void setNodeActiveRecursively(int nodeIdx, bool active);
    // 设置卡片的激活状态，并保存所有引用该卡片的单元文件
    void setCardActive(int cardId, bool active);

    int addCardToUnit(int unitIdx, const std::string& question, const std::string& answer, bool active = true);
    bool removeCardFromUnit(int unitIdx, int cardId);
    bool updateCard(int cardId, const std::string& newQuestion, const std::string& newAnswer, bool newActive);
    int createUnit(int parentFolderIdx, const std::string& unitName);
    int createFolder(int parentFolderIdx, const std::string& folderName);

    std::vector<int> getActiveCardIds() const;
    void printTree(int nodeIdx = -1, int depth = 0) const;

    const std::vector<TreeNode>& getNodes() const { return m_nodes; }

signals:
    void dataChanged();

private:
    std::filesystem::path m_cardsRoot;
    std::vector<TreeNode> m_nodes;
    std::vector<Card> m_cards;
    std::unordered_map<int, int> m_cardToNode;

    bool scanDirectory(const std::filesystem::path& dirPath, int parentNodeIdx);
    bool loadUnitFile(const std::filesystem::path& unitPath, int parentIdx);
    bool saveUnitToFile(int unitIdx);
    bool saveFolderTreeFile(int folderIdx);
    int generateNewCardId() const { return m_cards.empty() ? 1 : m_cards.back().id + 1; }
    int findDuplicateCard(const std::string& question, const std::string& answer) const;
    void addCardToGlobal(const Card& card);
    void rebuildCardToNodeMap();
    // 获取节点对应的文件系统路径（对于 Folder 返回目录路径，对于 Unit 返回 .unit.json 文件路径）
    std::filesystem::path getNodePath(int nodeIdx) const;
    // 确保文件夹路径存在（创建目录）
    bool ensureFolderPath(const std::filesystem::path& path);
};

#endif // CARDMANAGER_H