#include "StatsTreeModel.h"
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QDebug>

StatsTreeModel::StatsTreeModel(QObject* parent) : QAbstractItemModel(parent) {}

void StatsTreeModel::setRootPath(const std::filesystem::path& rootPath) {
    m_rootPath = rootPath;
    rebuildTree();
}

void StatsTreeModel::refresh() {
    rebuildTree();
}

void StatsTreeModel::rebuildTree() {
    beginResetModel();
    m_nodes.clear();
    Node root;
    root.name = "统计根目录";
    root.fullPath = m_rootPath;
    root.isFolder = true;
    root.parentIdx = -1;
    m_nodes.push_back(root);

    if (std::filesystem::exists(m_rootPath) && std::filesystem::is_directory(m_rootPath)) {
        for (const auto& entry : std::filesystem::directory_iterator(m_rootPath)) {
            if (entry.is_directory()) {
                addDirectory(entry.path(), 0);
            } else if (entry.is_regular_file() && entry.path().extension() == ".session.json") {
                addFile(entry.path(), 0);
            }
        }
    }
    endResetModel();
}

void StatsTreeModel::addDirectory(const std::filesystem::path& dirPath, int parentIdx) {
    Node node;
    node.name = dirPath.filename().string();
    node.fullPath = dirPath;
    node.isFolder = true;
    node.parentIdx = parentIdx;
    int idx = static_cast<int>(m_nodes.size());
    m_nodes.push_back(node);
    m_nodes[parentIdx].children.push_back(idx);

    for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
        if (entry.is_directory()) {
            addDirectory(entry.path(), idx);
        } else if (entry.is_regular_file() && entry.path().extension() == ".session.json") {
            addFile(entry.path(), idx);
        }
    }
}

void StatsTreeModel::addFile(const std::filesystem::path& filePath, int parentIdx) {
    Node node;
    node.name = filePath.filename().string();
    node.fullPath = filePath;
    node.isFolder = false;
    node.parentIdx = parentIdx;
    int idx = static_cast<int>(m_nodes.size());
    m_nodes.push_back(node);
    m_nodes[parentIdx].children.push_back(idx);
}

int StatsTreeModel::findNodeByPath(const std::filesystem::path& path) const {
    for (size_t i = 0; i < m_nodes.size(); ++i) {
        if (m_nodes[i].fullPath == path) return static_cast<int>(i);
    }
    return -1;
}

QModelIndex StatsTreeModel::index(int row, int column, const QModelIndex &parent) const {
    if (!hasIndex(row, column, parent)) return QModelIndex();
    int parentIdx = parent.isValid() ? parent.internalId() : 0;
    if (parentIdx >= static_cast<int>(m_nodes.size())) return QModelIndex();
    const auto& parentNode = m_nodes[parentIdx];
    if (row < 0 || row >= static_cast<int>(parentNode.children.size())) return QModelIndex();
    int childIdx = parentNode.children[row];
    return createIndex(row, column, childIdx);
}

QModelIndex StatsTreeModel::parent(const QModelIndex &child) const {
    if (!child.isValid()) return QModelIndex();
    int childIdx = child.internalId();
    if (childIdx <= 0 || childIdx >= static_cast<int>(m_nodes.size())) return QModelIndex();
    int parentIdx = m_nodes[childIdx].parentIdx;
    if (parentIdx <= 0) return QModelIndex(); // root's parent is invalid
    // find row of parent in its parent's children
    int grandParentIdx = m_nodes[parentIdx].parentIdx;
    if (grandParentIdx < 0) return QModelIndex();
    const auto& grandParent = m_nodes[grandParentIdx];
    int row = -1;
    for (size_t i = 0; i < grandParent.children.size(); ++i) {
        if (grandParent.children[i] == parentIdx) { row = static_cast<int>(i); break; }
    }
    if (row >= 0) return createIndex(row, 0, parentIdx);
    return QModelIndex();
}

int StatsTreeModel::rowCount(const QModelIndex &parent) const {
    int nodeIdx = parent.isValid() ? parent.internalId() : 0;
    if (nodeIdx >= static_cast<int>(m_nodes.size())) return 0;
    return static_cast<int>(m_nodes[nodeIdx].children.size());
}

int StatsTreeModel::columnCount(const QModelIndex &) const { return 1; }

QVariant StatsTreeModel::data(const QModelIndex &index, int role) const {
    if (!index.isValid()) return QVariant();
    int nodeIdx = index.internalId();
    if (nodeIdx >= static_cast<int>(m_nodes.size())) return QVariant();
    const auto& node = m_nodes[nodeIdx];
    if (role == Qt::DisplayRole) {
        return QString::fromStdString(node.name);
    } else if (role == Qt::DecorationRole) {
        return node.isFolder ? QVariant() : QVariant(); // 可加图标
    }
    return QVariant();
}

Qt::ItemFlags StatsTreeModel::flags(const QModelIndex &index) const {
    if (!index.isValid()) return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

std::filesystem::path StatsTreeModel::pathForIndex(const QModelIndex &index) const {
    if (!index.isValid()) return {};
    int nodeIdx = index.internalId();
    if (nodeIdx >= 0 && nodeIdx < static_cast<int>(m_nodes.size()))
        return m_nodes[nodeIdx].fullPath;
    return {};
}

bool StatsTreeModel::isFolder(const QModelIndex &index) const {
    if (!index.isValid()) return false;
    int nodeIdx = index.internalId();
    if (nodeIdx >= 0 && nodeIdx < static_cast<int>(m_nodes.size()))
        return m_nodes[nodeIdx].isFolder;
    return false;
}

bool StatsTreeModel::createFolder(const QModelIndex &parentIdx, const QString& folderName) {
    std::filesystem::path parentPath = pathForIndex(parentIdx);
    if (parentPath.empty()) parentPath = m_rootPath;
    std::filesystem::path newPath = parentPath / folderName.toStdString();
    if (std::filesystem::exists(newPath)) return false;
    if (!std::filesystem::create_directory(newPath)) return false;
    refresh();
    return true;
}

bool StatsTreeModel::renameNode(const QModelIndex &index, const QString& newName) {
    std::filesystem::path oldPath = pathForIndex(index);
    if (oldPath.empty()) return false;
    std::filesystem::path newPath = oldPath.parent_path() / newName.toStdString();
    if (std::filesystem::exists(newPath)) return false;
    std::error_code ec;
    std::filesystem::rename(oldPath, newPath, ec);
    if (ec) return false;
    refresh();
    return true;
}

bool StatsTreeModel::deleteNode(const QModelIndex &index) {
    std::filesystem::path path = pathForIndex(index);
    if (path.empty()) return false;
    std::error_code ec;
    if (std::filesystem::is_directory(path))
        std::filesystem::remove_all(path, ec);
    else
        std::filesystem::remove(path, ec);
    if (ec) return false;
    refresh();
    return true;
}

bool StatsTreeModel::moveNode(const QModelIndex &from, const QModelIndex &toFolder) {
    std::filesystem::path fromPath = pathForIndex(from);
    std::filesystem::path toFolderPath = pathForIndex(toFolder);
    if (fromPath.empty() || toFolderPath.empty()) return false;
    if (!std::filesystem::is_directory(toFolderPath)) return false;
    std::filesystem::path newPath = toFolderPath / fromPath.filename();
    if (std::filesystem::exists(newPath)) return false;
    std::error_code ec;
    std::filesystem::rename(fromPath, newPath, ec);
    if (ec) return false;
    refresh();
    return true;
}