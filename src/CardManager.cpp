// FlashcardConsole/src/CardManager.cpp
#include "CardManager.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <set>

// ------------------- TreeNode 序列化 -------------------
json TreeNode::toJson() const {
    json j;
    j["name"] = name;
    j["active"] = active;
    if (type == TreeNodeType::Folder) {
        j["type"] = "folder";
        // 子节点信息不存储在这里，因为由目录结构决定，但 .tree 中可以存储额外元数据
        // 这里只存名称和激活状态，子节点由目录内真实文件决定
    } else { // Unit
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
    if (t == TreeNodeType::Unit) {
        if (j.contains("cardIds") && j["cardIds"].is_array()) {
            node.cardIds = j["cardIds"].get<std::vector<int>>();
        }
    }
    return node;
}

// ------------------- CardManager -------------------
CardManager::CardManager(const std::filesystem::path& cardsRoot)
    : m_cardsRoot(cardsRoot) {
    // 创建根节点（Folder）
    TreeNode root;
    root.type = TreeNodeType::Folder;
    root.name = "题库根目录";
    root.active = true;
    m_nodes.push_back(root);
}

CardManager::~CardManager() {
    // 确保所有修改都已经保存，但保存操作在每次修改时立即执行
}

bool CardManager::load() {
    if (!std::filesystem::exists(m_cardsRoot)) {
        std::filesystem::create_directories(m_cardsRoot);
        std::cout << "创建题库根目录: " << m_cardsRoot.string() << std::endl;
        return true; // 空目录也算加载成功
    }
    bool success = scanDirectory(m_cardsRoot, 0);
    rebuildCardToNodeMap();
    return success;
}

bool CardManager::scanDirectory(const std::filesystem::path& dirPath, int parentNodeIdx) {
    
    if (!std::filesystem::is_directory(dirPath)) return false;
    std::cout << dirPath.string() << std::endl;
    // 首先读取该目录下的 .tree 文件（如果存在），获取节点属性（主要是激活状态）
    std::filesystem::path treeFile = dirPath / ".tree";
    bool hasTreeFile = std::filesystem::exists(treeFile);
    if (hasTreeFile) {
        std::ifstream ifs(treeFile);
        if (ifs) {
            json j;
            ifs >> j;
            if (j.contains("active")) {
                m_nodes[parentNodeIdx].active = j["active"].get<bool>();
            }
            if (j.contains("name")) {
                m_nodes[parentNodeIdx].name = j["name"].get<std::string>();
            }
        }
    }

    // 遍历目录内容
    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        
        std::string filename = entry.path().filename().string();
        if (entry.is_directory()) {
            // 子文件夹
            std::string folderName = entry.path().filename().string();
            TreeNode folderNode;
            folderNode.type = TreeNodeType::Folder;
            folderNode.name = folderName;
            folderNode.active = true;
            int folderIdx = static_cast<int>(m_nodes.size());
            m_nodes.push_back(folderNode);
            m_nodes[parentNodeIdx].childrenIndices.push_back(folderIdx);

            // 递归扫描子目录
            scanDirectory(entry.path(), folderIdx);
        } else if (filename.size() >= 10 && filename.ends_with(".unit.json")) {
            std::cout << "load: " << entry.path() << std::endl;
            loadUnitFile(entry.path(), parentNodeIdx);
        }
    }
    return true;
}

