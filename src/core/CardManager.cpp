#include "CardManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <set>

// TreeNode 序列化
json TreeNode::toJson() const {
    json j;
    j["name"] = name;
    j["active"] = active;
    if (type == TreeNodeType::Folder) {
        j["type"] = "folder";
    } else {
        j["type"] = "unit";
        j["cardIds"] = cardIds;
    }
    return j;
}

TreeNode TreeNode::fromJson(const json& j, TreeNodeType t) {
    TreeNode node;
    node.type = t;
    node.name = j.value("name", "");
    node.active = j.value("active", true);
    if (t == TreeNodeType::Unit && j.contains("cardIds")) {
        node.cardIds = j["cardIds"].get<std::vector<int>>();
    }
    return node;
}

CardManager::CardManager(const std::filesystem::path& cardsRoot, QObject* parent)
    : QObject(parent), m_cardsRoot(cardsRoot) {
    TreeNode root;
    root.type = TreeNodeType::Folder;
    root.name = "题库根目录";
    root.active = true;
    m_nodes.push_back(root);
}

CardManager::~CardManager() {}

bool CardManager::load() {
    if (!std::filesystem::exists(m_cardsRoot)) {
        std::filesystem::create_directories(m_cardsRoot);
        return true;
    }
    bool success = scanDirectory(m_cardsRoot, 0);
    rebuildCardToNodeMap();
    emit dataChanged();
    return success;
}

bool CardManager::scanDirectory(const std::filesystem::path& dirPath, int parentNodeIdx) {
    if (!std::filesystem::is_directory(dirPath)) return false;

    // 读取 .tree 文件
    auto treeFile = dirPath / ".tree";
    if (std::filesystem::exists(treeFile)) {
        std::ifstream ifs(treeFile);
        if (ifs) {
            json j;
            ifs >> j;
            if (j.contains("active")) m_nodes[parentNodeIdx].active = j["active"].get<bool>();
            if (j.contains("name")) m_nodes[parentNodeIdx].name = j["name"].get<std::string>();
        }
    }

    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        auto filename = entry.path().filename().string();
        if (entry.is_directory()) {
            TreeNode folderNode;
            folderNode.type = TreeNodeType::Folder;
            folderNode.name = filename;
            folderNode.active = true;
            int folderIdx = static_cast<int>(m_nodes.size());
            m_nodes.push_back(folderNode);
            m_nodes[parentNodeIdx].childrenIndices.push_back(folderIdx);
            scanDirectory(entry.path(), folderIdx);
        } else if (filename.size() >= 10 && filename.ends_with(".unit.json")) {
            loadUnitFile(entry.path(), parentNodeIdx);
        }
    }
    return true;
}

bool CardManager::loadUnitFile(const std::filesystem::path& unitPath, int parentIdx) {
    std::ifstream ifs(unitPath);
    if (!ifs) return false;
    json j;
    try { ifs >> j; } catch (...) { return false; }

    TreeNode unitNode;
    unitNode.type = TreeNodeType::Unit;
    unitNode.name = j.value("name", unitPath.stem().string());
    unitNode.active = j.value("active", true);

    std::vector<int> cardIds;
    if (j.contains("cards") && j["cards"].is_array()) {
        for (const auto& cardJson : j["cards"]) {
            std::string question = cardJson.value("question", "");
            std::string answer = cardJson.value("answer", "");
            bool active = cardJson.value("active", true);

            int existingId = findDuplicateCard(question, answer);
            if (existingId != -1) {
                cardIds.push_back(existingId);
                for (auto& c : m_cards) {
                    if (c.id == existingId) { c.active = active; break; }
                }
            } else {
                Card newCard;
                newCard.id = generateNewCardId();
                newCard.question = question;
                newCard.answer = answer;
                newCard.active = active;
                m_cards.push_back(newCard);
                cardIds.push_back(newCard.id);
            }
        }
    }
    unitNode.cardIds = cardIds;

    int unitIdx = static_cast<int>(m_nodes.size());
    m_nodes.push_back(unitNode);
    m_nodes[parentIdx].childrenIndices.push_back(unitIdx);
    return true;
}

int CardManager::findDuplicateCard(const std::string& question, const std::string& answer) const {
    for (const auto& card : m_cards) {
        if (card.question == question && card.answer == answer) return card.id;
    }
    return -1;
}

void CardManager::addCardToGlobal(const Card& card) { m_cards.push_back(card); }

const Card* CardManager::getCardById(int id) const {
    for (const auto& card : m_cards) {
        if (card.id == id) return &card;
    }
    return nullptr;
}

// 设置卡片激活状态，并更新所有包含此卡片的单元文件
void CardManager::setCardActive(int cardId, bool active) {
    Card* card = const_cast<Card*>(getCardById(cardId));
    if (!card) return;
    if (card->active == active) return;
    
    card->active = active;
    
    // 找到所有包含此卡片的单元，重新保存
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].type == TreeNodeType::Unit) {
            auto& ids = m_nodes[i].cardIds;
            if (std::find(ids.begin(), ids.end(), cardId) != ids.end()) {
                saveUnitToFile(static_cast<int>(i));
            }
        }
    }
    emit dataChanged();
}