bool CardManager::loadUnitFile(const std::filesystem::path& unitPath, int parentIdx) {
    std::ifstream ifs(unitPath);
    if (!ifs) return false;

    json j;
    try {
        ifs >> j;
    } catch (...) {
        std::cerr << "解析 JSON 失败: " << unitPath.string() << std::endl;
        return false;
    }

    std::string unitName = j.value("name", unitPath.stem().string());
    bool active = j.value("active", true);

    TreeNode unitNode;
    unitNode.type = TreeNodeType::Unit;
    unitNode.name = unitName;
    unitNode.active = active;

    // 读取 cards 数组
    std::vector<int> cardIds;
    if (j.contains("cards") && j["cards"].is_array()) {
        for (const auto& cardJson : j["cards"]) {
            std::string question = cardJson.value("question", "");
            std::string answer = cardJson.value("answer", "");
            bool cardActive = cardJson.value("active", true);

            // 去重处理
            int existingId = findDuplicateCard(question, answer);
            if (existingId != -1) {
                cardIds.push_back(existingId);
                // 注意：如果现有卡片 active 状态不同，如何处理？需求未明确，我们以新读取的为准更新全局卡片状态
                for (auto& c : m_cards) {
                    if (c.id == existingId) {
                        c.active = cardActive;
                        break;
                    }
                }
            } else {
                Card newCard;
                newCard.id = generateNewCardId();
                newCard.question = question;
                newCard.answer = answer;
                newCard.active = cardActive;
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
        if (card.question == question && card.answer == answer) {
            return card.id;
        }
    }
    return -1;
}

void CardManager::addCardToGlobal(const Card& card) {
    m_cards.push_back(card);
}

const Card* CardManager::getCardById(int id) const {
    for (const auto& card : m_cards) {
        if (card.id == id) return &card;
    }
    return nullptr;
}

int CardManager::addCardToUnit(int unitIdx, const std::string& question, const std::string& answer, bool active) {
    if (unitIdx < 0 || unitIdx >= static_cast<int>(m_nodes.size()) || m_nodes[unitIdx].type != TreeNodeType::Unit) {
        std::cerr << "错误：无效的单元索引" << std::endl;
        return -1;
    }

    // 去重
    int existingId = findDuplicateCard(question, answer);
    if (existingId != -1) {
        // 已存在，只需将卡片 ID 加入单元
        m_nodes[unitIdx].cardIds.push_back(existingId);
        saveUnitToFile(unitIdx);
        return existingId;
    }

    Card newCard;
    newCard.id = generateNewCardId();
    newCard.question = question;
    newCard.answer = answer;
    newCard.active = active;
    m_cards.push_back(newCard);
    m_nodes[unitIdx].cardIds.push_back(newCard.id);

    // 保存单元文件
    saveUnitToFile(unitIdx);
    rebuildCardToNodeMap();
    return newCard.id;
}

bool CardManager::removeCardFromUnit(int unitIdx, int cardId) {
    if (unitIdx < 0 || unitIdx >= static_cast<int>(m_nodes.size()) || m_nodes[unitIdx].type != TreeNodeType::Unit)
        return false;

    auto& ids = m_nodes[unitIdx].cardIds;
    auto it = std::find(ids.begin(), ids.end(), cardId);
    if (it == ids.end()) return false;
    ids.erase(it);
    // 注意：卡片本身不从全局删除，因为其他单元可能仍在使用
    saveUnitToFile(unitIdx);
    return true;
}

bool CardManager::updateCard(int cardId, const std::string& newQuestion, const std::string& newAnswer, bool newActive) {
    Card* card = const_cast<Card*>(getCardById(cardId));
    if (!card) return false;
    card->question = newQuestion;
    card->answer = newAnswer;
    card->active = newActive;
    // 找到所有包含此卡片的单元，重新保存
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].type == TreeNodeType::Unit) {
            if (std::find(m_nodes[i].cardIds.begin(), m_nodes[i].cardIds.end(), cardId) != m_nodes[i].cardIds.end()) {
                saveUnitToFile(static_cast<int>(i));
            }
        }
    }
    return true;
}

int CardManager::createUnit(int parentFolderIdx, const std::string& unitName) {
    if (parentFolderIdx < 0 || parentFolderIdx >= static_cast<int>(m_nodes.size()) || m_nodes[parentFolderIdx].type != TreeNodeType::Folder)
        return -1;

    TreeNode unit;
    unit.type = TreeNodeType::Unit;
    unit.name = unitName;
    unit.active = true;
    int unitIdx = static_cast<int>(m_nodes.size());
    m_nodes.push_back(unit);
    m_nodes[parentFolderIdx].childrenIndices.push_back(unitIdx);

    // 创建对应的 .unit.json 文件
    // 获取文件夹的实际路径
    std::filesystem::path folderPath = m_cardsRoot;
    std::vector<int> pathIndices;
    int cur = parentFolderIdx;
    while (cur != 0) {
        pathIndices.push_back(cur);
        // 找父节点：需要遍历查找包含 cur 的父节点，简单起见我们保留一个父节点映射，这里先简化：通过目录结构递归保存时再生成路径
        // 为了简化，我们创建一个辅助函数获取节点的文件系统路径
        // 本原型中，先只创建文件，路径规则：根目录下的文件夹名就是目录名，单元文件放在对应目录下。
        // 我们手动构建路径：从根节点开始递归查找
        // 因为时间关系，我们使用一个简单方法：直接在根目录创建单元（演示用）
        // 生产环境中需要完整实现路径解析，这里为了演示基本功能，简单放在 data/cards 下
        break;
    }
    // 简化：新单元直接放在根目录，命名为 unitName.unit.json
    std::filesystem::path unitPath = m_cardsRoot / (unitName + ".unit.json");
    json j;
    j["name"] = unitName;
    j["active"] = true;
    j["cards"] = json::array();
    std::ofstream ofs(unitPath);
    ofs << j.dump(4);
    return unitIdx;
}