// 递归设置节点及其所有后代的激活状态
void CardManager::setNodeActiveRecursively(int nodeIdx, bool active) {
    if (nodeIdx < 0 || nodeIdx >= static_cast<int>(m_nodes.size()))
        return;
    
    TreeNode& node = m_nodes[nodeIdx];
    if (node.active == active)
        return;  // 状态相同则跳过，避免无效递归
    
    node.active = active;
    
    // 1. 如果当前节点是 Folder，递归处理所有子节点
    if (node.type == TreeNodeType::Folder) {
        for (int childIdx : node.childrenIndices) {
            setNodeActiveRecursively(childIdx, active);
        }
    } 
    // 2. 如果当前节点是 Unit，则处理其包含的所有卡片
    else if (node.type == TreeNodeType::Unit) {
        for (int cardId : node.cardIds) {
            setCardActive(cardId, active);
        }
        // 保存单元文件（因为单元自身的 active 状态已改变）
        saveUnitToFile(nodeIdx);
    }
    
    // 3. 如果当前节点是 Folder，需要保存其 .tree 文件
    if (node.type == TreeNodeType::Folder) {
        saveFolderTreeFile(nodeIdx);
    }
    
    emit dataChanged();
}

std::filesystem::path CardManager::getNodePath(int nodeIdx) const {
    if (nodeIdx < 0 || nodeIdx >= static_cast<int>(m_nodes.size()))
        return {};
    
    // 递归构建路径：从根节点开始向下
    std::vector<int> pathIndices;
    int cur = nodeIdx;
    while (cur != 0) {
        pathIndices.push_back(cur);
        // 查找父节点
        for (size_t i = 0; i < m_nodes.size(); ++i) {
            for (int child : m_nodes[i].childrenIndices) {
                if (child == cur) {
                    cur = static_cast<int>(i);
                    goto found;
                }
            }
        }
        // 未找到父节点（理论上根节点除外）
        return {};
        found:;
    }
    // 反转路径（从根到目标）
    std::reverse(pathIndices.begin(), pathIndices.end());
    
    std::filesystem::path result = m_cardsRoot;
    for (int idx : pathIndices) {
        const auto& node = m_nodes[idx];
        if (node.type == TreeNodeType::Folder) {
            result /= node.name;
        } else { // Unit
            result /= (node.name + ".unit.json");
        }
    }
    return result;
}

bool CardManager::ensureFolderPath(const std::filesystem::path& path) {
    if (std::filesystem::exists(path))
        return std::filesystem::is_directory(path);
    return std::filesystem::create_directories(path);
}

int CardManager::addCardToUnit(int unitIdx, const std::string& question, const std::string& answer, bool active) {
    if (unitIdx < 0 || unitIdx >= static_cast<int>(m_nodes.size()) || m_nodes[unitIdx].type != TreeNodeType::Unit)
        return -1;

    int existingId = findDuplicateCard(question, answer);
    if (existingId != -1) {
        m_nodes[unitIdx].cardIds.push_back(existingId);
        saveUnitToFile(unitIdx);
        emit dataChanged();
        return existingId;
    }

    Card newCard;
    newCard.id = generateNewCardId();
    newCard.question = question;
    newCard.answer = answer;
    newCard.active = active;
    m_cards.push_back(newCard);
    m_nodes[unitIdx].cardIds.push_back(newCard.id);
    saveUnitToFile(unitIdx);
    rebuildCardToNodeMap();
    emit dataChanged();
    return newCard.id;
}

bool CardManager::removeCardFromUnit(int unitIdx, int cardId) {
    if (unitIdx < 0 || unitIdx >= static_cast<int>(m_nodes.size()) || m_nodes[unitIdx].type != TreeNodeType::Unit)
        return false;

    auto& ids = m_nodes[unitIdx].cardIds;
    auto it = std::find(ids.begin(), ids.end(), cardId);
    if (it == ids.end()) return false;
    ids.erase(it);
    saveUnitToFile(unitIdx);
    emit dataChanged();
    return true;
}

bool CardManager::updateCard(int cardId, const std::string& newQuestion, const std::string& newAnswer, bool newActive) {
    Card* card = const_cast<Card*>(getCardById(cardId));
    if (!card) return false;
    card->question = newQuestion;
    card->answer = newAnswer;
    card->active = newActive;
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].type == TreeNodeType::Unit) {
            if (std::find(m_nodes[i].cardIds.begin(), m_nodes[i].cardIds.end(), cardId) != m_nodes[i].cardIds.end()) {
                saveUnitToFile(static_cast<int>(i));
            }
        }
    }
    emit dataChanged();
    return true;
}