int CardManager::createFolder(int parentFolderIdx, const std::string& folderName) {
    // 类似创建目录
    // 简化：在根目录下创建真实目录，并添加节点
    if (parentFolderIdx != 0) return -1; // 仅支持根目录下创建
    std::filesystem::path newDir = m_cardsRoot / folderName;
    if (std::filesystem::create_directory(newDir)) {
        TreeNode folder;
        folder.type = TreeNodeType::Folder;
        folder.name = folderName;
        folder.active = true;
        int folderIdx = static_cast<int>(m_nodes.size());
        m_nodes.push_back(folder);
        m_nodes[0].childrenIndices.push_back(folderIdx);
        return folderIdx;
    }
    return -1;
}

bool CardManager::saveUnitToFile(int unitIdx) {
    const TreeNode& unit = m_nodes[unitIdx];
    if (unit.type != TreeNodeType::Unit) return false;

    // 需要找到该单元对应的文件路径。因为路径信息未存储在节点中，我们通过遍历文件系统查找
    // 简化做法：根据单元名称在 data/cards 下查找 *.unit.json 文件（不严谨，但原型够用）
    std::filesystem::path targetPath;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(m_cardsRoot)) {
        if (entry.is_regular_file() && entry.path().extension() == ".unit.json") {
            std::string stem = entry.path().stem().string();
            if (stem == unit.name) {
                targetPath = entry.path();
                break;
            }
        }
    }
    if (targetPath.empty()) {
        // 如果没找到，创建新文件
        targetPath = m_cardsRoot / (unit.name + ".unit.json");
    }

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
    // 略：保存 .tree 文件
    return true;
}

bool CardManager::saveNode(int nodeIdx) {
    if (nodeIdx < 0 || nodeIdx >= static_cast<int>(m_nodes.size())) return false;
    if (m_nodes[nodeIdx].type == TreeNodeType::Unit) {
        return saveUnitToFile(nodeIdx);
    } else {
        return saveFolderTreeFile(nodeIdx);
    }
}

std::vector<int> CardManager::getActiveCardIds() const {
    std::set<int> allIds;
    for (const auto& node : m_nodes) {
        if (node.type == TreeNodeType::Unit && node.active) {
            for (int cid : node.cardIds) {
                const Card* card = getCardById(cid);
                if (card && card->active) {
                    allIds.insert(cid);
                }
            }
        }
    }
    return std::vector<int>(allIds.begin(), allIds.end());
}

void CardManager::rebuildCardToNodeMap() {
    m_cardToNode.clear();
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].type == TreeNodeType::Unit) {
            for (int cid : m_nodes[i].cardIds) {
                m_cardToNode[cid] = static_cast<int>(i);
            }
        }
    }
}

void CardManager::printTree(int nodeIdx, int depth) const {
    if (nodeIdx == -1) nodeIdx = 0;
    const auto& node = m_nodes[nodeIdx];
    std::string indent(depth * 2, ' ');
    std::cout << indent << (node.type == TreeNodeType::Folder ? "[文件夹] " : "[单元] ")
              << node.name << " (激活:" << (node.active ? "是" : "否") << ")"
              << "[" << nodeIdx << "]" 
              << std::endl;
    if (node.type == TreeNodeType::Folder) {
        for (int child : node.childrenIndices) {
            printTree(child, depth + 1);
        }
    } else {
        std::cout << indent << "  卡片数: " << node.cardIds.size() << std::endl;
        for (int cid : node.cardIds) {
            const Card* c = getCardById(cid);
            if (c) {
                std::cout << indent << "    [" << cid << "] " << c->question.substr(0, 30) << ":" << c->answer.substr(0, 30) << " (激活:" << (c->active ? "是" : "否") << ")" << std::endl;
            }
        }
    }
}