int CardManager::createUnit(int parentFolderIdx, const std::string& unitName) {
    if (parentFolderIdx < 0 || parentFolderIdx >= static_cast<int>(m_nodes.size()) ||
        m_nodes[parentFolderIdx].type != TreeNodeType::Folder)
        return -1;

    // 获取父文件夹路径
    std::filesystem::path parentPath = getNodePath(parentFolderIdx);
    if (parentPath.empty()) return -1;
    
    // 确保文件夹存在
    if (!ensureFolderPath(parentPath)) return -1;
    
    // 生成唯一文件名
    std::filesystem::path unitPath = parentPath / (unitName + ".unit.json");
    int counter = 1;
    while (std::filesystem::exists(unitPath)) {
        unitPath = parentPath / (unitName + "_" + std::to_string(counter) + ".unit.json");
        counter++;
    }
    
    // 创建 JSON 文件
    json j;
    j["name"] = unitName;
    j["active"] = true;
    j["cards"] = json::array();
    std::ofstream ofs(unitPath);
    ofs << j.dump(4);
    
    TreeNode unit;
    unit.type = TreeNodeType::Unit;
    unit.name = unitName;
    unit.active = true;
    int unitIdx = static_cast<int>(m_nodes.size());
    m_nodes.push_back(unit);
    m_nodes[parentFolderIdx].childrenIndices.push_back(unitIdx);
    
    emit dataChanged();
    return unitIdx;
}

int CardManager::createFolder(int parentFolderIdx, const std::string& folderName) {
    if (parentFolderIdx < 0 || parentFolderIdx >= static_cast<int>(m_nodes.size()) ||
        m_nodes[parentFolderIdx].type != TreeNodeType::Folder)
        return -1;

    // 获取父文件夹路径
    std::filesystem::path parentPath = getNodePath(parentFolderIdx);
    if (parentPath.empty()) return -1;
    
    std::filesystem::path newDir = parentPath / folderName;
    if (std::filesystem::exists(newDir)) return -1;
    
    if (!std::filesystem::create_directory(newDir)) return -1;
    
    TreeNode folder;
    folder.type = TreeNodeType::Folder;
    folder.name = folderName;
    folder.active = true;
    int folderIdx = static_cast<int>(m_nodes.size());
    m_nodes.push_back(folder);
    m_nodes[parentFolderIdx].childrenIndices.push_back(folderIdx);
    
    // 创建 .tree 文件保存属性
    saveFolderTreeFile(folderIdx);
    
    emit dataChanged();
    return folderIdx;
}

bool CardManager::saveUnitToFile(int unitIdx) {
    const TreeNode& unit = m_nodes[unitIdx];
    if (unit.type != TreeNodeType::Unit) return false;

    std::filesystem::path targetPath = getNodePath(unitIdx);
    if (targetPath.empty()) return false;
    
    // 确保所在目录存在
    ensureFolderPath(targetPath.parent_path());
    
    json j;
    j["name"] = unit.name;
    j["active"] = unit.active;
    json cardsArray = json::array();
    for (int cid : unit.cardIds) {
        const Card* card = getCardById(cid);
        if (card) {
            json cardJson;
            cardJson["question"] = card->question;
            cardJson["answer"] = card->answer;
            cardJson["active"] = card->active;
            cardsArray.push_back(cardJson);
        }
    }
    j["cards"] = cardsArray;
    std::ofstream ofs(targetPath);
    ofs << j.dump(4);
    return true;
}

bool CardManager::saveFolderTreeFile(int folderIdx) {
    const TreeNode& folder = m_nodes[folderIdx];
    if (folder.type != TreeNodeType::Folder) return false;
    
    std::filesystem::path folderPath = getNodePath(folderIdx);
    if (folderPath.empty()) return false;
    
    std::filesystem::path treePath = folderPath / ".tree";
    json j;
    j["name"] = folder.name;
    j["active"] = folder.active;
    std::ofstream ofs(treePath);
    ofs << j.dump(4);
    return true;
}

bool CardManager::saveNode(int nodeIdx) {
    if (nodeIdx < 0 || nodeIdx >= static_cast<int>(m_nodes.size())) return false;
    if (m_nodes[nodeIdx].type == TreeNodeType::Unit) return saveUnitToFile(nodeIdx);
    else return saveFolderTreeFile(nodeIdx);
}

std::vector<int> CardManager::getActiveCardIds() const {
    std::set<int> allIds;
    for (const auto& node : m_nodes) {
        if (node.type == TreeNodeType::Unit && node.active) {
            for (int cid : node.cardIds) {
                const Card* card = getCardById(cid);
                if (card && card->active) allIds.insert(cid);
            }
        }
    }
    return std::vector<int>(allIds.begin(), allIds.end());
}

void CardManager::rebuildCardToNodeMap() {
    m_cardToNode.clear();
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].type == TreeNodeType::Unit) {
            for (int cid : m_nodes[i].cardIds) m_cardToNode[cid] = static_cast<int>(i);
        }
    }
}

void CardManager::printTree(int nodeIdx, int depth) const {
    // 仅用于调试
}